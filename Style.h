#pragma once
#include <gdiplus.h>


/////////////////////////////////////////////////////////////////////////////
//
// class Style
//
// Created:     By:
// 2005-03-16 - Hugo
//
// COMMENTS: 
//  
/////////////////////////////////////////////////////////////////////////////
extern const UINT wm_style_installed_message;

class Style
{
	/////////////////////////////////////////////////////////////////////////////
	// ctor/dtor:
protected:
	Style(void);
	virtual ~Style(void);

private:
	static BOOL CALLBACK _UpdateDisplayCallback(HWND wnd, LPARAM param);

protected:
	static Style* DefaultStyleInstance()
	{
		static Style theStyle;
		return &theStyle;
	}

protected:

	static Style* GetSetInstance(Style* installStyle=NULL);

	virtual void OnUninstall()
	{
	}

	virtual void OnInstalled();

	/////////////////////////////////////////////////////////////////////////////
	// Methods:
	public:

	static Style& Instance()
	{
		Style * current = GetSetInstance();
		if(!current)
			return *DefaultStyleInstance();
		return *current;
	}

	static void Install()
	{
		GetSetInstance(DefaultStyleInstance());
	}

	static bool IsInstalled()
	{
		return DefaultStyleInstance()==GetSetInstance()||NULL==GetSetInstance();
	}

	void UpdateDisplay(HWND mainWindow);

	static void Release(Gdiplus::GdiplusBase* object)
	{
		delete object;
	}
	static void Release(Gdiplus::Brush* object)
	{
		delete object;
	}
	static void Release(Gdiplus::Pen* object)
	{
		delete object;
	}
	static void Release(CFont* font)
	{
		delete font;
	}

	virtual LPCWSTR				GetTopImageResourceName();

	/////////////////////////////////////////////////////////////////////////////
	// standard colors
	virtual Gdiplus::Color		GetDarkColor();
	virtual Gdiplus::Color		GetHighlightColor();

	/////////////////////////////////////////////////////////////////////////////
	// Main window
	virtual Gdiplus::Brush*		GetMainTopBrush();
	virtual Gdiplus::Pen*		GetMainTopLine();
	virtual Gdiplus::Font*		GetMainTitleFont();		

	/////////////////////////////////////////////////////////////////////////////
	// default/main
	virtual Gdiplus::Brush*		GetDefaultBodyBrush(const CRect& bodyDimensions);
	virtual Gdiplus::Pen*		GetDefaultBodyBorderPen();

	virtual Gdiplus::Font*		GetDefaultLabelFont();
	virtual Gdiplus::Brush*		GetDefaultLabelBrush();
	virtual Gdiplus::Font*		GetDefaultHeaderFont();

	/////////////////////////////////////////////////////////////////////////////
	//  progress bar
	virtual Gdiplus::Font*		GetProgressbarCaptionFont();
	virtual Gdiplus::Brush*		GetProgressbarAlternateCaptionBrush();
	virtual Gdiplus::Brush*		GetProgressbarCaptionBrush();
	virtual Gdiplus::Brush*		GetProgressbarBackgroundBrush();
	virtual Gdiplus::Color		GetProgressbarActiveColor();
	virtual Gdiplus::Brush*		GetProgressbarActiveBrush(const CRect& activeDimensions);
	
	/////////////////////////////////////////////////////////////////////////////
	//  standard controls
	virtual CFont*				GetEditFont();
	virtual COLORREF			GetDefaultEditBackground();
	virtual HBRUSH				GetDefaultEditPermanentBrush();
	virtual COLORREF			GetDefaultEditColor();
	virtual COLORREF			GetDefaultEditDisabledBackground();
	virtual HBRUSH				GetDefaultEditDisabledPermanentBrush();
	virtual COLORREF			GetDefaultEditDisabledColor();

	/////////////////////////////////////////////////////////////////////////////
	//  Tip window
	virtual Gdiplus::Brush*		GetTipWindowBackground(const CRect& dimensions);
	virtual Gdiplus::Pen*		GetTipWindowBorder();
	virtual Gdiplus::Brush*		GetTipWindowTextBrush();

	/////////////////////////////////////////////////////////////////////////////
	//  Buttons
	virtual Gdiplus::Brush*		GetButtonBackground(const CRect& dimensions);
	virtual Gdiplus::Brush*		GetButtonTextBrush();
	virtual Gdiplus::Color		GetButtonBorderColor();
	virtual Gdiplus::Color		GetButtonBevelTopColor();
	virtual Gdiplus::Color		GetButtonBevelBottomColor();

	/////////////////////////////////////////////////////////////////////////////
	//  preview
	virtual Gdiplus::Brush*		GetDetailsPreviewBackgroundBrush(const Gdiplus::Rect& bounds);

	/////////////////////////////////////////////////////////////////////////////
	// Image preview
	virtual Gdiplus::Brush*		GetImagePreviewBackground(const CRect& dimensions);
		
		
}; // End of class Style.

/////////////////////////////////////////////////////////////////////////////
//
// class BlueStyle
//
// Created:     By:
// 2005-03-16 - Hugo
//
// COMMENTS: 
//  
/////////////////////////////////////////////////////////////////////////////

class BlueStyle : public Style
{
	/////////////////////////////////////////////////////////////////////////////
	// ctor/dtor:
	private:
		BlueStyle();
		virtual ~BlueStyle();
		virtual void OnUninstall();
		
	/////////////////////////////////////////////////////////////////////////////
	// Methods:
	public:
	static void Install();
	static bool IsInstalled()
	{
		return GetTheBlueInstance() == GetSetInstance();
	}
	static BlueStyle* GetTheBlueInstance()
	{
		static BlueStyle theBlue;
		return &theBlue;
	}
		
		
	public:

	virtual LPCWSTR				GetTopImageResourceName();

	virtual Gdiplus::Color		GetDarkColor();
	virtual Gdiplus::Color		GetHighlightColor();

	virtual Gdiplus::Brush*		GetDefaultBodyBrush(const CRect& bodyDimensions);
	virtual HBRUSH				GetDefaultEditPermanentBrush();
	virtual Gdiplus::Color		GetProgressbarActiveColor();

	/////////////////////////////////////////////////////////////////////////////
	//  Tip window
	virtual Gdiplus::Brush*		GetTipWindowBackground(const CRect& dimensions);
	virtual Gdiplus::Pen*		GetTipWindowBorder();
	virtual Gdiplus::Brush*		GetTipWindowTextBrush();

	/////////////////////////////////////////////////////////////////////////////
	//  Buttons
	virtual Gdiplus::Brush*		GetButtonBackground(const CRect& dimensions);
	virtual Gdiplus::Brush*		GetButtonTextBrush();
	virtual Gdiplus::Color		GetButtonBorderColor();
	virtual Gdiplus::Color		GetButtonBevelTopColor();
	virtual Gdiplus::Color		GetButtonBevelBottomColor();

	/////////////////////////////////////////////////////////////////////////////
	// Image preview
	virtual Gdiplus::Brush*		GetImagePreviewBackground(const CRect& dimensions);
		
}; // End of class BlueStyle.




template<class T> class StyleObject
{
public:
	StyleObject() {
		this->object = NULL;
	}
	StyleObject(T* object) {
		this->object = object;
	}
	StyleObject(const StyleObject<T>&) {
		ASSERT(FALSE);
	}
	~StyleObject() {
		Style::Release(this->object);
	}
	operator = (const StyleObject<T>&) {
		ASSERT(FALSE);
	}
	operator = (T* arg) {
		if(this->object == arg)
			return reinterpret_cast<INT_PTR>(this->object);
		Style::Release(this->object);
		this->object = arg;
		return reinterpret_cast<INT_PTR>(this->object);
	}
	T* operator -> () {
		return this->object;
	}
	operator T*() {
		return this->object;
	}
	T* object;
};


