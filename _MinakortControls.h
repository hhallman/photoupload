

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 6.00.0361 */
/* at Tue Jun 21 23:42:35 2005
 */
/* Compiler settings for _MinakortControls.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef ___MinakortControls_h__
#define ___MinakortControls_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IMinakortWebUpload_FWD_DEFINED__
#define __IMinakortWebUpload_FWD_DEFINED__
typedef interface IMinakortWebUpload IMinakortWebUpload;
#endif 	/* __IMinakortWebUpload_FWD_DEFINED__ */


#ifndef __CMinakortWebUpload_FWD_DEFINED__
#define __CMinakortWebUpload_FWD_DEFINED__

#ifdef __cplusplus
typedef class CMinakortWebUpload CMinakortWebUpload;
#else
typedef struct CMinakortWebUpload CMinakortWebUpload;
#endif /* __cplusplus */

#endif 	/* __CMinakortWebUpload_FWD_DEFINED__ */


/* header files for imported files */
#include "mshtml.h"
#include "mshtmhst.h"
#include "oledb.h"
#include "objsafe.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IMinakortWebUpload_INTERFACE_DEFINED__
#define __IMinakortWebUpload_INTERFACE_DEFINED__

/* interface IMinakortWebUpload */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IMinakortWebUpload;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9C0BEABD-415D-40F5-B082-7221DC5E479A")
    IMinakortWebUpload : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ShowDialog( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ShowDialogOutOfProcess( void) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Quota( 
            /* [retval][out] */ ULONG *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Quota( 
            /* [in] */ ULONG newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_UploadPath( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_UploadPath( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_UsedQuota( 
            /* [retval][out] */ UINT *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_UsedQuota( 
            /* [in] */ UINT newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_LatestRelease( 
            /* [retval][out] */ LONG *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_LatestRelease( 
            /* [in] */ LONG newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ReleaseNumber( 
            /* [retval][out] */ LONG *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_RequiredRelease( 
            /* [in] */ LONG newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE RunUpdate( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMinakortWebUploadVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMinakortWebUpload * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMinakortWebUpload * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMinakortWebUpload * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMinakortWebUpload * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMinakortWebUpload * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMinakortWebUpload * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMinakortWebUpload * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *ShowDialog )( 
            IMinakortWebUpload * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *ShowDialogOutOfProcess )( 
            IMinakortWebUpload * This);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Quota )( 
            IMinakortWebUpload * This,
            /* [retval][out] */ ULONG *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Quota )( 
            IMinakortWebUpload * This,
            /* [in] */ ULONG newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_UploadPath )( 
            IMinakortWebUpload * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_UploadPath )( 
            IMinakortWebUpload * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_UsedQuota )( 
            IMinakortWebUpload * This,
            /* [retval][out] */ UINT *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_UsedQuota )( 
            IMinakortWebUpload * This,
            /* [in] */ UINT newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_LatestRelease )( 
            IMinakortWebUpload * This,
            /* [retval][out] */ LONG *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_LatestRelease )( 
            IMinakortWebUpload * This,
            /* [in] */ LONG newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ReleaseNumber )( 
            IMinakortWebUpload * This,
            /* [retval][out] */ LONG *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_RequiredRelease )( 
            IMinakortWebUpload * This,
            /* [in] */ LONG newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *RunUpdate )( 
            IMinakortWebUpload * This);
        
        END_INTERFACE
    } IMinakortWebUploadVtbl;

    interface IMinakortWebUpload
    {
        CONST_VTBL struct IMinakortWebUploadVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMinakortWebUpload_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMinakortWebUpload_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMinakortWebUpload_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMinakortWebUpload_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMinakortWebUpload_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMinakortWebUpload_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMinakortWebUpload_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMinakortWebUpload_ShowDialog(This)	\
    (This)->lpVtbl -> ShowDialog(This)

#define IMinakortWebUpload_ShowDialogOutOfProcess(This)	\
    (This)->lpVtbl -> ShowDialogOutOfProcess(This)

#define IMinakortWebUpload_get_Quota(This,pVal)	\
    (This)->lpVtbl -> get_Quota(This,pVal)

#define IMinakortWebUpload_put_Quota(This,newVal)	\
    (This)->lpVtbl -> put_Quota(This,newVal)

#define IMinakortWebUpload_get_UploadPath(This,pVal)	\
    (This)->lpVtbl -> get_UploadPath(This,pVal)

#define IMinakortWebUpload_put_UploadPath(This,newVal)	\
    (This)->lpVtbl -> put_UploadPath(This,newVal)

#define IMinakortWebUpload_get_UsedQuota(This,pVal)	\
    (This)->lpVtbl -> get_UsedQuota(This,pVal)

#define IMinakortWebUpload_put_UsedQuota(This,newVal)	\
    (This)->lpVtbl -> put_UsedQuota(This,newVal)

#define IMinakortWebUpload_get_LatestRelease(This,pVal)	\
    (This)->lpVtbl -> get_LatestRelease(This,pVal)

#define IMinakortWebUpload_put_LatestRelease(This,newVal)	\
    (This)->lpVtbl -> put_LatestRelease(This,newVal)

#define IMinakortWebUpload_get_ReleaseNumber(This,pVal)	\
    (This)->lpVtbl -> get_ReleaseNumber(This,pVal)

#define IMinakortWebUpload_put_RequiredRelease(This,newVal)	\
    (This)->lpVtbl -> put_RequiredRelease(This,newVal)

#define IMinakortWebUpload_RunUpdate(This)	\
    (This)->lpVtbl -> RunUpdate(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IMinakortWebUpload_ShowDialog_Proxy( 
    IMinakortWebUpload * This);


void __RPC_STUB IMinakortWebUpload_ShowDialog_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IMinakortWebUpload_ShowDialogOutOfProcess_Proxy( 
    IMinakortWebUpload * This);


void __RPC_STUB IMinakortWebUpload_ShowDialogOutOfProcess_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IMinakortWebUpload_get_Quota_Proxy( 
    IMinakortWebUpload * This,
    /* [retval][out] */ ULONG *pVal);


void __RPC_STUB IMinakortWebUpload_get_Quota_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IMinakortWebUpload_put_Quota_Proxy( 
    IMinakortWebUpload * This,
    /* [in] */ ULONG newVal);


void __RPC_STUB IMinakortWebUpload_put_Quota_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IMinakortWebUpload_get_UploadPath_Proxy( 
    IMinakortWebUpload * This,
    /* [retval][out] */ BSTR *pVal);


void __RPC_STUB IMinakortWebUpload_get_UploadPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IMinakortWebUpload_put_UploadPath_Proxy( 
    IMinakortWebUpload * This,
    /* [in] */ BSTR newVal);


void __RPC_STUB IMinakortWebUpload_put_UploadPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IMinakortWebUpload_get_UsedQuota_Proxy( 
    IMinakortWebUpload * This,
    /* [retval][out] */ UINT *pVal);


void __RPC_STUB IMinakortWebUpload_get_UsedQuota_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IMinakortWebUpload_put_UsedQuota_Proxy( 
    IMinakortWebUpload * This,
    /* [in] */ UINT newVal);


void __RPC_STUB IMinakortWebUpload_put_UsedQuota_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IMinakortWebUpload_get_LatestRelease_Proxy( 
    IMinakortWebUpload * This,
    /* [retval][out] */ LONG *pVal);


void __RPC_STUB IMinakortWebUpload_get_LatestRelease_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IMinakortWebUpload_put_LatestRelease_Proxy( 
    IMinakortWebUpload * This,
    /* [in] */ LONG newVal);


void __RPC_STUB IMinakortWebUpload_put_LatestRelease_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IMinakortWebUpload_get_ReleaseNumber_Proxy( 
    IMinakortWebUpload * This,
    /* [retval][out] */ LONG *pVal);


void __RPC_STUB IMinakortWebUpload_get_ReleaseNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IMinakortWebUpload_put_RequiredRelease_Proxy( 
    IMinakortWebUpload * This,
    /* [in] */ LONG newVal);


void __RPC_STUB IMinakortWebUpload_put_RequiredRelease_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IMinakortWebUpload_RunUpdate_Proxy( 
    IMinakortWebUpload * This);


void __RPC_STUB IMinakortWebUpload_RunUpdate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IMinakortWebUpload_INTERFACE_DEFINED__ */



#ifndef __MinakortControlsLib_LIBRARY_DEFINED__
#define __MinakortControlsLib_LIBRARY_DEFINED__

/* library MinakortControlsLib */
/* [helpstring][uuid][version] */ 


EXTERN_C const IID LIBID_MinakortControlsLib;

EXTERN_C const CLSID CLSID_CMinakortWebUpload;

#ifdef __cplusplus

class DECLSPEC_UUID("215DA363-C55E-4597-861A-753F6C927E02")
CMinakortWebUpload;
#endif
#endif /* __MinakortControlsLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


