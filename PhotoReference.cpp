#include "StdAfx.h"
#include ".\Defines.h"
#include ".\photoreference.h"


PhotoReference::PhotoReference(const CString& path)
{
	ASSERT(ownerThread = ::GetCurrentThreadId());

	locked		= false;
	finished	= false;
	ZeroMemory(&this->remoteId, sizeof(this->remoteId));

	this->path = path;
	this->path.Trim();
	this->path.FreeExtra();

	::GetFileTitle(path, title.GetBuffer(MAX_PHOTO_NAME_LEN),MAX_PHOTO_NAME_LEN);
	title.ReleaseBuffer();
	int n=0,last=0;
	while(-1 != (n = title.Find(_T('.'),last))) last=n+1;
	if(last != -1)
		title = title.Left(last-1);
	title.Trim();
	title = title.Left(MAX_PHOTO_NAME_LEN);

	//check that title isn't too many digits:
	int digits = 0;
	for(int i=0;i<title.GetLength();i++)
	{
		TCHAR c = title[i];
		if(c >= _T('0') && c<=_T('9'))
			digits++;
	}

	if(digits >= MAX_DIGITS_TITLE)
		title.Empty();

	title.FreeExtra();
}

PhotoReference::~PhotoReference(void)
{
	PR_ASSERTOWNER;
}

void PhotoReference::Lock()
{
	ASSERT(ownerThread == ::GetCurrentThreadId());
	this->locked = true;
	this->lockedUpdateTitle	= this->title;
	this->lockedUpdateText	= this->text;
}

void PhotoReference::Unlock()
{
	ASSERT(ownerThread == ::GetCurrentThreadId());
	this->locked = false;
	this->SetTitle(this->lockedUpdateTitle);
	this->SetText(this->lockedUpdateText);
	this->lockedUpdateTitle.Empty();
	this->lockedUpdateText.Empty();
}

void PhotoReference::SetTitle(LPCTSTR carg)
{
//	PR_ASSERTOWNER;

	CString arg = carg;
	arg.Trim();
	arg.Left(MAX_PHOTO_NAME_LEN);

	if(LockedStatus()) {
		this->lockedUpdateTitle = arg;
		return;
	}

	if(this->title != arg) {
		dirty = true;
		this->title = arg;
		this->title.FreeExtra();
	}
}

void PhotoReference::SetText(LPCTSTR carg)
{
//	PR_ASSERTOWNER;

	CString arg = carg;
	arg.Trim();

	if(LockedStatus()) {
		this->lockedUpdateText = arg;
		return;
	}

	if(this->text != arg) {
		dirty = true;
		this->text = arg;
		this->text.FreeExtra();
	}
}

