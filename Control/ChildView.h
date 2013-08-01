// ChildView.h : interface of the CChildView class
//


#pragma once


// CChildView window

class CChildView : public CWnd
{
// Construction
public:
	CChildView();
	virtual ~CChildView();

private:
	BOOL CreateChildCtrl();
	void Relayout();

	void PeopleConnected();

private:
	CListCtrl m_lcListPeople;

protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);


	// Generated message map functions
protected:
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	afx_msg LRESULT OnRemoveFromList(WPARAM, LPARAM);
	afx_msg LRESULT OnAddToList(WPARAM, LPARAM);
};

