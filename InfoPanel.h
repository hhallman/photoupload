#pragma once
#include "afxwin.h"
#include ".\prettybutton.h"

namespace Gdiplus { class Graphics; }

#define INFOCOMMAND_BUTTON1		(NM_FIRST-2)
#define INFOCOMMAND_BUTTON2		(NM_FIRST-3)

class CInfoPanel :
	public CWnd
{
public:
	CInfoPanel(void);
	~CInfoPanel(void);

	/////////////////////////////////////////////////////////////////////////////
	// Message handlers
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnNcLButtonUp(UINT nHitTest, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnButton1Clicked();
	afx_msg void OnButton2Clicked();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);

	/////////////////////////////////////////////////////////////////////////////
	// Private methods
private:
	void DoPaint(Gdiplus::Graphics* g);
	void ShowModal(CWnd* parent);
	static bool RegisterRuntimeClass();

	/////////////////////////////////////////////////////////////////////////////
	// Private members
private:
	CStatic staticIcon;
	CString title;
	CString text;
	bool immuneMouseDown;
	bool volatileMode;
	CPrettyButton button;

	/////////////////////////////////////////////////////////////////////////////
	// Public methods
public:
	enum DirectionMode { Top, Bottom };
	virtual BOOL Create(CWnd* pParentWnd, UINT nID=0);
	void Show(const CPoint& position,DirectionMode positionMode=DirectionMode::Bottom, bool volatileMode=true);
	void Hide();
	void SetText(const CString& title, const CString& text);
	void SetButtonText(const CString& button1Text, const CString& button2Text);
};
