#pragma once
#include "afxwin.h"

class CPrettyListCtrl : public CListCtrl
{
protected:
virtual LRESULT WindowProc(
   UINT message,
   WPARAM wParam,
   LPARAM lParam 
	);
};
