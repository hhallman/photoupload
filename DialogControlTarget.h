#pragma once
#include "afxwin.h"

class CDialogControlTarget :
	public CWnd
{
public:
	CDialogControlTarget(void);
	virtual ~CDialogControlTarget(void);

private:
	HWND cmdTarget;

public:
	static bool RegisterRuntimeClass();
	bool Create(HWND cmdTarget, const CString& instanceName);
	static CWnd* FindInstance(const CString& instanceName);

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);
};
