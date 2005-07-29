// MinakortControls.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "MinakortControls.h"
#include <initguid.h>
#include "MinakortControls_i.c"
#include ".\defines.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

// CMinakortControlsApp



int ReleaseVersion = CMinakortControlsApp::ReleaseVersion;

class CMinakortControlsModule :
	public CAtlMfcModule
{
public:
	DECLARE_LIBID(LIBID_MinakortControlsLib);
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_MINAKORTCONTROLS, "{DA78B3D8-1745-4650-9AF7-A929262F5DF5}");};

CMinakortControlsModule _AtlModule;

BEGIN_MESSAGE_MAP(CMinakortControlsApp, CWinApp)
END_MESSAGE_MAP()


// CMinakortControlsApp construction

CMinakortControlsApp::CMinakortControlsApp()
{
}


// The one and only CMinakortControlsApp object

CMinakortControlsApp theApp;


// CMinakortControlsApp initialization

BOOL CMinakortControlsApp::InitInstance()
{
	COleObjectFactory::RegisterAll();
	CWinApp::InitInstance();

	InitCommonControls();

	this->SetRegistryKey(_T("Hugonet"));

	USAGE("CMinakortControlsApp::InitInstance");

	return TRUE;
}

// DllCanUnloadNow - Allows COM to unload DLL
#pragma comment(linker, "/EXPORT:DllCanUnloadNow=_DllCanUnloadNow@0,PRIVATE")
#pragma comment(linker, "/EXPORT:DllGetClassObject=_DllGetClassObject@12,PRIVATE")
#pragma comment(linker, "/EXPORT:DllRegisterServer=_DllRegisterServer@0,PRIVATE")
#pragma comment(linker, "/EXPORT:DllUnregisterServer=_DllUnregisterServer@0,PRIVATE")

STDAPI DllCanUnloadNow(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (_AtlModule.GetLockCount() > 0)
		return S_FALSE;
	return S_OK;
}

// DllGetClassObject - Returns class factory
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (S_OK == _AtlModule.GetClassObject(rclsid, riid, ppv))
		return S_OK;
	return AfxDllGetClassObject(rclsid, riid, ppv);
}

// DllRegisterServer - Adds entries to the system registry
STDAPI DllRegisterServer(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	_AtlModule.UpdateRegistryAppId(TRUE);
	HRESULT hRes = _AtlModule.RegisterServer(TRUE);
	if (hRes != S_OK)
		return hRes;
	if (!COleObjectFactory::UpdateRegistryAll(TRUE))
		return ResultFromScode(SELFREG_E_CLASS);
	return S_OK;
}

// DllUnregisterServer - Removes entries from the system registry
STDAPI DllUnregisterServer(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	_AtlModule.UpdateRegistryAppId(FALSE);
	HRESULT hRes = _AtlModule.UnregisterServer(TRUE);
	if (hRes != S_OK)
		return hRes;
	if (!COleObjectFactory::UpdateRegistryAll(FALSE))
		return ResultFromScode(SELFREG_E_CLASS);
	return S_OK;
}


[ module(unspecified, uuid = "{B3D3DFE9-DEB4-44BC-B7FD-C92224D11E68}", helpstring = "MinakortControls 1.0 Type Library", name = "MinakortControlsLib") ];
