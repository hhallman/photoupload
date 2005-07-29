#include "StdAfx.h"
#include ".\launchuploaddlg.h"
#include ".\UploadDialog.h"
#include ".\UploadManager.h"

#include ".\UpdateManager.h"

CLaunchUploadDlg::CLaunchUploadDlg(void)
{
	theDlg		= NULL;
	quota		= 0;
	usedQuota	= -1;
	instanceName.Empty();
	uploadPath.Empty();
}

CLaunchUploadDlg::~CLaunchUploadDlg(void)
{
}

UINT CLaunchUploadDlg::LaunchThread(LPVOID threadArgs)
{
	StartupParameters* parameters = static_cast<StartupParameters*>(threadArgs);
	return CLaunchUploadDlg::Instance().Launch(parameters);
}

UINT CLaunchUploadDlg::Launch(StartupParameters* parameters)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

//	UpdateManager u;
//	u.StartUpdate();

	HWND parent = 0;
	try
	{
		parent = parameters->hParentWindow;

#ifndef DEBUG
	//	MessageBox(parent, _T("Detta är en betaversion.\r\nOm den slutar fungera kan det vara för att den är så gammal att servern inte känner igen den.\r\n\r\nDu får inte sprida programmet!\r\n\r\nKlagomål eller förslag tas mer än gärna emot."), _T("Minakort.com - betatest"), MB_ICONINFORMATION|MB_OK);
#endif

		USAGE("Launch");

		CUploadDialog ctrl(CWnd::FromHandle(parameters->hParentWindow));
		ctrl.instanceName = parameters->commandLine;
		if(parameters->doDelete)
			delete parameters;
		theDlg = &ctrl;

		try 
		{
			ctrl.DoModal();
		} 
		catch(...)
		{
			theDlg = NULL;
			UploadManager::Instance().WaitStop();
			throw;
		}

		theDlg = NULL;
		UploadManager::Instance().WaitStop();
	
		return 0;
	}
	catch(CException* exc)
	{
		exc->Delete();
	}
	catch(...)
	{
	}
	MessageBox(parent, R2T(IDS_ERROROCCURED), R2T(IDS_ERROROCCURED_CAPTION), MB_ICONERROR);
	return 1;
}

extern "C" {
	//Called by Rundll32
	__declspec( dllexport ) 
	void CALLBACK UploadMain (
	HWND hwnd,        // handle to owner window
	HINSTANCE hinst,  // instance handle for the DLL
	LPTSTR lpCmdLine, // string the DLL will parse
	int nCmdShow      // show state
	)
	{
		CLaunchUploadDlg::StartupParameters prm = { hwnd, false, lpCmdLine };
		CLaunchUploadDlg::Instance().Launch(&prm);
	}
}
#pragma comment(linker, "/EXPORT:UploadMain=_UploadMain@16")
#include "DialogControlTarget.h"

void CLaunchUploadDlg::LaunchDialog(bool launchWithSurrogate, HWND parent, UINT latestRelease, UINT requiredRelease)
{
	//BOOL COneT32App::FirstInstance()
	//{
	//   CWnd *pWndPrev, *pWndChild;

	//   // Determine if a window with the class name exists...
	//   if (pWndPrev = CWnd::FindWindow(_T("MyNewClass"),NULL))
	//   {
	//      // If so, does it have any popups?
	//      pWndChild = pWndPrev->GetLastActivePopup();

	//      // If iconic, restore the main window
	//      if (pWndPrev->IsIconic())
	//         pWndPrev->ShowWindow(SW_RESTORE);

	//      // Bring the main window or its popup to the foreground
	//      pWndChild->SetForegroundWindow();

	//      // and you are done activating the other application
	//      return FALSE;
	//   }
	//}
	//if(false && parent)
	//{
	//	CUploadDialog dlg(CWnd::FromHandle(parent));
	//	dlg.Create(IDD_UPLOAD_DLG, CWnd::FromHandle(parent));
	//	dlg.ShowWindow(SW_SHOW);
	//	//dlg.Create("STATIC", _T("Upload"), WS_CHILD|WS_VISIBLE, CRect(0,0,100,100), CWnd::FromHandle(parent), 0);
	//}
	//else
	

	if(launchWithSurrogate)
	{
		if(instanceName.GetLength() > 0) {
			if(CUploadDialog::StaticBringWindowToTop(instanceName))
				return;
		}

		instanceName.Format(_T("instance-%d"), ::GetTickCount());

		CString moduleName;
		::GetModuleFileName(AfxGetInstanceHandle(), moduleName.GetBuffer(MAX_PATH*5), MAX_PATH*5);
		moduleName.ReleaseBuffer();

		CString file;
	//	HMODULE module = ::GetModuleHandle(_T("MinakortControls.dll"));
	//	::GetModuleFileName(module, file.GetBuffer(MAX_PATH), MAX_PATH);
	//	file.ReleaseBuffer();
		file = moduleName;
		CString argument =  file + _T(" ")+_T("UploadMain")+_T(" ")+instanceName;
		::ShellExecute(0, _T("open"), _T("rundll32.exe"), argument, _T(""), SW_SHOW);
	}
	else
	{
		if(theDlg)
		{
			try {
				theDlg->BringWindowToTop();
			}
			catch(...) {
				PUTTRACE("theDlg was invalid.");
				theDlg = NULL;
			}
		}
		else
		{
			instanceName.Format(_T("instance-%d"), ::GetTickCount());
			StartupParameters* prm	= new StartupParameters;
			prm->hParentWindow		= parent;//::GetDesktopWindow();
			prm->doDelete			= true;
			prm->commandLine		= instanceName;
			CWinThread* thread = AfxBeginThread(LaunchThread, prm, 0,0,0,0);
		}
	}

	for(int i=0;i<4;i++)
		if(CUploadDialog::StaticSetQuota(instanceName, quota))
		{
			if(usedQuota != (UINT)-1)
				CUploadDialog::StaticSetUsedQuota(instanceName, usedQuota);
			if(uploadPath.GetLength()>0)
				CUploadDialog::StaticSetTargetCategory(instanceName, uploadPath);
			break;
		}
		else
			Sleep(200);
}

void CLaunchUploadDlg::SetUploadPath(const CString& path)
{
	this->uploadPath = path;
	if(instanceName.GetLength() == 0)
		return;

	for(int i=0;i<3;i++)
		if(CUploadDialog::StaticSetTargetCategory(instanceName, path))
			break;
		else
			Sleep(100);
}
void CLaunchUploadDlg::SetQuota(UINT quota)
{
	this->quota = quota;
	if(instanceName.GetLength() == 0)
		return;

	for(int i=0;i<3;i++)
		if(CUploadDialog::StaticSetQuota(instanceName, quota))
			break;
		else
			Sleep(100);
}
void CLaunchUploadDlg::SetUsedQuota(UINT usedQuota)
{
	this->usedQuota = usedQuota;
	if(instanceName.GetLength() == 0)
		return;

	for(int i=0;i<3;i++)
		if(CUploadDialog::StaticSetUsedQuota(instanceName, usedQuota))
			break;
		else
			Sleep(100);
}

