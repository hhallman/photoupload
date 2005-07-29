
#pragma once

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include "Log.h"

#ifndef SAFE_CLOSE
#define SAFE_CLOSE(handle) do { if(handle) VERIFY(::CloseHandle(handle)); handle=0; } while(false)
#endif

#ifndef SAFE_DELETEOBJECT
#define SAFE_DELETEOBJECT(handle) do { if(handle) VERIFY(::DeleteObject(handle)); handle=0;} while(false)
#endif
#ifndef SAFE_DELETEOBJECT_NOL
#define SAFE_DELETEOBJECT_NOL(handle) do { if(handle) VERIFY(::DeleteObject(handle)); } while(false)
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) do { if(p) delete (p); p=NULL; } while(false)
#endif

#define R2W(resId) (CT2W(CString((LPCTSTR)(resId))))
#define R2T(resId) (    (CString((LPCTSTR)(resId))))

#ifndef IDC_HAND
#define IDC_HAND            MAKEINTRESOURCE(32649)
#endif

#define MAGENTA				RGB(0xff,0x00,0xff)

#define FINISHED_OVERLAY	1
#define RUNNING_OVERLAY		2
#define ERROR_OVERLAY		0

#define SECTION_UISETTINGS	_T("ui-settings")

#define null NULL

#define HTTP_AGENT			_T("Minakort upload control")
#define STR_WM_STYLE_UPDATED _T("WM_STYLE_UPDATED")


//#define USAGE(s) Log::LogUsage(_T( s ));
#define USAGE(s)
#define PUTTRACE(s) 

#define STATIC_ASSERT(eval) do { __noop; } while(false)


#ifdef DEBUG__
static const TCHAR SERVER_ADDRESS[]					= _T("192.168.3.20");
static const TCHAR APP_PATH[]						= _T("http://192.168.3.20/");
static const TCHAR APP_DIR[]						= _T("/photoalbum/");
#else
static const TCHAR SERVER_ADDRESS[]					= _T("www.minakort.com");
static const TCHAR APP_PATH[]						= _T("http://www.minakort.com/");
static const TCHAR APP_DIR[]						= _T("/");
#endif
static const TCHAR AUTH_COOKIE[]					= _T(".ASPXCOOKIEAUTH2");



class WaitCursor
{
	HCURSOR curs;
public:
	WaitCursor()
	{
		curs = LoadCursor(NULL, IDC_WAIT);
		curs = SetCursor(curs);
	}
	~WaitCursor()
	{
		SetCursor(curs);
	}
};
