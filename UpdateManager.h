#pragma once

class UpdateManager
{
public:
	UpdateManager(void);
	virtual ~UpdateManager(void);

public:
	static const CString GetVersionsQueryString();
	HRESULT StartUpdate(/*GUID *newClsId = 0*/);

private:
	static UINT UpdateProc(LPVOID);
};

