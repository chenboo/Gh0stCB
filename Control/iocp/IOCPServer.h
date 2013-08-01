#pragma once

#pragma warning(disable:4311)
#pragma warning(disable:4018)

#include <winsock2.h>
#include <MSTcpIP.h>
#pragma comment(lib,"ws2_32.lib")

#include "Buffer.h"
#include "CpuUsage.h"

#include <process.h>

#define	NC_CLIENT_CONNECT		0x0001
#define	NC_CLIENT_DISCONNECT	0x0002
#define	NC_TRANSMIT				0x0003
#define	NC_RECEIVE				0x0004
#define NC_RECEIVE_COMPLETE		0x0005 // 完整接收

class CLock
{
public:
	CLock(CRITICAL_SECTION& cs, const CString& strFunc)
	{
		m_pcs = &cs;
		m_strFunc = strFunc;

		TRACE(_T("EC %d %s\n") , GetCurrentThreadId(), m_strFunc);
		EnterCriticalSection(m_pcs);
	}

	~CLock()
	{
		LeaveCriticalSection(m_pcs);
		TRACE(_T("LC %d %s\n") , GetCurrentThreadId() , m_strFunc);
	}

protected:
	CString m_strFunc;
	CRITICAL_SECTION* m_pcs;
};

enum IOType 
{
	IOInitialize,
	IORead,
	IOWrite,
	IOIdle
};

class OVERLAPPEDPLUS
{
public:
	OVERLAPPED m_ol;
	IOType	m_ioType;

	OVERLAPPEDPLUS(IOType ioType)
	{
		ZeroMemory(this, sizeof(OVERLAPPEDPLUS));
		m_ioType = ioType;
	}
};

struct ClientContext
{
    SOCKET	m_Socket;

	CBuffer	m_WriteBuf;
	CBuffer	m_RecvCompressBuf;	// 接收到的压缩的数据
	CBuffer	m_DeCompressBuf;	// 解压后的数据
	CBuffer	m_ResendWriteBuf;	// 上次发送的数据包，接收失败时重发时用

	int	m_Dialog[2]; // 放对话框列表用，第一个int是类型，第二个是CDialog的地址
	int	m_nTransferProgress;

	WSABUF m_wsaInBuffer;
	WSABUF m_wsaOutBuffer;

	BYTE m_byInBuffer[1024 * 8];

	HANDLE m_hWriteComplete;

	LONG m_nMsgIn;
	LONG m_nMsgOut;

	BOOL m_bIsMainSocket; // 是不是主socket

	ClientContext* m_pWriteContext;
	ClientContext* m_pReadContext;
};

typedef void (CALLBACK* NOTIFYPROC)(LPVOID, ClientContext*, UINT nCode);

typedef CList<ClientContext*, ClientContext*> ContextList;

#include "Mapper.h"

class CMainFrame;
class CIOCPServer
{
public:
	CIOCPServer();
	virtual ~CIOCPServer();

	bool Initialize(NOTIFYPROC pNotifyProc, CMainFrame* pFrame, int nMaxConnections, int nPort);

	void Send(ClientContext* pContext, LPBYTE lpData, UINT nSize);
	void PostRecv(ClientContext* pContext);

	bool IsRunning();
	void Shutdown();
	void DisconnectAll();
	void ResetConnection(ClientContext* pContext);

	static unsigned WINAPI ListenThreadProc(LPVOID lpVoid);
	static unsigned WINAPI ThreadPoolFunc(LPVOID WorkContext);

protected:
	ClientContext*  AllocateContext();

	void InitializeClientRead(ClientContext* pContext);
	BOOL AssociateSocketWithCompletionPort(SOCKET device, HANDLE hCompletionPort, DWORD dwCompletionKey);
	void RemoveStaleClient(ClientContext* pContext, BOOL bGraceful);
	void MoveToFreePool(ClientContext *pContext);

	void CloseCompletionPort();
	void OnAccept();
	bool InitializeIOCP(void);
	void Stop();

	CString GetHostName(SOCKET socket);
	void CreateStream(ClientContext* pContext);

	BEGIN_IO_MSG_MAP()
		IO_MESSAGE_HANDLER(IORead, OnClientReading)
		IO_MESSAGE_HANDLER(IOWrite, OnClientWriting)
		IO_MESSAGE_HANDLER(IOInitialize, OnClientInitializing)
	END_IO_MSG_MAP()

	bool OnClientInitializing(ClientContext* pContext, DWORD dwSize = 0);
	bool OnClientReading(ClientContext* pContext, DWORD dwSize = 0);
	bool OnClientWriting(ClientContext* pContext, DWORD dwSize = 0);

public:
	static CRITICAL_SECTION	m_cs;

	NOTIFYPROC m_pNotifyProc;
	CMainFrame *m_pFrame;

	LONG m_nCurrentThreads;
	LONG m_nBusyThreads;

	UINT m_nSendKbps; // 发送即时速度
	UINT m_nRecvKbps; // 接受即时速度
	UINT m_nMaxConnections; // 最大连接数

private:
	LONG m_nWorkerCnt;

	bool m_bInit;
	bool m_bDisconnectAll;
	BYTE m_bPacketFlag[5];

	ContextList	m_listContexts;
	ContextList	m_listFreePool;

	WSAEVENT m_hEvent;
	SOCKET m_socketListen;
	HANDLE m_hKillEvent;
	HANDLE m_hThread;
	HANDLE m_hCompletionPort;
	bool m_bTimeToKill;
	CCpuUsage m_cpu;

	LONG m_nKeepLiveTime; // 心跳超时

	LONG m_nThreadPoolMin;
	LONG m_nThreadPoolMax;
	LONG m_nCPULoThreshold;
	LONG m_nCPUHiThreshold;
};
