#pragma once
#include "pch.h"
#include "framework.h"
#include <map>
#include <atlimage.h>
#include <direct.h>
#include "ServerSocket.h"
#include "EdoyunTool.h"
#include <stdio.h>
#include <io.h>
#include <list>
#include "LockInfoDialog.h"
#include "Resource.h"
#include "Packet.h"
#pragma warning(disable:4996)

class CCommand
{
public:
    CCommand();
    ~CCommand() {}
    int ExcuteCommand(int nCmd, std::list<CPacket>& lstPacket,CPacket& inPacket);
    static void RunCommand(void* arg, int status,std::list<CPacket>& lstPacket, CPacket& inPacket) {
        CCommand* thiz = (CCommand*)arg;
        if(status>0){
            int ret = thiz->ExcuteCommand(status, lstPacket,inPacket);
            if (ret) {
                    TRACE("�����ִ������ʧ��: %d ret = %d \r\n", status, ret);
            }
        }
        else {
            MessageBox(NULL, _T("�޷����������û�,�Զ�����"), _T("�����û�ʧ��!"), MB_OK | MB_ICONERROR);
        }
    }
protected:
    typedef int(CCommand::* CMDFUNC)( std::list<CPacket>&, CPacket&);//��Ա����ָ��
    std::map<int, CMDFUNC> m_mapFunction;//������ŵ����ܵ�ӳ��
    CLockInfoDialog dlg;
    unsigned threadid;//���洰�������̵߳�ID���ڿ��߳�PostThreadMessage();
protected:

    static unsigned WINAPI threadLockDlg(void* arg)
    {
        CCommand* thiz = (CCommand*)arg;
        thiz->threadLockDlgMain();
        _endthreadex(0);//��ֹ�߳�
        return 0;
    }
    void threadLockDlgMain() {
        TRACE("");
        dlg.Create(IDD_DIALOG_INFO, NULL);
        dlg.ShowWindow(SW_SHOW);
        CRect rect;
        rect.left = 0;
        rect.top = 0;
        rect.right = GetSystemMetrics(SM_CXFULLSCREEN);//SM_CXSCREEN��SM_CXFULLSCREENһ��
        rect.bottom = GetSystemMetrics(SM_CYSCREEN);//SM_CYSCREEN��SM_CYFULLSCREEN����27������
        //���öԻ����С��λ��
        dlg.MoveWindow(rect);
        CWnd* pText = dlg.GetDlgItem(IDC_STATIC);
        if (pText->GetSafeHwnd()) {
            //pText->CenterWindow();
            CRect reText;//����̬�ı�����
            pText->GetWindowRect(reText);
            int nWidth = reText.Width();
            int x = (rect.right - nWidth) / 2;
            int nHeight = reText.Height();
            int y = (rect.bottom - nHeight) / 2;
            pText->MoveWindow(x, y, reText.Width(), reText.Height());
        }
        //SWP set windows pos,�����ö�
        dlg.SetWindowPos(&dlg.wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
        //�������
        ShowCursor(false);
        //����������
        ::ShowWindow(::FindWindow(_T("ShelL_TrayWnd"), NULL), SW_HIDE);
        rect.left = 0;
        rect.top = 0;
        rect.right = rect.left + 1;
        rect.bottom = rect.top + 1;//��С�ɻ��Χ��һ�����ص�
        ClipCursor(rect);//�̶����Ļ��Χ
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {//��ģ̬�Ի��������Ϣѭ��
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_KEYDOWN) {//�м��̰�������
                TRACE("msg:%08x wparam:%08x lparam:%08x \n", msg.message, msg.wParam, msg.lParam);
                if (msg.wParam == 0x41 || msg.wParam == 0x1b)break;
            }
        }
        ClipCursor(NULL);//�ָ������Χ
        ShowCursor(true);//ȡ�������ʾ
        ::ShowWindow(::FindWindow(_T("ShelL_TrayWnd"), NULL), SW_SHOW);
        dlg.DestroyWindow();//�������߳̽���֮ǰ����
    }
    int MakeDriverInfo(std::list<CPacket>& lstPacket, CPacket& inPacket) {//���ʹ��̷�����Ϣ
        std::string result;
        for (int i = 1; i <= 26; i++) {
            if (0 == _chdrive(i)) {//���̷���1:A��,���ش��̷����Ƿ����
                if (result.size() > 0)
                    result += ',';
                result += 'A' + i - 1;
            }
        }
        lstPacket.push_back(CPacket(1, (BYTE*)result.c_str(), result.size()));
        return 0;
    }


	int MakeDirectoryInfo(std::list<CPacket>& lstPacket, CPacket& inPacket) {//����Ŀ¼�ļ���Ϣ
		std::string strPath = inPacket.strData;
		//std::list<FILEINFO> lstFileInfos;
		if (_chdir(strPath.c_str()) != 0) {
			FILEINFO finfo;
			finfo.HasNext = FALSE;
			lstPacket.push_back(CPacket(2, (BYTE*)&finfo, sizeof(finfo)));
			OutputDebugString(_T("û��Ȩ��,����Ŀ¼!!"));
			return -2;
		}
        _finddata_t fdata;
        int hfind = 0;
        if ((hfind = _findfirst("*", &fdata)) == -1) {
            OutputDebugString(_T("û���ҵ��κ��ļ�!!"));
            FILEINFO finfo;
            finfo.HasNext = FALSE;
            lstPacket.push_back(CPacket(2, (BYTE*)&finfo, sizeof(finfo)));
            return -3;
        }
        int count = 0;
        do {
            FILEINFO finfo;
            finfo.IsDirectory = (fdata.attrib & _A_SUBDIR) != 0;
            memcpy(finfo.szFileName, fdata.name, strlen(fdata.name));
            TRACE("%s\r\n", fdata.name);
            //lstFileInfos.push_back(finfo);
            lstPacket.push_back(CPacket(2, (BYTE*)&finfo, sizeof(finfo)));
            count++;
        } while (!_findnext(hfind, &fdata));
        //������Ϣ�����ƶ�
        FILEINFO finfo;
        finfo.HasNext = FALSE;
        lstPacket.push_back(CPacket(2, (BYTE*)&finfo, sizeof(finfo)));
        TRACE("server: count =%d\r\n", count);
        return 0;
    }

    int RunFile(std::list<CPacket>& lstPacket, CPacket& inPacket) {
        std::string strPath = inPacket.strData;
        ShellExecuteA(NULL, NULL, strPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
        lstPacket.push_back(CPacket(3, NULL, 0));
        return 0;
    }

    int DownloadFile(std::list<CPacket>& lstPacket, CPacket& inPacket) {
        std::string strPath = inPacket.strData;
        long long data = 0;
        FILE* pFile = NULL;
        errno_t err = fopen_s(&pFile, strPath.c_str(), "rb");
        if (err != 0) {//�ļ���ʧ�ܷ���NULL
            lstPacket.push_back(CPacket(4, (BYTE*)&data, 8));
            return -1;
        }
        if (pFile != NULL) {
            fseek(pFile, 0, SEEK_END);
            data = _ftelli64(pFile);//��ȡ�ļ�����
            lstPacket.push_back(CPacket(4, (BYTE*)&data, 8));//�����ļ���С
            fseek(pFile, 0, SEEK_SET);
            char buffer[1024] = "";
            size_t rlen = 0;
            do {
                rlen = fread(buffer, 1, 1024, pFile);;
                lstPacket.push_back(CPacket(4, (BYTE*)buffer, rlen));
            } while (rlen >= 1024);
            fclose(pFile);
        }
        else {
            lstPacket.push_back(CPacket(4, NULL, 0));
        }
        
        return 0;
    }

    int  MouseEvent(std::list<CPacket>& lstPacket, CPacket& inPacket) {
        MOUSEEV mouse;
        memcpy(&mouse, inPacket.strData.c_str(), sizeof(MOUSEEV));
		DWORD nFlags = 0;
		switch (mouse.nButton) {
		case 0://���
			nFlags = 1;
			break;
		case 1://�Ҽ�
			nFlags = 2;
			break;
		case 2://�м�
			nFlags = 4;
			break;
		case 4://û�а���
			nFlags = 8;
			break;
		}
		if (nFlags != 8)SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);

		switch (mouse.nAction) {
		case 0://����
			nFlags |= 0x10;
			break;
		case 1://˫��
			nFlags |= 0x20;
			break;
		case 2://����
			nFlags |= 0x40;
			break;
		case 3://̧��
			nFlags |= 0x80;
			break;
		default:
			break;
		}
		TRACE("mouse event : %08X x %d y %d \r\n", nFlags, mouse.ptXY.x, mouse.ptXY.y);
		switch (nFlags) {
		case 0x21://���˫���ȼ������ε���
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());//ȫ�����ģ��ϵͳAPI,GetMessageExtraInfo()��ȡ��ǰ�̵߳Ķ�����Ϣ
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
		case 0x11://�������
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x41://�������
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x81://���̧��
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x22://�Ҽ�˫��
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
		case 0x12://�Ҽ�����
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x42://�Ҽ�����
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x82://�Ҽ�̧��
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x24://�м�˫��
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
		case 0x14://�м�����
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x44://�м�����
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x84://�м�̧��
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x08://��������ƶ�
			mouse_event(MOUSEEVENTF_MOVE, mouse.ptXY.x, mouse.ptXY.y, 0, GetMessageExtraInfo());
			break;
		}
		lstPacket.push_back(CPacket(5, NULL, 0));

		return 0;
    }
    //��Ļ����
    int SendScreen(std::list<CPacket>& lstPacket, CPacket& inPacket)
    {
        CImage screen;//GDIȫ���豸�ӿ�,����һ����ͼ���豸�ӿ�
        HDC hScreen = ::GetDC(NULL);//��ȡ��Ļ���
        int nBitPerPixel = GetDeviceCaps(hScreen, BITSPIXEL);//��ȡ�豸ָ�����ݵ�����,BITSPIXEL��ȡһ�����ж��ٸ�����;//ARGB8888 32bit RGB888 24bit RGB565 16bit
        int nWidth = GetDeviceCaps(hScreen, HORZRES);
        int nHeight = GetDeviceCaps(hScreen, VERTRES);
        screen.Create(nWidth, nHeight, nBitPerPixel);//����һ��ͼ��

        BitBlt(screen.GetDC(), 0, 0, nWidth, nHeight, hScreen, 0, 0, SRCCOPY);//�൱�������һ�ν���,����4��5����Ļ�ֱ���
        //BitBlt(screen.GetDC(), 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), hScreen, 0, 0, SRCCOPY);//�൱�������һ�ν���,����4��5����Ļ�ֱ���
        ReleaseDC(NULL, hScreen);
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);//�ڱ��ط���һ�����Ե����Ķ�
        if (hMem == NULL) return -1;
        IStream* pStream = NULL;
        HRESULT ret = CreateStreamOnHGlobal(hMem, TRUE, &pStream);//�����ͱ��ط�����ڴ��������
        if (ret == S_OK) {
            screen.Save(pStream, Gdiplus::ImageFormatPNG);//������ؿ���ֱ�ӽ��ļ��������ڴ���
            LARGE_INTEGER bg = { 0 };
            ret = pStream->Seek(bg, STREAM_SEEK_SET, NULL);//Ĭ����ָ��ָֻ�����һ��
            PBYTE pData = (PBYTE)GlobalLock(hMem);//���뽫�����ڴ����������ܶ�ȡ������
            SIZE_T nSize = GlobalSize(hMem);
            lstPacket.push_back(CPacket(6, pData, nSize));
            GlobalUnlock(hMem);
        }

        //screen.Save(_T("test2022.png"), Gdiplus::ImageFormatPNG);//�������ı����ʽ,������һ������

        pStream->Revert();
        GlobalFree(hMem);
        screen.ReleaseDC();

        return 0;
    }
    int LockMachine(std::list<CPacket>& lstPacket, CPacket& inPacket)
    {
        if (dlg.m_hWnd == NULL || dlg.m_hWnd == INVALID_HANDLE_VALUE) {
            //_beginthread(threadLockDlg, 0, NULL);
            _beginthreadex(NULL, 0, CCommand::threadLockDlg, this, 0, &threadid);
        }
        lstPacket.push_back(CPacket(7, NULL, 0));
        return 0;
    }
    int UnlockMachine(std::list<CPacket>& lstPacket, CPacket& inPacket)
    {
        Sleep(50);//�ȴ��̺߳ʹ�����ȫ����
        //dlg.PostMessage(WM_KEYDOWN, 0x1b, 0);
        //dlg.SendMessageW(WM_KEYDOWN, 0x1b, 0);
        //PostMessage(dlg.m_hWnd, WM_KEYDOWN, 0x1b, 0);
        //SendMessage(dlg.m_hWnd, WM_KEYDOWN, 0x1b, 0);
        PostThreadMessage(threadid, WM_KEYDOWN, 0x41, 0);
        lstPacket.push_back(CPacket(8, NULL, 0));
        return 0;
    }

    int TestConnect(std::list<CPacket>& lstPacket, CPacket& inPacket)
    {
        lstPacket.push_back(CPacket(1981, NULL, 0));
        return 0;
    }


    int DeleteLocalFile(std::list<CPacket>& lstPacket, CPacket& inPacket)
    {
        //TODO:
        std::string strPath = inPacket.strData;
        TCHAR sPath[MAX_PATH] = _T("");
        //TCHAR buffer[MAX_PATH] = _T("");
        //int nret = MultiByteToWideChar(936, 0, strPath.c_str(), strPath.size() + 1, NULL, 0);
        //MultiByteToWideChar(CP_OEMCP, 0, strPath.c_str(), strPath.size() + 1, buffer, nret);
        //mbstowcs(buffer, strPath.c_str(), strPath.size()+1);//������������
        MultiByteToWideChar(CP_ACP, 0, strPath.c_str(), strPath.size(), sPath, sizeof(sPath) / sizeof(TCHAR));
        DeleteFileA(strPath.c_str());
        lstPacket.push_back(CPacket(9, NULL, 0));
        return 0;
    }
};