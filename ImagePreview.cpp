#include "StdAfx.h"
#include ".\imagepreview.h"
#include <gdiplus.h>
#include "defines.h"
#include ".\Style.h"
using namespace Gdiplus;

CImagePreview::CImagePreview(void)
{
	this->image = NULL;
}

CImagePreview::~CImagePreview(void)
{
	Unload();
}

#ifdef DEBUG
void CImagePreview::AssertValid() const
{
	ASSERT(this->GetSafeHwnd());
}
#endif

bool CImagePreview::RegisterRuntimeClass()
{
	WNDCLASS wndcls;
	ZeroMemory(&wndcls, sizeof(WNDCLASS));
	wndcls.style			= /*CS_DBLCLKS|CS_SAVEBITS|CS_BYTEALIGNCLIENT|*/CS_HREDRAW|CS_VREDRAW;
	wndcls.lpfnWndProc		= ::DefWindowProc; 
	wndcls.hInstance		= AfxGetInstanceHandle();
	wndcls.lpszClassName	= _T("MINAKORT_PREVIEW_CONTROL_CLASS");

	// Register the new class and exit if it fails
	if(!AfxRegisterClass(&wndcls))
	{
		TRACE("Class Registration Failed\n");
		return true;
	}
	return false;
}


bool CImagePreview::Create(CWnd* parent, UINT id)
{
	ASSERT(parent);

	static int _n_ = RegisterRuntimeClass();

	CRect rect;
	parent->GetClientRect(rect);
//	rect.DeflateRect(100,100);
	BOOL res = CWnd::CreateEx(WS_EX_TOPMOST, _T("MINAKORT_PREVIEW_CONTROL_CLASS"), _T("Preview"), WS_CHILD, rect, parent, id);
	return res?true:false;
}

int CImagePreview::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

void CImagePreview::OnDestroy()
{
	CWnd::OnDestroy();

	Unload();
}

bool CImagePreview::SetImage(const CString& imagePath)
{
	if(this->imagePath == imagePath)
		return true;
	this->imagePath = imagePath;
	if(IsWindowVisible()) {
		Load();
		Invalidate();
	}
	return true;
}

void CImagePreview::Show(void)
{
	ASSERT_VALID(this);
	if(!GetSafeHwnd())
		return;
	if(IsWindowVisible())
		return;
	Load();
	this->ShowWindow(SW_SHOW);
	this->SetFocus();
	SetCapture();
}

void CImagePreview::Hide(void)
{
	if(!IsWindowVisible())
		return;
	Unload();
	this->ShowWindow(SW_HIDE);
	ReleaseCapture();
}

BEGIN_MESSAGE_MAP(CImagePreview, CWnd)
	ON_WM_LBUTTONUP()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()

void CImagePreview::OnLButtonUp(UINT nFlags, CPoint point)
{
	//Hide();
	CWnd::OnLButtonUp(nFlags, point);
}

void CImagePreview::OnLButtonDown(UINT nFlags, CPoint point)
{
	Hide();
	CWnd::OnLButtonDown(nFlags, point);
}

void CImagePreview::Load(void)
{
	Unload();
	SAFE_DELETE(image);
#undef new
	this->image = new Image(CT2W(this->imagePath));
#define new DEBUG_NEW
}

void CImagePreview::Unload(void)
{
	SAFE_DELETE(image);
}

RectF CImagePreview::ImageRect(const CRect& imageArea, Image* image)
{
	int curX = image->GetWidth();
	int curY = image->GetHeight();

	bool growFit	= false;
	bool shrinkFit	= true;
	
	double resizeX, resizeY;

	resizeX = (double)imageArea.Width()/(double)curX;
	resizeY = (double)imageArea.Height()/(double)curY;

	// image fits.
	if(!growFit && resizeX >= 1.0f && resizeY >= 1.0f)
		return RectF(0,0, curX, curY);
	if(!shrinkFit && (resizeX <= 1.0f || resizeY <= 1.0f))
		return RectF(0,0, curX, curY);
	if(resizeX < resizeY)
	{
		return RectF(0,0,(int)(curX*resizeX), (int)(curY*resizeX));
	}
	else
	{
		return RectF(0,0,(int)(curX*resizeY), (int)(curY*resizeY));
	}
}


void CImagePreview::OnPaint()
{
	ASSERT(this->image);

	CPaintDC	tempDC					(this);
	Graphics	g						(tempDC.m_hDC);
	CRect		clientRect;

	this->GetClientRect(&clientRect);
	Style& style = Style::Instance();

	/////////////////////////////////////////////////////////////////////////////
	// Draw background
	StyleObject<Brush> backgroundBrush = style.GetImagePreviewBackground(clientRect);
	g.FillRectangle(backgroundBrush, 0,0,clientRect.right,clientRect.bottom);
	Pen border(Color(0,0,0));
	g.DrawRectangle(&border,0,0,clientRect.right-1,clientRect.bottom-1);

	CRect imageArea(10,10,clientRect.right-10,clientRect.bottom-10);
	RectF imageRect = ImageRect(imageArea, image);
	imageRect.X += imageArea.left + (imageArea.Width()-imageRect.Width)/2;
	imageRect.Y += imageArea.top + (imageArea.Height()-imageRect.Height)/2;
	g.DrawRectangle(&border,RectF(imageRect.X-1,imageRect.Y-1,imageRect.Width+1,imageRect.Height+1));
	g.DrawImage(image, imageRect);
}


void CImagePreview::OnKillFocus(CWnd* pNewWnd)
{
	CWnd::OnKillFocus(pNewWnd);

	Hide();
}
