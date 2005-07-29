#include "StdAfx.h"
#include ".\infopanel.h"
#include ".\style.h"
#include ".\resource.h"
#include ".\defines.h"

using namespace Gdiplus;

//CRect buttonRect(20,200,100+20,200+26+30);
CRect buttonRect(20,200,100+20,200+26);
#define IDSC_BUTTON1 1001
#define IDSC_BUTTON2 1002

CInfoPanel::CInfoPanel(void)
: immuneMouseDown(false)
{
	volatileMode = true;
}

CInfoPanel::~CInfoPanel(void)
{
}

int CInfoPanel::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	Bitmap icon(AfxGetInstanceHandle(), MAKEINTRESOURCEW(IDB_ICON_INFO));
	HBITMAP hicon;
	icon.GetHBITMAP(Color(0xF1,0xEC,0xE6), &hicon);
	staticIcon.Create(NULL, WS_CHILD|WS_VISIBLE|SS_BITMAP, CRect(10,20,25,25), this);
	staticIcon.SetBitmap(hicon);

	button.Create(_T(""), 0, buttonRect,this, IDSC_BUTTON2);

	return 0;
}

void CInfoPanel::OnDestroy()
{
	CWnd::OnDestroy();

	SAFE_DELETEOBJECT_NOL(staticIcon.GetBitmap());
}

BEGIN_MESSAGE_MAP(CInfoPanel, CWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_NCLBUTTONUP()
	ON_WM_LBUTTONUP()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_COMMAND(IDSC_BUTTON1, OnButton1Clicked)
	ON_COMMAND(IDSC_BUTTON2, OnButton2Clicked)
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()


void CInfoPanel::OnPaint()
{
	CPaintDC tempDC(this);
	Graphics g(tempDC.m_hDC);
	DoPaint(&g);
}


void CInfoPanel::DoPaint(Graphics* a_graphics)
{
	Graphics& g = *a_graphics;

	ASSERT(text.GetLength()>0);
	ASSERT(title.GetLength()>0);

	CRect		rect;

	GetClientRect(rect);

	Style& style = Style::Instance();

	//Draw background
	{
		StyleObject<Brush> backgroundBrush = style.GetTipWindowBackground(rect);
		g.FillRectangle(backgroundBrush, Rect(0,0,rect.right,rect.bottom));
	}

	//Draw main border
	{
		StyleObject<Pen> borderPen = style.GetTipWindowBorder();
		Rect borderRect(1,1,rect.right-2,rect.bottom-2);
		g.DrawRectangle(borderPen, borderRect);
	}

	/////////////////////////////////////////////////////////////////////////////
	// Draw text
 
 	g.SetSmoothingMode(SmoothingModeHighQuality);
	g.SetTextRenderingHint(TextRenderingHintAntiAlias);
	StyleObject<Brush> textBrush = style.GetTipWindowTextBrush();
	StringFormat format;
	//format.SetAlignment(StringAlignmentCenter);
		
	//Draw title
	Font		titleFont(L"Trebuchet MS", 12, FontStyleBold, UnitPoint);
	RectF		titleRect(40, 20, rect.right-10, 50.0f);
	g.DrawString(
		CT2W(title),
		title.GetLength(),
		&titleFont,
		titleRect,
		&format,
		textBrush);

	Font	textFont(L"Trebuchet MS", 9, FontStyleBold, UnitPoint);
	RectF	textRect(10,70,rect.right-10,rect.bottom-10);

	g.DrawString(
		CT2W(text),
		text.GetLength(),
		&textFont,
		textRect,
		&format,
		textBrush);
	
}

bool CInfoPanel:: RegisterRuntimeClass()
{
	WNDCLASS wndcls;
	ZeroMemory(&wndcls, sizeof(WNDCLASS));
	wndcls.style			= CS_SAVEBITS|CS_BYTEALIGNCLIENT|CS_HREDRAW|CS_VREDRAW;
	#ifdef CS_DROPSHADOW
	wndcls.style |= CS_DROPSHADOW;
	#endif
	wndcls.lpfnWndProc		= ::DefWindowProc; 
	wndcls.hbrBackground	= (HBRUSH)::GetStockObject(WHITE_BRUSH);
	wndcls.hInstance		= AfxGetInstanceHandle();
	wndcls.lpszClassName	= _T("MINAKORT_INFO_CONTROL_CLASS");

	// Register the new class and exit if it fails
	if(!AfxRegisterClass(&wndcls))
	{
		TRACE("Class Registration Failed\n");
		return false;
	}
	return true;
}


BOOL CInfoPanel::Create(CWnd* pParentWnd, UINT nID)
{
	static int a = RegisterRuntimeClass();
	BOOL b = CWnd::CreateEx(WS_EX_TOPMOST , _T("MINAKORT_INFO_CONTROL_CLASS"), _T("INFO"),  WS_CHILD, CRect(10, 10, 251, 244), pParentWnd, nID, NULL);
	return b;
}

void CInfoPanel::SetText(const CString& title, const CString& text)
{
	this->title	= title;
	this->text	= text;
	this->Invalidate();
}
void CInfoPanel::SetButtonText(const CString& button1Text, const CString& button2Text)
{
	this->button.SetWindowText(button2Text);
	this->button.ShowWindow(button2Text.GetLength()>0?SW_SHOW:SW_HIDE);
}

void CInfoPanel::Show(const CPoint& position, DirectionMode posMode, bool volatileMode)
{
	ASSERT(text.GetLength() > 0 && title.GetLength() > 0);

	this->volatileMode = volatileMode;
	button.SetForceHighlight(false);
	CRect rect;
	GetClientRect(rect);
	if(posMode==DirectionMode::Bottom)
		rect.MoveToXY(CPoint(position.x, position.y-rect.Height()));
	else
		rect.MoveToXY(CPoint(position.x, position.y));
	MoveWindow(rect); 
	ShowWindow(SW_SHOW);
	if(volatileMode)
		SetCapture();
}

void CInfoPanel::ShowModal(CWnd* parent)
{
	ASSERT(!this->GetSafeHwnd());
	this->Create(parent);
	CRect rect;
	parent->GetClientRect(rect);
	int width = 241;
	int height = 234;
	int x = (rect.right-width)/2;
	int y = (rect.bottom-height)/2;
	this->Show(CPoint(x,y));
	ReleaseCapture();
	this->RunModalLoop();
}

void CInfoPanel::Hide()
{
	if(!GetSafeHwnd())
		return;

	ReleaseCapture();
	ShowWindow(SW_HIDE);
	button.SetForceHighlight(false);
	immuneMouseDown = false;

//	this->EndModalLoop(0);
}

void CInfoPanel::OnNcLButtonUp(UINT nHitTest, CPoint point)
{
	if(volatileMode)
		Hide();

	CWnd::OnNcLButtonDown(nHitTest, point);
}

void CInfoPanel::OnLButtonUp(UINT nFlags, CPoint point)
{
	if(buttonRect.PtInRect(point))
	{
		LPARAM lp = point.x+(point.y<<16);
	//	button.SendMessage(WM_LBUTTONUP, 0, lp);
	//	SetCapture();
	//	if(0!=0x0004&button.GetState())
		if(button.GetForceHighligt())
			OnButton2Clicked();
	}
	else if(!immuneMouseDown)
	{
		if(volatileMode)
			Hide();
	}
	immuneMouseDown = false;

	CWnd::OnLButtonDown(nFlags, point);
}

void CInfoPanel::OnLButtonDown(UINT nFlags, CPoint point)
{
	if(buttonRect.PtInRect(point))
	{
		button.SetForceHighlight(true);
		immuneMouseDown = true;
	//	LPARAM lp = point.x+(point.y<<16);
	//	button.SendMessage(WM_LBUTTONDOWN, 0, lp);
	//	SetCapture();
	}

	CWnd::OnLButtonDown(nFlags, point);
}

void CInfoPanel::OnButton2Clicked()
{
	if(volatileMode)
		Hide();

	if(this->GetDlgCtrlID()) {
		CWnd* parent	= this->GetParent();
		HWND hwnd		= this->GetSafeHwnd();
 		LRESULT a		= ::SendMessage(parent->m_hWnd, WM_COMMAND, MAKELONG(this->GetDlgCtrlID(), INFOCOMMAND_BUTTON2), (LPARAM)hwnd);
	}
}

void CInfoPanel::OnButton1Clicked()
{
	if(volatileMode)
		Hide();

	if(this->GetDlgCtrlID()) {
		CWnd* parent	= this->GetParent();
		HWND hwnd		= this->GetSafeHwnd();
 		LRESULT a		= ::SendMessage(parent->m_hWnd, WM_COMMAND, MAKELONG(this->GetDlgCtrlID(), INFOCOMMAND_BUTTON1), (LPARAM)hwnd);
	}
}

void CInfoPanel::OnMouseMove(UINT nFlags, CPoint point)
{
	if(button.GetForceHighligt())
		button.SetForceHighlight(buttonRect.PtInRect(point)?TRUE:FALSE);

	CWnd::OnMouseMove(nFlags, point);
}

