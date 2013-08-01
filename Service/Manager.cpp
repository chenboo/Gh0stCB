#include "stdafx.h"
#include "Manager.h"
#include "until.h"

CManager::CManager(CClientSocket* pClient)
{
	m_pClient = pClient;
	m_pClient->setManagerCallBack(this);

	m_hEventDlgOpen = CreateEvent(NULL, true, false, NULL);
}

CManager::~CManager()
{
	CloseHandle(m_hEventDlgOpen);
}

void CManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
	
}

int CManager::Send(LPBYTE lpData, UINT nSize)
{
	return m_pClient->Send((LPBYTE)lpData, nSize);
}

void CManager::WaitForDialogOpen()
{
	WaitForSingleObject(m_hEventDlgOpen, INFINITE);

	// �����Sleep,��ΪԶ�̴��ڴ�InitDialog�з���COMMAND_NEXT����ʾ��Ҫһ��ʱ��
	Sleep(150);
}

void CManager::NotifyDialogIsOpen()
{
	SetEvent(m_hEventDlgOpen);
}