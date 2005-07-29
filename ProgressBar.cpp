#include "StdAfx.h"
#include ".\progressbar.h"
#include ".\infopanel.h"
#include ".\style.h"

CProgressBar::CProgressBar(void)
{
	maxValue		= 100;
	currentValue	= 0;
	caption			= _T("");
}

CProgressBar::~CProgressBar(void)
{
}

int CProgressBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}


BEGIN_MESSAGE_MAP(CProgressBar, CWnd)
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_CREATE()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

using namespace Gdiplus;
void CProgressBar::OnPaint()
{
	int			borderWidth		= 2;					//Width of border between control and active-rect.
	CRect		rect			;GetClientRect(rect);
	CPaintDC	tempDC			(this);
	Graphics	g				(tempDC.m_hDC);

	g.SetSmoothingMode(Gdiplus::SmoothingModeNone);
	g.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias);

	StyleObject<Brush> backgroundBrush = Style::Instance().GetProgressbarBackgroundBrush();
	g.FillRectangle(backgroundBrush, RectF(rect.left,rect.top,rect.right,rect.bottom));

 	float drawWidth = (currentValue/(float)maxValue) * (rect.Width()-2*borderWidth);
	/////////////////////////////////////////////////////////////////////////////
	// Draw the active part:
	CRect activeBound(borderWidth,borderWidth,borderWidth+(int)drawWidth,rect.Height()-borderWidth*2);
	
	backgroundBrush = Style::Instance().GetProgressbarActiveBrush(activeBound);
	g.FillRectangle(backgroundBrush, RectF(activeBound.left,activeBound.top,activeBound.right-borderWidth,activeBound.bottom));

	/////////////////////////////////////////////////////////////////////////////
	// Draw caption
	StyleObject<Font> captionFont = Style::Instance().GetProgressbarCaptionFont();
	StringFormat format;
	format.SetLineAlignment(StringAlignmentCenter);
	RectF textSize;
 	g.MeasureString(
		CT2W(caption),
		caption.GetLength(),
		captionFont,
		RectF(borderWidth+5, borderWidth+2, rect.right-borderWidth-2, rect.bottom-borderWidth-2),
		&textSize
		);
	StyleObject<Brush> textBrush;
	RectF captionRect;
	if(textSize.GetRight() + 5 + 5 > activeBound.right) { 
		//Text to large to fit in active region.
		textBrush = Style::Instance().GetProgressbarAlternateCaptionBrush();
		captionRect = RectF(rect.left,rect.top,rect.right,rect.bottom);
		format.SetAlignment(StringAlignmentCenter);
	}
	else {
		textBrush = Style::Instance().GetProgressbarCaptionBrush();
		captionRect = RectF(borderWidth+5, borderWidth, rect.right-borderWidth, rect.bottom-borderWidth-2);
	}
	g.DrawString(
		CT2W(caption),
		caption.GetLength(),
		captionFont,
		captionRect,
		&format,
		textBrush);
}

void CProgressBar::OnMouseMove(UINT nFlags, CPoint point)
{
	CWnd::OnMouseMove(nFlags, point);
}

void CProgressBar::SetMaxValue(UINT max)
{
	this->maxValue = max;
	Invalidate();
}

void CProgressBar::SetCurrentValue(UINT c)
{
	this->currentValue = c;
	Invalidate();
}

void CProgressBar::SetCaption(const CString& caption)
{
	this->caption = caption;
	Invalidate();
}

void CProgressBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	int id = this->GetDlgCtrlID();
	if(id)
	{
		CWnd* parent	= this->GetParent();
		HWND hwnd		= this->GetSafeHwnd();
		LRESULT res		= parent->SendMessage(WM_COMMAND, MAKELONG(id, BN_CLICKED), (LPARAM)hwnd);
	}

	CWnd::OnLButtonUp(nFlags, point);
}
