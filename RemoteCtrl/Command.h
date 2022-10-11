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
                    TRACE("服务端执行命令失败: %d ret = %d \r\n", status, ret);
            }
        }
        else {
            MessageBox(NULL, _T("无法正常接入用户,自动重试"), _T("接入用户失败!"), MB_OK | MB_ICONERROR);
        }
    }
protected:
    typedef int(CCommand::* CMDFUNC)( std::list<CPacket>&, CPacket&);//成员函数指针
    std::map<int, CMDFUNC> m_mapFunction;//从命令号到功能的映射
    CLockInfoDialog dlg;
    unsigned threadid;//保存窗口所在线程的ID便于跨线程PostThreadMessage();
protected:

    static unsigned WINAPI threadLockDlg(void* arg)
    {
        CCommand* thiz = (CCommand*)arg;
        thiz->threadLockDlgMain();
        _endthreadex(0);//终止线程
        return 0;
    }
    void threadLockDlgMain() {
        TRACE("");
        dlg.Create(IDD_DIALOG_INFO, NULL);
        dlg.ShowWindow(SW_SHOW);
        CRect rect;
        rect.left = 0;
        rect.top = 0;
        rect.right = GetSystemMetrics(SM_CXFULLSCREEN);//SM_CXSCREEN和SM_CXFULLSCREEN一样
        rect.bottom = GetSystemMetrics(SM_CYSCREEN);//SM_CYSCREEN比SM_CYFULLSCREEN多了27个像素
        //设置对话框大小和位置
        dlg.MoveWindow(rect);
        CWnd* pText = dlg.GetDlgItem(IDC_STATIC);
        if (pText->GetSafeHwnd()) {
            //pText->CenterWindow();
            CRect reText;//将静态文本居中
            pText->GetWindowRect(reText);
            int nWidth = reText.Width();
            int x = (rect.right - nWidth) / 2;
            int nHeight = reText.Height();
            int y = (rect.bottom - nHeight) / 2;
            pText->MoveWindow(x, y, reText.Width(), reText.Height());
        }
        //SWP set windows pos,窗口置顶
        dlg.SetWindowPos(&dlg.wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
        //隐藏鼠标
        ShowCursor(false);
        //隐藏任务栏
        ::ShowWindow(::FindWindow(_T("ShelL_TrayWnd"), NULL), SW_HIDE);
        rect.left = 0;
        rect.top = 0;
        rect.right = rect.left + 1;
        rect.bottom = rect.top + 1;//缩小可活到范围到一个像素点
        ClipCursor(rect);//固定鼠标的活动范围
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {//非模态对话框添加消息循环
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_KEYDOWN) {//有键盘按键按下
                TRACE("msg:%08x wparam:%08x lparam:%08x \n", msg.message, msg.wParam, msg.lParam);
                if (msg.wParam == 0x41 || msg.wParam == 0x1b)break;
            }
        }
        ClipCursor(NULL);//恢复鼠标活动范围
        ShowCursor(true);//取消鼠标显示
        ::ShowWindow(::FindWindow(_T("ShelL_TrayWnd"), NULL), SW_SHOW);
        dlg.DestroyWindow();//必须在线程结束之前销毁
    }
    int MakeDriverInfo(std::list<CPacket>& lstPacket, CPacket& inPacket) {//发送磁盘分区信息
        std::string result;
        for (int i = 1; i <= 26; i++) {
            if (0 == _chdrive(i)) {//磁盘分区1:A盘,返回磁盘分区是否存在
                if (result.size() > 0)
                    result += ',';
                result += 'A' + i - 1;
            }
        }
        lstPacket.push_back(CPacket(1, (BYTE*)result.c_str(), result.size()));
        return 0;
    }


	int MakeDirectoryInfo(std::list<CPacket>& lstPacket, CPacket& inPacket) {//发送目录文件信息
		std::string strPath = inPacket.strData;
		//std::list<FILEINFO> lstFileInfos;
		if (_chdir(strPath.c_str()) != 0) {
			FILEINFO finfo;
			finfo.HasNext = FALSE;
			lstPacket.push_back(CPacket(2, (BYTE*)&finfo, sizeof(finfo)));
			OutputDebugString(_T("没有权限,访问目录!!"));
			return -2;
		}
        _finddata_t fdata;
        int hfind = 0;
        if ((hfind = _findfirst("*", &fdata)) == -1) {
            OutputDebugString(_T("没有找到任何文件!!"));
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
        //发送信息到控制端
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
        if (err != 0) {//文件打开失败返回NULL
            lstPacket.push_back(CPacket(4, (BYTE*)&data, 8));
            return -1;
        }
        if (pFile != NULL) {
            fseek(pFile, 0, SEEK_END);
            data = _ftelli64(pFile);//获取文件长度
            lstPacket.push_back(CPacket(4, (BYTE*)&data, 8));//发送文件大小
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
		case 0://左键
			nFlags = 1;
			break;
		case 1://右键
			nFlags = 2;
			break;
		case 2://中键
			nFlags = 4;
			break;
		case 4://没有按键
			nFlags = 8;
			break;
		}
		if (nFlags != 8)SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);

		switch (mouse.nAction) {
		case 0://单击
			nFlags |= 0x10;
			break;
		case 1://双击
			nFlags |= 0x20;
			break;
		case 2://按下
			nFlags |= 0x40;
			break;
		case 3://抬起
			nFlags |= 0x80;
			break;
		default:
			break;
		}
		TRACE("mouse event : %08X x %d y %d \r\n", nFlags, mouse.ptXY.x, mouse.ptXY.y);
		switch (nFlags) {
		case 0x21://左键双击等价于两次单击
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());//全局鼠标模拟系统API,GetMessageExtraInfo()获取当前线程的额外信息
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
		case 0x11://左键单击
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x41://左键按下
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x81://左键抬起
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x22://右键双击
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
		case 0x12://右键单击
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x42://右键按下
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x82://右键抬起
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x24://中键双击
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
		case 0x14://中键单击
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x44://中键按下
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x84://中键抬起
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x08://单纯鼠标移动
			mouse_event(MOUSEEVENTF_MOVE, mouse.ptXY.x, mouse.ptXY.y, 0, GetMessageExtraInfo());
			break;
		}
		lstPacket.push_back(CPacket(5, NULL, 0));

		return 0;
    }
    //屏幕截屏
    int SendScreen(std::list<CPacket>& lstPacket, CPacket& inPacket)
    {
        CImage screen;//GDI全局设备接口,还有一个是图形设备接口
        HDC hScreen = ::GetDC(NULL);//获取屏幕句柄
        int nBitPerPixel = GetDeviceCaps(hScreen, BITSPIXEL);//获取设备指定内容的属性,BITSPIXEL获取一个点有多少个比特;//ARGB8888 32bit RGB888 24bit RGB565 16bit
        int nWidth = GetDeviceCaps(hScreen, HORZRES);
        int nHeight = GetDeviceCaps(hScreen, VERTRES);
        screen.Create(nWidth, nHeight, nBitPerPixel);//创建一个图像

        BitBlt(screen.GetDC(), 0, 0, nWidth, nHeight, hScreen, 0, 0, SRCCOPY);//相当于完成了一次截屏,参数4和5是屏幕分辨率
        //BitBlt(screen.GetDC(), 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), hScreen, 0, 0, SRCCOPY);//相当于完成了一次截屏,参数4和5是屏幕分辨率
        ReleaseDC(NULL, hScreen);
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);//在本地分配一个可以调整的堆
        if (hMem == NULL) return -1;
        IStream* pStream = NULL;
        HRESULT ret = CreateStreamOnHGlobal(hMem, TRUE, &pStream);//将流和本地分配堆内存关联起来
        if (ret == S_OK) {
            screen.Save(pStream, Gdiplus::ImageFormatPNG);//这个重载可以直接将文件保存在内存中
            LARGE_INTEGER bg = { 0 };
            ret = pStream->Seek(bg, STREAM_SEEK_SET, NULL);//默认流指针只指向最后一个
            PBYTE pData = (PBYTE)GlobalLock(hMem);//必须将本地内存句柄上锁才能读取到东西
            SIZE_T nSize = GlobalSize(hMem);
            lstPacket.push_back(CPacket(6, pData, nSize));
            GlobalUnlock(hMem);
        }

        //screen.Save(_T("test2022.png"), Gdiplus::ImageFormatPNG);//参数二的保存格式,这是另一个重载

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
        Sleep(50);//等待线程和窗口完全创建
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
        //mbstowcs(buffer, strPath.c_str(), strPath.size()+1);//中文容易乱码
        MultiByteToWideChar(CP_ACP, 0, strPath.c_str(), strPath.size(), sPath, sizeof(sPath) / sizeof(TCHAR));
        DeleteFileA(strPath.c_str());
        lstPacket.push_back(CPacket(9, NULL, 0));
        return 0;
    }
};