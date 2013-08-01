// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "Control.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_STAUTSTIP,           // status line indicator
	ID_STAUTSSPEED,
	ID_STAUTSPORT,
	ID_STAUTSCOUNT
};


// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	m_iocpServer = NULL;
}

CMainFrame::~CMainFrame()
{
	if (m_iocpServer) {
		delete m_iocpServer;
		m_iocpServer = NULL;
	}
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	// create a view to occupy the client area of the frame
	if (!m_wndView.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,
		CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL))
	{
		TRACE0("Failed to create view window\n");
		return -1;
	}


	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	m_wndStatusBar.SetPaneInfo(0, m_wndStatusBar.GetItemID(0), SBPS_STRETCH, NULL);
	m_wndStatusBar.SetPaneInfo(1, m_wndStatusBar.GetItemID(1), SBPS_NORMAL, 160);
	m_wndStatusBar.SetPaneInfo(2, m_wndStatusBar.GetItemID(2), SBPS_NORMAL, 70);
	m_wndStatusBar.SetPaneInfo(3, m_wndStatusBar.GetItemID(3), SBPS_NORMAL, 80);

	m_wndStatusBar.SetPaneText(0, _T("端口: 110"));
		m_wndStatusBar.SetPaneText(1, _T("端口: 220"));
			m_wndStatusBar.SetPaneText(2, _T("端口: 0333"));
				m_wndStatusBar.SetPaneText(3, _T("端口: 4440"));


	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0);
	return TRUE;
}


// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG


// CMainFrame message handlers

void CMainFrame::OnSetFocus(CWnd* /*pOldWnd*/)
{
	// forward focus to the view window
	m_wndView.SetFocus();
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// let the view have first crack at the command
	if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// otherwise, do default handling
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CALLBACK CMainFrame::NotifyProc(LPVOID lpParam, ClientContext *pContext, UINT nCode)
{
	//try
	//{
	//	CMainFrame* pFrame = (CMainFrame*) lpParam;
	//	CString str;
	//	// 对g_pConnectView 进行初始化
	//	g_pConnectView = (CGh0stView *)((CGh0stApp *)AfxGetApp())->m_pConnectView;

	//	// g_pConnectView还没创建，这情况不会发生
	//	if (((CGh0stApp *)AfxGetApp())->m_pConnectView == NULL)
	//		return;

	//	g_pConnectView->m_iocpServer = m_iocpServer;

	//	str.Format("S: %.2f kb/s R: %.2f kb/s", (float)m_iocpServer->m_nSendKbps / 1024, (float)m_iocpServer->m_nRecvKbps / 1024);
	//	g_pFrame->m_wndStatusBar.SetPaneText(1, str);

	//	switch (nCode)
	//	{
	//	case NC_CLIENT_CONNECT:
	//		break;
	//	case NC_CLIENT_DISCONNECT:
	//		g_pConnectView->PostMessage(WM_REMOVEFROMLIST, 0, (LPARAM)pContext);
	//		break;
	//	case NC_TRANSMIT:
	//		break;
	//	case NC_RECEIVE:
	//		ProcessReceive(pContext);
	//		break;
	//	case NC_RECEIVE_COMPLETE:
	//		ProcessReceiveComplete(pContext);
	//		break;
	//	}
	//}catch(...){}
}

void CMainFrame::StartIocp(int nPort, int nMaxConnections)
{
	if (m_iocpServer != NULL) {
		m_iocpServer->Shutdown();
		delete m_iocpServer;
	}

	m_iocpServer = new CIOCPServer;

	CString strStatus;

	if (m_iocpServer->Initialize(NotifyProc, this, nMaxConnections, nPort)) {
		char hostname[256] = {0};
		gethostname(hostname, sizeof(hostname));
		HOSTENT *host = gethostbyname(hostname);
		if (host != NULL) {
			for (int i=0;; i++) {
				strStatus += inet_ntoa(*(IN_ADDR*)host->h_addr_list[i]);
				if ( host->h_addr_list[i] + host->h_length >= host->h_name )
					break;
				strStatus += "/";
			}
		}

		m_wndStatusBar.SetPaneText(0, strStatus);
		strStatus.Format(_T("端口: %d"), nPort);
		m_wndStatusBar.SetPaneText(2, strStatus);
	}
	else {
		strStatus.Format(_T("端口%d绑定失败"), nPort);
		m_wndStatusBar.SetPaneText(0, strStatus);
		m_wndStatusBar.SetPaneText(2, _T("端口: 0"));
	}

	m_wndStatusBar.SetPaneText(3, _T("连接: 0"));
}
