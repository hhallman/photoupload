// MinakortControls.h : main header file for the MinakortControls DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "MinakortControls_i.h"


// CMinakortControlsApp
// See MinakortControls.cpp for the implementation of this class
//

class CMinakortControlsApp : public CWinApp
{
public:
	CMinakortControlsApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()

public:
	static const int ReleaseVersion = 2;
};
