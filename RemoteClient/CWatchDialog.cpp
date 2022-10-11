// CWatchDialog.cpp: 实现文件
//

#include "pch.h"
#include "RemoteClient.h"
#include "CWatchDialog.h"
#include "afxdialogex.h"
#include "ClientController.h"
// CWatchDialog 对话框

IMPLEMENT_DYNAMIC(CWatchDialog, CDialogEx)

CWatchDialog::CWatchDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DLG_WATCH, pParent)
{
	m_nObjWidth = -1;
	m_nObjHeight = -1;
	m_isFull = false;
}

CWatchDialog::~CWatchDialog()
{
}

void CWatchDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_WATCH, m_picture);
}


BEGIN_MESSAGE_MAP(CWatchDialog, CDialogEx)
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_STN_CLICKED(IDC_WATCH, &CWatchDialog::OnStnClickedWatch)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_LOCK, &CWatchDialog::OnBnClickedBtnLock)
	ON_BN_CLICKED(IDC_BTN_UNLOCK, &CWatchDialog::OnBnClickedBtnUnlock)
	ON_MESSAGE(WM_SEND_PACK_ACK, &CWatchDialog::OnSendPacketAck)
END_MESSAGE_MAP()


// CWatchDialog 消息处理程序


CPoint CWatchDialog::UserPoint2RemoteScreenPoint(CPoint& point,bool isScreen)
{//客户端800 450,转换坐标
	CPoint cur = point;
	CRect clientRect;
	if (!isScreen) ClientToScreen(&point);//转换为相对屏幕左上角的坐标(屏幕内的绝对坐标)
	m_picture.ScreenToClient(&point);//转换为客户区域坐标(相对picture左上角的坐标)
	//m_picture.ClientToScreen(&point);//全局坐标到客户区坐标
	//本地坐标,到远程坐标
	m_picture.GetWindowRect(clientRect);
	//坐标转换

	//CRect A,B;
	//m_picture.ClientToScreen(A);
	//ClientToScreen(B);
	//int top = A.top - B.top;
	//return CPoint(point.x * m_nObjWidth / clientRect.Width(), (point.y - top) * m_nObjHeight / clientRect.Height());//m_nObjHeight和m_nObjWidth是从被控端获取的
	int px = point.x * m_nObjWidth / clientRect.Width();
	int py = point.y * m_nObjHeight / clientRect.Height();
	char buffer[256];
	sprintf(buffer, "x=%d,y=%d", px, py);
	SetWindowText(buffer);
	return CPoint(px, py);//m_nObjHeight和m_nObjWidth是从被控端获取的
}

BOOL CWatchDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	if (false) {//读取注册表数据,失败使用默认的尺寸
		int x = GetSystemMetrics(SM_CXSCREEN) ;
		int y = GetSystemMetrics(SM_CYSCREEN);//获取当前屏幕分辨率
		CRect rect(0, 0, x, y);
		MoveWindow(rect);
		CenterWindow();
	}
	// TODO:  在此添加额外的初始化
	m_isFull = false;
	//SetTimer(0, 45,NULL );
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CWatchDialog::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//if (nIDEvent == 0) {		
	//	CClientController* pParent = CClientController::getInstance();
	//	if (m_isFull) {
	//		CRect rect;
	//		m_picture.GetWindowRect(rect);//依照当前对话框大小显示缩放
	//		m_nObjWidth = m_image.GetWidth();
	//		m_nObjHeight = m_image.GetHeight();//获取被控端传过来图片的尺寸
	//		m_image.StretchBlt(m_picture.GetDC()->GetSafeHdc(), 0, 0, rect.Width(), rect.Height(), SRCCOPY);//缩放
	//		m_picture.InvalidateRect(NULL);//调用重绘函数
	//		m_image.Destroy();
	//		m_isFull = false;//设置状态为空
	//		TRACE("更新图片完成%d %d\r\n", m_nObjWidth, m_nObjHeight);
	//	}
	//}
	CDialogEx::OnTimer(nIDEvent);
}



LRESULT CWatchDialog::OnSendPacketAck(WPARAM wParam, LPARAM lParam)
{
	if ((int)lParam == -1 || ((int)lParam ==-2)) {
		//TODO:错误处理
	}
	else if (lParam == 1) {
		//对方关闭了套接字
	}
	else {
		CPacket* pPacket = (CPacket*)wParam;
		if (pPacket != NULL) {
			CPacket head = *(CPacket*)wParam;
			delete (CPacket*)wParam;
			switch (head.sCmd) {
			case 6:
			{
					CEdoyunTool::Bytes2Image(m_image, head.strData);
					CRect rect;
					m_picture.GetWindowRect(rect);//依照当前对话框大小显示缩放
					m_nObjWidth = m_image.GetWidth();
					m_nObjHeight = m_image.GetHeight();//获取被控端传过来图片的尺寸
					m_image.StretchBlt(m_picture.GetDC()->GetSafeHdc(), 0, 0, rect.Width(), rect.Height(), SRCCOPY);//缩放
					m_picture.InvalidateRect(NULL);//调用重绘函数
					TRACE("更新图片完成%d %d\r\n", m_nObjWidth, m_nObjHeight);
					m_image.Destroy();
				break;
			}
			case 5:
				TRACE("远程端应答鼠标操作!\r\n");
				break;
			case 7:
			case 8:
			default:
				break;
			}
		}
	}
	 
	return 0;
}

void CWatchDialog::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	int x = GetSystemMetrics(SM_CXSCREEN);
	int y = GetSystemMetrics(SM_CYSCREEN);





	if (m_picture.GetSafeHwnd()) {
		


		CRect rect;
		GetClientRect(rect);
		rect.bottom = rect.right * y / x;
		//rect.bottom = rect.right * (x / y);
		m_picture.MoveWindow(rect);
		TRACE("left =%d top =%d right =%d bottom =%d\r\n", rect.left, rect.top, rect.right, rect.bottom);

		CRect temp;
		//ClientToScreen(temp);
		//temp.top -= ( 20);
		//temp.bottom = (temp.right) * 9 / 16;
		//temp.right = rect.bottom * GetSystemMetrics(SM_CXSCREEN) / GetSystemMetrics(SM_CYSCREEN);
		//temp.bottom = temp.top + rect.right * x / y;
		m_picture.ClientToScreen(temp);
		//temp.right = temp.left + rect.right;


		
		GetWindowRect(rect);
		CRect r1, r2;
		GetWindowRect(r1);
		m_picture.ClientToScreen(r2);
		temp.left += r1.left - r2.left;//不修改边框的初始宽度宽度
		temp.top += r1.top - r2.top;//
		temp.right += r1.right - r2.right;//
		temp.bottom += r1.bottom - r2.bottom;//
		temp.bottom = temp.right * y / x;
		temp.right = temp.bottom * x / y;
		MoveWindow(temp);
		TRACE("left =%d top =%d right =%d bottom =%d\r\n", temp.left, temp.top, temp.right, temp.bottom);
	}




	TRACE("cx = %d cy = %d\r\n", cx, cy);


}




void CWatchDialog::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if((m_nObjWidth != -1)&&(m_nObjHeight != -1)){
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;//左键
		event.nAction = 2;//双击
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(),5, true, (BYTE*)&event, sizeof(event));
	}
	CDialogEx::OnLButtonDblClk(nFlags, point);
}


void CWatchDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1)) {
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;//左键
		event.nAction = 2;//弹起
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));
	}
	CDialogEx::OnLButtonDown(nFlags, point);
}


void CWatchDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1)) {
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;
		event.nAction = 3;//弹起
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));
	}
	CDialogEx::OnLButtonUp(nFlags, point);
}


void CWatchDialog::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1)) {
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 1;
		event.nAction = 2;
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));
	}
	CDialogEx::OnRButtonDblClk(nFlags, point);
}


void CWatchDialog::OnRButtonDown(UINT nFlags, CPoint point)
{
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1)) {
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 1;
		event.nAction = 2;//按下 //TIDO:服务端要做对应的修改
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));
	}
	CDialogEx::OnRButtonDown(nFlags, point);
}


void CWatchDialog::OnRButtonUp(UINT nFlags, CPoint point)
{
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1)) {
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 1;
		event.nAction = 3;//弹起
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));
	}
	CDialogEx::OnRButtonUp(nFlags, point);
}


void CWatchDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1)) {
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 8;//没有按键
		event.nAction = 0;//移动
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));
		TRACE("x%d,y%d\r\n", point.x, point.y);
	}
	CDialogEx::OnMouseMove(nFlags, point);
}


void CWatchDialog::OnStnClickedWatch()
{
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1)) {
		CPoint point;
		GetCursorPos(&point);//获取屏幕坐标
		CPoint remote = UserPoint2RemoteScreenPoint(point, true);
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;//左键
		event.nAction = 0;//单击
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 6, true, (BYTE*)&event, sizeof(event));
	}
}


void CWatchDialog::OnDestroy()
{
	CDialogEx::OnDestroy();
//	((CRemoteClientDlg*)GetParent())->GetImage().Destroy();
	// TODO: 在此处添加消息处理程序代码
}


void CWatchDialog::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类

	//CDialogEx::OnOK();
}


void CWatchDialog::OnBnClickedBtnLock()
{

	CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 7);
}


void CWatchDialog::OnBnClickedBtnUnlock()
{
	CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 8);
}
