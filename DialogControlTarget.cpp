#include "StdAfx.h"
#include ".\dialogcontroltarget.h"

CDialogControlTarget::CDialogControlTarget(void)
{
	cmdTarget = 0;
}

CDialogControlTarget::~CDialogControlTarget(void)
{
}

const static TCHAR DIALOGCONTROLTARGET_CLASS_NAME[] = _T("MINAKORT_CONTROLTARGET_CLASS");
CWnd* CDialogControlTarget::FindInstance(const CString& instanceName)
{
	CWnd* w = CWnd::FindWindow(DIALOGCONTROLTARGET_CLASS_NAME, instanceName);
	return w;
}

bool CDialogControlTarget::RegisterRuntimeClass()
{
	WNDCLASS wndcls;
	ZeroMemory(&wndcls, sizeof wndcls);
	wndcls.lpfnWndProc		= ::DefWindowProc; 
	wndcls.hInstance		= AfxGetInstanceHandle();
	wndcls.lpszClassName	= DIALOGCONTROLTARGET_CLASS_NAME;
	if(!AfxRegisterClass(&wndcls))
	{
		TRACE("Class Registration Failed\n");
		return false;
	}
	return true;
}

bool CDialogControlTarget::Create(HWND cmdTarget, const CString& instanceName)
{
	ASSERT(cmdTarget);
	ASSERT(!FindInstance(instanceName));

	static bool wndClass = CDialogControlTarget::RegisterRuntimeClass();
	ASSERT(wndClass);

	if(!wndClass)
		return false;
	this->cmdTarget = cmdTarget;
	
	HWND parent = cmdTarget; //= HWND_MESSAGE on winnt5
	CWnd* pw = NULL;// CWnd::FromHandle(cmdTarget);
	return CWnd::CreateEx(0,DIALOGCONTROLTARGET_CLASS_NAME, instanceName, 0, CRect(0),  pw, 0)?true:false;
}

BEGIN_MESSAGE_MAP(CDialogControlTarget, CWnd)
	ON_WM_COPYDATA()
END_MESSAGE_MAP()

BOOL CDialogControlTarget::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct)
{
	if(!cmdTarget)
		return FALSE;

	return (BOOL)::SendMessage(cmdTarget, WM_COPYDATA, (WPARAM)pWnd->GetSafeHwnd(), reinterpret_cast<LPARAM>(pCopyDataStruct));
}
