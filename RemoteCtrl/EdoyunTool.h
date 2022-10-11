#pragma once
class CEdoyunTool
{
public:
	static void Dump(BYTE* pData, size_t nSize) {
        std::string strOut;
        for (size_t i = 0; i < nSize; i++)
        {
            char buf[8] = "";
            if (i > 0 && (i % 16 == 0))strOut += "\n";
            snprintf(buf, sizeof(buf), "%02X ", pData[i] & 0xFF);
            strOut += buf;
        }
        strOut += "\n";
        OutputDebugStringA(strOut.c_str());
    }
    //权限管理查看
    static bool IsAdmin() {
        HANDLE hToken = NULL;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
        {
            ShowError();
            return false;
        }
        TOKEN_ELEVATION eve;
        DWORD len = 0;
        //TOKEN_INFORMATION_CLASS cl;
        if (GetTokenInformation(hToken, TokenElevation, &eve, sizeof(eve), &len) == FALSE) {
            ShowError();
            return false;
        }
        CloseHandle(hToken);
        if (len == sizeof(eve)) {
            return eve.TokenIsElevated;//该成员权限
        }
        printf("length of tokeninformation is %d\r\n", len);
        return false;
    }
    static bool RunAsAdmin()
    {//获取管理员权限,使用该权限创建进程
        //本地策略组 开启Administrator 禁止空密码只能登陆本地控制台
        STARTUPINFO si = { 0 };
        PROCESS_INFORMATION pi = { 0 };
        TCHAR sPath[MAX_PATH] = _T("");
        GetModuleFileName(NULL, sPath, MAX_PATH);
        //创建拥有管理员权限的子进程
        BOOL ret = CreateProcessWithLogonW(_T("Administrator"), NULL, NULL, LOGON_WITH_PROFILE, NULL, sPath, CREATE_UNICODE_ENVIRONMENT, NULL, NULL, &si, &pi);
        if (!ret) {
            ShowError();//TODO:去除调试信息
            MessageBox(NULL, sPath, _T("程序错误,创建进程失败"), 0);//TODO:去除调试信息
            return false;
        }
        WaitForSingleObject(pi.hProcess, INFINITE);//等待子进程结束
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return true;
    }
    //获取window错误消息
    static void ShowError() {
        LPWSTR lpMessageBuf = NULL;
        //strerror(errno);//标准C语言库的错误
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
            NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPWSTR)&lpMessageBuf, 0, NULL);//类似错误查找,自动申请缓冲区内并将错误信息存入
        OutputDebugString(lpMessageBuf);
        MessageBox(NULL, lpMessageBuf, _T("发生错误"), 0);
        LocalFree(lpMessageBuf);//释放
    }

    /*
改bug的思路
1观察现象
2确定范围
3分析错误的可能性
4调试或者打日志排除错误
5处理错误
6验证/长时间验证/多次验证/多条件的验证
*/
	static BOOL WriteStartupDir(const CString& strPath) {//C:\ProgramData\Microsoft\Windows\Start Menu\Programs\Startup
        //通过复制文件到开机启动文件夹,实现开机启动
		TCHAR sPath[MAX_PATH] = _T("");
		GetModuleFileName(NULL, sPath, MAX_PATH);
		return CopyFile(sPath, strPath, FALSE);
	}


//操作注册表添加自启动
//开机启动的时候,程序的权限是跟随启动用户的
//如果两者权限不一致,则会导致程序启动失败
//开机启动对环境变量有影响,如果以来dll(动态库),则可能启动失败
//解决方案:
//[赋值这些dll到system32下面或者sysWOW64下面]
//system32下面,多是64为程序,syswow64下面多少32位程序
//使用静态库

    static bool WriteRegisterTable(const CString& strPath) {
        //通过修改注册表来实现开机启动
        CString strSubKey = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");//系统自启动固定位置
        TCHAR sPath[MAX_PATH] = _T("");
        GetModuleFileName(NULL, sPath, MAX_PATH);
        BOOL ret = CopyFile(sPath, strPath, FALSE);
        if (ret == FALSE) {
            MessageBox(NULL, _T("复制文件失败,是否权限不足\r\n"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
        }
        HKEY hKey = NULL;
        ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hKey);
        if (ret != ERROR_SUCCESS) {
            RegCloseKey(hKey);
            MessageBox(NULL, _T("设置自动开机启动失败!是否权限不足"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
            return false;
        }
        TCHAR sSysPath[MAX_PATH] = _T("");
        GetSystemDirectoryW(sSysPath, MAX_PATH);
        ret = RegSetValueEx(hKey, _T("RemoteCtrl"), 0, REG_EXPAND_SZ, (BYTE*)(LPCTSTR)strPath, strPath.GetLength() * sizeof(TCHAR));
        if (ret != ERROR_SUCCESS) {
            RegCloseKey(hKey);
            MessageBox(NULL, _T("设置自动开机启动失败!是否权限不足"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
            return false;
        }
        RegCloseKey(hKey);
        return true;
    }

    static bool Init() 
    {//用于带mfc命令行项目初始化(通用)
        HMODULE hModule = ::GetModuleHandle(nullptr);
        if (hModule == nullptr) {
            wprintf(L"错误: GetModuleHandle 失败\n");
            return false;
        }
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: 在此处为应用程序的行为编写代码。
            wprintf(L"错误: MFC 初始化失败\n");
            return false;
        }
        return true;
    }
};

