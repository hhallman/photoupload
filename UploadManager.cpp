#include "StdAfx.h"
#include ".\uploadmanager.h"
#include ".\PhotoReference.h"
#include ".\ImageUtility.h"
#include ".\Defines.h"
#include ".\resource.h"
//#include <boost/static_assert.hpp>
#include <gdiplus.h>
#include ".\HttpStream.h"

#ifdef DEBUG
int _lockedSend = 0; //Increment when SENDing a message from within a critical section.
#endif

#ifdef DEBUG
static const bool simulate							= false;
#else
static const bool simulate							= false;
#endif
static const TCHAR UPDATEDETAILS_QUERY_STRING[]		= _T("public/controlCommand.aspx?uploadctrl=y&v=1&action=updatedetails");
static const TCHAR LOGIN_QUERY_STRING[]				= _T("public/controlCommand.aspx?uploadctrl=y&v=1&action=login");
static const TCHAR UPLOAD_QUERY_STRING[]			= _T("public/controlCommand.aspx?uploadctrl=y&v=1&action=upload");
static const TCHAR CATEGORY_QUERY_STRING[]			= _T("public/controlCommand.aspx?uploadctrl=y&v=1&action=createcategory");
static const TCHAR USAGE_REPORT_STRING[]			= _T("public/controlCommand.aspx?uploadctrl=y&v=1&action=usage");

UploadManager::UploadManager(void)
: wakeEvent(FALSE, TRUE) //not initially signaled, manual reset
{
	abort				= false;
	shouldTerminate		= false;
	uploadState			= false;
	invalidCookie		= false;
	lastDetailsUpdate	= false;
	categoryUploaded	= false;
	quota				= 0;
	UsedQuota			= -1;
	authCookieName		= AfxGetApp()->GetProfileString(_T("login"), _T("authname"), AUTH_COOKIE);
	ZeroMemory(&targetCategory, sizeof(targetCategory));
}

UploadManager::~UploadManager(void)
{
}

//Called from GUI-thread
void UploadManager::Start(const ShareSettings& settings, CSize maxsize, UINT quality)
{
	USAGE("Starting uploadmanager");

	this->shareSettings.Copy(settings);
	this->categoryUploaded	= false;

	this->maxPhotoSize		= maxsize;
	this->photoQuality		= quality;

	this->shouldTerminate	= false;
	this->uploadState		= true;

	ASSERT(!_lockedSend);
	CSingleLock(&lockObject, TRUE);
	if(this->thread==0)
	{
		threadArgument.caller = this;
		abort = false;
		wakeEvent.ResetEvent();
		int priority = THREAD_PRIORITY_BELOW_NORMAL;
		this->thread = ::AfxBeginThread(UploadManager::StaticThreadProc, reinterpret_cast<LPVOID>(&threadArgument), priority,0,0,0);
	}
	else
		wakeEvent.PulseEvent();
}

//Called from GUI-thread
void UploadManager::Abort(void)
{
//	uploadState		= false;
	abort			= true;
	wakeEvent.PulseEvent();
}

//Called from GUI-thread
void UploadManager::WaitStop()
{
	ASSERT(!_lockedSend);
	Abort();
	shouldTerminate = true;
	wakeEvent.SetEvent();
	HANDLE hThread;
	{ 
		CSingleLock(&lockObject, TRUE);
		if(!thread)
			return;
		hThread = thread->m_hThread;
	}
	WaitForSingleObject(hThread, INFINITE);
	return;
}

//Called from GUI-thread
void UploadManager::GetStatus(UploadStatus* status)
{
	memcpy(status, &this->status, sizeof UploadStatus);
}

//Called from GUI-thread
void UploadManager::SetNotify(HWND wnd)
{
	ASSERT(!_lockedSend);
	CSingleLock(&lockObject, TRUE);
	this->hwndNotify = wnd;
}


//Called from worker-thread
UINT UploadManager::StaticThreadProc(LPVOID threadArgument)
{
	ThreadProcArgument* arg = reinterpret_cast<ThreadProcArgument*>(threadArgument);
	UINT res = arg->caller->ThreadProc();
	{
		CSingleLock(&arg->caller->lockObject, TRUE);
		arg->caller->thread->m_bAutoDelete = TRUE;
		arg->caller->thread = NULL;
	}
	return res;
}

enum ExceptionCode {
	ExceptionCodeOk				= 0

	,ERRORFLAG_REQUE_JOB		= 0x0001000
	,ERRORFLAG_ABORTPROCESS		= 0x0002000
	,ERRORFLAG_ABORTJOB			= 0x0004000
	,ERRORFLAG_NETWORK			= 0x0008000
	,ERRORFLAG_UPGRADECLIENT	= 0x0010000

	,ImageLoadingFailure		=  1|ERRORFLAG_ABORTJOB
	,ImageProcessingFailure		=  2|ERRORFLAG_ABORTJOB
	,MemoryError				=  3|ERRORFLAG_REQUE_JOB|ERRORFLAG_ABORTPROCESS
	,ConnectionError			=  4|ERRORFLAG_REQUE_JOB|ERRORFLAG_ABORTPROCESS		|ERRORFLAG_NETWORK
	,CommunicationError			=  5|ERRORFLAG_REQUE_JOB							|ERRORFLAG_NETWORK
	,UnexpectedResponseError	=  5|ERRORFLAG_REQUE_JOB|ERRORFLAG_ABORTPROCESS		|ERRORFLAG_NETWORK
	,ErrorAborted				=  6|ERRORFLAG_REQUE_JOB
	,ErrorNetworkRead			=  7|ERRORFLAG_REQUE_JOB
	,PermanentNetworkError		=  8|ERRORFLAG_REQUE_JOB|ERRORFLAG_ABORTPROCESS		|ERRORFLAG_NETWORK
	,UnexpectedException		=  9|ERRORFLAG_REQUE_JOB|ERRORFLAG_ABORTPROCESS
	,ProtocolError				= 10|ERRORFLAG_REQUE_JOB|ERRORFLAG_ABORTPROCESS		|ERRORFLAG_UPGRADECLIENT

	,NotLoggedIn				= 11|ERRORFLAG_REQUE_JOB
};


//void UploadManager::ReportUsage()
//{
//}

void UploadManager::UploadCategory(CInternetSession* session)
{
	if(this->categoryUploaded)
		return;

	CHttpConnection* connection;
	if(!(connection = session->GetHttpConnection(SERVER_ADDRESS, 0, SERVER_PORT, 0, 0)))
		throw ConnectionError;
	
	HttpStream s(connection, CString(APP_DIR) + CString(CATEGORY_QUERY_STRING), true, true);
    
	const UINT version = 1;
	s.Write<UINT>(version); //VERSION
	s.WriteStringW(shareSettings.categoryName);
	s.Write<GUID>(shareSettings.categoryId);
	s.Write<char>(shareSettings.letFriendsAdd);
	s.Write<char>(shareSettings.shareToFriends);
	s.Write<char>(shareSettings.sendEmail);

	s.Write<UINT>(shareSettings.sharedMembers.GetCount());
	for(int i=0;i<shareSettings.sharedMembers.GetCount();i++)
	{
		s.Write(&shareSettings.sharedMembers[i].id, sizeof(shareSettings.sharedMembers[i].id));
	}

	s.StartReading();

	if(version != s.Read<UINT>())
		throw ERRORFLAG_UPGRADECLIENT;
	
	int resultCode = s.Read<int>();
	if(resultCode < 0)
		return;//TODO: REPORT

	s.Read<GUID>(this->targetCategory);
	this->categoryUploaded = true;
}

//Called form worker-thread
bool UploadManager::UploadJob(PhotoReference* job, CInternetSession* session)
{
	if(!simulate)
	{
		static const UINT	MESSAGE_VERSION		= 1;
		static const UINT	BUFFER_SIZE			= 4096;
		Gdiplus::Image*		image				= null;
		CHttpConnection*	connection			= null;
		CHttpFile*			request				= null;
		HGLOBAL				hMemory				= 0;
		ExceptionCode		exceptionCode		= ExceptionCodeOk;
		bool				retry				= false;
		UINT				uploadResult		= -1;
		
		CStringW			uploadPath;			
		{
			CSingleLock(&lockObject, TRUE);
			uploadPath = CT2W(this->uploadPath);
		}

		try
		{
			/////////////////////////////////////////////////////////////////////////////
			// Prepare image
			{
				Gdiplus::Image* load = Gdiplus::Image::FromFile(CT2W(job->GetPath()));
				if(!load || load->GetLastStatus() != Gdiplus::Ok) {
					delete load;
					throw ImageLoadingFailure;
				}
				
				if(!(image = CImageUtility::ShrinkImage(load, this->maxPhotoSize))) {
					delete load;
					throw ImageProcessingFailure;
				}
				
				if(image != load)
					delete load;
			}
			// Do as much as possible before touching the network:

			UINT size = CImageUtility::SaveImage(image, hMemory);
			if(!size)
				throw ImageProcessingFailure;
			
			void* rawImage	= ::GlobalLock(hMemory);
			if(!rawImage)
				throw MemoryError;


			status.StartImage(size);

			/////////////////////////////////////////////////////////////////////////////
			// Prepare connection

			if(!(connection = session->GetHttpConnection(SERVER_ADDRESS, 0, SERVER_PORT, 0, 0)))
				throw ConnectionError;

			if(!(request = connection->OpenRequest(_T("POST"), CString(APP_DIR) + CString(UPLOAD_QUERY_STRING))))
				throw ConnectionError;

			/////////////////////////////////////////////////////////////////////////////
			// Prepare data

			byte hash[]		= { 0 };
			UINT hashLen	= 0;

			unsigned char hintable = 1;

			CStringW title	= CT2W(job->GetTitleLocked());
			CStringW text	= CT2W(job->GetTextLocked());
			CStringW path	= CT2W(job->GetPath());

			UINT msgLen		= 0
				+ sizeof UINT // version
				+ sizeof UINT + hashLen //hash, with it's 32bit size prepended.
				+ sizeof targetCategory
				+ sizeof hintable
				+ sizeof UINT + sizeof WCHAR * (1 + uploadPath.GetLength())
				//strings are sent in UNICODE with uint32 length prepended:
				+ sizeof UINT + sizeof WCHAR * (1 + title.GetLength())
				+ sizeof UINT + sizeof WCHAR * (1 + text .GetLength())
				+ sizeof UINT + sizeof WCHAR * (1 + path .GetLength())
				+ size + sizeof UINT //size + prepended len
				;

			/////////////////////////////////////////////////////////////////////////////
			// Begin send

			if(!request->SendRequestEx(msgLen))
				throw ConnectionError; //todo: comm eror instead?

			/////////////////////////////////////////////////////////////////////////////
			// Send header

			request->Write(&MESSAGE_VERSION, sizeof UINT);

			//write hash
			request->Write(&hashLen, sizeof hashLen);
			if(hashLen>0)
				request->Write(hash, hashLen);
			
			//write target category
			request->Write(&targetCategory, sizeof targetCategory);

			//write hintable
			request->Write(&hintable, 1); ASSERT(sizeof hintable == 1);

			UINT slen;

			//write target path
			slen	= sizeof WCHAR * (uploadPath.GetLength()+1);
			request->Write(&slen, sizeof slen);
			request->Write(static_cast<LPCWSTR>(uploadPath), slen);
			
			//write title
			slen	= sizeof WCHAR * (title.GetLength()+1);
			request->Write(&slen, sizeof slen);
			request->Write(static_cast<LPCWSTR>(title), slen);
			
			//write text
			slen	= sizeof WCHAR * (text.GetLength()+1);
			request->Write(&slen, sizeof slen);
			request->Write(static_cast<LPCWSTR>(text), slen);

			//write path
			slen	= sizeof WCHAR * (path.GetLength()+1);
			request->Write(&slen, sizeof slen);
			request->Write(static_cast<LPCWSTR>(path), slen);

			/////////////////////////////////////////////////////////////////////////////
			// Send photo data
			request->Write(&size,4);
			UINT pos = 0;
			while(pos<size)
			{
				if(abort)
					throw ErrorAborted;

				int write	= size-pos;
				write		= min(write, BUFFER_SIZE);
				request->Write(static_cast<char*>(rawImage)+pos, write);
				pos			+= write;

				status.ProgressImage(write);

				if(hwndNotify)
					::PostMessage(hwndNotify, WM_UPLOAD_PROGRESSED, reinterpret_cast<WPARAM>(&status),0);
			}

			/////////////////////////////////////////////////////////////////////////////
			// End request
			if(!request->EndRequest())
				throw CommunicationError;

			/////////////////////////////////////////////////////////////////////////////
			// Verify result

			UINT resultCode		= 1;
			UINT resultVersion	= -1;
			GUID photoId		;ZeroMemory(&photoId, sizeof photoId);
			UINT l_quota		= 0;
			UINT l_usedQuota	= (UINT)-1;
			msgLen				= 0;
			CStringW msg;

			STATIC_ASSERT(sizeof resultVersion	== 4);
			STATIC_ASSERT(sizeof resultCode		== 4);
			STATIC_ASSERT(sizeof photoId		== 16);
			STATIC_ASSERT(sizeof l_quota		== 4);
			STATIC_ASSERT(sizeof l_usedQuota	== 4);
			STATIC_ASSERT(sizeof msgLen			== 4);

			//TODO: we might throw here, when we actually should just read again.
			//One solution would be to read into a buffer first, and then process that instead.
			//NOTE: above is probably not true. The framework handles read polling.

			if(4 != request->Read(&resultVersion, 4))
				throw ErrorNetworkRead;
			if(resultVersion != 1)
				throw ProtocolError;

			if(4 != request->Read(&resultCode,	4))
				throw ErrorNetworkRead;
			if(16 != request->Read(&photoId,	16))
				throw ErrorNetworkRead;
			if(4  != request->Read(&l_quota,	4))
				throw ErrorNetworkRead;
			if(4 != request->Read(&l_usedQuota,	4))
				throw ErrorNetworkRead;
			if(4 != request->Read(&msgLen,		4))
				throw ErrorNetworkRead;
			PWSTR buff = msg.GetBuffer(msgLen+1);
			if(msgLen != request->Read(buff, msgLen))
				throw ErrorNetworkRead;
			buff[msgLen] = 0;
			msg.ReleaseBuffer();

			job->SetUploadResultMessage(CW2T(msg));

			if(resultCode == 0) {
				job->SetRemoteId(photoId);
			}
			if(resultCode == -2)
				throw NotLoggedIn;

			if(resultCode == 2)
				__noop; //store full.

			this->quota			= l_quota;
			this->UsedQuota		= l_usedQuota;

			uploadResult = resultCode == 0?0:1;
		
			if(resultCode == 0)
				job->SetFinished();

			::SetLastError(0);
		}
		catch(CInternetException*)
		{
			exceptionCode = CommunicationError;
		}
		catch(ExceptionCode err)
		{
			exceptionCode = err;
		}
		catch(...)
		{
			exceptionCode = UnexpectedException;
		}

		// Remove resources.
		if(hMemory) {
			::GlobalUnlock(hMemory);
			::GlobalFree(hMemory);
		}

		status.FinishImage();
		
		if(request)
			request->Close();
		if(connection)
			connection->Close();
		delete request;
		delete connection;
		delete image;

		if(!retry && hwndNotify) {
			::SendMessage(hwndNotify, WM_UPLOAD_UNLOCKITEM, reinterpret_cast<WPARAM>(job),		Unlock_Default);
			ASSERT(!job->LockedStatus());
			::SendMessage(hwndNotify, WM_UPLOAD_ITEMFINISHED, reinterpret_cast<WPARAM>(job),	uploadResult);
			ASSERT(uploadResult!=0 || job->Finished());
		}

		if(exceptionCode != ExceptionCodeOk)
			throw exceptionCode;

		return true;

	} //!simulate

	if(simulate)
	{
		Gdiplus::Image* image;
		{
			Gdiplus::Image* load = Gdiplus::Image::FromFile(CT2W(job->GetPath()));
			if(load->GetLastStatus() != Gdiplus::Ok)
				__noop; //todo.
			image = CImageUtility::ShrinkImage(load, this->maxPhotoSize);
			if(image != load)
				delete load;
		}
		HGLOBAL hmem=0;
		UINT len = CImageUtility::SaveImage(image, hmem, _T("image/jpeg"), photoQuality);
		delete image;
		void* mem = ::GlobalLock(hmem);

		status.StartImage(len);
		//for(int i=0;i<10&&!abort;i++)
		//{
		//	status.ProgressImage(len/10);
		//	if(hwndNotify)
		//		::SendMessage(hwndNotify, WM_UPLOAD_PROGRESSED, reinterpret_cast<WPARAM>(&status),0);
		//	Sleep(900);
		//}
		status.FinishImage();

		CString outFile;
		outFile.Format(_T("upload\\%d.jpg"), ::GetTickCount());
		HANDLE hFile = ::CreateFile(outFile, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
		DWORD written=0;
		::WriteFile(hFile, mem, len, &written, 0);
		::CloseHandle(hFile);
		::GlobalUnlock(hmem);
		::GlobalFree(hmem);
		GUID id;
		memset(&id, 0x11, sizeof id);
		job->SetRemoteId(id);
		job->SetFinished();
		if(hwndNotify) {
			::SendMessage(hwndNotify, WM_UPLOAD_UNLOCKITEM, reinterpret_cast<WPARAM>(job),		Unlock_Default);
			ASSERT(!job->LockedStatus());
			::SendMessage(hwndNotify, WM_UPLOAD_ITEMFINISHED, reinterpret_cast<WPARAM>(job),	0);
			ASSERT(job->Finished());
		}

		return true;
	}
}

//Called form worker-thread
bool UploadManager::InitSession(CInternetSession* a_session, bool forceLogin /*=false*/)
{
	CInternetSession& session = *a_session;
	CString auth;

	//we have already saved credentials!
	BOOL hasCookie = session.GetCookie(APP_PATH, this->authCookieName, auth);
	static const int minLen = 131+30;//sometimes there's an invalid cookie of length 130 (as seen. could be longer)
	if(false
		|| forceLogin
		|| auth.GetLength()<minLen 
		|| invalidCookie 
		|| !hasCookie 
		|| auth.GetLength()+4<_tcslen(this->authCookieName)
		)
	{
		UploadManager::LoginCredentials credentials;
		UploadManager::LoginResult result;
		result.failed = true;
		CStringW loginCookieName;
		CStringW loginCookieValue;
		GUID userId;
		ZeroMemory(&userId, sizeof userId);
		bool tried = false;
		while(result.failed)
		{
			try
			{
				LPARAM l = tried&&result.failed?reinterpret_cast<LPARAM>(&result):null;
				tried = true;
				LRESULT loginResult = ::SendMessage(hwndNotify, WM_UPLOAD_REQUIRELOGIN, l, reinterpret_cast<LPARAM>(&credentials));
				if(IDCANCEL == loginResult) {
					return false;
				}

				CAutoPtr<CHttpConnection> connection;
				connection.Attach(session.GetHttpConnection(SERVER_ADDRESS, 0, SERVER_PORT));
				if(!connection)
					throw ConnectionError;

				CAutoPtr<CHttpFile> request;
				request.Attach(connection->OpenRequest(_T("POST"), CString(APP_DIR) + LOGIN_QUERY_STRING));

				if(!request)
					throw ConnectionError;

				static const UINT MESSAGE_VERSION = 1;
				LPWSTR username = CT2W(credentials.username);
				LPWSTR password = CT2W(credentials.password);
				UINT len;

				STATIC_ASSERT(sizeof MESSAGE_VERSION == 4);
				STATIC_ASSERT(sizeof len == 4);

				UINT usernameLen = sizeof WCHAR * (credentials.username.GetLength()+1);
				UINT passwordLen = sizeof WCHAR * (credentials.password.GetLength()+1);

				UINT messageLen = 0
					+ sizeof MESSAGE_VERSION
					+ 1 // persist
					+ sizeof UINT + usernameLen
					+ sizeof UINT + passwordLen
					+ 0;
				
				if(!request->SendRequestEx(messageLen))
					throw ConnectionError;

				request->Write(&MESSAGE_VERSION, sizeof MESSAGE_VERSION);

				byte persistent = credentials.persistLogin?0xFF:0x00;
				request->Write(&persistent, 1);

				request->Write(&usernameLen, sizeof usernameLen);
				request->Write(username, usernameLen);

				request->Write(&passwordLen, sizeof passwordLen);
				request->Write(password, passwordLen);

				if(!request->EndRequest())
					throw CommunicationError;

				UINT resultCode;
				UINT resultVersion;

				// read result version
				if(sizeof resultVersion != request->Read(&resultVersion, sizeof resultVersion))
					throw CommunicationError;
				if(resultVersion != 1)
					throw ProtocolError;
				
				// read result code
				if(sizeof resultCode != request->Read(&resultCode, sizeof resultCode))
					throw CommunicationError;

				// read user id
				if(sizeof userId != request->Read(&userId, sizeof userId))
					throw CommunicationError;

				// read cookie name
				if(sizeof len != request->Read(&len, sizeof len))
					throw CommunicationError;
				if(len > 0) {
					if(len != request->Read(loginCookieName.GetBuffer(len/sizeof WCHAR), len))
						throw CommunicationError;
					loginCookieName.ReleaseBuffer();
				}

				// read cookie value
				if(sizeof len != request->Read(&len, sizeof len))
					throw CommunicationError;
				if(len > 0) {
					if(len != request->Read(loginCookieValue.GetBuffer(len/sizeof WCHAR), len))
						throw CommunicationError;
					loginCookieValue.ReleaseBuffer();
				}
				
				// read message
				if(sizeof len != request->Read(&len, sizeof len))
					throw CommunicationError;
				if(len > 0) {
					CStringW message;
					if(len != request->Read(message.GetBuffer(len/sizeof WCHAR), len))
						throw CommunicationError;
					message.ReleaseBuffer();
					result.message = CW2T(message);
				}

				if(resultCode == 0)
				{
					if(false 
						|| loginCookieName	.GetLength() == 0
						|| loginCookieValue	.GetLength() == 0
						)
						throw UnexpectedResponseError;
					result.failed = false;
				}

				if(resultCode != 0) {
					result.failed = true;
					if(resultCode == 1) {
						result.message.Insert(0, _T("\r\n\r\n"));
						result.message.Insert(0, R2T(IDS_LOGIN_WRONGUSERNAMEPWD));
					}
					else
						result.message.Insert(0, R2T(IDS_LOGIN_SERVERERROR));
				}

				request->Close();
				connection->Close();
			}
			catch(CInternetException*)
			{
				throw CommunicationError;
			}
		} // while(result.failed)
		
		this->invalidCookie		= false;
		this->authCookieName	= loginCookieName;
		AfxGetApp()->WriteProfileString(_T("login"), _T("authname"), CW2T(loginCookieName));
	//	CString auth;
	//	BOOL hasCookie = session.GetCookie(APP_PATH, this->authCookieName, auth);

		//Don't set the cookie. it's set from the server, with proper expirations.
	//	if(!session.SetCookie(APP_PATH, CW2T(loginCookieName), CW2T(loginCookieValue)))
	//		throw PermanentNetworkError;
	}
	
	return true;
}

//Called form worker-thread
void UploadManager::UpdateDirtyJobs(CInternetSession* a_session)
{
	ASSERT(a_session);
	ASSERT(this->hwndNotify);
	if(!this->hwndNotify)
		return;

	CArray<PhotoReference*> dirtyPhotos;
	::SendMessage(hwndNotify, WM_UPLOAD_GETDIRTYPHOTOTEXT, reinterpret_cast<WPARAM>(&dirtyPhotos), 0);

	if(dirtyPhotos.GetCount() > 0)
	{
		int a = 3;
		//TODO: post the changes, ignoring errors except login. and update dirty. Objects need locking.

		CArray<byte> buffer;
		buffer.SetSize(8, 2048);

		UINT* pHeader	= reinterpret_cast<UINT*>(buffer.GetData());
		pHeader[0]		= 1;
		pHeader[1]		= dirtyPhotos.GetCount();

		for(int i=0;i<dirtyPhotos.GetCount();i++)
		{
			PhotoReference& p = *dirtyPhotos[i];
			ASSERT(p.LockedStatus());
			ASSERT(p.Finished());
			
			UINT len = 0
				+ 16 
				+ 4 + sizeof WCHAR * (1 + p.GetTitleLocked().GetLength())
				+ 4 + sizeof WCHAR * (1 + p.GetTextLocked ().GetLength())
				+ 0;

			UINT position	= buffer.GetSize();
			buffer.SetSize(len + position);
			byte* array		= buffer.GetData();

			// write id
			memcpy(array + position, &p.GetRemoteId(), 16);
			position += 16;

			CStringW title	= CT2W(p.GetTitleLocked());
			CStringW text	= CT2W(p.GetTextLocked());

			// write title
			len = sizeof WCHAR * (1 + title.GetLength());
			memcpy(array + position, &len, 4);
			position += 4;
			memcpy(array + position, title, len);
 			position += len;

			// write text
			len = sizeof WCHAR * (1 + text.GetLength());
			memcpy(array + position, &len, 4);
			position += 4;
			memcpy(array + position, text, len);
			position += len;
		}

		int returnCode = -1;
		ExceptionCode exceptionCode = ExceptionCodeOk;
		try
		{
			CAutoPtr<CHttpConnection> connection;
			connection.Attach(a_session->GetHttpConnection(SERVER_ADDRESS, 0, SERVER_PORT));
			if(!connection)
				throw ConnectionError;

			CAutoPtr<CHttpFile> request;
			request.Attach(connection->OpenRequest(_T("POST"), CString(APP_DIR) + UPDATEDETAILS_QUERY_STRING));

			if(!request)
				throw ConnectionError;

			if(!request->SendRequest(CString(_T("")), buffer.GetData(), buffer.GetSize()))
				throw CommunicationError;

			UINT responseVersion;
			STATIC_ASSERT(sizeof responseVersion == 4);
			if(4 != request->Read(&responseVersion, 4))
				throw CommunicationError;
			if(responseVersion != 1)
				throw ProtocolError;

			STATIC_ASSERT(sizeof returnCode == 4);
			if(4 != request->Read(&returnCode, 4))
				throw CommunicationError;
			if(-2 == returnCode)
				throw NotLoggedIn;

			if(request)
				request->Close();
			if(connection)
				connection->Close();
		}
		catch(CInternetException*)
		{
			exceptionCode = CommunicationError;
		}
		catch(ExceptionCode code)
		{
			exceptionCode = code;
		}

		if(returnCode == 0)
			for(int i=0;i<dirtyPhotos.GetCount();i++)
			{
				dirtyPhotos[i]->ClearDirty();
			}


		if(exceptionCode != ExceptionCodeOk)
			throw exceptionCode;
	}

	::SendMessage(hwndNotify, WM_UPLOAD_GETDIRTYPHOTOTEXT, reinterpret_cast<WPARAM>(&dirtyPhotos), 1);
}

void UploadManager::RunDetailsUpdate()
{
	lastDetailsUpdate = true;
	VERIFY(wakeEvent.PulseEvent());
}

//Called from worker-thread
UINT UploadManager::ThreadProc()
{
	CInternetSession session(HTTP_AGENT);
	int consecutiveCommunicationErrors = 0;
	static const int MAX_COMMUNICATION_ERRORS = 3;

	while(true
		&& (!shouldTerminate || lastDetailsUpdate)
		&& consecutiveCommunicationErrors<MAX_COMMUNICATION_ERRORS
		)
	{
		UploadEndedResult result = UploadEndedResult::UploadEndSuccess;
		status.Reset();
		try
		{
			bool hasJobs;
			{	CSingleLock lock(&lockObject, TRUE);
				hasJobs = 0<queue.GetCount();
				status.AddQueue(queue.GetCount());
			}

			if(!shouldTerminate && uploadState && hasJobs)
			{
				status.StartProcess();

				//do as many jobs as we can right now.
				while(!abort)
				{
					try {
						if(!InitSession(&session)) {
							result = UploadEndedResult::UploadCancelled;
							//break from uploading job, but not from updating photo texts.
							break;
						}
					} catch (ExceptionCode code) {
						if(code & ERRORFLAG_NETWORK)
							::MessageBox(
								this->hwndNotify,
								R2T(IDS_MAIN_UPLOADRESULT_NETWORKERROR),
								R2T(IDS_MAIN_UPLOADINGCAPTION),
								MB_OK|MB_ICONERROR
								);
						throw;
					}

					UploadCategory(&session);

					PhotoReference* job;
					try 
					{
						if(0!=(job = GetNext()))
						{
							UploadJob(job, &session);
							consecutiveCommunicationErrors = 0;
						}
						else
							break;
					}
					catch (ExceptionCode code) 
					{
						if( code & ERRORFLAG_REQUE_JOB )
						{
							CSingleLock lock(&lockObject, TRUE);
							ASSERT(job);
							this->queue.InsertAt(0, job);
							job = null;
						}
						throw;
					}
				}
				
				if(result != UploadEndedResult::UploadCancelled)
					result = abort	?UploadEndedResult::UploadAborted
									:UploadEndedResult::UploadEndSuccess;
			}

			UpdateDirtyJobs(&session);
			lastDetailsUpdate = false;
		}
		catch(ExceptionCode code)
		{
			if(code == NotLoggedIn) {
				this->invalidCookie = true;
			}
			if(code == CommunicationError) {
				consecutiveCommunicationErrors++;
				result = UploadEndedResult::UploadNetworkError;
			}
			if(code & ERRORFLAG_ABORTPROCESS) {
				result = UploadEndedResult::UploadPermanentError;
			}
			if(code & ERRORFLAG_UPGRADECLIENT) {
				if(hwndNotify)
					::SendMessage(hwndNotify, WM_UPLOAD_OLDCLIENT, 0, 0);
			}
			else if(uploadState && !abort)
				continue;
		}
		catch(...) {
			result = UploadEndedResult::UploadPermanentError;
		}
		if(uploadState && hwndNotify) {
			uploadState = false;
			::SendMessage(hwndNotify, WM_UPLOAD_FINISH, reinterpret_cast<WPARAM>(&status), result);
			if(result == UploadEndedResult::UploadPermanentError)
				Abort();
		}
		if(!uploadState)
			wakeEvent.Lock(POLL_WAIT_TIME);
	}//(!shouldTerminate)
	return 0;
}

//Called from GUI-thread
void UploadManager::Enqueue(PhotoReference* photo)
{
	ASSERT(!_lockedSend);
	CSingleLock lock(&lockObject, TRUE);
	queue.Add(photo);
	status.AddQueue(1);
}

//Called from GUI-thread
bool UploadManager::Unenqueue(PhotoReference* photo)
{
	ASSERT(!_lockedSend);
	CSingleLock(&lockObject, TRUE);
	for(int i=0;i<queue.GetCount();i++)
		if(queue[i] == photo) {
			if(queue[i]->LockedStatus())
				return false;
			queue.RemoveAt(i);
			status.AddQueue(-1);
			return true;
		}
	return false;
}

//Called from worker-thread
PhotoReference* UploadManager::GetNext(void)
{
	CSingleLock(&lockObject, TRUE);
	ASSERT(++_lockedSend); //assign true
	for(int i=0;i<queue.GetCount();i++)
	{
		PhotoReference* ref = queue[i];
		UploadLockResult res = (UploadLockResult)
			((!hwndNotify)?(LockResult_Ok):SendMessage(hwndNotify, WM_UPLOAD_TRYLOCKITEM, reinterpret_cast<WPARAM>(ref), 0));

		queue.RemoveAt(i);
	ASSERT(--_lockedSend>=0); //pop
		return ref;
	}
	ASSERT(--_lockedSend>=0); //pop

	return 0;
}

bool UploadManager::HasJobs()
{
	ASSERT(!_lockedSend);
	CSingleLock(&lockObject, TRUE);
	return queue.GetCount()>0;
}

void UploadManager::GetJobs(CArray<PhotoReference*> &jobs)
{
	ASSERT(!_lockedSend);
	CSingleLock(&lockObject, TRUE);
	queue.Copy(jobs);
}







/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//  CCCCC LL        AA     SSSSS   SSSSS                                   //
// CC     LL       AAAA   SS      SS                                       //
// CC     LL      AA  AA   SSSSS   SSSSS                                   //
// CC     LL     AAAAAAAA      SS      SS                                  //
//  CCCCC LLLLL AA      AA SSSSS   SSSSS                                   //
//                                                                         //
//                                                                         //
// class UploadManager::UploadStatus                                       //
//                                                                         //
// Created:     By:                                                        //
// 2005-03-21 - Hugo                                                       //
//                                                                         //
// COMMENTS:
//
/////////////////////////////////////////////////////////////////////////////


UploadManager::UploadStatus::UploadStatus()
{
	Reset();
}

void UploadManager::UploadStatus::Reset()
{
	ZeroMemory(this, sizeof UploadStatus);
}

void UploadManager::UploadStatus::Calculate()
{
	queuedBytesEstimate		= averageSize * queuedItems;
	percentEstimate			= UINT((finishedBytes/float(queuedBytesEstimate))*100);
	percentEstimate			= min(99, percentEstimate);

	totalTime				= ::GetTickCount() - this->processStartTick;
	if(totalTime && finishedBytes>1024)
	{
		UINT timePerKByte		= totalTime/(finishedBytes/1024);

		UINT remainingKBytes	= (queuedBytesEstimate-finishedBytes)/1024;
		remainingTime			= remainingKBytes*timePerKByte;

		minutesRemainingEstimate = (remainingTime/1000)/60;
		kBpS					= finishedBytes/totalTime;
		MBpM					= ((float(finishedBytes)/1024.0f)/1024.0f)/((float(totalTime)/1000.0f)/60.0f);
	}
}

void UploadManager::UploadStatus::AddQueue(int num)
{
	this->queuedItems += num; //num may be negative.
	ASSERT(this->queuedItems >= 0);

	Calculate();
}

void UploadManager::UploadStatus::FinishImage()
{
	this->finishedItems += 1;
	DWORD jobTime = ::GetTickCount() - jobStartTick;

	Calculate();
}

void UploadManager::UploadStatus::ProgressImage(int bytes)
{
	ASSERT(bytes>=0);
	this->finishedBytes += bytes;

	Calculate();
}

void UploadManager::UploadStatus::StartProcess()
{
	this->processStartTick = ::GetTickCount();
}

void UploadManager::UploadStatus::StartImage(int size)
{
	ASSERT(this->processStartTick);

	jobStartTick = ::GetTickCount();
	//recalculate average size.
	UINT totalStarted = size + averageSize * finishedItems;
	this->averageSize = UINT(totalStarted/(float)(finishedItems+1));

	Calculate();
}

