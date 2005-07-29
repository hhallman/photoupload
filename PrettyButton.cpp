#include "stdafx.h"
#include ".\PrettyButton.h"
#include ".\Defines.h"
#include ".\Style.h"

using namespace Gdiplus;

CPrettyButton::CPrettyButton()
{
	forceHighlight = false;
}

CPrettyButton::~CPrettyButton()
{
}


int CPrettyButton::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CButton::OnCreate(lpCreateStruct) == -1)
		return -1;
	this->ModifyStyle(0,BS_OWNERDRAW);
	return 0;
}

void CPrettyButton::PreSubclassWindow()
{
	this->ModifyStyle(0,BS_OWNERDRAW);

	CButton::PreSubclassWindow();
}

BEGIN_MESSAGE_MAP(CPrettyButton, CButton)
	ON_WM_CREATE()
END_MESSAGE_MAP()


void CPrettyButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	// This code only works with buttons.
	ASSERT(lpDrawItemStruct->CtlType == ODT_BUTTON);

	Style& style					= Style::Instance();

	CString		text				;GetWindowText(text);
	CDC			tempDC				;tempDC.Attach(lpDrawItemStruct->hDC);
	Graphics	g					(tempDC);//(lpDrawItemStruct->hDC);
	Color		borderColor			= style.GetButtonBorderColor();
	Color		bevelTopColor		= style.GetButtonBevelTopColor();
	Color		bevelBottomColor	= style.GetButtonBevelBottomColor();
	Pen			borderPen			(borderColor);
	CRect		clientRect			;GetClientRect(clientRect);
	int			fontPointSize		= text.GetLength()>10?7:10;//TODO: trim!
	Font		textFont			(L"Trebuchet MS", fontPointSize, FontStyleBold);
	RectF		fClientRect			(0,0,clientRect.right,clientRect.bottom);

	bool invert = forceHighlight||(lpDrawItemStruct->itemState & ODS_SELECTED);

	//Draw background and border
	StyleObject<Brush> backgroundBrush = style.GetButtonBackground(clientRect);
	g.FillRectangle(backgroundBrush,fClientRect);
	RectF borderRect = fClientRect;
	borderRect.Width--;
	borderRect.Height--;
	g.DrawRectangle(&borderPen, borderRect);
	borderPen.SetColor(invert?bevelBottomColor:bevelTopColor);
	Point ps[] = { Point(1,1), Point(clientRect.right-2,1), Point(1,1), Point(1, clientRect.bottom-2) };
	g.DrawLines(&borderPen, ps, 4);
	borderPen.SetColor(invert?bevelTopColor:bevelBottomColor);
	Point ps2[] = { Point(clientRect.right-2,1), Point(clientRect.right-2,clientRect.bottom-2), Point(1, clientRect.bottom-2), Point(clientRect.right-2,clientRect.bottom-2) };
	g.DrawLines(&borderPen, ps2, 4);


	//Draw text
	g.SetTextRenderingHint(TextRenderingHintAntiAlias);
	StringFormat format;
	format.SetAlignment(StringAlignmentCenter);
	format.SetLineAlignment(StringAlignmentCenter);
	StyleObject<Brush> textBrush = style.GetButtonTextBrush();
	g.DrawString(CT2W(text), text.GetLength(), &textFont, fClientRect, &format, textBrush);
}
