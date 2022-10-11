#pragma once
#include "ClientSocket.h"
#include "CWatchDialog.h"
#include "RemoteClientDlg.h"
#include "StatusDlg.h"
#include "resource.h"
#include <map>
#include "EdoyunTool.h"
//#define WM_SEND_DATA (WM_USER+2)//发送原始数据 
#define WM_SHOW_SATAUS (WM_USER+3)//展示状态
#define WM_SHOW_WATCH (WM_USER+4)//远程监控
#define WM_SEND_MESSAGE (WM_USER+0x1000)//自定义的消息处理

//业务逻辑和流程,是随时可能发生改变的!!!

class CClientController
{
public:
	//获取全局唯一对象
	static CClientController* getInstance();
	//初始化操作
	int InitController();
	//启动
	int Invoke(CWnd*& pMainWnd);
	//更新网络服务器的地址
	void UpdateAddress(int nIP, int nPort) {
		CClientSocket::getInstance()->UpdateAddress(nIP, nPort);
	}
	int DealCommand() {
		return CClientSocket::getInstance()->DealCommand();
	}
	void CloseSocket() {
		CClientSocket::getInstance()->CloseSocket();
	}
	//1 查看磁盘分区
	//2 查看指定目录下的文件
	//3 打开文件
	//4 下载文件
	//9 删除文件
	//5 鼠标操作
	//6 发送屏幕内容
	//7 锁机
	//8 解锁
	//1981 测试连接
	//返回值: 是状态,true是成功 false失败
	bool SendCommandPacket(
		HWND hWnd,//数据包接收到后,需要应答的窗口
		int nCmd, bool bAutoClose = true, BYTE* pData = NULL, size_t nLength = 0,WPARAM wParam = 0);

	int GetImage(CImage& image) {
		CClientSocket* pClient = CClientSocket::getInstance();
		return CEdoyunTool::Bytes2Image(image, pClient->GetPacket().strData);
	}
	void DownloadEnd();
	int DownFile(CString strPath);

	void StartWatchScreen();
protected:
	void threadWatchScreen();
	static void threadWatchScreen(void* arg);

	CClientController():
		//认父
		m_statusDlg(&m_remoteDlg),m_watchDlg(&m_remoteDlg) 
	{
		m_isClosed = true;
		m_hThread = INVALID_HANDLE_VALUE;
		m_nThreadID = -1;
		m_hThreadWatch = INVALID_HANDLE_VALUE;
		//m_statusDlg.Create(IDD_DLG_STATUS, &m_remoteDlg);
		//m_watchDlg.Create(IDD_DLG_WATCH, &m_remoteDlg);
	}
	~CClientController() {
		WaitForSingleObject(&m_hThread, 100);
		TRACE("CClientController::~CClientController has destroy\r\n");
	}
	void threadFunc();
	static unsigned __stdcall threadEntry(void* arg);
	static void releaseInstance() {
		if (m_instance != NULL) {
			delete m_instance;
			m_instance = NULL;
		}
	}
	LRESULT OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnShwoWatcher(UINT nMsg, WPARAM wParam, LPARAM lParam);
private:
	typedef struct MsgInfo{
		MSG msg;
		LRESULT result;
		MsgInfo(MSG m) {
			result = 0;
			memcpy(&msg, &m, sizeof(MSG));
		}
		MsgInfo(const MsgInfo& m) {
			result = m.result;
			memcpy(&msg, &m.msg, sizeof(MSG));
		}
		MsgInfo& operator = (const MsgInfo& m) {
			if (this == &m) {
				result = m.result;
				memcpy(&msg, &m.msg, sizeof(MSG));
			}			
			return *this;
		}
	}MSGINFO;
	typedef LRESULT (CClientController::*MSGFUNC)(UINT,WPARAM,LPARAM);
	static std::map<UINT, MSGFUNC> m_mapFunc;
	CWatchDialog m_watchDlg;//消息包在对话框关闭之后可能导致内存泄漏
	CRemoteClientDlg m_remoteDlg;
	CStatusDlg m_statusDlg;
	HANDLE m_hThread;
	unsigned m_nThreadID;
	//监视窗口是否关闭
	bool m_isClosed;
	//下载文件的远程路径
	CString m_strRemote;
	//下载文件的本地保存路劲
	CString m_strLocal;
	HANDLE m_hThreadWatch;
	
	static CClientController* m_instance;	
	class CHelper {
	public:
		CHelper() {
			//CClientController::getInstance();
		}
		~CHelper() {
			CClientController::releaseInstance();
			TRACE("CClientController CHelper has destroy\r\n");
		}
	};
	static CHelper m_helper;
};

