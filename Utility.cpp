#include "StdAfx.h"
#include ".\Defines.h"
#include ".\utility.h"



Utility::Utility(void)
{
}

Utility::~Utility(void)
{
}

CString Utility::GetLocalSettingsPath()
{
	CString path;
	HRESULT hr = SHGetFolderPath(NULL, 
					CSIDL_LOCAL_APPDATA, 
					NULL, 
					0, 
					path.GetBuffer(MAX_PATH));
	path.ReleaseBuffer();

	if(SUCCEEDED(hr))
	{
		path.Append(_T("\\Minakort\\"));
		if(!::PathFileExists(path))
			::CreateDirectory(path,NULL);
		return path;
	}
	return _T("");
}

//requires shell32.dll version 5.0 or later
const CString Utility::GetMyPicturesPath()
{
	CString path;

	
	HRESULT hr = SHGetFolderPath(NULL, 
					CSIDL_MYPICTURES, 
					NULL, 
					0, 
					path.GetBuffer(MAX_PATH));
	path.ReleaseBuffer();

	if(SUCCEEDED(hr))
		return path;

	hr = SHGetFolderPath(NULL, 
					CSIDL_PERSONAL, 
					NULL, 
					0, 
					path.GetBuffer(MAX_PATH));
	path.ReleaseBuffer();
	if(SUCCEEDED(hr))
		return path;

	return _T("C:\\");
}

bool Utility::OpenMyPictures()
{
	const CString path = Utility::GetMyPicturesPath();
	::ShellExecute(NULL, _T("explore"),path, _T(""),_T(""),SW_SHOWNORMAL);
	return path != _T("C:\\");
}

//struct CREDUI_INFO
//{
//	DWORD cbSize;
//	HWND hwndParent;
//	LPCTSTR pszMessageText;
//	LPCTSTR pszCaptionText;
//	HBITMAP hbmBanner;
//}
//
////#include <WinCred.h>
//DWORD WINAPI CredUIPromptForCredentials(
//  CREDUI_INFO* pUiInfo,
//  PCTSTR pszTargetName,
//  PCtxtHandle Reserved,
//  DWORD dwAuthError,
//  PCTSTR pszUserName,
//  ULONG ulUserNameMaxChars,
//  PCTSTR pszPassword,
//  ULONG ulPasswordMaxChars,
//  PBOOL pfSave,
//  DWORD dwFlags
//);
//
//
//#define MAX_USERNAME_LEN	20
//#define MAX_PWD_LEN			60
//void WinXpLogon(CWnd* parent)
//{
//	CREDUI_INFO info;
//	info.cbSize				= sizeof info;
//	info.hwndParent			= parent->GetSafeHwnd();
//	info.pszMessageText		= _T("Ange ditt användarnamn och lösenord till minakort.com");
//	info.pszCaptionText		= _T("Logga in på minakort.com");
//	info.hbmBanner			= NULL;
//
//	PCTSTR systemName		= _T("MINAKORT.COM");
//	DWORD errReason			= 0;
//	char name[MAX_USERNAME_LEN+1];
//	char pwd[MAX_PWD_LEN+1];
//	DWORD nameLen			= MAX_USERNAME_LEN;
//	DWORD pwdLen			= MAX_PWD_LEN;
//
//	DWORD err = ::CredUIPromptForCredentials(&info, systemName, 0, 0, name, nameLen, pwd, pwdlen, FALSE, 0);
//
//	return;
//
//}

void Utility::SetOverlay(CListCtrl& view, int index, int overlay)
{
	LVITEM item;
	ZeroMemory(&item, sizeof(LVITEM));
	item.iItem		= index;
	item.mask		= LVIF_STATE;
	item.stateMask	= LVIS_OVERLAYMASK;
	item.state		= INDEXTOOVERLAYMASK(overlay);
	view.SetItem(&item);
}


typedef LONG (APIENTRY *RegOverridePredefKeyFunc)(
  HKEY hKey,
  HKEY hNewHKey
);


HRESULT Utility::OverrideClassesRoot(HKEY hKeyBase, LPCTSTR szOverrideKey)
{
	HKEY hKey;
	LONG wres = ::RegOpenKey(hKeyBase, szOverrideKey, &hKey);
	
	if (wres == ERROR_SUCCESS)
	{
		HMODULE advapi = ::LoadLibrary(_T("advapi32.dll"));
		if(advapi)
		{
			RegOverridePredefKeyFunc pRegOverridePredefKey = reinterpret_cast<RegOverridePredefKeyFunc>(::GetProcAddress(advapi, ("RegOverridePredefKey")));
			if(pRegOverridePredefKey)
				wres = pRegOverridePredefKey(HKEY_CLASSES_ROOT, hKey);
			else wres = ERROR_FILE_NOT_FOUND;
		}
		else
			wres = ERROR_FILE_NOT_FOUND;

		::RegCloseKey(hKey);
	}

	return HRESULT_FROM_WIN32(wres);
}


typedef  BOOL (WINAPI *AllocateAndInitializeSidFunc)(
  PSID_IDENTIFIER_AUTHORITY pIdentifierAuthority,
  BYTE nSubAuthorityCount,
  DWORD dwSubAuthority0,
  DWORD dwSubAuthority1,
  DWORD dwSubAuthority2,
  DWORD dwSubAuthority3,
  DWORD dwSubAuthority4,
  DWORD dwSubAuthority5,
  DWORD dwSubAuthority6,
  DWORD dwSubAuthority7,
  PSID* pSid
);

typedef BOOL (WINAPI *CheckTokenMembershipFunc)(
  HANDLE TokenHandle,
  PSID SidToCheck,
  PBOOL IsMember
);

typedef PVOID (WINAPI *FreeSidFunc)(
  PSID pSid
);


BOOL Utility::IsUserAdmin()
{
	BOOL b = FALSE;
	//If we're on win95, then consider us administrators
	OSVERSIONINFO version;
	ZeroMemory(&version, sizeof(version));
	version.dwOSVersionInfoSize = sizeof(version);
	b = ::GetVersionEx(&version);
	if(b && version.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
		return TRUE;

	//We might be on win95, so don't implicitly link to nt functions here.

	HMODULE advapi = ::LoadLibrary(_T("advapi32.dll"));
	if(!advapi)
		return FALSE;

	AllocateAndInitializeSidFunc pAllocateAndInitializeSid = (AllocateAndInitializeSidFunc)::GetProcAddress(advapi, ("AllocateAndInitializeSid"));
	CheckTokenMembershipFunc pCheckTokenMembership = (CheckTokenMembershipFunc)::GetProcAddress(advapi, ("CheckTokenMembership"));
	FreeSidFunc pFreeSid = (FreeSidFunc)::GetProcAddress(advapi, ("FreeSid"));

	if(pFreeSid && pCheckTokenMembership && pAllocateAndInitializeSid)
	{
		SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
		PSID AdministratorsGroup; 
		b = pAllocateAndInitializeSid(
			&NtAuthority,
			2,
			SECURITY_BUILTIN_DOMAIN_RID,
			DOMAIN_ALIAS_RID_ADMINS,
			0, 0, 0, 0, 0, 0,
			&AdministratorsGroup); 
		if(b) 
		{
			if (!pCheckTokenMembership( NULL, AdministratorsGroup, &b)) 
				b = FALSE;
			pFreeSid(AdministratorsGroup); 
		}
	}

	::FreeLibrary(advapi);

	return b;
}

CString EscapeUrl(const CString query)
{
	CString out;
	DWORD len=0;
	DWORD len2=(query.GetLength()+10)*2;
	AtlEscapeUrl(query, out.GetBuffer(len2), &len, len2);
	out.ReleaseBuffer();
	return out;
}
