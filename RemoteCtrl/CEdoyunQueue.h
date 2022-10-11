#pragma once
#include "pch.h"
#include <atomic>
#include <list>
#include "EdoyunThread.h"
template<class T>
class CEdoyunQueue
{//线程安全的队列(利用IOCP实现)
public:
	enum {
		EQNone,
		EQPush,
		EQPop,
		EQSize,
		EQClear
	};
	typedef struct IocpParam {
		size_t nOperator;//操作
		T Data;//数据
		HANDLE hEvent;//pop操作需要的
		IocpParam(int op, const T& data, HANDLE hEve = NULL) {
			nOperator = op;
			Data = data;
			hEvent = hEve;
		}
		IocpParam() {
			nOperator = EQNone;
		}
	}PPARAM;//Post Parameter 用于投递信息的结构体
public:
	CEdoyunQueue() {
		m_lock = false;
		m_hCompeletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1);
		m_hThread = INVALID_HANDLE_VALUE;
		if (m_hCompeletionPort != NULL) {
			m_hThread = (HANDLE)_beginthread(CEdoyunQueue<T>::threadEntry, 0, this);
		}
	}
	virtual ~CEdoyunQueue() {
		if (m_lock)return;
		m_lock = true;
		PostQueuedCompletionStatus(m_hCompeletionPort, 0, NULL, NULL);//向完成端口投递一个状态
		WaitForSingleObject(m_hThread, INFINITE);
		if (m_hCompeletionPort != NULL) {
			HANDLE hTemp = m_hCompeletionPort;
			m_hCompeletionPort = NULL;//防御性编程
			CloseHandle(hTemp);
		}
	}
	bool PushBack(const T& data) {
		IocpParam* pParam = new IocpParam(EQPush, data);
		if (m_lock) {//原子操作,减少与重要代码的运行时间间隔
			return false;
		}
		bool ret = PostQueuedCompletionStatus(m_hCompeletionPort, sizeof(PPARAM), (ULONG_PTR)pParam, NULL);
		if (ret == false)delete pParam;//防御性编程:防止post失败导致内存泄漏
		//printf("push back done %d %08p\r\n", ret,(void*)pParam);
		printf("PushBack = %08x\r\n", (void*)pParam);
		return ret;
	}
	virtual bool PopFront(T& data) {
		HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		IocpParam pParam(EQPop, data, hEvent);
		if (m_lock) {
			if (hEvent)CloseHandle(hEvent);
			return false;
		}
		bool ret = PostQueuedCompletionStatus(m_hCompeletionPort, sizeof(PPARAM), (ULONG_PTR)&pParam, NULL);
		if (ret == false) {
			CloseHandle(hEvent);
			return false;
		}
		ret = WaitForSingleObject(hEvent, INFINITE) == WAIT_OBJECT_0;//等带丢列
		if (ret) {
			data = pParam.Data;
		}
		return ret;
	}
	virtual size_t Size() {
		return m_lstData.size();
		HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		IocpParam pParam(EQSize, T(), hEvent);
		if (m_lock) {
			if (hEvent)CloseHandle(hEvent);
			return -1;
		}
		bool ret = PostQueuedCompletionStatus(m_hCompeletionPort, sizeof(PPARAM), (ULONG_PTR)&pParam, NULL);
		printf("Size = %08X\r\n", (void*)&pParam);
		if (ret == false) {
			CloseHandle(hEvent);
			return -1;
		}
		ret = WaitForSingleObject(hEvent, INFINITE) == WAIT_OBJECT_0;//等带丢列
		if (ret) {
			return pParam.nOperator;
		}
		return -1;
	}
	bool Clear() {
		if (m_lock) return false;
		IocpParam* pParam = new IocpParam(EQClear, T());
		bool ret = PostQueuedCompletionStatus(m_hCompeletionPort, sizeof(PPARAM), (ULONG_PTR)pParam, NULL);
		if (ret == false)delete pParam;//防御性编程:防止post失败导致内存泄漏
		//printf("Clear %08p\r\n", (void*)pParam);
		return ret;
	}
protected:
	static void threadEntry(void* arg) {
		CEdoyunQueue<T>* thiz = (CEdoyunQueue<T>*)arg;
		thiz->threadMain();
		_endthread();
	}

	virtual void threadMain() {
		DWORD dwTransferred = 0;
		PPARAM* pParam = NULL;
		ULONG_PTR CompletionKey = 0;
		OVERLAPPED* pOverlapped = NULL;
		while (GetQueuedCompletionStatus(
			m_hCompeletionPort,
			&dwTransferred,
			&CompletionKey,
			&pOverlapped, INFINITE))
		{
			if ((dwTransferred == 0) || (CompletionKey == 0)) {
				printf("thread is prepare to exit!\r\n");
				break;
			}
			pParam = (PPARAM*)CompletionKey;
			printf("GetQueueCompletionStatus = %08X\r\n", (void*)pParam);
			DealParam(pParam);
		}
		while (GetQueuedCompletionStatus(
			m_hCompeletionPort,
			&dwTransferred,
			&CompletionKey,
			&pOverlapped, 0)) {//处理队列里面可能残余的数据
			if ((dwTransferred == 0) || (CompletionKey == 0)) {
				printf("thread is prepare to exit!\r\n");
				continue;
			}
			pParam = (PPARAM*)CompletionKey;
			DealParam(pParam);
		}
		HANDLE hTemp = m_hCompeletionPort;
		m_hCompeletionPort = NULL;
		CloseHandle(hTemp);
	}

	virtual void DealParam(PPARAM* pParam) {
		switch (pParam->nOperator) {
		case EQPush:
			m_lstData.push_back(pParam->Data);
			printf("DealParam %08p\r\n", (void*)pParam);
			delete pParam;
			break;
		case EQPop:
			if (m_lstData.size() > 0) {
				pParam->Data = m_lstData.front();
				m_lstData.pop_front();
			}
			if (pParam->hEvent != NULL)  SetEvent(pParam->hEvent);
			break;
		case EQSize:
			pParam->nOperator = m_lstData.size();
			if (pParam->hEvent != NULL)  SetEvent(pParam->hEvent);
			break;
		case EQClear:
			m_lstData.clear();
			delete pParam;
			//printf("delete %08p\r\n", (void*)pParam);
			break;
		default:
			OutputDebugStringA("unknown operator!\r\n");
			break;
		}
	}
protected:
	std::list<T> m_lstData;
	HANDLE m_hCompeletionPort;
	HANDLE m_hThread;
	std::atomic<bool> m_lock;//队列正在析构
};


template<class T>
class EdoyunSendQueue :public CEdoyunQueue<T>, public ThreadFuncBase
{
public:
	typedef int (ThreadFuncBase::* EDYCALLBACK)(T& data);
	EdoyunSendQueue(ThreadFuncBase* obj, EDYCALLBACK callback)
		:CEdoyunQueue<T>(), m_base(obj), m_callback(callback)
	{
		m_thread.Start();
		m_thread.UpdateWorker(::ThreadWorker(this, (FUNCTYPE)&EdoyunSendQueue<T>::threadTick));//启动队列盯梢兵
	}
	virtual ~EdoyunSendQueue() {
		m_base = NULL;
		m_callback = NULL;
		m_thread.Stop();
	}
	//size_t Size() {}
protected:
	virtual bool PopFront(T& data) {
		return false;
	}
	virtual bool PopFront() {
		HANDLE hEvent = CreateEvent(0, TRUE, FALSE, NULL);
		typename CEdoyunQueue<T>::IocpParam* pParam = new typename CEdoyunQueue<T>::IocpParam(CEdoyunQueue<T>::EQPop, T(), hEvent);
		if (CEdoyunQueue<T>::m_lock) {//类还没有析构
			delete pParam;
			return false;
		}
		bool ret = PostQueuedCompletionStatus(CEdoyunQueue<T>::m_hCompeletionPort, sizeof(*pParam), (ULONG_PTR)pParam, NULL);
		printf("PopFront = %08X\r\n", pParam);
		if (ret == false) {
			delete pParam;
			return false;
		}
		WaitForSingleObject(hEvent, INFINITE);
		return ret;
	}
	int threadTick() {//哨兵
		if (WaitForSingleObject(CEdoyunQueue<T>::m_hThread, 0) != WAIT_TIMEOUT)//确保m_hThread线程正在运行
			return 0;
		if (CEdoyunQueue<T>::m_lstData.size() > 0) {
			PopFront();
		}
		return 0;
	}
	virtual void DealParam(typename CEdoyunQueue<T>::PPARAM* pParam) {
		switch (pParam->nOperator) {
		case CEdoyunQueue<T>::EQPush:
			CEdoyunQueue<T>::m_lstData.push_back(pParam->Data);
			delete pParam;
			break;
		case CEdoyunQueue<T>::EQPop:
			if (CEdoyunQueue<T>::m_lstData.size() > 0) {
				if ((m_base->*m_callback)(CEdoyunQueue<T>::m_lstData.front()) == 0)//调用SendData(vector<char>)
					CEdoyunQueue<T>::m_lstData.pop_front();
			}
			SetEvent(pParam->hEvent);
			delete pParam;
			break;
		case CEdoyunQueue<T>::EQSize:
			pParam->nOperator = CEdoyunQueue<T>::m_lstData.size();
			if (pParam->hEvent != NULL)  SetEvent(pParam->hEvent);
			break;
		case CEdoyunQueue<T>::EQClear:
			CEdoyunQueue<T>::m_lstData.clear();
			delete pParam;
			//printf("delete %08p\r\n", (void*)pParam);
			break;
		default:
			OutputDebugStringA("unknown operator!\r\n");
			break;
		}
	}
private:
	ThreadFuncBase* m_base;
	EDYCALLBACK m_callback;//EdoyunClient::SendData
	EdoyunThread m_thread;
};

typedef EdoyunSendQueue<std::vector<char>>::EDYCALLBACK SENDCALLBACK;