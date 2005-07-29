#include "stdafx.h"
#include "PrettyList.h"
#include ".\Defines.h"

using namespace Gdiplus;

class CPrettyList : public CWnd
{
	virtual LRESULT WindowProc(
   UINT message,
   WPARAM wParam,
   LPARAM lParam 
	)
	{
		if(false&&message == WM_ERASEBKGND)
		{
			CPaintDC	dc(this); // device context for painting
			RECT clip;
			int a = dc.GetClipBox(&clip);
			CRect window;
			GetWindowRect(&window);
			ScreenToClient(&window);
			TRIVERTEX v[2] = { {0,0,0xb800,0xc500,0x9700,0xffff }, {window.right,window.bottom,0x9a00,0xa900,0x7500,0xffff } };
			GRADIENT_RECT g[1] = {{0,1}};
			dc.GradientFill(v,2,g,1,GRADIENT_FILL_RECT_H);
            return 1; 
		}
		return CWnd::WindowProc(message, wParam, lParam);
	//	return GetSuperWndProcAddr()(this->GetSafeHwnd(),message, wParam, lParam);
	}

//void OnNMCustomdrawFolderImages(NMHDR *pNMHDR, LRESULT *pResult)
//{
////	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
//	NMLVCUSTOMDRAW* p = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);
//	*pResult = CDRF_DODEFAULT;
//	return;
//
//	int a;
//	switch(p->nmcd.dwDrawStage){
//		case CDDS_POSTERASE:
//			//After the erasing cycle is complete.
//			a=0;
//			break;
//		case CDDS_POSTPAINT:
//			//After the painting cycle is complete.
//			a=0;
//			break;
//		case CDDS_PREERASE:
//			//Before the erasing cycle begins.
//			a=0;
//			break;
//		case CDDS_PREPAINT:
//			{ 
//			*pResult = CDRF_NOTIFYITEMDRAW;
//			CDC dc;
//			dc.Attach(p->nmcd.hdc);
//			RECT clip;
//			int a = dc.GetClipBox(&clip);
//			CRect window;
//			GetWindowRect(&window);
//			ScreenToClient(&window);
//			window = p->nmcd.rc;
//			TRIVERTEX v[2] = { {0,0,0xf500,0xf300,0xef00,0xffff }, {window.right,window.bottom,0xf000,0xeb00,0xe300,0xffff } };
//			GRADIENT_RECT g[1] = {{0,1}};
//			dc.GradientFill(v,2,g,1,GRADIENT_FILL_RECT_V);
//			}
//			break;
//		case CDDS_ITEMPOSTERASE:
//			//After an item has been erased.
//			a=0;
//			break;
//		case CDDS_ITEMPOSTPAINT:
//			//After an item has been drawn.
//			a=0;
//			break;
//		case CDDS_ITEMPREERASE:
//			//Before an item is erased.
//			a=0;
//			break;
//		case CDDS_ITEMPREPAINT:
//			{
//				int item = (int)p->nmcd.dwItemSpec;
//				RECT itemRect;
////HACK				VERIFY(m_FolderImages.GetItemRect(item, &itemRect, LVIR_BOUNDS));
//				if(false)
//				{
//				//	*pResult = CDRF_SKIPDEFAULT;
//				//	CDC dc;
//				//	dc.Attach(p->nmcd.hdc);
//					CPaintDC dc(CWnd::FromHandle(p->nmcd.hdr.hwndFrom));
//					dc.FillRect(&itemRect, CBrush::FromHandle((HBRUSH)::GetStockObject(BLACK_BRUSH)));
//					RECT clip;
//					int a = dc.GetClipBox(&clip);
//				}
//				else
//				{
//					Graphics g(m_FolderImages);
//					g.FillRectangle(&SolidBrush(Color(0x22,0x00,0x00)), RectF(50, 50, 100, 100));
//				}
//			}
//			break;
//		default:
//			a=0;
//				;
//	}
//}
//
};


LRESULT CPrettyListCtrl::WindowProc(
   UINT message,
   WPARAM wParam,
   LPARAM lParam 
	)
	{
		if(false && message == WM_ERASEBKGND)
		{
			CDC dc;
			dc.Attach((HDC)wParam);
			RECT clip;
			int a = dc.GetClipBox(&clip);
			CRect window;
			GetWindowRect(&window);
			ScreenToClient(&window);
			TRIVERTEX v[2] = { {0,0,0xf500,0xf300,0xef00,0xffff }, {window.right,window.bottom,0xf000,0xeb00,0xe300,0xffff } };
			GRADIENT_RECT g[1] = {{0,1}};
			dc.GradientFill(v,2,g,1,GRADIENT_FILL_RECT_V);
            return 1; 
		}
		if(false && message == WM_PAINT)
		{
			CPaintDC dc(this);
			RECT clip;
			int a = dc.GetClipBox(&clip);
			CRect window;
			GetWindowRect(&window);
			ScreenToClient(&window);
			TRIVERTEX v[2] = { {0,0,0x2800,0xc500,0x9700,0xffff }, {window.right,window.bottom,0x9a00,0xa900,0x7500,0xffff } };
			GRADIENT_RECT g[1] = {{0,1}};
			dc.GradientFill(v,2,g,1,GRADIENT_FILL_RECT_H);
		}
		return CWnd::WindowProc(message, wParam, lParam);
	//	return GetSuperWndProcAddr()(this->GetSafeHwnd(),message, wParam, lParam);
	}
