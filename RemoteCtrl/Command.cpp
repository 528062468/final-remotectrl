#include "pch.h"
#include "Command.h"

CCommand::CCommand():threadid(0)
{
	struct {
		int nCmd;
		CMDFUNC func;
	}data[]{
		{1,&CCommand::MakeDriverInfo},
		{2,&CCommand::MakeDirectoryInfo},
		{3,&CCommand::RunFile},
		{4,&CCommand::DownloadFile},
		{5,&CCommand::MouseEvent},
		{6,&CCommand::SendScreen},
		{7,&CCommand::LockMachine},
		{8,&CCommand::UnlockMachine},
		{9,&CCommand::DeleteLocalFile},
		{1981,&CCommand::TestConnect},
		{-1,NULL}
	};
	for (int i = 0; data[i].nCmd != -1; i++) {
		//m_mapFunction[data[i].nCmd] = data[i].func;
		m_mapFunction.insert(std::pair<int, CMDFUNC>(data[i].nCmd,data[i].func));//¥¥Ω®√¸¡Ó”≥…‰±Ì
	}
}

int CCommand::ExcuteCommand(int nCmd,std::list<CPacket>& lstPacket,  CPacket& inPacket)
{
	std::map<int, CMDFUNC>::iterator it = m_mapFunction.find(nCmd);
	return it != m_mapFunction.end() ? (this->*it->second)(lstPacket,inPacket) : -1;//	return it != m_mapFunction.end() ? (this->*m_mapFunction[nCmd])() : -1;
}
