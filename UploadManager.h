#pragma once
#include <afxtempl.h>
#include <afxmt.h>

#include "ShareSettings.h"

class CWinThread;
class CInternetSession;


enum {
		__WM_UPLOAD_BASE			= (WM_USER+100)

		// Uploading has been stopped. Items may remain in queue Allways sent.
		,WM_UPLOAD_DONE				= (__WM_UPLOAD_BASE + 1)
		// Upload successfully finished. All items processed (with successes or failures)
		,WM_UPLOAD_FINISH			= (__WM_UPLOAD_BASE + 2)	//wparam=UploadStatus*, LPARAM=UploadEndedResult
		// There has been progress in the uploading task.
		,WM_UPLOAD_PROGRESSED		= (__WM_UPLOAD_BASE + 3)	//wparam=UploadStatus*
		// Tries to lock a queued item to send. Return "busy" when an item is temporarily unavailable
		,WM_UPLOAD_TRYLOCKITEM		= (__WM_UPLOAD_BASE + 4)	//WPARAM=PhotoReference* - LRESULT=UploadLockResult
		// Sent when an item has been sent/cancelled/failed or any other reason to stop sending.
		,WM_UPLOAD_UNLOCKITEM		= (__WM_UPLOAD_BASE + 5)	//WPARAM=PhotoReference* LPARAM:0=default,1=aborted
		// Sent when an item has been sent with fail or success.
		,WM_UPLOAD_ITEMFINISHED		= (__WM_UPLOAD_BASE + 6)	//WPARAM=PhotoReference* 

		// Sent when a login is required.
		,WM_UPLOAD_REQUIRELOGIN									//WPARAM=LoginResult*(null) LPARAM=UploadManager::LoginCredentials*

		// Get a list of photos that need to save new texts to the server.
		,WM_UPLOAD_GETDIRTYPHOTOTEXT							//WPARAM=CArray<PhotoReference*>*

		// This client is outdated.
		,WM_UPLOAD_OLDCLIENT
};

class PhotoReference;

class UploadManager
{
private:
	UploadManager(void);
	~UploadManager(void);

	struct ThreadProcArgument { UploadManager* caller; };
	ThreadProcArgument threadArgument;
	static UINT StaticThreadProc(LPVOID);
	UINT ThreadProc();
	
	CWinThread* thread;
	HWND hwndNotify;

	bool abort;
	bool shouldTerminate;
	bool uploadState;

	PhotoReference* GetNext();
	bool UploadJob(PhotoReference* job, CInternetSession* connection);
	void UpdateDirtyJobs(CInternetSession*);

	CEvent	wakeEvent;
	CSize	maxPhotoSize;
	UINT	photoQuality;
	CString authCookieName;
	bool	invalidCookie;
	bool	lastDetailsUpdate;
	UINT	quota;
	CString uploadPath;
	CArray<PhotoReference*> queue;
	CCriticalSection lockObject;
	ShareSettings shareSettings;
	GUID targetCategory;
	bool categoryUploaded;

	static const DWORD	POLL_WAIT_TIME		= 30000;
	static const UINT	SERVER_PORT			= 80;

public:
	bool InitSession(CInternetSession* a_session, bool forceLogin=false);
	enum UploadEndedResult {
		UploadEndSuccess	= 0,
		UploadAborted		= 1,
		UploadCancelled,
		UploadPermanentError,
		UploadNetworkError
	};

	enum UploadLockResult	{ 
		LockResult_Ok=0, 
		LockResult_Busy=1, 
		LockResult_Cancelled=2 
	};

	enum UnlockReason		{ 
		Unlock_Default=0, 
		Unlock_Aborted 
	};

	struct UploadStatus
	{
		UINT queuedBytesEstimate;
		UINT averageSize;
		UINT queuedItems;
		UINT finishedItems;
		UINT finishedBytes;
		UINT totalTime;
		UINT remainingTime;
		UINT percentEstimate;
		UINT minutesRemainingEstimate;
		DWORD processStartTick;
		DWORD jobStartTick;
		UINT kBpS;
		UINT MBpM;

		UploadStatus();
		void Reset();
		void Calculate();
		void StartProcess();
		void AddQueue(int num);
		void FinishImage();
		void ProgressImage(int bytes);
		void StartImage(int size);
	};

	struct LoginCredentials
	{
		CString server;
		CString username;
		CString password;
		bool	persistLogin;
	};

	struct LoginResult
	{
		bool failed;
		CString message;
	};


	static UploadManager& Instance()
	{
		static UploadManager theInstance;
		return theInstance;
	}

	void RunDetailsUpdate();
	void Enqueue(PhotoReference*);
	bool Unenqueue(PhotoReference*);
	bool HasJobs();
	void GetJobs(CArray<PhotoReference*> &jobs);
	void Start(const ShareSettings& settings, CSize maxPhotoSize=CSize(0,0), UINT quality=-1);
	void Abort();
	void GetStatus(UploadStatus* status);
	void SetNotify(HWND wnd);
	void WaitStop();
	void UploadCategory(CInternetSession*);

	__declspec(property(get=GetThreadId)) DWORD ThreadId;
	DWORD GetThreadId() const
	{
		return thread?thread->m_nThreadID:0;
	}
	__declspec(property(get=GetUploadState)) bool UploadState;
	bool GetUploadState() { return this->uploadState; }

	__declspec(property(get=GetQuota, put=SetQuota)) UINT Quota;
	UINT GetQuota()
	{
		return this->quota;
	}
	void SetQuota(UINT quota)
	{
		this->quota = quota;
	}
	UINT	UsedQuota;

private:
		UploadStatus status;

};
