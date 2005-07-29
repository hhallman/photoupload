#pragma once
#include "afxcmn.h"
#include "ShareSettings.h"

class UserListControl : public CListCtrl
{
public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
public:
	void* GetParam_(int item)
	{
		LVITEM n;
		n.iItem = item;
		n.lParam = 0;
		n.stateMask = LVIF_PARAM;
		if(!GetItem(&n))
			return NULL;
		return reinterpret_cast<void*>(n.lParam);
	}
	BOOL SetParam_(int item, void* param)
	{
		LVITEM n;
		n.iItem = item;
		n.stateMask = LVIF_PARAM;
		n.lParam = reinterpret_cast<LPARAM>(param);
		return SetItem(&n);
	}
private:
	CComboBox* ShowInPlaceList( int nItem, int nCol, CStringList &lstItems, int nSel );
	int HitTestEx(CPoint & point, int * col) const;
	int GetColumnCount() const;
public:
	void DeleteSelected();
};

// ShareDialog dialog

class ShareDialog : public CDialog
{
	DECLARE_DYNAMIC(ShareDialog)

public:
	ShareDialog(const ShareSettings& settings, CWnd* pParent = NULL);   // standard constructor
	virtual ~ShareDialog();

// Dialog Data
	enum { IDD = IDD_SHARING_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	void UpdateCategoryName();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnNMDblclkSharelist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSharelist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnEndlabeleditSharelist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnItemActivateSharelist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedSharetofriends();
	afx_msg void OnEnKillfocusCategoryname();
	afx_msg void OnLvnDeleteitemSharelist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedOk();

public:
	void GetShareSettings(ShareSettings& settings) const { settings.Copy(loadSettings); }

private:
	void SaveShareSettings(ShareSettings& settings) const;
	ShareSettings loadSettings;
	UserListControl shareList;
};
