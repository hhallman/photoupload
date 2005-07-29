#pragma once

class Utility
{
private:
	Utility(void);
	~Utility(void);
public:
	static const CString GetMyPicturesPath();
	static bool OpenMyPictures();
	static void SetOverlay(CListCtrl& view, int index, int overlay);
	static CString GetLocalSettingsPath();
		
	static void Focus(CEdit& wnd)
	{
		wnd.SetFocus();
		CString text;
		wnd.GetWindowText(text);
		wnd.SetSel(text.GetLength(),text.GetLength());
	}

	static int FindItem(CListCtrl& view, LPARAM param)
	{
		LVITEM item;
		ZeroMemory(&item, sizeof(item));
		item.mask = LVIF_PARAM;
		for(int i=0;i<view.GetItemCount();i++)
		{
			item.iItem = i;
			if(!view.GetItem(&item))
				return -1;
			if(item.lParam == param)
				return i;
		}
		return -1;
	}

	static HRESULT OverrideClassesRoot(HKEY hKeyBase, LPCTSTR szOverrideKey);
	static BOOL IsUserAdmin();

};

CString EscapeUrl(const CString query);