#pragma once

class Log
{
private:
	Log(void);
	~Log(void);
public:
	static void LogUsage(LPCTSTR usage, ...);
	static CString GetUsageString();
};
