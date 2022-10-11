
// RemoteClientDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "RemoteClient.h"
#include "RemoteClientDlg.h"
#include "afxdialogex.h"
#include "ClientController.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include "CWatchDialog.h"

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CRemoteClientDlg dialog



CRemoteClientDlg::CRemoteClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_REMOTECLIENT_DIALOG, pParent)
	, m_server_address(0)
	, m_nPort(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRemoteClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_IPAddress(pDX, IDC_IPADDRESS_SERV, m_server_address);
	DDX_Text(pDX, IDC_EDIT_PORT, m_nPort);
	DDX_Control(pDX, IDC_TREE_DIR, m_Tree);
	DDX_Control(pDX, IDC_LIST_FILE, m_List);
}




BEGIN_MESSAGE_MAP(CRemoteClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_TEST, &CRemoteClientDlg::OnBnClickedBtnTest)//WM_COMMAND
	ON_BN_CLICKED(IDC_BTN_FILEINFO, &CRemoteClientDlg::OnBnClickedBtnFileinfo)//WM_COMMAND
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_DIR, &CRemoteClientDlg::OnNMDblclkTreeDir)//WM_NOTIFY
	ON_NOTIFY(NM_CLICK, IDC_TREE_DIR, &CRemoteClientDlg::OnNMClickTreeDir)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_FILE, &CRemoteClientDlg::OnNMRClickListFile)
	ON_COMMAND(ID_DOWNLOAD_FILE, &CRemoteClientDlg::OnDownloadFile)
	ON_COMMAND(ID_DELETE_FILE, &CRemoteClientDlg::OnDeleteFile)
	ON_COMMAND(ID_RUN_FILE, &CRemoteClientDlg::OnRunFile)
	//ON_MESSAGE(WM_SEND_PACKET,&CRemoteClientDlg::OnSendPacket)//自定义消息,添加函数,SendMessage(WM_SEND_PACKET,,,)可调用该函数
	ON_BN_CLICKED(IDC_BTN_START_WATCH, &CRemoteClientDlg::OnBnClickedBtnStartWatch)
	ON_WM_TIMER()
	ON_NOTIFY(IPN_FIELDCHANGED, IDC_IPADDRESS_SERV, &CRemoteClientDlg::OnIpnFieldchangedIpaddressServ)
	ON_MESSAGE(WM_SEND_PACK_ACK, &CRemoteClientDlg::OnSendPacketAck)
END_MESSAGE_MAP()


// CRemoteClientDlg message handlers

BOOL CRemoteClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	InitUIData();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CRemoteClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CRemoteClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CRemoteClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CRemoteClientDlg::OnBnClickedBtnTest()
{
	CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 1981);
}



//制作文件树状图
void CRemoteClientDlg::OnBnClickedBtnFileinfo()
{
	std::list<CPacket> lstPackets;
	bool ret = CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 1, true, NULL, 0);
	if (ret == 0) {
		AfxMessageBox(_T("命令处理失败!!!"));
		return;
	}

}





void CRemoteClientDlg::DealCommand(WORD nCmd, const std::string& strData, LPARAM lParam)
{
	switch (nCmd) {
	case 1://获取驱动信息
		Str2Tree(strData, m_Tree);
		break;
	case 2://获取文件信息
		UpdateFileInfo(*(PFILEINFO)strData.c_str(), (HTREEITEM)lParam);
		break;
	case 3:
		MessageBox("打开文件完成", "提示", MB_ICONINFORMATION);
		break;
	case 4:
		UpdateDownloadFile(strData, (FILE*)lParam);
		break;
	case 9:
		MessageBox("删除文件完成", "提示", MB_ICONINFORMATION);
		break;
	case 1981:
		MessageBox("连接成功", "提示", MB_ICONINFORMATION);
		break;
	default:
		TRACE("unknow data received!\r\n", nCmd);
		break;
	}
}

void CRemoteClientDlg::InitUIData()
{
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	// TODO: Add extra initialization here
	UpdateData();
	m_server_address = htonl(inet_addr("127.0.0.1"));
	m_nPort = _T("9527");
	CClientController* pController = CClientController::getInstance();
	pController->UpdateAddress(m_server_address, atoi((LPCTSTR)m_nPort));
	UpdateData(false);
	m_dlgStatus.Create(IDD_DLG_STATUS, this);
	m_dlgStatus.ShowWindow(SW_HIDE);
}

void CRemoteClientDlg::LoadFileCurrent()//删除文件后重新加载 
{
	HTREEITEM hTree = m_Tree.GetSelectedItem();
	CString strPath = GetPath(hTree);
	m_List.DeleteAllItems();//清除列表控件
	int nCmd = CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 2, false, (BYTE*)(LPCTSTR)strPath, strPath.GetLength());
	PFILEINFO pInfo = (PFILEINFO)CClientSocket::getInstance()->GetPacket().strData.c_str();

	while (pInfo->HasNext) {
		TRACE("[%s] isdir%d\r\n", pInfo->szFileName, pInfo->IsDirectory);
		if (!pInfo->IsDirectory) {//只处理文件列表
			m_List.InsertItem(m_List.GetItemCount(), pInfo->szFileName);//只需要重新插入列表
		}
		int cmd = CClientController::getInstance()->DealCommand();
		TRACE("ack:%d \r\n", cmd);
		if (cmd < 0)  break;
		pInfo = (PFILEINFO)CClientSocket::getInstance()->GetPacket().strData.c_str();
	}
}

void CRemoteClientDlg::LoadFileInfo()
{
	CPoint ptMouse;
	GetCursorPos(&ptMouse);//获取鼠标的全局屏幕坐标
	m_Tree.ScreenToClient(&ptMouse);
	HTREEITEM hTreeSelected = m_Tree.HitTest(ptMouse, 0);//获取用户点击到哪个节点
	if (hTreeSelected == NULL)//判断用户是否双击到节点
		return;
	//if (m_Tree.GetChildItem(hTreeSelected) == NULL)
	//return;//在定义树时将存是文件属性是文件夹的添加了空子节点
	DeleteTreeChildrenItem(hTreeSelected);//当用户重复点击时删除之前的子节点
	m_List.DeleteAllItems();
	CString strPath = GetPath(hTreeSelected);
	TRACE("hTreeSelected %08X\r\n", hTreeSelected);
	CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 2, false, (BYTE*)(LPCTSTR)strPath, strPath.GetLength(), (LPARAM)hTreeSelected);
}

void CRemoteClientDlg::Str2Tree(const std::string drivers, CTreeCtrl& tree)
{
	std::string dr;
	tree.DeleteAllItems();
	TRACE("OnBnClickedBtnFileinfo %s\r\n", drivers.c_str());
	for (size_t i = 0; i < drivers.size(); i++)
	{
		if (drivers[i] == ',')
		{
			dr += ":";
			HTREEITEM hTemp = tree.InsertItem(dr.c_str(), TVI_ROOT, TVI_LAST);
			tree.InsertItem(NULL, hTemp, TVI_LAST);
			dr.clear();
			continue;
		}
		dr += drivers[i];
	}
	if (dr.size() > 0) {
		dr += ":";
		HTREEITEM hTemp = tree.InsertItem(dr.c_str(), TVI_ROOT, TVI_LAST);
		tree.InsertItem(NULL, hTemp, TVI_LAST);
	}
}

void CRemoteClientDlg::UpdateFileInfo(const FILEINFO& finfo, HTREEITEM hParent)
{
	TRACE("hasnext %d isdirectory %d %s\r\n", finfo.HasNext, finfo.IsDirectory, finfo.szFileName);
	if (finfo.HasNext == FALSE) return;
	if (finfo.IsDirectory) {
		if ((CString(finfo.szFileName) == _T(".")) || (CString(finfo.szFileName) == _T("..")))
			return;
		TRACE("hselected %08X %08X\r\n", hParent, m_Tree.GetSelectedItem());
		HTREEITEM hTemp = m_Tree.InsertItem(finfo.szFileName, hParent);
		m_Tree.InsertItem(_T(""), hTemp, TVI_LAST);
		m_Tree.Expand(hParent, TVE_EXPAND);//展开文件夹
	}
	else {
		m_List.InsertItem(m_List.GetItemCount(), finfo.szFileName);
	}
}

void CRemoteClientDlg::UpdateDownloadFile(const std::string& strData, FILE* pFile)
{
	static LONGLONG length = 0, index = 0;
	if (length == 0) {
		length = *(long long*)strData.c_str();
		if (length == 0) {
			AfxMessageBox("文件长度为0或者无法接收文件!!!");
			CClientController::getInstance()->DownloadEnd();
		}
	}
	else if (length > 0 && index >= length) {
		fclose(pFile);
		length = 0;
		index = 0;
		CClientController::getInstance()->DownloadEnd();
	}
	else {
		fwrite(strData.c_str(), 1, strData.size(), pFile);
		index += strData.size();
		TRACE("index = %d\r\n", index);
		if (index >= length) {
			fclose(pFile);
			length = 0;
			index = 0;
			CClientController::getInstance()->DownloadEnd();
		}
	}
}

CString CRemoteClientDlg::GetPath(HTREEITEM hTree)//子节点找父节点
{
	CString strRet, strTmp;
	do {
		strTmp = m_Tree.GetItemText(hTree);
		strRet = strTmp + '\\' + strRet;
		hTree = m_Tree.GetParentItem(hTree);//获取父节点
	} while (hTree != NULL);
	return strRet;
}

void CRemoteClientDlg::DeleteTreeChildrenItem(HTREEITEM hTree)
{
	HTREEITEM hSub = NULL;
	do {
		hSub = m_Tree.GetChildItem(hTree);
		TRACE("PATH DELETE TREE IS%s\r\n", m_Tree.GetItemText(hSub));
		if (hSub != NULL) m_Tree.DeleteItem(hSub);
	} while (hSub != NULL);
}

void CRemoteClientDlg::OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	LoadFileInfo();
}


void CRemoteClientDlg::OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	LoadFileInfo();
}


void CRemoteClientDlg::OnNMRClickListFile(NMHDR* pNMHDR, LRESULT* pResult)//右键ListcCtrl控件弹出菜单
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	CPoint ptMouse, ptList;
	GetCursorPos(&ptMouse);
	ptList = ptMouse;
	m_List.ScreenToClient(&ptList);//将屏幕坐标转,客户区坐标
	int ListSelected = m_List.HitTest(ptList);
	if (ListSelected < 0)return;
	CMenu menu;
	menu.LoadMenu(IDR_MENU_RCLICK);
	CMenu* pPupup = menu.GetSubMenu(0);//选择第几排的菜单,如不存在将返回空
	if (pPupup != NULL) {
		pPupup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, ptMouse.x, ptMouse.y, this);//在指定屏幕坐标显示菜单,这东西会阻塞
	}

}
static TCHAR szFilter[] = _T("Chart Files (*.xlc)|*.xlc|")
_T("Worksheet Files (*.xls)|*.xls|Data Files (*.xlc;*.xls)|")
_T("*.xlc; *.xls|All Files (*.*)|*.*||");

void CRemoteClientDlg::OnDownloadFile()
{
	int nListSelected = m_List.GetSelectionMark();
	CString strFile = m_List.GetItemText(nListSelected, 0);
	HTREEITEM hSelected = m_Tree.GetSelectedItem();
	strFile = GetPath(hSelected) + strFile;
	int ret = CClientController::getInstance()->DownFile(strFile);
	if (ret != 0) {
		AfxMessageBox(_T("下载失败!!"));
		TRACE("下载失败 ret = %d\r\n", ret);
	}
}


void CRemoteClientDlg::OnDeleteFile()
{
	// TODO: 在此添加命令处理程序代码
	HTREEITEM hSelected = m_Tree.GetSelectedItem();
	CString strFile = GetPath(hSelected);
	int nListSelected = m_List.GetSelectionMark();
	strFile += m_List.GetItemText(nListSelected, 0);
	int ret = CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 9, true, (BYTE*)(LPCSTR)strFile, strFile.GetLength());
	if (ret < 0) {
		AfxMessageBox(_T("删除文件命令执行失败!!"));
	}
	LoadFileCurrent();
}


void CRemoteClientDlg::OnRunFile()
{
	HTREEITEM hSelected = m_Tree.GetSelectedItem();
	CString strFile = GetPath(hSelected);
	int nListSelected = m_List.GetSelectionMark();
	strFile += m_List.GetItemText(nListSelected, 0);
	int ret = CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 3, true, (BYTE*)(LPCSTR)strFile, strFile.GetLength());
	if (ret < 0) {
		AfxMessageBox(_T("打开文件命令执行失败!!"));
	}
}




void CRemoteClientDlg::OnBnClickedBtnStartWatch()
{//开启监控对话框

	CClientController::getInstance()->StartWatchScreen();
}



void CRemoteClientDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialogEx::OnTimer(nIDEvent);
}


void CRemoteClientDlg::OnIpnFieldchangedIpaddressServ(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMIPADDRESS pIPAddr = reinterpret_cast<LPNMIPADDRESS>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	CClientController* pController = CClientController::getInstance();
	pController->UpdateAddress(m_server_address, atoi((LPCTSTR)m_nPort));
	UpdateData();
}

LRESULT CRemoteClientDlg::OnSendPacketAck(WPARAM wParam, LPARAM lParam)
{
	if ((int)lParam == -1 || ((int)lParam == -2)) {
		TRACE("socket is error %d\r\n", lParam);
		//TODO:错误处理
	}
	else if (lParam == 1) {
		//对方关闭了套接字
		TRACE("socket is closed!\r\n");
	}
	else {
		if (wParam != NULL) {
			CPacket head = *(CPacket*)wParam;
			delete (CPacket*)wParam;
			DealCommand(head.sCmd, head.strData, lParam);
		}
	}
	return 0;
}
