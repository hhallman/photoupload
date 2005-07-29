#pragma once


class CInPlaceList : public CComboBox
{
// Construction
public:
	CInPlaceList(int iItem, int iSubItem, CStringList *plstItems, int nSel);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInPlaceList)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CInPlaceList();

	// Generated message map functions
protected:
	//{{AFX_MSG(CInPlaceList)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnNcDestroy();
	afx_msg void OnCloseup();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
private:
	int 	m_iItem;
	int 	m_iSubItem;
	CStringList m_lstItems;
	int 	m_nSel;
	BOOL	m_bESC;				// To indicate whether ESC key was pressed
};
