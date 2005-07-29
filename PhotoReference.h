#pragma once



class PhotoReference
{
public:
	PhotoReference(const CString& path);
	virtual ~PhotoReference(void);
private:
	CString path;
	CString title;
	CString text;
	CString keywords;
	bool	dirty;
	GUID	remoteId;
	CString	uploadResultMessage;
	bool	finished;
	bool	locked; //must only be set from gui thread.
	CString lockedUpdateTitle;
	CString lockedUpdateText;
public:

#ifdef DEBUG
	DWORD ownerThread;
#endif
#define PR_ASSERTOWNER ASSERT(ownerThread == ::GetCurrentThreadId())

	static const UINT MAX_DIGITS_TITLE		= 4;
	static const UINT MAX_PHOTO_NAME_LEN	= 50;//sync with db

	bool LockedStatus()			const { return locked; }
	void Lock();
	void Unlock();
	bool Finished()				const { return finished; }
	void SetFinished() {
		PR_ASSERTOWNER;
#ifdef DEBUG
		byte z[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
		ASSERT(memcmp(z,&remoteId,16) != 0);
#endif
		this->dirty		= false;
		this->finished	= true;
	}

	const CString& GetPath()	const {
	//	PR_ASSERTOWNER;
		return path; 
	}

	// the worker thread gets lockedUpdateTitle, which is ok so long as it doesn't try to
	// write to it.
	const CString& GetTitle()	const {
	// We don't need to lock here, since the bg-thread don't change these, and
	// the assertions is made on GetPath.
	//	PR_ASSERTOWNER;
		if(LockedStatus())
			return lockedUpdateTitle;
		return title;
	}

	const CString& GetText()	const {
	// see GetTitle:
	//	PR_ASSERTOWNER;
		if(LockedStatus())
			return lockedUpdateText;
		return text;
	}

	const CString& GetTitleLocked() const {
		PR_ASSERTOWNER;
		return title;
	}
	const CString& GetTextLocked() const {
		PR_ASSERTOWNER;
		return text;
	}

	const GUID& GetRemoteId() const {
		return this->remoteId;
	}

	void SetTitle(LPCTSTR);

	void SetText(LPCTSTR);

	bool GetDirty() {
		PR_ASSERTOWNER;
		return dirty;
	}

	void ClearDirty() {
		PR_ASSERTOWNER;
		this->dirty = false; 
	}

	void SetRemoteId(GUID photoId) {
		PR_ASSERTOWNER;
#ifdef DEBUG
		byte z[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
		ASSERT(memcmp(z,&remoteId,16)	== 0);
		ASSERT(memcmp(z,&photoId,16)	!= 0);
#endif
		memcpy(&this->remoteId, &photoId, 16);
	}

	void SetUploadResultMessage(const LPCTSTR msg) {
		PR_ASSERTOWNER;
		this->uploadResultMessage = msg;
	}


public:
	int		imageIndex;
};
