// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "Control.h"
#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChildView

CChildView::CChildView()
{
}

CChildView::~CChildView()
{
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_SIZE()

	ON_MESSAGE(WM_ADDTOLIST, OnAddToList)
	ON_MESSAGE(WM_REMOVEFROMLIST, OnRemoveFromList)
END_MESSAGE_MAP()



// CChildView message handlers

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);

	return TRUE;
}

void CChildView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	
	// Do not call CWnd::OnPaint() for painting messages
}

int AddItem(CListCtrl* lcList, int nItem)
{
	int nRet = lcList->InsertItem(nItem, _T(""));
	CString strNum;
	strNum.Format("%d.", nItem);
	lcList->SetItemText(nItem, 0, strNum);  
	lcList->SetItemText(nItem, 1, _T("郝帅"));  
	lcList->SetItemText(nItem, 2, _T("192.168.0.123"));  
	lcList->SetItemText(nItem, 3, _T("江苏南京"));
	lcList->SetItemText(nItem, 4, _T("WIN7 SP1"));
	lcList->SetItemText(nItem, 5, _T("10:20:10 2013/08/12"));

	return nRet;
}

void CChildView::PeopleConnected()
{
	int nItem = 0;
	do {
		nItem = AddItem(&m_lcListPeople, nItem);
		nItem++;
	} while (nItem < 100);
	
	//m_lcListPeople.EnsureVisible(99, TRUE);
}

BOOL CChildView::CreateChildCtrl()
{
	m_lcListPeople.Create(WS_VISIBLE | WS_CHILD | LVS_REPORT, CRect(0,0,0,0), this, ID_LISTPEOPLE);

	m_lcListPeople.SetExtendedStyle(m_lcListPeople.GetExtendedStyle()|LVS_EX_FULLROWSELECT);
	m_lcListPeople.SetExtendedStyle(m_lcListPeople.GetExtendedStyle()|LVS_EX_GRIDLINES);

	m_lcListPeople.InsertColumn(0, _T(""), LVCFMT_RIGHT, 30);
	m_lcListPeople.InsertColumn(1, _T("机器名称"), LVCFMT_CENTER, 100);
	m_lcListPeople.InsertColumn(2, _T("ip地址"), LVCFMT_CENTER, 200);
	m_lcListPeople.InsertColumn(3, _T("所处位置"), LVCFMT_CENTER, 300);
	m_lcListPeople.InsertColumn(4, _T("操作系统"), LVCFMT_CENTER, 300);
	m_lcListPeople.InsertColumn(5, _T("上线时间"), LVCFMT_CENTER, 300);

	PeopleConnected();

	return true;
}

void CChildView::Relayout()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	m_lcListPeople.MoveWindow(rcClient);
}

int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!CreateChildCtrl()) {
		return FALSE;
	}


	return 0;
}

void CChildView::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	if (cx <= 0|| cy <= 0)	
	{
		return;
	}

	Relayout();
}

LRESULT CChildView::OnAddToList(WPARAM wParam, LPARAM lParam)
{
	//ClientContext	*pContext = (ClientContext *)lParam;

	//if (pContext == NULL)
	//	return -1;

	//CString	strToolTipsText, strOS;
	//try
	//{
	//	int nCnt = m_pListCtrl->GetItemCount();

	//	// 不合法的数据包
	//	if (pContext->m_DeCompressionBuffer.GetBufferLen() != sizeof(LOGININFO))
	//		return -1;

	//	LOGININFO*	LoginInfo = (LOGININFO*)pContext->m_DeCompressionBuffer.GetBuffer();

	//	// ID
	//	CString	str;
	//	str.Format("%d", m_nCount++);	

	//	// IP地址
	//	int i = m_pListCtrl->InsertItem(nCnt, str, 15);

	//	// 外网IP

	//	sockaddr_in  sockAddr;
	//	memset(&sockAddr, 0, sizeof(sockAddr));
	//	int nSockAddrLen = sizeof(sockAddr);
	//	BOOL bResult = getpeername(pContext->m_Socket,(SOCKADDR*)&sockAddr, &nSockAddrLen);
	//	CString IPAddress = bResult != INVALID_SOCKET ? inet_ntoa(sockAddr.sin_addr) : "";
	//	m_pListCtrl->SetItemText(i, 1, IPAddress);

	//	// 内网IP
	//	m_pListCtrl->SetItemText(i, 2, inet_ntoa(LoginInfo->IPAddress));

	//	// 主机名
	//	m_pListCtrl->SetItemText(i, 3, LoginInfo->HostName);

	//	// 系统

	//	////////////////////////////////////////////////////////////////////////////////////////
	//	// 显示输出信息
	//	char *pszOS = NULL;
	//	switch (LoginInfo->OsVerInfoEx.dwPlatformId)
	//	{

	//	case VER_PLATFORM_WIN32_NT:
	//		if (LoginInfo->OsVerInfoEx.dwMajorVersion <= 4 )
	//			pszOS = "NT";
	//		if ( LoginInfo->OsVerInfoEx.dwMajorVersion == 5 && LoginInfo->OsVerInfoEx.dwMinorVersion == 0 )
	//			pszOS = "2000";
	//		if ( LoginInfo->OsVerInfoEx.dwMajorVersion == 5 && LoginInfo->OsVerInfoEx.dwMinorVersion == 1 )
	//			pszOS = "XP";
	//		if ( LoginInfo->OsVerInfoEx.dwMajorVersion == 5 && LoginInfo->OsVerInfoEx.dwMinorVersion == 2 )
	//			pszOS = "2003";
	//		if ( LoginInfo->OsVerInfoEx.dwMajorVersion == 6 && LoginInfo->OsVerInfoEx.dwMinorVersion == 0 )
	//			pszOS = "Vista";  // Just Joking
	//	}
	//	strOS.Format
	//		(
	//		"%s SP%d (Build %d)",
	//		//OsVerInfo.szCSDVersion,
	//		pszOS, 
	//		LoginInfo->OsVerInfoEx.wServicePackMajor, 
	//		LoginInfo->OsVerInfoEx.dwBuildNumber
	//		);
	//	m_pListCtrl->SetItemText(i, 4, strOS);

	//	// CPU
	//	str.Format("%dMHz", LoginInfo->CPUClockMhz);
	//	m_pListCtrl->SetItemText(i, 5, str);

	//	// Speed
	//	str.Format("%d", LoginInfo->dwSpeed);
	//	m_pListCtrl->SetItemText(i, 6, str);


	//	str = LoginInfo->bIsWebCam ? "有" : "--";
	//	m_pListCtrl->SetItemText(i, 7, str);

	//	strToolTipsText.Format("New Connection Information:\nHost: %s\nIP  : %s\nOS  : Windows %s", LoginInfo->HostName, IPAddress, strOS);

	//	if (((CGh0stApp *)AfxGetApp())->m_bIsQQwryExist)
	//	{

	//		str = m_QQwry->IPtoAdd(IPAddress);
	//		m_pListCtrl->SetItemText(i, 8, str);

	//		strToolTipsText += "\nArea: ";
	//		strToolTipsText += str;
	//	}
	//	// 指定唯一标识
	//	m_pListCtrl->SetItemData(i, (DWORD) pContext);

	//	g_pFrame->ShowConnectionsNumber();

	//	if (!((CGh0stApp *)AfxGetApp())->m_bIsDisablePopTips)
	//		g_pFrame->ShowToolTips(strToolTipsText);
	//}catch(...){}

	return 0;
}

LRESULT CChildView::OnRemoveFromList(WPARAM wParam, LPARAM lParam)
{
	//ClientContext	*pContext = (ClientContext *)lParam;
	//if (pContext == NULL)
	//	return -1;
	//// 删除链表过程中可能会删除Context
	//try
	//{
	//	int nCnt = m_pListCtrl->GetItemCount();
	//	for (int i=0; i < nCnt; i++)
	//	{
	//		if (pContext == (ClientContext *)m_pListCtrl->GetItemData(i))
	//		{
	//			m_pListCtrl->DeleteItem(i);
	//			break;
	//		}		
	//	}

	//	// 关闭相关窗口

	//	switch (pContext->m_Dialog[0])
	//	{
	//	case FILEMANAGER_DLG:
	//	case SCREENSPY_DLG:
	//	case WEBCAM_DLG:
	//	case AUDIO_DLG:
	//	case KEYBOARD_DLG:
	//	case SYSTEM_DLG:
	//	case SHELL_DLG:
	//		//((CDialog*)pContext->m_Dialog[1])->SendMessage(WM_CLOSE);
	//		((CDialog*)pContext->m_Dialog[1])->DestroyWindow();
	//		break;
	//	default:
	//		break;
	//	}
	//}catch(...){}

	//// 更新当前连接总数
	//g_pFrame->ShowConnectionsNumber();
	return 0;
}

