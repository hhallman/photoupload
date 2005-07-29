#pragma once

class CUploadDialog;

class CLaunchUploadDlg
{
private:
	CLaunchUploadDlg(void);
	~CLaunchUploadDlg(void);
	
	CUploadDialog* theDlg;
	static UINT LaunchThread(LPVOID threadArgs);
	CString uploadPath;
	CString instanceName;
	UINT quota;
	UINT usedQuota;

public:
	struct StartupParameters
	{
		HWND	hParentWindow;
		bool	doDelete;
		CString	commandLine;
	};


	UINT Launch(StartupParameters* parameters);
	void SetUploadPath(const CString& path);
	void SetQuota(UINT quota);
	void SetUsedQuota(UINT usedQuota);

public:
	void LaunchDialog(bool launchWithSurrogate, HWND wndParent, UINT latestRelease, UINT requiredRelease);
	static CLaunchUploadDlg& Instance()
	{
		static CLaunchUploadDlg theInstance;
		return theInstance;
	}
};
