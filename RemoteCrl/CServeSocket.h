#pragma once
#include "pch.h"
#include "framework.h"

//#pragma comment( linker, "/subsystem:windows /entry:WinMainCRTStartup" )
//#pragma comment( linker, "/subsystem:windows /entry:mainCRTStartup" )
//#pragma comment( linker, "/subsystem:console /entry:mainCRTStartup" )
//#pragma comment( linker, "/subsystem:console /entry:WinMainCRTStartup" )
class CPacket
{
public:
	CPacket() :sHead(0), nLength(0), sCmd(0), sSum(0) {}
	CPacket(const CPacket& pack)
	{
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
	}
	CPacket(const BYTE* pData, size_t& nSize) {

		size_t i = 0;
		for (; i < nSize; i++)
		{
			if (*(WORD*)(pData + i) == 0xFEFF)
			{
				sHead = *(WORD*)(pData + i);
				break;
			}
		}
		if (i + 4 + 2 + 2 > nSize)
		{
			nSize = 0;
			return;
		}
		nLength = *(DWORD*)(pData + i); i += 4;
		if (nLength + i > nSize)
		{
			nSize = 0;
			return;
		}

		sCmd = *(DWORD*)(pData + i); i += 2;
		if (nLength > 4)
		{
			strData.resize(nLength - 2 - 2);
			memcpy((void*)strData.c_str(), pData + i, nLength - 4);
			i += nLength - 4;
		}
		sSum = *(WORD*)(pData + i); i += 2;
		WORD sum = 0;
		for (size_t j = 0; j < strData.size(); j++)
		{
			sum += BYTE(strData[i]) & 0xFF;

		}
		if (sum == sSum)
		{
			nSize = i;
			return;
		}
		nSize = 0;
	}
	~CPacket() {}
	CPacket& operator =(const CPacket& pack)
	{
		if (this != &pack)
		{
			sHead = pack.sHead;
			nLength = pack.nLength;
			sCmd = pack.sCmd;
			strData = pack.strData;
			sSum = pack.sSum;
		}
		return *this;
	}
public:
	WORD sHead;
	DWORD nLength;
	WORD sCmd;
	WORD sSum;
	std::string strData;
};


class CServeSocket
{

public:
	static CServeSocket* getInsance() {
		if (m_instance == NULL)
		{
			m_instance = new CServeSocket();
		}
		return m_instance;

	}
	bool InitSocket()
	{
	//	SOCKET serv_sock = socket(PF_INET, SOCK_STREAM, 0);

		//TODO :校验
		if (m_Sock == -1)  return false;
		sockaddr_in serv_addr;//, client_adr;
		memset(&serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = INADDR_ANY;
		serv_addr.sin_port = htons(9527);
		//绑定
		if (bind(m_Sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		{
			return false;
		}
		if (listen(m_Sock, 1) == -1)
		{
			return false;
		}

		return true;
	}
	bool AccepClient()
	{
		sockaddr_in client_adr;
		//char buffer[1024];

		int cli_sz = sizeof(client_adr);
		SOCKET client = accept(m_Sock, (sockaddr*)&client_adr, &cli_sz);

		if (m_client == -1)return false;
		return true;
	}
#define BUFFER_SIZE 4096
	int DealCommand() 
	{
		if (m_client == -1)return -1;
		//char buffer[1024] = "";
		char* buffer = new char[BUFFER_SIZE];
		size_t index = 0;
		memset(buffer, 0, BUFFER_SIZE);
		while (true) 
		{
			size_t len = recv(m_client, buffer+ index, BUFFER_SIZE -index,0);
			if (len  <= 0)
			{
				return -1;
			}
			index += len;
			len = index;
			m_packet =  CPacket((BYTE*)buffer, len);
			if (len > 0)
			{
				memmove(buffer, buffer + len, BUFFER_SIZE -len);
				index -= len;
				return m_packet.sCmd;
			}
		}
		return -1;
	}
	bool Send(const char* pData, size_t nSize)
	{
		if (m_client == -1)return false;
		return send(m_client, pData, nSize, 0);
	}

private:
	SOCKET m_client;
	SOCKET m_Sock;
	CPacket m_packet;
	CServeSocket& operator=(const CServeSocket&& ss) 
	{
		m_Sock = ss.m_Sock;
		m_client = ss.m_client;
	};

	//CServeSocket(const CServeSocket&) {}

	CServeSocket() {
		m_client = INVALID_SOCKET;
		if (InitSockEnv() == FALSE)
		{
			MessageBox(NULL, _T("无法初始化套字节环境,请检查网络设置"), _T("初始化错误"), MB_OK | MB_ICONERROR);
			exit(0);
		}
		m_Sock = socket(PF_INET, SOCK_STREAM, 0);
	}
	~CServeSocket() {
		closesocket(m_Sock);
		WSACleanup();

	}
	BOOL InitSockEnv()
	{
		WSADATA data;
		if (WSAStartup(MAKEWORD(1, 1), &data) != 0)
		{
			return FALSE;
		}//TODO 返回值处理
		return TRUE;
		//SOCKET serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	}
	static void ReleaseInstance()
	{
		if (m_instance != NULL)
		{
			CServeSocket* Tmp = m_instance;
			m_instance = NULL;
			delete Tmp;
		}
	}

	static CServeSocket* m_instance;
	class CHelper
	{
	public:
		CHelper() {
			CServeSocket::getInsance();
		}
		~CHelper()
		{
			CServeSocket::ReleaseInstance();
		}
	};

	static CHelper m_helper;
};

//extern CServeSocket sevice;