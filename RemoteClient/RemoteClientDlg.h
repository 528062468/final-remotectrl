
// RemoteClientDlg.h : header file
//

#pragma once
#include "ClientSocket.h"
#include "StatusDlg.h"
#ifndef WM_SEND_PACK_ACK
#define WM_SEND_PACK_ACK (WM_USER+2)//���Ͱ�����Ӧ��
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
	bool m_isClosed;//�����߳��Ƿ�ر�
private://TODO:���뼴�ĵ�
	void DealCommand(WORD nCmd,const std::string& strData, LPARAM lParam);
	void InitUIData();
	void LoadFileCurrent();
	void Str2Tree(const std::string drivers,CTreeCtrl& tree);
	void UpdateFileInfo(const FILEINFO& finfo, HTREEITEM hParent);
	void UpdateDownloadFile(const std::string &strData,FILE* pFile);
	CString GetPath(HTREEITEM hTree);
	void DeleteTreeChildrenItem(HTREEITEM hTree);
	//ʵ��
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
	// ��ʾ�ļ�
	CListCtrl m_List;
	afx_msg void OnNMRClickListFile(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDownloadFile();
	afx_msg void OnDeleteFile();
	afx_msg void OnRunFile();
	//�Զ�����Ϣ
	afx_msg void OnBnClickedBtnStartWatch();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnIpnFieldchangedIpaddressServ(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnSendPacketAck(WPARAM wParam, LPARAM lParam);
};







#if 0
void CRemoteClientDlg::OnNMRClickListFile(NMHDR* pNMHDR, LRESULT* pResult)//�Ҽ�ListcCtrl�ؼ������˵�
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	*pResult = 0;
	CPoint ptMouse, ptList;
	GetCursorPos(&ptMouse);
	ptList = ptMouse;
	m_List.ScreenToClient(&ptList);//����Ļ����ת,�ͻ�������
	int ListSelected = m_List.HitTest(ptList);
	if (ListSelected < 0)return;
	CMenu menu;
	menu.LoadMenu(IDR_MENU_RCLICK);
	CMenu* pPupup = menu.GetSubMenu(0);//ѡ��ڼ��ŵĲ˵�,�粻���ڽ����ؿ�
	if (pPupup != NULL) {
		int nRet = pPupup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, ptMouse.x, ptMouse.y, this);//��ָ����Ļ������ʾ�˵�,�ⶫ��������
		HTREEITEM hTree = m_Tree.GetSelectedItem();
		CString str = GetPath(hTree) + m_List.GetItemText(m_List.GetSelectionMark(), 0);
		switch (nRet) {
		case ID_DOWNLOAD_FILE:
		{
			CFileDialog dlgFile(FALSE, 0, m_List.GetItemText(m_List.GetSelectionMark(), 0));//����Ĭ���ļ����������ļ���һ��
			dlgFile.DoModal();
			CString filename = dlgFile.GetPathName();//��ȡ������ļ�
			TRACE("ID_DOWNLOAD_FILE PATH:%s :choice FilePath = %s\r\n", str, filename);//���Կ�����CFileDialog��ָ�����������ļ�·��
			//TODO:��������������ļ�
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