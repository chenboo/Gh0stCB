// Control.h : main header file for the Control application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CControlApp:
// See Control.cpp for the implementation of this class
//

class CControlApp : public CWinApp
{
public:
	CControlApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

public:
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CControlApp theApp;