// MinakortWebUpload.cpp : Implementation of CMinakortWebUpload
#include "stdafx.h"
#include ".\minakortwebupload.h"
#include ".\LaunchUploadDlg.h"
#include ".\UpdateManager.h"
#include ".\defines.h"

// CMinakortWebUpload

extern int ReleaseVersion;

//When we have updated ourselves, "aggregate" this control. (by forwarding calls)

//TODO: When we're in an update phase. Don't respond to commands.

HRESULT CMinakortWebUpload::FinalConstruct()
{
	this->quota				= 0;
	this->usedQuota			= -1;
	this->RequiredRelease	= 0;
	this->pInnerControl		= 0;
	this->LatestRelease		= ReleaseVersion;

	return __super::FinalConstruct();
}

void CMinakortWebUpload::FinalRelease()
{
	if(this->pInnerControl)
		pInnerControl->Release();
	pInnerControl = 0;
}

LRESULT CMinakortWebUpload::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return 0;
}

LRESULT CMinakortWebUpload::OnClicked(UINT /*uMsg*/, WPARAM /*wParam*/, HWND /*lParam*/, BOOL& /*bHandled*/)
{
	return 0;
}

STDMETHODIMP CMinakortWebUpload::EnsureVersion(HWND hwnd)
{
	HRESULT hr;

	if(pInnerControl)
		return true;

	if(LatestRelease<=(UINT)ReleaseVersion)
		return S_OK;
	else
	{
		if(RequiredRelease>(UINT)ReleaseVersion) {
			if(IDYES != ::MessageBox(hwnd, R2T(IDS_UPDATE_TOOOLDCLIENT), R2T(IDS_UPDATE_TOOOLDCLIENT_CAPTION), MB_YESNO|MB_ICONERROR))
				return E_FAIL;
		}
		else
			if(IDYES != ::MessageBox(hwnd, R2T(IDS_UPDATE_NEWVERSIONAVAILABLETEXT), R2T(IDS_UPDATE_NEWVVERSIONAVAILABLE), MB_YESNO|MB_ICONINFORMATION))
				return S_FALSE;

		UpdateManager upd;
		GUID clsId;
		ZeroMemory(&clsId, sizeof(clsId));
		if(FAILED(hr = upd.StartUpdate(/*&clsId*/)))
			return hr;
		//Get the new clsid
		CStringW progId;
		progId.Format(L"MinakortControls.MinakortWebUpload.%u", LatestRelease);
		if(FAILED(hr = ::CLSIDFromProgID(progId, &clsId)))
			return hr; //TODO: tell the user

		if(FAILED(hr = ::CoCreateInstance(clsId, NULL, CLSCTX_ALL, __uuidof(IMinakortWebUpload), reinterpret_cast<LPVOID*>(&pInnerControl))))
			return hr;//TODO: tell the user
		else
		{
			LONG newrelease=0;
			pInnerControl->get_ReleaseNumber(&newrelease);
			if(newrelease<=ReleaseVersion)
			{
				pInnerControl->Release();
				pInnerControl = 0;
				return E_FAIL; //TODO: tell the user
			}
			pInnerControl->put_LatestRelease(this->LatestRelease);
			pInnerControl->put_RequiredRelease(this->RequiredRelease);
			pInnerControl->put_Quota(this->quota);
			pInnerControl->put_UsedQuota(this->usedQuota);
			BSTR path = this->uploadPath.AllocSysString();
			pInnerControl->put_UploadPath(path);
			::SysFreeString(path);
			return S_OK;
		}
	}
}

STDMETHODIMP CMinakortWebUpload::ShowDialog(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	HWND hwnd = 0; 
	if(m_spInPlaceSite)
		m_spInPlaceSite->GetWindow(&hwnd);

	HRESULT hr;
	if(FAILED(hr = EnsureVersion(hwnd)))
		return hr;

	if(pInnerControl)
		return pInnerControl->ShowDialog();

	CLaunchUploadDlg::Instance().LaunchDialog(false, hwnd, this->LatestRelease, this->RequiredRelease);

	//This is a cool way to affect the html dom of the containing document.s
	//CComPtr<IOleContainer> spContainer; 
	//m_spClientSite->GetContainer(&spContainer); 
	//CComQIPtr<IHTMLDocument2, &IID_IHTMLDocument2> spDoc(spContainer); 
	//VARIANT v;
	//CComBSTR s(_T("pink"));
	//s.CopyTo(&v);
	//if (spDoc)
	//	spDoc->put_bgColor(v);

	return S_OK;
}

STDMETHODIMP CMinakortWebUpload::ShowDialogOutOfProcess(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	HWND hwnd = 0; 
	if(m_spInPlaceSite)
		m_spInPlaceSite->GetWindow(&hwnd);

	HRESULT hr;
	if(FAILED(hr = EnsureVersion(hwnd)))
		return hr;

	if(pInnerControl)
		return pInnerControl->ShowDialogOutOfProcess();

	CLaunchUploadDlg::Instance().LaunchDialog(true, 0, this->LatestRelease, this->RequiredRelease);

	return S_OK;
}

LRESULT CMinakortWebUpload::OnDraw(ATL_DRAWINFO& di)
{
	RECT& rc = *(RECT*)di.prcBounds;
	// Set Clip region to the rectangle specified by di.prcBounds
	HRGN hRgnOld = NULL;
	if (GetClipRgn(di.hdcDraw, hRgnOld) != 1)
		hRgnOld = NULL;
	bool bSelectOldRgn = false;

	HRGN hRgnNew = CreateRectRgn(rc.left, rc.top, rc.right, rc.bottom);

	if (hRgnNew != NULL)
	{
		bSelectOldRgn = (SelectClipRgn(di.hdcDraw, hRgnNew) != ERROR);
	}

	Rectangle(di.hdcDraw, rc.left, rc.top, rc.right, rc.bottom);
	SetTextAlign(di.hdcDraw, TA_CENTER|TA_BASELINE);
	LPCTSTR pszText = _T("This control should be invisible.");
	TextOut(di.hdcDraw, 
		(rc.left + rc.right) / 2, 
		(rc.top + rc.bottom) / 2, 
		pszText, 
		lstrlen(pszText));

	if (bSelectOldRgn)
		SelectClipRgn(di.hdcDraw, hRgnOld);

	return S_OK;
}

STDMETHODIMP CMinakortWebUpload::get_Quota(ULONG* pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if(pInnerControl)
		return pInnerControl->get_Quota(pVal);

	ASSERT(pVal);
	if(!pVal)
		return E_INVALIDARG;

	*pVal = this->quota;

	return S_OK;
}

STDMETHODIMP CMinakortWebUpload::put_Quota(ULONG newVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if(pInnerControl)
		pInnerControl->put_Quota(newVal);

	this->quota = newVal;
	CLaunchUploadDlg::Instance().SetQuota(quota);

	return S_OK;
}

STDMETHODIMP CMinakortWebUpload::get_UploadPath(BSTR* pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if(pInnerControl)
		return pInnerControl->get_UploadPath(pVal);

	ASSERT(pVal);
	if(!pVal)
		return E_INVALIDARG;

	*pVal = this->uploadPath.AllocSysString();

	return S_OK;
}

STDMETHODIMP CMinakortWebUpload::put_UploadPath(BSTR newVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if(pInnerControl)
		pInnerControl->put_UploadPath(newVal);

	this->uploadPath = newVal;
	CLaunchUploadDlg::Instance().SetUploadPath(this->uploadPath);

	return S_OK;
}

STDMETHODIMP CMinakortWebUpload::get_UsedQuota(UINT* pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if(pInnerControl)
		return pInnerControl->get_UsedQuota(pVal);


	ASSERT(pVal);
	if(!pVal)
		return E_INVALIDARG;

	*pVal = this->usedQuota;

	return S_OK;
}

STDMETHODIMP CMinakortWebUpload::put_UsedQuota(UINT newVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if(pInnerControl)
		pInnerControl->put_UsedQuota(newVal);

	this->usedQuota = newVal;
	CLaunchUploadDlg::Instance().SetUsedQuota(this->usedQuota);

	return S_OK;
}

STDMETHODIMP CMinakortWebUpload::get_LatestRelease(LONG* pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if(pInnerControl)
		return pInnerControl->get_LatestRelease(pVal);

	ASSERT(pVal);
	if(!pVal)
		return E_INVALIDARG;
	*pVal = this->LatestRelease;

	return S_OK;
}

STDMETHODIMP CMinakortWebUpload::put_LatestRelease(LONG newVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if(pInnerControl)
		pInnerControl->put_LatestRelease(newVal);

	this->LatestRelease = newVal;

	return S_OK;
}

STDMETHODIMP CMinakortWebUpload::get_ReleaseNumber(LONG* pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if(pInnerControl)
		return pInnerControl->get_ReleaseNumber(pVal);

	ASSERT(pVal);
	if(!pVal)
		return E_INVALIDARG;
	*pVal = ReleaseVersion;

	return S_OK;
}

STDMETHODIMP CMinakortWebUpload::put_RequiredRelease(LONG newVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if(pInnerControl)
		pInnerControl->put_RequiredRelease(newVal);

	this->RequiredRelease = newVal;
	if(this->LatestRelease < (UINT)newVal)
		this->LatestRelease = newVal;

	return S_OK;
}

STDMETHODIMP CMinakortWebUpload::RunUpdate(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	UpdateManager upd;
	HRESULT hr;
	hr = upd.StartUpdate();

	return hr;
}
