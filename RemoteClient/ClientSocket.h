#pragma once
#include <list>
#include <assert.h>
#include "pch.h"
#include "framework.h"
#include <string>
#include <vector>
#include <map>
#include <mutex>
#define WM_SEND_PACK (WM_USER+1)//���Ͱ�����
#define WM_SEND_PACK_ACK (WM_USER+2)//���Ͱ�����Ӧ��

#pragma pack(push)
#pragma pack(1)
class CPacket//������
{
public:
	CPacket() :sHead(0), nLength(0), sCmd(0), sSum(0) {}
	CPacket(WORD nCmd, const BYTE* pData, size_t nSize) {//����õ�
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
	CPacket(const BYTE* pData, size_t& nSize){//���������������������nSize����
		//assert(!(nSize == BUFFER_SIZE));
		size_t i = 0;
		for (i = 0; i < nSize; i++) {//���˵���Ч�İ�
			if (*(WORD*)(pData + i) == 0xFEFF) {//���һ����������������С,�ⶫ���㲻�û���ѭ��
				sHead = *(WORD*)(pData + i);
				i += 2;
				break;
			}
		}
		if (i + 4 + 2 + 2 > nSize) {//�����ݿ��ܲ�ȫ,���߰�ͷδ��ȫ�����յ�
			nSize = 0;
			return;
		}
		nLength = *(DWORD*)(pData + i); i += 4;
		if (nLength + i > nSize) {//��û����ȫ
			nSize = 0;
			return;
		}
		sCmd = *(WORD*)(pData + i); i += 2;
		if (nLength > 4) {//Э��Լ��nLength����2�ֽ�nCmd��2�ֽ�sSum
			strData.resize(nLength - 2 - 2);
			memcpy((void*)strData.c_str(), pData + i, nLength - 4);
			TRACE("%s\r\n", strData.c_str()+12);
			i += nLength - 4;
		}
		sSum = *(WORD*)(pData + i); i += 2;//ʹiָ����һ����
		WORD sum = 0;
		for (size_t j = 0; j < strData.size(); j++)
		{
			sum += BYTE(strData[j]) & 0xFF;
		}
		if (sum == sSum) {//һ�����ɹ��������
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
	int Size() {//һ�������������ݴ�С
		return nLength + 6;
	}
	const char* Data(std::string & strOut)const {
		strOut.resize(nLength + 6);//string strOut,ר������������ݵ�
		BYTE* pData = (BYTE*)strOut.c_str();
		*(WORD*)pData = sHead; pData += 2;
		*(DWORD*)pData = nLength; pData += 4;
		*(WORD*)pData = sCmd; pData += 2;
		memcpy(pData, strData.c_str(), strData.size()); pData += strData.size();
		*(WORD*)pData = sSum;

		return strOut.c_str();
	}

public:
	WORD sHead;//�̶�λFE FF
	DWORD nLength;//������(�ӿ������ʼ,����У�����)
	WORD sCmd;//��������
	std::string strData;//������,����������Ա
	WORD sSum;//��У��
	//std::string strOut;//������������
};
#pragma pack(pop)
//����
typedef struct MouseEvent {
	MouseEvent() {
		nAction = 0;
		nButton = (WORD)-1;
		ptXY.x = 0;
		ptXY.y = 0;
	}
	WORD nAction;//��Ϊ:���,�ƶ�,˫��
	WORD nButton;//���,�Ҽ�.�н�
	POINT ptXY;//����
}MOUSEEV, PMOUSEEV;


typedef struct file_info {
	file_info() {
		IsInvalid = FALSE;
		IsDirectory = -1;
		HasNext = TRUE;
		memset(szFileName, 0, sizeof(szFileName));
	}
	BOOL IsInvalid;//�Ƿ�����Ч�ļ�
	BOOL IsDirectory;//�Ƿ���Ŀ¼,1��
	BOOL HasNext;//�Ƿ��к���0û��
	char szFileName[256];//�ļ���

}FILEINFO, * PFILEINFO;


enum {
	CSM_AUTOCLOSE = 1,//CSM = Clieent Socket Mode�Զ��ر�ģʽ

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
		char* buffer = m_buffer.data();//TODO:���̷߳�������ʱ���ܻ���ֳ�ͻ
		static size_t index = 0;
		while (true) {
			if (index == BUFFER_SIZE) 	AfxMessageBox(_T("����������"));
			size_t len = recv(m_sock, buffer + index, BUFFER_SIZE - index, 0);//���������ճ��
			if (((int)len <= 0) && ((int)index <= 0)) {
				return -1;
			}
			TRACE("recv len = %d(%08x) index = %d(%08x)\r\n", len, len, index, index);
			index += len;
			len = index;
			m_packet = CPacket((BYTE*)buffer, len);//��ȡ��һ�������İ����캯���Ż��޸�len��ֵʹ֮Ϊһ�����������ֽڴ�С,�����޸�Ϊ0
			
			if (len > 0) {//if (len == m_packet.nLength+6) {//������һ���Ĳ���len��ֵֻ���������,0����m_packet.nLength+6
				memmove(buffer, buffer + len, BUFFER_SIZE - len);
				index -= len;
				return m_packet.sCmd;//����ճ����������غ��ٴν��뽫���»�������0��ʼ��ȡ
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
	HANDLE m_eventInvoke;//�����¼�
	UINT m_nThreadID;
	typedef void(CClientSocket::* MSGFUNC)(UINT, WPARAM, LPARAM);
	std::map<UINT, MSGFUNC> m_mapFunc;
	HANDLE m_hThread;
	bool m_bAutoClose;
	std::mutex m_lock;
	std::list<CPacket> m_lstSend;
	std::map<HANDLE, std::list<CPacket>&> m_mapAck;
	std::map<HANDLE, bool> m_mapAutoClosed;
	int m_nIP;//��ַ
	int m_nPort;//�˿�
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
	void SendPack(UINT nMsg, WPARAM wParam/*��������ֵ*/, LPARAM lParam/*�������ĳ���*/);
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
