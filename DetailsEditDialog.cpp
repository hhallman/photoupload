#include "StdAfx.h"
#include ".\detailseditdialog.h"
#include ".\style.h"
#include ".\defines.h"
#include ".\resource.h"
#include ".\PhotoReference.h"
#include ".\Utility.h"

using namespace Gdiplus;

#define DID_TITLE1	1001
#define DID_TEXT1	1002
#define DID_TITLE2	1003
#define DID_TEXT2	1004

#define GETNEXTINDEX(index)		((index)|GETDETAILS_NEXTFLAG)
#define GETPREVIOUSINDEX(index)	((index)|GETDETAILS_PREVIOUSFLAG)

DetailsEditDialog::DetailsEditDialog(void)
{
	returnFocusWindow	= NULL;
	imageList			= NULL;
	itemIndex			= -1;
	ref1 = ref2			= NULL;
}

DetailsEditDialog::~DetailsEditDialog(void)
{
	if(m_editFont)
		Style::Release(m_editFont);
}

BEGIN_MESSAGE_MAP(DetailsEditDialog, CWnd)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_MESSAGE(WM_CTLCOLOREDIT, OnCtrlColorEdit)
	ON_MESSAGE(WM_CTLCOLORSTATIC, OnDisabledCtrlColorEdit)
	ON_NOTIFY(WM_CHAR, DID_TITLE1, OnCtrlKeyDown)
	ON_NOTIFY(WM_CHAR, DID_TITLE2, OnCtrlKeyDown)
	ON_NOTIFY(WM_CHAR, DID_TEXT1,  OnCtrlKeyDown)
	ON_NOTIFY(WM_CHAR, DID_TEXT2,  OnCtrlKeyDown)
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(NotifyingEdit, CEdit)
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

void NotifyingEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch(nChar)
	{
	case VK_RETURN:
		if(ES_MULTILINE&GetStyle())
			break;
	case VK_UP:
	case VK_DOWN: {
		int lines = GetLineCount();
		if(false
			|| lines == 1
			|| (nChar == VK_UP   && HIWORD(GetSel()) <= LineLength(0))
			|| (nChar == VK_DOWN && HIWORD(GetSel()) >= LineIndex(lines-1))
			)
		{
			OnChar(nChar, nRepCnt, nFlags);
			return;
		}
				  }
	}

	CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}

void NotifyingEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	ASSERT(GetParent());
	ASSERT(GetDlgCtrlID());
	
	bool process = true;
	switch(nChar)
	{
	case VK_RETURN: {

		if(ES_WANTRETURN&GetStyle())
		// If multiline, check that we are on the last line, and that it is empty.
		if(ES_MULTILINE&GetStyle()) {
			int n			= GetLineCount();
			int pos			= LOWORD(GetSel());
			int lastIndex	= LineIndex(n-1);
			if(pos<lastIndex) {
			//	ReplaceSel("\r\n", TRUE);
				break;
			}
			TCHAR s[4];
			GetLine(n-1,s, 4);
			if(s[0] != _T('\0')) {
			//	ReplaceSel("\r\n", TRUE);
				break;
			}
			if(0xA000 & ::GetKeyState(VK_LSHIFT))
				break;
		}
					}
	case VK_UP:
	case VK_DOWN:
	case VK_TAB:
		{
		NMKEY nm;
		nm.hdr.code		= WM_CHAR;
		nm.hdr.hwndFrom	= this->GetSafeHwnd();
		nm.hdr.idFrom	= this->GetDlgCtrlID();
		nm.nVKey		= nChar;
		nm.uFlags		= nFlags;
		process 		= 0==GetParent()->SendMessage(WM_NOTIFY, static_cast<WPARAM>(nm.hdr.idFrom), reinterpret_cast<LPARAM>(&nm));
				}
	default:
		break;
	}

	if(process)
		CEdit::OnChar(nChar, nRepCnt, nFlags);
}

int DetailsEditDialog::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	ASSERT(GetParent());
	ASSERT(GetDlgCtrlID());
	
	DWORD style		= ES_MULTILINE|WS_VISIBLE|WS_BORDER|ES_AUTOVSCROLL;
	DWORD textStyle	= ES_MULTILINE|ES_WANTRETURN|style;
	int blockY	= 40;
	title1.Create(style,		CRect(240,blockY+10,465,blockY+40), this, DID_TITLE1);
	text1.Create( textStyle,	CRect(240,blockY+60,465,blockY+160), this, DID_TEXT1);
	
	blockY += 210;
	title2.Create(style,		CRect(240,blockY+10,465,blockY+40), this, DID_TITLE2);
	text2.Create( textStyle,	CRect(240,blockY+60,465,blockY+160), this, DID_TEXT2);

	editArea = CRect(240, 50, 465, blockY+160);

	title1.LimitText(PhotoReference::MAX_PHOTO_NAME_LEN);
	title2.LimitText(PhotoReference::MAX_PHOTO_NAME_LEN);

	m_editFont = Style::Instance().GetEditFont();
	title1.SetFont(m_editFont);
	title2.SetFont(m_editFont);
	text1.SetFont(m_editFont);
	text2.SetFont(m_editFont);

	return 0;
}

void DetailsEditDialog::OnPaint()
{
	CPaintDC	dc		(this);
//	Graphics	final	(dc.m_hDC);
	Graphics	g		(dc.m_hDC);
	CRect		rect	;GetClientRect(rect);

	//TODO: draw the images natively
//	Metafile backgroundBuffer(dc.m_hDC);
//	Graphics g(&backgroundBuffer);

	//Draw background
	{
		Brush* brush = Style::Instance().GetDefaultBodyBrush(rect);
		g.FillRectangle(brush, Rect(0,0,rect.right,rect.bottom));
		Style::Release(brush);
	}

	//Draw main border
	{
		Pen* borderPen = Style::Instance().GetDefaultBodyBorderPen();
		Rect borderRect(1,1,rect.right-2,rect.bottom-2);
		g.DrawRectangle(borderPen, borderRect);
		Style::Release(borderPen);
	}

	//Draw labels
	{
		g.SetTextRenderingHint(TextRenderingHintAntiAlias);
		Font*	labelFont	= Style::Instance().GetDefaultLabelFont();
		Brush*	labelBrush	= Style::Instance().GetDefaultLabelBrush();
		StringFormat format;
		int blockY	= 30;
		int blockX	= 240;
		g.DrawString(R2W(IDS_EDIT_NAME), -1, labelFont, PointF(blockX+2, blockY+5), labelBrush);
		g.DrawString(R2W(IDS_EDIT_TEXT), -1, labelFont, PointF(blockX+2, blockY+55), labelBrush);
		
		blockY += 210;
		g.DrawString(R2W(IDS_EDIT_NAME), -1, labelFont, PointF(blockX+2, blockY+5), labelBrush);
		g.DrawString(R2W(IDS_EDIT_TEXT), -1, labelFont, PointF(blockX+2, blockY+55), labelBrush);

		Style::Release(labelFont);
		Style::Release(labelBrush);
	}

	if(ref1 || ref2)
	{
		if(ref1) {
			Rect r(27,27,198,198);
			Brush* brush = Style::Instance().GetDetailsPreviewBackgroundBrush(r);
			g.FillRectangle(brush, r);
			Style::Release(brush);
		}
		if(ref2) {
			Rect r(27,237,198,198);
			Brush* brush = Style::Instance().GetDetailsPreviewBackgroundBrush(r);
			g.FillRectangle(brush, r);
			Style::Release(brush);
		}
	}

	//Draw previews
	if(imageList)
	{
		if(this->ref1 && ref1->imageIndex >= 0)
		{
			DWORD finishedStyle = ref1->Finished()?INDEXTOOVERLAYMASK(FINISHED_OVERLAY):ref1->LockedStatus()?INDEXTOOVERLAYMASK(RUNNING_OVERLAY):0;
			imageList->DrawEx(&dc, ref1->imageIndex, CPoint(30,30), CSize(192,192), CLR_NONE,CLR_NONE,ILD_NORMAL|finishedStyle);
		}
		if(this->ref2 && ref2->imageIndex >= 0)
		{
			DWORD finishedStyle = ref2->Finished()?INDEXTOOVERLAYMASK(FINISHED_OVERLAY):ref2->LockedStatus()?INDEXTOOVERLAYMASK(RUNNING_OVERLAY):0;
			imageList->DrawEx(&dc, ref2->imageIndex, CPoint(30,240), CSize(192,192), CLR_NONE,CLR_NONE,ILD_NORMAL|finishedStyle);
		}
	}

//	final.DrawImage(&backgroundBuffer, 0, 0);
}

LRESULT DetailsEditDialog::OnCtrlColorEdit(WPARAM wParam, LPARAM lParam)
{
	HDC hdc		= reinterpret_cast<HDC>(wParam);
	HWND hwnd	= reinterpret_cast<HWND>(lParam);
	::SetTextColor(hdc, Style::Instance().GetDefaultEditColor());
	::SetBkColor(hdc, Style::Instance().GetDefaultEditBackground());
	return reinterpret_cast<LRESULT>(Style::Instance().GetDefaultEditPermanentBrush());
}

LRESULT DetailsEditDialog::OnDisabledCtrlColorEdit(WPARAM wParam, LPARAM lParam)
{
	HDC hdc		= reinterpret_cast<HDC>(wParam);
	HWND hwnd	= reinterpret_cast<HWND>(lParam);
	::SetTextColor(hdc, Style::Instance().GetDefaultEditDisabledColor());
	::SetBkColor(hdc, Style::Instance().GetDefaultEditDisabledBackground());
	return reinterpret_cast<LRESULT>(Style::Instance().GetDefaultEditDisabledPermanentBrush());
}

void DetailsEditDialog::OnCtrlKeyDown(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMKEY* p		= reinterpret_cast<NMKEY*>(pNMHDR);
	bool goNext		= false;
	bool goPrevious	= false;
	if(p->nVKey == VK_UP)
		goPrevious = true;
	if(p->nVKey == VK_DOWN)
		goNext = true;
	if(p->nVKey == VK_TAB || p->nVKey == VK_RETURN)
	{
		if(0xA000 & ::GetKeyState(VK_LSHIFT))
			goPrevious = true;
		else
			goNext = true;
	}
	if(goNext)
	{
		if(p->hdr.idFrom == DID_TITLE1)
			Utility::Focus(text1);
		else if(p->hdr.idFrom == DID_TEXT1 && ref2)
			Utility::Focus(title2);
		else if(p->hdr.idFrom == DID_TITLE2)
			Utility::Focus(text2);
		else if(p->hdr.idFrom == DID_TEXT2)
		{
			SaveData();
			this->itemIndex = GETNEXTINDEX(this->itemIndex);
			UpdateItems();
			if(!ref2)
				Utility::Focus(title1);
			else
				Utility::Focus(title2);
		}
	}
	else if(goPrevious)
	{
		if(p->hdr.idFrom == DID_TEXT1)
			Utility::Focus(title1);
		else if(p->hdr.idFrom == DID_TITLE2)
			Utility::Focus(text1);
		else if(p->hdr.idFrom == DID_TEXT2)
			Utility::Focus(title2);
		else if(p->hdr.idFrom == DID_TITLE1)
		{
			SaveData();
			this->itemIndex = GETPREVIOUSINDEX(this->itemIndex);
			UpdateItems();
			Utility::Focus(title1);
		}
	}

	if(goPrevious || goNext)
		SendScrollMessage();

	*pResult = 1;
}


void DetailsEditDialog::SetItemIndex(int index)
{
	if(this->itemIndex != index)
	{
		this->itemIndex = index;
		Invalidate();
	}
}

void DetailsEditDialog::SendScrollMessage()
{
	ASSERT(GetParent());
	ASSERT(GetDlgCtrlID());

	DetailsEditDialog::NMGetItem nm;
	nm.hdr.code		= DETAILS_NOTIFY_SCROLL;
	nm.hdr.hwndFrom	= this->GetSafeHwnd();
	nm.hdr.idFrom	= this->GetDlgCtrlID();
	nm.index		= this->itemIndex;

	LRESULT res		= GetParent()->SendMessage(WM_NOTIFY, static_cast<WPARAM>(nm.hdr.idFrom), reinterpret_cast<LPARAM>(&nm));
}

void DetailsEditDialog::Show(const CString& autoAppend)
{
	ASSERT(itemIndex >= 0);
	ASSERT(imageList);
	ref1 = ref2 = NULL;
	UpdateItems();
	ShowWindow(SW_SHOW);
	SetWindowPos(&wndTopMost, 0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
	returnFocusWindow = title1.SetFocus();
	CString text;
	title1.GetWindowText(text);
	if(text.GetLength()==0) {
		text += autoAppend;
		title1.SetWindowText(text);
	}
	title1.SetSel(text.GetLength(),text.GetLength());
	SetCapture();
}

void DetailsEditDialog::Hide(void)
{
	ReleaseCapture();
	ShowWindow(SW_HIDE);
	SaveData();
	ref1 = ref2 = NULL;
	if(returnFocusWindow)
		returnFocusWindow->SetFocus();
	returnFocusWindow = NULL;
}

void DetailsEditDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	CRect rect = this->editArea;

	if(rect.PtInRect(point))
		ReleaseCapture();
	else
		SetCapture();

	//GetWindowRect(rect);
	//ClientToScreen(&point);
	//if(!rect.PtInRect(point))
	//{
	//	GetParent()->GetWindowRect(rect);
	//	if(rect.PtInRect(point))
	//		::SetCursor((HCURSOR)IDC_HAND);
	//	else
	//		SetCursor((HCURSOR)IDC_ARROW);
	//}
	//else
	//	SetCursor((HCURSOR)IDC_ARROW);

	CWnd::OnMouseMove(nFlags, point);
}

void DetailsEditDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRect rect;
	GetWindowRect(rect);
	ClientToScreen(&point);
	if(!rect.PtInRect(point))
		Hide();

	CWnd::OnLButtonDown(nFlags, point);
}


void DetailsEditDialog::UpdateItems(void)
{
	ASSERT(GetParent());
	ASSERT(GetDlgCtrlID());
	
	/////////////////////////////////////////////////////////////////////////////
	// Retreive items from parent
	DetailsEditDialog::NMGetItem nm;
	nm.hdr.code		= DETAILS_NOTIFY_GETITEM;
	nm.hdr.hwndFrom	= this->GetSafeHwnd();
	nm.hdr.idFrom	= this->GetDlgCtrlID();
	nm.index		= this->itemIndex;

	LRESULT res		= GetParent()->SendMessage(WM_NOTIFY, static_cast<WPARAM>(nm.hdr.idFrom), reinterpret_cast<LPARAM>(&nm));
	ref1			= nm.reference;
	this->itemIndex	= nm.index;

	nm.index		= GETNEXTINDEX(nm.index);	// negative value signals next from 0-index
	res				= GetParent()->SendMessage(WM_NOTIFY, static_cast<WPARAM>(nm.hdr.idFrom), reinterpret_cast<LPARAM>(&nm));
	ref2			= nm.reference;

	/////////////////////////////////////////////////////////////////////////////
	//  Update view
	title1	.EnableWindow(ref1?TRUE:FALSE);
	text1	.EnableWindow(ref1?TRUE:FALSE);
	title2	.EnableWindow(ref2?TRUE:FALSE);
	text2	.EnableWindow(ref2?TRUE:FALSE);
	title1	.SetWindowText(ref1?ref1->GetTitle():_T(""));
	text1	.SetWindowText(ref1?ref1->GetText() :_T(""));
	title2	.SetWindowText(ref2?ref2->GetTitle():_T(""));
	text2	.SetWindowText(ref2?ref2->GetText()	:_T(""));

	Invalidate();
}

void DetailsEditDialog::SaveData(void)
{
	ASSERT(GetParent());
	ASSERT(GetDlgCtrlID());

	/////////////////////////////////////////////////////////////////////////////
	// Save changes to parent.
	DetailsEditDialog::NMGetItem nm;
	nm.hdr.code		= DETAILS_NOTIFY_ITEMUPDATED;
	nm.hdr.hwndFrom	= this->GetSafeHwnd();
	nm.hdr.idFrom	= this->GetDlgCtrlID();
	nm.index		= this->itemIndex;

	if(ref1)
	{
		CString g;
		title1.GetWindowText(g);
		ref1->SetTitle(g);
		text1.GetWindowText(g);
		ref1->SetText(g);

		GetParent()->SendMessage(WM_NOTIFY, static_cast<WPARAM>(nm.hdr.idFrom), reinterpret_cast<LPARAM>(&nm));
	}
	if(ref2)
	{
		CString g;
		title2.GetWindowText(g);
		ref2->SetTitle(g);
		text2.GetWindowText(g);
		ref2->SetText(g);

		nm.index = this->itemIndex + 1;
		GetParent()->SendMessage(WM_NOTIFY, static_cast<WPARAM>(nm.hdr.idFrom), reinterpret_cast<LPARAM>(&nm));
	}

}


