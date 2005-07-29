#pragma once

#include <gdiplus.h>
//namespace Gdiplus { class Image; }

class CImagePreview : public CWnd
{
public:
	CImagePreview(void);
	virtual ~CImagePreview(void);
	bool SetImage(const CString& imagePath);
	bool Create(CWnd* parent, UINT id=0);
	void Show(void);
	void Hide(void);
#ifdef DEBUG
	void AssertValid() const;
#endif
private:
	void Load(void);
	void Unload(void);
	static bool RegisterRuntimeClass();
	static Gdiplus::RectF ImageRect(const CRect& imageArea, Gdiplus::Image* image);

private:
	Gdiplus::Image* image;
	CString imagePath;
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
};
