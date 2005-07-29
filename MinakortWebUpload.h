// MinakortWebUpload.h : Declaration of the CMinakortWebUpload
#pragma once
#include "resource.h"       // main symbols
#include <atlctl.h>

// IMinakortWebUpload
[
	object,
	uuid(9C0BEABD-415D-40F5-B082-7221DC5E479A),
	dual,
	helpstring("IMinakortWebUpload Interface"),
	pointer_default(unique)
]
__interface IMinakortWebUpload : public IDispatch
{
	[id(1), helpstring("method ShowDialog")] HRESULT ShowDialog(void);
	[id(2), helpstring("method ShowDialogOutOfProcess")] HRESULT ShowDialogOutOfProcess(void);
	[propget, id(3), helpstring("property Quota")] HRESULT Quota([out, retval] ULONG* pVal);
	[propput, id(3), helpstring("property Quota")] HRESULT Quota([in] ULONG newVal);
	[propget, id(4), helpstring("property UploadPath")] HRESULT UploadPath([out, retval] BSTR* pVal);
	[propput, id(4), helpstring("property UploadPath")] HRESULT UploadPath([in] BSTR newVal);
	[propget, id(5), helpstring("property UsedQuota")] HRESULT UsedQuota([out, retval] UINT* pVal);
	[propput, id(5), helpstring("property UsedQuota")] HRESULT UsedQuota([in] UINT newVal);
	[propget, id(6), helpstring("property LatestRelease")] HRESULT LatestRelease([out, retval] LONG* pVal);
	[propput, id(6), helpstring("property LatestRelease")] HRESULT LatestRelease([in] LONG newVal);
	[propget, id(7), helpstring("property ReleaseNumber")] HRESULT ReleaseNumber([out, retval] LONG* pVal);
	[propput, id(8), helpstring("property RequiredRelease")] HRESULT RequiredRelease([in] LONG newVal);
	[id(9), helpstring("method RunUpdate")] HRESULT RunUpdate(void);
};


// CMinakortWebUpload
[
	coclass,
	threading("apartment"),
	vi_progid("MinakortControls.MinakortWebUpload"),
	progid("MinakortControls.MinakortWebUpload.2"),
	version(1.0),
	uuid("215DA363-C55E-4597-861A-753F6C927E02"),
	helpstring("MinakortWebUpload Class"),
	support_error_info(IMinakortWebUpload),
	registration_script("control.rgs")
]
class ATL_NO_VTABLE CMinakortWebUpload : 
	public IMinakortWebUpload,
	public IPersistStreamInitImpl<CMinakortWebUpload>,
	public IOleControlImpl<CMinakortWebUpload>,
	public IOleObjectImpl<CMinakortWebUpload>,
	public IOleInPlaceActiveObjectImpl<CMinakortWebUpload>,
	public IViewObjectExImpl<CMinakortWebUpload>,
	public IOleInPlaceObjectWindowlessImpl<CMinakortWebUpload>,
	public IPersistStorageImpl<CMinakortWebUpload>,
	public ISpecifyPropertyPagesImpl<CMinakortWebUpload>,
	public IQuickActivateImpl<CMinakortWebUpload>,
	public IDataObjectImpl<CMinakortWebUpload>,
	public IProvideClassInfo2Impl<&__uuidof(CMinakortWebUpload), NULL>,
	public CComControl<CMinakortWebUpload>,
	public IObjectSafetyImpl<CMinakortWebUpload, INTERFACESAFE_FOR_UNTRUSTED_CALLER|INTERFACESAFE_FOR_UNTRUSTED_DATA>
{
public:

	CMinakortWebUpload()
	{
	}

DECLARE_OLEMISC_STATUS(OLEMISC_RECOMPOSEONRESIZE | 
	OLEMISC_CANTLINKINSIDE | 
	OLEMISC_INSIDEOUT | 
	OLEMISC_ACTIVATEWHENVISIBLE | 
	OLEMISC_SETCLIENTSITEFIRST
)


BEGIN_PROP_MAP(CMinakortWebUpload)
	PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
	PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
	// Example entries
	// PROP_ENTRY("Property Description", dispid, clsid)
	// PROP_PAGE(CLSID_StockColorPage)
END_PROP_MAP()

LRESULT OnClicked(UINT /*uMsg*/, WPARAM /*wParam*/, HWND /*lParam*/, BOOL& /*bHandled*/);

BEGIN_MSG_MAP(CMinakortWebUpload)
	MESSAGE_HANDLER(WM_CREATE, OnCreate)
	CHAIN_MSG_MAP(CComControl<CMinakortWebUpload>)
	DEFAULT_REFLECTION_HANDLER()
END_MSG_MAP()
// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

// IViewObjectEx
	DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)
private:
	UINT quota;
	UINT usedQuota;
	CString uploadPath;
	UINT LatestRelease;
	UINT RequiredRelease;
	IMinakortWebUpload* pInnerControl;

	STDMETHOD(EnsureVersion)(HWND parent);

// IMinakortWebUpload
public:
	LRESULT OnDraw(ATL_DRAWINFO& di);


	DECLARE_PROTECT_FINAL_CONSTRUCT()
//	HRESULT FinalConstruct();

	
	void FinalRelease();
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	STDMETHOD(ShowDialog)(void);
//	static void WINAPI ObjectMain(bool bStarting);
	HRESULT FinalConstruct();
	STDMETHOD(ShowDialogOutOfProcess)(void);
	STDMETHOD(get_Quota)(ULONG* pVal);
	STDMETHOD(put_Quota)(ULONG newVal);
	STDMETHOD(get_UploadPath)(BSTR* pVal);
	STDMETHOD(put_UploadPath)(BSTR newVal);
	STDMETHOD(get_UsedQuota)(UINT* pVal);
	STDMETHOD(put_UsedQuota)(UINT newVal);
	STDMETHOD(get_LatestRelease)(LONG* pVal);
	STDMETHOD(put_LatestRelease)(LONG newVal);
	STDMETHOD(get_ReleaseNumber)(LONG* pVal);
	STDMETHOD(put_RequiredRelease)(LONG newVal);
	STDMETHOD(RunUpdate)(void);
};

