#pragma once
#include "afxwin.h"
#include ".\defines.h"

class CInfoPanel;

class CProgressBar :
	public CWnd
{
public:
	CProgressBar(void);
	virtual ~CProgressBar(void);

private:
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

public:
	static void RegisterRuntimeClass()
	{
		WNDCLASS wndcls;
		memset(&wndcls, 0, sizeof(WNDCLASS));
		wndcls.style			= CS_HREDRAW | CS_VREDRAW;
		wndcls.lpfnWndProc		= ::DefWindowProc; 
		wndcls.hInstance		= AfxGetInstanceHandle();
		wndcls.hCursor			= ::LoadCursor(NULL, IDC_HAND );
		wndcls.lpszClassName	= _T("MINAKORT_PROGRESSBAR_CONTROL_CLASS");
		if(!AfxRegisterClass(&wndcls))
		{
			TRACE("Class Registration Failed\n");
		}
	}
public:
	void SetMaxValue(UINT maxValue);
	void SetCurrentValue(UINT currentValue);
	void SetCaption(const CString& caption);
	CString GetCaption() { return caption; }
	UINT GetMaxValue(UINT maxValue) { return maxValue; }
	UINT GetCurrentValue(UINT currentValue) { return currentValue; }

private:
	UINT maxValue;
	UINT currentValue;
	CString caption;
};
