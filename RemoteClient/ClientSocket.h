#pragma once
#include <list>
#include <assert.h>
#include "pch.h"
#include "framework.h"
#include <string>
#include <vector>
#include <map>
#include <mutex>
#define WM_SEND_PACK (WM_USER+1)//发送包数据
#define WM_SEND_PACK_ACK (WM_USER+2)//发送包数据应答

#pragma pack(push)
#pragma pack(1)
class CPacket//包解析
{
public:
	CPacket() :sHead(0), nLength(0), sCmd(0), sSum(0) {}
	CPacket(WORD nCmd, const BYTE* pData, size_t nSize) {//打包用的
		sHead = 0xFEFF;
		nLength = nSize + 2 + 2;
		sCmd = nCmd;
		if (nSize > 0) {
			strData.resize(nSize);
			memcpy((void*)strData.c_str(), pData, nSize);
		}
		else {
			strData.clear();
		}
		sSum = 0;
		for (size_t j = 0; j < strData.size(); j++)
		{
			sSum += BYTE(strData[j]) & 0xFF;
		}
	}
	CPacket(const CPacket& pack) {
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
	}
	CPacket(const BYTE* pData, size_t& nSize){//但凡包不完整的情况都把nSize置零
		//assert(!(nSize == BUFFER_SIZE));
		size_t i = 0;
		for (i = 0; i < nSize; i++) {//过滤掉无效的包
			if (*(WORD*)(pData + i) == 0xFEFF) {//如果一个包超过缓冲区大小,这东西搞不好会死循环
				sHead = *(WORD*)(pData + i);
				i += 2;
				break;
			}
		}
		if (i + 4 + 2 + 2 > nSize) {//包数据可能不全,或者包头未能全部接收到
			nSize = 0;
			return;
		}
		nLength = *(DWORD*)(pData + i); i += 4;
		if (nLength + i > nSize) {//包没接收全
			nSize = 0;
			return;
		}
		sCmd = *(WORD*)(pData + i); i += 2;
		if (nLength > 4) {//协议约定nLength包含2字节nCmd和2字节sSum
			strData.resize(nLength - 2 - 2);
			memcpy((void*)strData.c_str(), pData + i, nLength - 4);
			TRACE("%s\r\n", strData.c_str()+12);
			i += nLength - 4;
		}
		sSum = *(WORD*)(pData + i); i += 2;//使i指向下一个包
		WORD sum = 0;
		for (size_t j = 0; j < strData.size(); j++)
		{
			sum += BYTE(strData[j]) & 0xFF;
		}
		if (sum == sSum) {//一个包成功解析完成
			nSize = i;//head+length+sizeof(length)
			return;
		}
		nSize = 0;
	}
	~CPacket() {}
	CPacket& operator = (const CPacket& pack) {
		if (this != &pack) {
			sHead = pack.sHead;
			nLength = pack.nLength;
			sCmd = pack.sCmd;
			strData = pack.strData;
			sSum = pack.sSum;
		}
		return *this;
	}
	int Size() {//一个完整包的数据大小
		return nLength + 6;
	}
	const char* Data(std::string & strOut)const {
		strOut.resize(nLength + 6);//string strOut,专门用来组合数据的
		BYTE* pData = (BYTE*)strOut.c_str();
		*(WORD*)pData = sHead; pData += 2;
		*(DWORD*)pData = nLength; pData += 4;
		*(WORD*)pData = sCmd; pData += 2;
		memcpy(pData, strData.c_str(), strData.size()); pData += strData.size();
		*(WORD*)pData = sSum;

		return strOut.c_str();
	}

public:
	WORD sHead;//固定位FE FF
	DWORD nLength;//包长度(从控制命令开始,到和校验结束)
	WORD sCmd;//控制命令
	std::string strData;//包数据,不含其他成员
	WORD sSum;//和校验
	//std::string strOut;//整个包的数据
};
#pragma pack(pop)
//测试
typedef struct MouseEvent {
	MouseEvent() {
		nAction = 0;
		nButton = (WORD)-1;
		ptXY.x = 0;
		ptXY.y = 0;
	}
	WORD nAction;//行为:点击,移动,双击
	WORD nButton;//左键,右键.中建
	POINT ptXY;//坐标
}MOUSEEV, PMOUSEEV;


typedef struct file_info {
	file_info() {
		IsInvalid = FALSE;
		IsDirectory = -1;
		HasNext = TRUE;
		memset(szFileName, 0, sizeof(szFileName));
	}
	BOOL IsInvalid;//是否是无效文件
	BOOL IsDirectory;//是否是目录,1是
	BOOL HasNext;//是否还有后续0没有
	char szFileName[256];//文件名

}FILEINFO, * PFILEINFO;


enum {
	CSM_AUTOCLOSE = 1,//CSM = Clieent Socket Mode自动关闭模式

};


typedef struct PacketData{
	std::string strData;
	UINT nMode;
	WPARAM wParam;
	PacketData(const char* pData, size_t nLen, UINT Mode,WPARAM nParam = 0){
		strData.resize(nLen);
		memcpy((char*)strData.c_str(), pData, nLen);
		nMode = Mode;
		wParam = nParam;
	}
	PacketData(const PacketData& data) {
		strData = data.strData;
		nMode = data.nMode;
		wParam = data.wParam;
	}
	PacketData& operator = (PacketData& data) {
		if (this != &data)
		{
			strData = data.strData;
			nMode = data.nMode;
			wParam = data.wParam;
		}
		return *this; 
	}
}PACKET_DATA;

std::string GetErrInfo(int wsaErrCode);
void Dump(BYTE* pData, size_t nSize);

class CClientSocket
{
public:
	static CClientSocket* getInstance() {
		if (m_instance == NULL) {
			m_instance = new CClientSocket();
		}
		return m_instance;
	}
	bool InitSocket();


#define BUFFER_SIZE 20480000
	int DealCommand() {
		if (m_sock == -1) return -1;
		char* buffer = m_buffer.data();//TODO:多线程发送命令时可能会出现冲突
		static size_t index = 0;
		while (true) {
			if (index == BUFFER_SIZE) 	AfxMessageBox(_T("缓冲区不足"));
			size_t len = recv(m_sock, buffer + index, BUFFER_SIZE - index, 0);//这里出现了粘包
			if (((int)len <= 0) && ((int)index <= 0)) {
				return -1;
			}
			TRACE("recv len = %d(%08x) index = %d(%08x)\r\n", len, len, index, index);
			index += len;
			len = index;
			m_packet = CPacket((BYTE*)buffer, len);//获取到一个完整的包后构造函数才会修改len的值使之为一个完整包的字节大小,否则修改为0
			
			if (len > 0) {//if (len == m_packet.nLength+6) {//经过上一步的操作len的值只用两种情况,0或者m_packet.nLength+6
				memmove(buffer, buffer + len, BUFFER_SIZE - len);
				index -= len;
				return m_packet.sCmd;//发生粘包的情况返回后再次进入将导致缓冲区从0开始读取
			}
		}
		return -1;
	}
	
	
	//bool SendPacket(const CPacket& pack, std::list<CPacket>& lstPacks, bool isAutoClosed = true);
	bool SendPacket(HWND hWnd, const CPacket& pack, bool isAutoClosed = true,WPARAM wParam = 0);
	bool GetFilePath(std::string& strPath) {
		if ((m_packet.sCmd >= 2) && (m_packet.sCmd <= 4)) {
			strPath = m_packet.strData;
			return true;
		}
		return false;
	}
	bool GetMouseEvent(MOUSEEV& mouse) {
		if (m_packet.sCmd == 5) {
			memcpy(&mouse, m_packet.strData.c_str(), sizeof(MOUSEEV));
			return true;
		}
		return false;
	}
	CPacket& GetPacket() {
		return m_packet;
	}
	void CloseSocket() {
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
	}
	void UpdateAddress(int nIP, int nPort) {
		if ((m_nIP != nIP) || (m_nPort != nPort)) {
			m_nIP = nIP;
			m_nPort = nPort;
		}
	}
private:
	HANDLE m_eventInvoke;//启动事件
	UINT m_nThreadID;
	typedef void(CClientSocket::* MSGFUNC)(UINT, WPARAM, LPARAM);
	std::map<UINT, MSGFUNC> m_mapFunc;
	HANDLE m_hThread;
	bool m_bAutoClose;
	std::mutex m_lock;
	std::list<CPacket> m_lstSend;
	std::map<HANDLE, std::list<CPacket>&> m_mapAck;
	std::map<HANDLE, bool> m_mapAutoClosed;
	int m_nIP;//地址
	int m_nPort;//端口
	std::vector<char> m_buffer;
	SOCKET m_sock;
	CPacket m_packet;
	CClientSocket& operator=(const CClientSocket& ss) {}
	CClientSocket(const CClientSocket& ss);
	
	CClientSocket();
	~CClientSocket() {
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
		WSACleanup();
	}
	//static void threadEntry(void* arg);
	static unsigned _stdcall threadEntry(void* arg);
	//void threadFunc();
	void threadFunc2();
	BOOL InitSockEnv() {
		WSADATA da;
		if (WSAStartup(MAKEWORD(1, 1), &da) != 0) {
			return FALSE;
		}
		return TRUE;
	}
	static void releaseInstance() {
		if (m_instance != NULL) {
			CClientSocket* tmp = m_instance;
			m_instance = NULL;
			delete tmp;
		}
	}

	bool Send(const char* pData, size_t nSize) {
		if (m_sock == -1) return false;
		return send(m_sock, pData, nSize, 0) > 0;
	}
	bool Send(const CPacket& pack);
	void SendPack(UINT nMsg, WPARAM wParam/*缓冲区的值*/, LPARAM lParam/*缓冲区的长度*/);
	static CClientSocket* m_instance;
	class CHelper {
	public:
		CHelper() {
			CClientSocket::getInstance();
		}
		~CHelper() {
			CClientSocket::releaseInstance();
		}
	};
	static CHelper m_helper;
};
