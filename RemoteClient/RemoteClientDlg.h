
// RemoteClientDlg.h : header file
//

#pragma once
#include "ClientSocket.h"
#include "StatusDlg.h"
#ifndef WM_SEND_PACK_ACK
#define WM_SEND_PACK_ACK (WM_USER+2)//发送包数据应答
#endif

// CRemoteClientDlg dialog
class CRemoteClientDlg : public CDialogEx
{
// Construction
public:
	CRemoteClientDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_REMOTECLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
public:
	void LoadFileInfo();
private:
	bool m_isClosed;//监视线程是否关闭
private://TODO:代码即文档
	void DealCommand(WORD nCmd,const std::string& strData, LPARAM lParam);
	void InitUIData();
	void LoadFileCurrent();
	void Str2Tree(const std::string drivers,CTreeCtrl& tree);
	void UpdateFileInfo(const FILEINFO& finfo, HTREEITEM hParent);
	void UpdateDownloadFile(const std::string &strData,FILE* pFile);
	CString GetPath(HTREEITEM hTree);
	void DeleteTreeChildrenItem(HTREEITEM hTree);
	//实现
protected:
	HICON m_hIcon;
	CStatusDlg m_dlgStatus;
	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnTest();
	DWORD m_server_address;
	CString m_nPort;
	afx_msg void OnBnClickedBtnFileinfo();
	CTreeCtrl m_Tree;
	afx_msg void OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	// 显示文件
	CListCtrl m_List;
	afx_msg void OnNMRClickListFile(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDownloadFile();
	afx_msg void OnDeleteFile();
	afx_msg void OnRunFile();
	//自定义消息
	afx_msg void OnBnClickedBtnStartWatch();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnIpnFieldchangedIpaddressServ(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnSendPacketAck(WPARAM wParam, LPARAM lParam);
};







#if 0
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
		int nRet = pPupup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, ptMouse.x, ptMouse.y, this);//在指定屏幕坐标显示菜单,这东西会阻塞
		HTREEITEM hTree = m_Tree.GetSelectedItem();
		CString str = GetPath(hTree) + m_List.GetItemText(m_List.GetSelectionMark(), 0);
		switch (nRet) {
		case ID_DOWNLOAD_FILE:
		{
			CFileDialog dlgFile(FALSE, 0, m_List.GetItemText(m_List.GetSelectionMark(), 0));//设置默认文件名与下载文件名一致
			dlgFile.DoModal();
			CString filename = dlgFile.GetPathName();//获取保存的文件
			TRACE("ID_DOWNLOAD_FILE PATH:%s :choice FilePath = %s\r\n", str, filename);//可以考虑用CFileDialog来指定保存下载文件路径
			//TODO:发送命令和下载文件
			break;
		}
		case ID_DELETE_FILE:
			TRACE("ID_DELETE_FILE PATH:%s\r\n", str);
			break;
		case ID_RUN_FILE:
			TRACE("ID_RUN_FILE PATH:%s\r\n", str);
			break;
		}
	}

}
#endif