#include "pch.h"
#include "ClientSocket.h"
#include "ClientController.h"
std::map<UINT, CClientController::MSGFUNC> CClientController::m_mapFunc;
CClientController* CClientController::m_instance;
CClientController::CHelper CClientController::m_helper;
CClientController* CClientController::getInstance()
{
	if (m_instance == NULL) {
		m_instance = new CClientController;
		struct { UINT nMsg; MSGFUNC func; }MsgFuncs[] = {
			{WM_SHOW_SATAUS,&CClientController::OnShowStatus},
			{WM_SHOW_WATCH,&CClientController::OnShwoWatcher},
			{(UINT)-1,NULL}
		};
		for (int i = 0; MsgFuncs[i].func; i++) {
			m_mapFunc.insert(std::pair<UINT, MSGFUNC>(MsgFuncs[i].nMsg, MsgFuncs[i].func));
		}		
	}
	return m_instance;
}

int CClientController::InitController()
{
	m_hThread = (HANDLE)_beginthreadex(NULL, 0, &CClientController::threadEntry, this, 0, &m_nThreadID);
	m_statusDlg.Create(IDD_DLG_STATUS,&m_remoteDlg);
	return 0;
}

int CClientController::Invoke(CWnd*& pMainWnd)
{
	pMainWnd = &m_remoteDlg;
	return m_remoteDlg.DoModal();
}

bool CClientController::SendCommandPacket(HWND hWnd ,int nCmd, bool bAutoClose, BYTE* pData, size_t nLength,WPARAM wParam)
{
	TRACE("Cmd = %d  %s start %lld \r\n", nCmd,__FUNCTION__, GetTickCount64());
	CClientSocket* pClient = CClientSocket::getInstance();
   	bool ret =  pClient->SendPacket(hWnd,CPacket(nCmd, pData, nLength), bAutoClose,wParam);
	return ret;
}

void CClientController::DownloadEnd()
{
	m_statusDlg.ShowWindow(SW_HIDE);
	m_remoteDlg.EndWaitCursor();
	m_remoteDlg.MessageBox(_T("下载完成!!!"), _T("完成"));
}

int CClientController::DownFile(CString strPath)
{
	CFileDialog dlg(FALSE, /*"*"*/NULL, strPath,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL/*szFilter*/, &m_remoteDlg);/*设置默认文件名与下载文件名一致,FALSE表示保存*/
	if (dlg.DoModal() == IDOK) {
		m_strRemote = strPath;
		m_strLocal = dlg.GetPathName();
		FILE* pFile = fopen(m_strLocal, "wb+");
		if (pFile == NULL) {
			AfxMessageBox(_T("本地没有权限保存该文件,或者文件无法创建"));
			return -1;
		}
		SendCommandPacket(m_remoteDlg, 4, false, (BYTE*)(LPCSTR)m_strRemote, m_strRemote.GetLength(), (WPARAM)pFile);
		m_remoteDlg.BeginWaitCursor();
		m_statusDlg.m_info.SetWindowTextA(_T("命令正在执行中"));
		m_statusDlg.ShowWindow(SW_SHOW);
		m_statusDlg.CenterWindow(&m_remoteDlg);//将窗口居中
		m_statusDlg.SetActiveWindow();//将窗口设置为活动窗口
	}
	return 0;
}

void CClientController::StartWatchScreen()
{
	m_isClosed = false;
	m_hThreadWatch = (HANDLE)_beginthread(CClientController::threadWatchScreen,0,this);
	m_watchDlg.DoModal();
	m_isClosed = true;
	WaitForSingleObject(m_hThreadWatch,500);
}

void CClientController::threadWatchScreen()
{
	Sleep(50);
	ULONGLONG nTick = GetTickCount64();
	while (!m_isClosed) {//退出条件
		if (m_watchDlg.isFull() == false) {//更新数据到缓存
			if (GetTickCount64() - nTick < 200) {
				Sleep(200-DWORD(GetTickCount64() - nTick));//控制消息的发送频率
				nTick = GetTickCount64();
			}
			int ret = SendCommandPacket(m_watchDlg.GetSafeHwnd(),6,true,NULL,0);
			if (ret == true) {
				//TRACE("成功发送请求图片命令\r\n");
			}
			else {
				TRACE("获取图片失败!ret = %d\r\n", ret);
			}
		}
		Sleep(1);
	}
	TRACE("thread end %d\r\n", m_isClosed);
}

void CClientController::threadWatchScreen(void* arg)
{
	CClientController* thiz = (CClientController*)arg;
	thiz->threadWatchScreen();
	_endthread();
}


void CClientController::threadFunc()
{
	MSG msg;
	while (::GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_SEND_MESSAGE)
		{
			MSGINFO* pmsg = (MSGINFO*)msg.wParam;
			HANDLE hEvent = (HANDLE)msg.lParam;
			std::map<UINT, MSGFUNC>::iterator it = m_mapFunc.find(msg.message);
			if (it != m_mapFunc.end()) {//(this->*m_mapFunc[msg.message])(0,0,0);//调用函数	
				pmsg->result = (this->*it->second)(pmsg->msg.message, pmsg->msg.wParam, pmsg->msg.lParam);
			}
			else {
				pmsg->result = -1;
			}
			SetEvent(hEvent);
		}
		else
		{
			std::map<UINT, MSGFUNC>::iterator it = m_mapFunc.find(msg.message);
			if (it != m_mapFunc.end()) {//(this->*m_mapFunc[msg.message])(0,0,0);				
				(this->*it->second)(msg.message, msg.wParam, msg.lParam);
			}
		}
	}
}

unsigned __stdcall CClientController::threadEntry(void* arg)
{
	CClientController* thiz = (CClientController*)arg;
	thiz->threadFunc();
	_endthreadex(0);
	return 0;
}


LRESULT CClientController::OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return m_statusDlg.ShowWindow(SW_SHOW);
}

LRESULT CClientController::OnShwoWatcher(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return m_watchDlg.DoModal();
}
