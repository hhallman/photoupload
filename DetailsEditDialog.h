#pragma once
#include "afxwin.h"

//Sent as WM_COMMAND. LPARAM = DetailsEditDialog::NMGetItem*
#define DETAILS_NOTIFY_GETITEM		1
#define DETAILS_NOTIFY_ITEMUPDATED	2
#define DETAILS_NOTIFY_SCROLL		3

#define GETDETAILS_NEXTFLAG			0x00100000
#define GETDETAILS_PREVIOUSFLAG		0x00200000
#define GETDETAILS_VALUEMASK		0x000FFFFF

class PhotoReference;

class NotifyingEdit : public CEdit
{
public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
};

class DetailsEditDialog :
	public CWnd
{
public:
	DetailsEditDialog(void);
	virtual ~DetailsEditDialog(void);
private:
	NotifyingEdit title1;
	NotifyingEdit text1;
	NotifyingEdit title2;
	NotifyingEdit text2;
	CRect editArea;
	CFont* m_editFont;
	CImageList* imageList;
	int itemIndex;
	CWnd* returnFocusWindow;
	PhotoReference* ref1;
	PhotoReference* ref2;
	int imageIndex1;
	int imageIndex2;
private:
	void UpdateItems(void);
	void SaveData(void);
	void SendScrollMessage(void);
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg LRESULT OnCtrlColorEdit(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDisabledCtrlColorEdit(WPARAM wParam, LPARAM lParam);
	afx_msg void OnCtrlKeyDown(NMHDR *pNMHDR, LRESULT *pResult);
public:
	void Show(const CString& autoAppend);
	void Hide();
	void SetImageList(CImageList* list) { this->imageList = list; }
	void SetItemIndex(int index);

	struct NMGetItem
	{
		NMGetItem() { ZeroMemory(this, sizeof(NMGetItem)); }
		NMHDR hdr;
		int index;	// negative value signals next from 0-index
		PhotoReference* reference;
	};
};


