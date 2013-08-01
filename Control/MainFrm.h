// MainFrm.h : interface of the CMainFrame class
//


#pragma once

#include "ChildView.h"
#include "iocp/IOCPServer.h"

class CMainFrame : public CFrameWnd
{
	
public:
	CMainFrame();
protected: 
	DECLARE_DYNAMIC(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	void StartIocp(int nPort, int nMaxConnection);

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CChildView    m_wndView;

	CIOCPServer *m_iocpServer;

private:
	void ProcessConnect(ClientContext *pContext);
	void ProcessDisconnect(ClientContext *pContext);
	void ProcessReceive(ClientContext *pContext);
	void ProcessReceiveComplete(ClientContext *pContext);

	static void CALLBACK NotifyProc(LPVOID lpParam, ClientContext* pContext, UINT nCode);

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	DECLARE_MESSAGE_MAP()
};


