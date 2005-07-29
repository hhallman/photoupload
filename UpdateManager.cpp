#include "StdAfx.h"
#include ".\updatemanager.h"
//#include ".\MinakortWebUpload.h"
#include ".\Resource.h"
#include ".\UpdateDialog.h"
#include ".\defines.h"
#include ".\Utility.h"
#include ".\HttpStream.h"

#pragma comment(lib, "version.lib")

const int FILE_ERROR			= 2;
const int OLD_CLIENT			= 3;
const int BINARY_ERROR			= 4;
const int REGISTRATION_ERROR	= 5;

/*

Automatic update should not be performed on a url supplied over an internet connection.
That would make it easier to redirect the download to a malicious site.

*/

extern int ReleaseVersion;

static const LPTSTR	UpdatePath			= _T("public/minakortcontrolsupdate.aspx");
static const UINT	UpdateServerPort	= 80;
static const UINT	UpdateManagerVersion= 1;
static const LPTSTR UPDATEMANAGER_AGENT	= _T("MinakortControlsUpdateManager");


UpdateManager::UpdateManager(void)
{
}

UpdateManager::~UpdateManager(void)
{
}


const CString UpdateManager::GetVersionsQueryString() 
{
	CString	q, t;
	BOOL	b;

	q.AppendFormat(_T("updateManagerVersion=%u&controlReleaseVersion=%u"), UpdateManagerVersion, ReleaseVersion);

	SYSTEM_INFO system;
	//TODO: Should use GetNativeSystemInfo, but that isn't available before Windows XP.
	::GetSystemInfo(&system);
	q.AppendFormat(_T("&processorArchitecture=%u"), (unsigned)system.wProcessorArchitecture);

	OSVERSIONINFO version;
	ZeroMemory(&version, sizeof(version));
	version.dwOSVersionInfoSize = sizeof(version);
	b = ::GetVersionEx(&version);
	if(version.dwPlatformId == VER_PLATFORM_WIN32_NT)
		t = _T("nt");
	else if(version.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
		t = _T("95");
	else t = _T("unknown");
	q += _T("&platform=");
	q += t;

	q.AppendFormat(_T("&winmajor=%u&winminor=%u&winCsd="), version.dwMajorVersion, version.dwMinorVersion);
	q.Append(EscapeUrl(version.szCSDVersion));

	q += _T("&gdiplusver=");
	t = _T("0.0");
	DWORD size = ::GetFileVersionInfoSize(_T("gdiplus.dll"), NULL);
	while(size != 0)
	{
		LPVOID gdiver = new byte[size];
		b = ::GetFileVersionInfo(_T("gdiplus.dll"), 0, size, gdiver);
		if(!b) break;
		VS_FIXEDFILEINFO* vsinfo;unsigned len;
		b = ::VerQueryValue(gdiver, _T("\\"), (void**)&vsinfo, &len);
		if(!b) break;
		t.Format(_T("%u.%u"), vsinfo->dwFileVersionMS, vsinfo->dwFileVersionLS);
		delete[] gdiver;
		break;
	}
	q += t;

	q += _T("&admin=");
	q += Utility::IsUserAdmin()?_T("True"):_T("False");

	DWORD mfc71 = ::GetFileVersionInfoSize(_T("mfc71.dll"), NULL);
	q += mfc71?_T("&mfc71=True"):_T("&mfc71=False");

    return q;
}

HANDLE CreateTempFile(CString& file)
{
	TCHAR t;
	DWORD len		= ::GetTempPath(0, &t);
	CString path;
	path.ReleaseBuffer(::GetTempPath(len, path.GetBuffer(len)));
	::GetTempFileName(path, _T("update"), 0, file.GetBuffer(MAX_PATH));
	file.ReleaseBuffer();

	DWORD flags = 0;//FILE_ATTRIBUTE_TEMPORARY|FILE_FLAG_DELETE_ON_CLOSE;
	HANDLE handle = ::CreateFile(file, GENERIC_READ|GENERIC_WRITE,0,NULL,CREATE_ALWAYS,flags,0);
	return handle;
}


CString GetBinaryPath(bool adminInstall)
{
	CString path;
	if(adminInstall)
	{
		HRESULT hr = SHGetFolderPath(NULL, 
						CSIDL_PROGRAM_FILES, 
						NULL, 
						0, 
						path.GetBuffer(MAX_PATH));
		path.ReleaseBuffer();
		path += _T("\\");

		if(SUCCEEDED(hr))
			path += _T("Minakort\\");
		
		else
			throw FILE_ERROR;
	}
	else
	{
		HRESULT hr = SHGetFolderPath(NULL, 
						CSIDL_LOCAL_APPDATA, 
						NULL, 
						0, 
						path.GetBuffer(MAX_PATH));
		path.ReleaseBuffer();
		path += _T("\\");

		if(SUCCEEDED(hr))
		{
			path += _T("Minakort");
			if(!::PathFileExists(path) && !::CreateDirectory(path, NULL))
				throw FILE_ERROR;
			path += _T("\\Binaries\\");
		
		}
		else
			throw FILE_ERROR;
	}

	if(!::PathFileExists(path) && !::CreateDirectory(path, NULL))
		throw FILE_ERROR;

    return path;
}

//__declspec(dllexport) HRESULT GetClsid(LPCWSTR className, GUID* guid)
//{
//	if(guid)
//		memset(guid, sizeof(GUID), 0x00);
//
//	if(!guid || !className)
//		return E_INVALIDARG;
//
//	if(wcscmp(className, L"CMinakortWebUpload") == 0)
//	{
//		*guid = __uuidof(CMinakortWebUpload);
//		return S_OK;
//	}
//	return E_FAIL;
//}

typedef HRESULT (*comvoid)();
typedef HRESULT (*regfunc)(int oldVersion, int oldUpdateManagerVersion, bool administratorInstall);
//typedef HRESULT (*getGuidFunc)(LPCWSTR className, GUID* guid);



struct UpdateProcPrm
{
	UpdateManager*	mgr;
	UpdateDialog*	dlg;
	CEvent*			finishedEvent;
	HRESULT			result;
//	GUID			uploadClsid;
};

HRESULT UpdateManager::StartUpdate(/*GUID *newClsID*/)
{
	UpdateDialog		dlg;
	CEvent				finishedEvent;
	UpdateProcPrm		prm;
	prm.dlg				= &dlg;
	prm.finishedEvent	= &finishedEvent;
	prm.mgr				= this;
	prm.result			= E_FAIL;

	CWinThread* thread	= ::AfxBeginThread(UpdateProc, &prm, THREAD_PRIORITY_BELOW_NORMAL, 0, 0, 0);
	dlg.DoModal();
	finishedEvent.Lock();

	//if(newClsID)
	//	memcpy(newClsID, &prm.uploadClsid, sizeof(GUID));

	return prm.result;
}

UINT UpdateManager::UpdateProc(LPVOID threadParameter)
{
	UpdateProcPrm*		prm				= static_cast<UpdateProcPrm*>(threadParameter);
	HANDLE				tempFile		= 0;
	CString				tempFileName	= _T("");
	HMODULE				newmod			= 0;
	BOOL				success			= FALSE;
	HRESULT				error			= S_OK;
	CString				errorMessage	= _T("");

	try
	{
		bool administratorInstall = Utility::IsUserAdmin()?true:false;

		CString queryString = CString(APP_DIR) + UpdatePath;
		queryString += CString(_T("?")) + GetVersionsQueryString();

		CInternetSession session(UPDATEMANAGER_AGENT);
		CHttpConnection* connection = session.GetHttpConnection(SERVER_ADDRESS, 0, UpdateServerPort, 0, 0);
		HttpStream s(connection, queryString);
		s.StartReading();
		
		tempFile = CreateTempFile(tempFileName);
		if(!tempFile)
			throw FILE_ERROR;

		if(s.Read<int>() != UpdateManagerVersion)
			throw OLD_CLIENT;

		DWORD responseCode = s.Read<DWORD>();

		//Read message
		CStringW message = s.ReadStringW();
		if(message.GetLength() > 0)
			MessageBox(prm->dlg->GetSafeHwnd(), CW2T(message), R2T(IDS_UPDATE_SERVERMESSAGECAPTION), MB_ICONINFORMATION);

		//Read file count
		int files = s.Read<int>();

		//Read total bytes count
		int totalBytes = s.Read<int>();

		prm->dlg->SetJobBytes(totalBytes);
		UINT totalFinishedBytes=0;

        
		for(int currentFile=0;currentFile<files;currentFile++)
		{
			//Read require registration
			char requireRegistration = s.Read<char>();

			//Read file name
			CString fileName = CW2T(s.ReadStringW());

			//Read file size
			UINT fileSize = s.Read<UINT>();

            UINT readBytes = 0;
			while(readBytes < fileSize)
			{
				DWORD read, toRead;
				byte buf[16*1024];
				toRead = min(fileSize-readBytes, sizeof(buf));
				read = s.ReadBuffer(buf, toRead);
				if(read == 0)
					throw CONNECTION_ERROR;
				readBytes += read;
				totalFinishedBytes += read;
				if(!::WriteFile(tempFile, buf, read, &toRead, NULL))
					throw FILE_ERROR;
				if(toRead != read)
					throw FILE_ERROR;

				prm->dlg->SetFinishedBytes(totalFinishedBytes);
			}

			::CloseHandle(tempFile);
			tempFile = 0;
			//move the file to "program files\minakort" or "~\Local settings\minakort\binaries"
			CString permanentPath = GetBinaryPath(administratorInstall);
			::SetCurrentDirectory(permanentPath); //So that download libraries will be found by windows.

			//if targetFile already exists, move it to a temp name
			if(::PathFileExists(permanentPath + fileName))
			{
				//Try to delete it first:
				if(!::DeleteFile(permanentPath + fileName))
				{
					CString tmp;
					tmp.Format(_T("old_%u"), ::GetTickCount);
					::MoveFile(permanentPath + fileName, permanentPath + tmp + fileName);
					//TODO: set a delete on the file upon reboot.
				}
			}

			permanentPath += fileName;

			if(!::MoveFile(tempFileName, permanentPath))
				throw FILE_ERROR;
			tempFileName.Empty();

			newmod = ::LoadLibrary(permanentPath);
			if(!newmod)
				throw BINARY_ERROR;

			//OverrideClassesRoot has no effect on Win95, which is ok, since we're allways admins there annyways.
			if(!administratorInstall)
				Utility::OverrideClassesRoot(HKEY_CURRENT_USER, _T("Software\\Classes"));

			regfunc prereg		= (regfunc)::GetProcAddress(newmod, ("UpdatedPreregInstall"));
			comvoid reg			= (comvoid)::GetProcAddress(newmod, ("DllRegisterServer"));
			regfunc postreg		= (regfunc)::GetProcAddress(newmod, ("UpdatedPostregInstall"));
//			getGuidFunc getGuid	= (getGuidFunc)::GetProcAddress(newmod, _T("GetClsid"));

			if(!reg && requireRegistration)
				throw BINARY_ERROR;

			HRESULT hr = S_OK;
			if(prereg)
				hr = prereg(ReleaseVersion, UpdateManagerVersion, administratorInstall);
			if(reg)
				hr = reg();
			if(FAILED(hr))
				throw REGISTRATION_ERROR;
			if(postreg)
				hr = postreg(ReleaseVersion, UpdateManagerVersion, administratorInstall);
//			if(getGuid)
//				hr = getGuid(L"CMinakortWebUpload", &prm->uploadClsid);

			if(newmod)
				::FreeLibrary(newmod);
			newmod = 0;

		}

		//TODO: selfdelete.
		//Begin by moving this dll, to a temp path, and then delete it on reboot.		
		
		success = TRUE;
		error	= S_OK;
	}
	catch(CInternetException* exc)
	{
		error = 0-CONNECTION_ERROR;
		exc->GetErrorMessage(errorMessage.GetBuffer(1000), 1000);
		errorMessage.ReleaseBuffer();
		exc->Delete();
	}
	catch(CException* exc)
	{
		error = E_FAIL;
		exc->GetErrorMessage(errorMessage.GetBuffer(1000), 1000);
		errorMessage.ReleaseBuffer();
		exc->Delete();
	}
	catch(int errNum)
	{
		error = 0-errNum; //TODO: create a real hresult

		switch(errNum)
		{
		case CONNECTION_ERROR:
			errorMessage = R2T(IDS_UPDATEERROR_COMM);
			break;
		case FILE_ERROR:
			errorMessage = R2T(IDS_UPDATEERROR_FILE);
			break;
		case OLD_CLIENT:
			errorMessage = R2T(IDS_UPDATEERROR_OLDCLIENT);
			break;
		case BINARY_ERROR:
			errorMessage = R2T(IDS_UPDATEERROR_BINARY);
			break;
		case REGISTRATION_ERROR:
			errorMessage = R2T(IDS_UPDATEERROR_REG);
			break;
		default:
			errorMessage = R2T(IDS_UPDATEERROR_UNKNOWN);
		}

	}
	catch(...)
	{
		errorMessage = R2T(IDS_UPDATEERROR_UNKNOWN);
		error = E_FAIL;
	}

	if(tempFile)
		::CloseHandle(tempFile);
	if(newmod)
		::FreeLibrary(newmod);
	if(tempFileName.GetLength())
		::DeleteFile(tempFileName);

	if(FAILED(error))
	{
		::MessageBox(prm->dlg->GetSafeHwnd(), CString(R2T(IDS_UPDATE_INSTALLATIONFAILED)) + errorMessage, R2T(IDS_UPDATE_INSTALLINGCAPTION), MB_ICONERROR|MB_OK);
	}
	else
	{
		::MessageBox(prm->dlg->GetSafeHwnd(), R2T(IDS_UPDATE_INSTALLATIONFINISHED), R2T(IDS_UPDATE_INSTALLINGCAPTION), MB_OK|MB_ICONINFORMATION);
	}

	prm->result = error;
	prm->dlg->EndDialog(IDOK);
	prm->finishedEvent->SetEvent();

	return error;
}



