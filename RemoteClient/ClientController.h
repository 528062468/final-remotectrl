#pragma once
#include "ClientSocket.h"
#include "CWatchDialog.h"
#include "RemoteClientDlg.h"
#include "StatusDlg.h"
#include "resource.h"
#include <map>
#include "EdoyunTool.h"
//#define WM_SEND_DATA (WM_USER+2)//����ԭʼ���� 
#define WM_SHOW_SATAUS (WM_USER+3)//չʾ״̬
#define WM_SHOW_WATCH (WM_USER+4)//Զ�̼��
#define WM_SEND_MESSAGE (WM_USER+0x1000)//�Զ������Ϣ����

//ҵ���߼�������,����ʱ���ܷ����ı��!!!

class CClientController
{
public:
	//��ȡȫ��Ψһ����
	static CClientController* getInstance();
	//��ʼ������
	int InitController();
	//����
	int Invoke(CWnd*& pMainWnd);
	//��������������ĵ�ַ
	void UpdateAddress(int nIP, int nPort) {
		CClientSocket::getInstance()->UpdateAddress(nIP, nPort);
	}
	int DealCommand() {
		return CClientSocket::getInstance()->DealCommand();
	}
	void CloseSocket() {
		CClientSocket::getInstance()->CloseSocket();
	}
	//1 �鿴���̷���
	//2 �鿴ָ��Ŀ¼�µ��ļ�
	//3 ���ļ�
	//4 �����ļ�
	//9 ɾ���ļ�
	//5 ������
	//6 ������Ļ����
	//7 ����
	//8 ����
	//1981 ��������
	//����ֵ: ��״̬,true�ǳɹ� falseʧ��
	bool SendCommandPacket(
		HWND hWnd,//���ݰ����յ���,��ҪӦ��Ĵ���
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
		//�ϸ�
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
	CWatchDialog m_watchDlg;//��Ϣ���ڶԻ���ر�֮����ܵ����ڴ�й©
	CRemoteClientDlg m_remoteDlg;
	CStatusDlg m_statusDlg;
	HANDLE m_hThread;
	unsigned m_nThreadID;
	//���Ӵ����Ƿ�ر�
	bool m_isClosed;
	//�����ļ���Զ��·��
	CString m_strRemote;
	//�����ļ��ı��ر���·��
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

