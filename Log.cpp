#include "StdAfx.h"
#include ".\log.h"
	
static CCriticalSection lockObject;
static CString logString;

Log::Log(void)
{
}

Log::~Log(void)
{
}

void Log::LogUsage(LPCTSTR usage, ...)
{
	CString log;

	try {
		va_list args;
		va_start(args, usage);
		CString s;
		s.FormatV(usage, args);
		va_end(args);

		static DWORD firstUseTickCount	= ::GetTickCount();
		static DWORD lastUseTickCount	= ::GetTickCount();
		DWORD time						= ::GetTickCount() - firstUseTickCount;
		DWORD timeDelta					= lastUseTickCount - lastUseTickCount;
		lastUseTickCount				= time;
		log.Format(_T("%u (%u): %s\r\n"), time/1000, timeDelta, s);
	} catch(...) {
		log = "Exception in Log::LogUsage()\r\n";
	}

	CSingleLock lock(&lockObject, TRUE);
	logString += log;

	return;
}

CString Log::GetUsageString()
{
	CSingleLock lock(&lockObject, TRUE);
	CString copy = static_cast<LPCTSTR>(logString);
	ASSERT(logString.GetLength() == 0 || copy.GetString() != logString.GetString());
	return copy;
}


