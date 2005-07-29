#pragma once

class CImageUtility
{
private:
	CImageUtility(void);
	~CImageUtility(void);
public:
	static void GetJpgCompressionEncoderParameter(int* quality, Gdiplus::EncoderParameters* encoderParameters);
	static int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

	static UINT SaveImage(Gdiplus::Image* image, HGLOBAL& hMemory, LPCTSTR encoder = _T("image/jpeg"), UINT quality=-1);
	static Gdiplus::Image* ShrinkImage(Gdiplus::Image* image, CSize size);
	static void CopyMetaData(Gdiplus::Image* from, Gdiplus::Image* to);


};
