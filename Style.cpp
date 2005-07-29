#include "StdAfx.h"
#include ".\Defines.h"
#include ".\style.h"
#include ".\resource.h"

using namespace Gdiplus;
#ifdef new
#undef new
#endif




/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//  CCCCC LL        AA     SSSSS   SSSSS                                   //
// CC     LL       AAAA   SS      SS                                       //
// CC     LL      AA  AA   SSSSS   SSSSS                                   //
// CC     LL     AAAAAAAA      SS      SS                                  //
//  CCCCC LLLLL AA      AA SSSSS   SSSSS                                   //
//                                                                         //
//                                                                         //
// class Style                                                             //
//                                                                         //
// Created:     By:                                                        //
// 2005-03-16 - Hugo                                                       //
//                                                                         //
// COMMENTS:
//
/////////////////////////////////////////////////////////////////////////////

Style::Style(void)
{
}

Style::~Style(void)
{
}

Style* Style::GetSetInstance(Style* installStyle)
{
	static Style* currentStyle = NULL;
	if(installStyle != NULL) {
		bool installed = false;
		if(currentStyle && currentStyle != installStyle) {
			currentStyle->OnUninstall();
			installed = true;
		}
		currentStyle = installStyle;
		if(installed)
			currentStyle->OnInstalled();
	}
	return currentStyle;
}


const static UINT wm_style_installed_message = ::RegisterWindowMessage(STR_WM_STYLE_UPDATED);
BOOL CALLBACK Style::_UpdateDisplayCallback(HWND wnd, LPARAM param)
{
	Style* style = reinterpret_cast<Style*>(param);


	::SendMessage	(wnd, wm_style_installed_message, reinterpret_cast<WPARAM>(style), 0);
	::InvalidateRect(wnd, NULL, TRUE);
	::UpdateWindow	(wnd);
	
	return TRUE;
}

void Style::UpdateDisplay(HWND mainWindow)
{
	_UpdateDisplayCallback(mainWindow, reinterpret_cast<LPARAM>(this));
	::EnumChildWindows(mainWindow, _UpdateDisplayCallback, reinterpret_cast<LPARAM>(this));
}

void Style::OnInstalled()
{
}

LPCWSTR Style::GetTopImageResourceName()
{
	return MAKEINTRESOURCEW(IDB_MINAKORT_TOP_LOGO);
}

Color Style::GetDarkColor()
{
	return Color(0x5A,0x61,0x34);
}

Color Style::GetHighlightColor()
{
	return Color(0xF0,0xF4,0xEC);
}

Brush* Style::GetDefaultBodyBrush(const CRect& bodyDimensions)
{
	Color		topColor	(0xB8,0xC5,0x97);
	Color		bottomColor	(0x9A,0xA9,0x75);
	LinearGradientBrush* brush = new LinearGradientBrush(Point(0,0), Point(bodyDimensions.right,bodyDimensions.bottom), topColor, bottomColor);
	return brush;
}

Pen* Style::GetDefaultBodyBorderPen()
{
	Color borderColor = GetDarkColor();
	Pen* borderPen = new Pen(borderColor, 2);
	return borderPen;
}

Font* Style::GetProgressbarCaptionFont()
{
	Font* captionFont = new Font(L"Trebuchet MS", 9, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
	return captionFont;
}
Brush* Style::GetProgressbarAlternateCaptionBrush()
{
	Color		textColor		(0xce,0xdb,0xc1);
	return new SolidBrush(textColor);
}

Brush* Style::GetProgressbarCaptionBrush()
{
	Color		textColor = GetDarkColor();
	return new SolidBrush(textColor);
}
Brush* Style::GetProgressbarBackgroundBrush()
{
	Color		backgroundColor = GetDarkColor();
	return new SolidBrush(backgroundColor);
}
Color Style::GetProgressbarActiveColor()
{
	return Color(0xCE, 0xDB, 0xC1);
}
Gdiplus::Brush* Style::GetProgressbarActiveBrush(const CRect& activeDimensions)
{
	Color		activeLeftColor	 = GetProgressbarActiveColor();
	Color		activeRightColor = GetHighlightColor();
	return new  LinearGradientBrush(Point(activeDimensions.left,activeDimensions.top), Point(activeDimensions.right,activeDimensions.top), activeLeftColor, activeRightColor);
}

Font* Style::GetDefaultLabelFont()
{
	Font* captionFont = new Font(L"Trebuchet MS", 9, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
	return captionFont;
}
Brush* Style::GetDefaultLabelBrush()
{
	Color		textColor	= GetDarkColor();
	return new SolidBrush(textColor);
}

CFont* Style::GetEditFont()
{
	CFont* font = new CFont();
	font->CreatePointFont(100, _T("Trebuchet MS"));
	return font;
}

/////////////////////////////////////////////////////////////////////////////
// Main window:
Brush* Style::GetMainTopBrush()
{
	Color topBackColor = GetHighlightColor();
	return new SolidBrush(topBackColor);
}

Pen* Style::GetMainTopLine()
{
	return new Pen(this->GetDarkColor(),2);
}

Font* Style::GetMainTitleFont()
{
	return new Font(L"Trebuchet MS", 32, 0, UnitPoint);
}

Font* Style::GetDefaultHeaderFont()
{
	return new Font(L"Georgia", 20, 0, UnitPoint);
}

COLORREF Style::GetDefaultEditBackground()
{
	return GetHighlightColor().ToCOLORREF();
}
HBRUSH Style::GetDefaultEditPermanentBrush()
{
	static CBrush brush(GetDefaultEditBackground());
	return brush;
}
COLORREF Style::GetDefaultEditColor()
{
	return this->GetDarkColor().ToCOLORREF();
}
COLORREF Style::GetDefaultEditDisabledBackground()
{
	return RGB(0xE1,0xDB,0xCC);
}
COLORREF Style::GetDefaultEditDisabledColor()
{
	return RGB(0x53,0x49,0x31);
}
HBRUSH Style::GetDefaultEditDisabledPermanentBrush()
{
	static CBrush brush(GetDefaultEditDisabledBackground());
	return brush;
}
Brush* Style::GetDetailsPreviewBackgroundBrush(const Rect& bounds)
{
	return new SolidBrush(Color(0xFF,0xFF,0xFF));
}


Brush* Style::GetTipWindowBackground(const CRect& dimensions)
{
	Color		topColor	(0xF3,0xEF,0xE9);
	Color		bottomColor	(0xce,0xc4,0xb6);
	return new LinearGradientBrush(Point(0,0), Point(dimensions.right, dimensions.bottom), topColor, bottomColor);
}

Pen* Style::GetTipWindowBorder()
{
	Color		borderColor	(0x69,0x4B,0x1F);
	return new Pen(borderColor, 2);
}

Brush* Style::GetTipWindowTextBrush()
{
	Color		textColor	(0x34,0x2D,0x1F);
	return new SolidBrush(textColor);
}


Brush* Style::GetButtonBackground(const CRect& clientRect)
{
	Color		topColor	(0xCB,0xC1,0xAD);
	Color		bottomColor	(0xB4,0xA4,0x83);
	return new LinearGradientBrush(PointF(0,0), PointF(clientRect.right,clientRect.bottom), topColor,bottomColor);
}

Brush* Style::GetButtonTextBrush()
{
	return 	new SolidBrush(Color(0x34,0x2D,0x1F));
}
Color Style::GetButtonBorderColor()
{
	return Color(0x69,0x4B,0x1F);
}
Color Style::GetButtonBevelTopColor()
{
	return Color(0xDC,0xD5,0xC7);
}
Color Style::GetButtonBevelBottomColor()
{
	return Color(0x70,0x66,0x51);
}

Brush* Style::GetImagePreviewBackground(const CRect& clientRect)
{
	Color		backgroundTopColor		(0xFD,0xFB,0xF7);
	Color		backgroundBottomColor	(0xFC,0xF8,0xEF);
	return new LinearGradientBrush(PointF(0,0), PointF(clientRect.right,clientRect.bottom), backgroundTopColor, backgroundBottomColor);
}


/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//  CCCCC LL        AA     SSSSS   SSSSS                                   //
// CC     LL       AAAA   SS      SS                                       //
// CC     LL      AA  AA   SSSSS   SSSSS                                   //
// CC     LL     AAAAAAAA      SS      SS                                  //
//  CCCCC LLLLL AA      AA SSSSS   SSSSS                                   //
//                                                                         //
//                                                                         //
// class BlueStyle                                                         //
//                                                                         //
// Created:     By:                                                        //
// 2005-03-16 - Hugo                                                       //
//                                                                         //
// COMMENTS:
//
/////////////////////////////////////////////////////////////////////////////

BlueStyle::BlueStyle()
{
}

BlueStyle::~BlueStyle()
{
}

LPCWSTR BlueStyle::GetTopImageResourceName()
{
	return MAKEINTRESOURCEW(IDB_MINAKORT_TOP_LOGO_BLUE);
}

void BlueStyle::Install()
{
	Style::GetSetInstance(GetTheBlueInstance());
}

void BlueStyle::OnUninstall()
{
}

Color BlueStyle::GetDarkColor()
{
	return Color(0x29,0x3C,0x45);
}

Color BlueStyle::GetHighlightColor()
{
	return Color(0xB6,0xCB,0xD6);
}
Color BlueStyle::GetProgressbarActiveColor()
{
	return Color(0x94,0xB2,0xC3);
}
Brush* BlueStyle::GetDefaultBodyBrush(const CRect& bodyDimensions)
{
	Color		topColor	(0x88,0xAA,0xD5);
	Color		bottomColor	(0x47,0x67,0x78);
	LinearGradientBrush* brush = new LinearGradientBrush(Point(0,0), Point(bodyDimensions.right,bodyDimensions.bottom), topColor, bottomColor);
	return brush;
}
HBRUSH BlueStyle::GetDefaultEditPermanentBrush()
{
	static CBrush brush(GetDefaultEditBackground());
	return brush;
}


Brush* BlueStyle::GetTipWindowBackground(const CRect& dimensions)
{
	Color		topColor	(0xF9,0xFA,0xFB);
	Color		bottomColor	(0xCF,0xDC,0xDE);
	return new LinearGradientBrush(Point(0,0), Point(dimensions.right, dimensions.bottom), topColor, bottomColor);
}

Pen* BlueStyle::GetTipWindowBorder()
{
	return new Pen(GetDarkColor(), 2);
}

Brush* BlueStyle::GetTipWindowTextBrush()
{
	return new SolidBrush(GetDarkColor());
}

Brush* BlueStyle::GetButtonBackground(const CRect& clientRect)
{
	Color		topColor	(0x44,0x64,0x73);
	Color		bottomColor	(0x29,0x3C,0x45);
	return new LinearGradientBrush(PointF(0,0), PointF(clientRect.right,clientRect.bottom), topColor,bottomColor);
}

Brush* BlueStyle::GetButtonTextBrush()
{
	return 	new SolidBrush(Color(0xFF,0xFF,0xFF));
}
Color BlueStyle::GetButtonBorderColor()
{
	return Color(0x1B,0x26,0x2C);
}
Color BlueStyle::GetButtonBevelTopColor()
{
	return Color(0x77,0x8E,0x99);
}
Color BlueStyle::GetButtonBevelBottomColor()
{
	return Color(0x35,0x4E,0x59);
}

Brush* BlueStyle::GetImagePreviewBackground(const CRect& clientRect)
{
	Color		backgroundTopColor		=this->GetDarkColor();
	Color		backgroundBottomColor	(0x10,0x17,0x1B);
	return new LinearGradientBrush(PointF(0,0), PointF(clientRect.right,clientRect.bottom), backgroundTopColor, backgroundBottomColor);
}