﻿// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "ServerSocket.h"
#include "EdoyunTool.h"
#include"Command.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
//#pragma comment(linker,"/subsystem:windows /entry:WinMainCRTStartup")
//#pragma comment(linker,"/subsystem:windows /entry:mainCRTStartup")
//#pragma comment(linker,"/subsystem:console /entry:WinMainCRTStartup")
//#pragma comment(linker,"/subsystem:console /entry:mainCRTStartup")

// 唯一的应用程序对象

CWinApp theApp;
using namespace std;
//操作注册表添加自启动
//开机启动的时候,程序的权限是跟随启动用户的
//如果两者权限不一致,则会导致程序启动失败
//开机启动对环境变量有影响,如果以来dll(动态库),则可能启动失败
//解决方案:
//[赋值这些dll到system32下面或者sysWOW64下面]
//system32下面,多是64为程序,syswow64下面多少32位程序
//使用静态库
void ChooseAutoInvoke() {
    //TCHAR wcsSystem[MAX_PATH] = _T("");
    //GetSystemDirectory(wcsSystem, MAX_PATH);
    //CString strPath = CString(wcsSystem) + _T("\\RemoteCtrl.exe");//%SystemRoot%会自动获取系统盘:\\Windows
    CString strPath = CString("C:\\Windows\\SysWOW64\\RemoteCtrl.exe");//%SystemRoot%会自动获取OS:\\Windows
    if (PathFileExists(strPath)) {//检测文件是否存在
        return;
    }
    //在HKEY_LOCAL_MACHINE下面
    CString strSubKey = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");//系统自启动固定位置
    CString strInfo = _T("该程序只允许用于合法的用途!\n");
    strInfo += _T("继续运行该程序,将使得这台机器处于被监控状态\n");
    strInfo += _T("如果你不希望这样,请按\"取消\"按钮,退出程序\n");
    strInfo += _T("按下\"是\"按钮,该程序将被复制到你的机器上,并随系统启动而自动运行!\n");
    strInfo += _T("按下\"否\"按钮,程序只运行一次,不会再系统内留下任何东西\n");
    int ret = MessageBox(NULL, strInfo, _T("警告"), MB_YESNOCANCEL | MB_ICONWARNING | MB_TOPMOST);
    if (ret == IDYES) {
        char sPath[MAX_PATH] = "";
	    char sSys[MAX_PATH] = "";
        std::string strExe = "\\RemoteCtrl.exe ";
        GetCurrentDirectoryA(MAX_PATH, sPath);//获取可执行文件的路径
	    GetSystemDirectoryA(sSys, sizeof(sSys));//在系统盘:\\Windows\\System32目录下创建软连接
        std::string strCmd = "mklink "+std::string(sSys)+ strExe + sPath+ strExe;
        system(strCmd.c_str());
        HKEY hKey = NULL;
        ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey, 0, KEY_ALL_ACCESS|KEY_WOW64_64KEY, &hKey);
        if (ret != ERROR_SUCCESS) {
            RegCloseKey(hKey);
            MessageBox(NULL, _T("设置自动开机启动失败!是否权限不足"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
            ::exit(0);
        }
        TCHAR sSysPath[MAX_PATH] =_T("");
        GetSystemDirectoryW(sSysPath, MAX_PATH);
        //CString strPath = sSysPath + CString(_T("\\RemoteCtrl.exe"));//通用做法
        //ret = RegSetValueEx(hKey, _T("RemoteCtrl"), 0, REG_SZ, (BYTE*)(LPCTSTR)strPath, strPath.GetLength());
        //strPath = sSysPath + CString(_T("\\RemoteCtrl.exe"));;
        ret = RegSetValueEx(hKey, _T("RemoteCtrl"), 0, REG_EXPAND_SZ, (BYTE*)(LPCTSTR)strPath, strPath.GetLength()*sizeof(TCHAR));
        if (ret != ERROR_SUCCESS) {
            RegCloseKey(hKey);
            MessageBox(NULL, _T("设置自动开机启动失败!是否权限不足"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
            ::exit(0);
        }
        RegCloseKey(hKey);
    }
    else if (ret == IDCANCEL) {
        ::exit(0);
    }
    return;
}

int main()
{
    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(nullptr);
    
    if (hModule != nullptr)
    {
        // 初始化 MFC 并在失败时显示错误
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: 在此处为应用程序的行为编写代码。
            wprintf(L"错误: MFC 初始化失败\n");
            nRetCode = 1;
        }
        else
        {
            CCommand cmd;
            ChooseAutoInvoke();
            int ret = CServerSocket::getInstance()->Run(&CCommand::RunCommand, &cmd);
			switch (ret) {
			case -1:
				MessageBox(NULL, _T("网络初始化异常,未能成功初始化,请检查网络状态!"), _T("网络初始化失败"), MB_OK | MB_ICONERROR);
				exit(0);
				break;
			case -2:
				MessageBox(NULL, _T("多次无法正常接入用户,结束程序!"), _T("接入用户失败!"), MB_OK | MB_ICONERROR);
				exit(0);
                break;
			}

		}
	}
	else
	{//TODO: 在此处为应用程序的行为编写代码。
        wprintf(L"错误: GetModuleHandle 失败\n");
        nRetCode = 1;
    }




    return nRetCode;
}