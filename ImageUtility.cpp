#include "StdAfx.h"
#include ".\imageutility.h"
#include ".\defines.h"
#include <gdiplus.h>
using namespace Gdiplus;

CImageUtility::CImageUtility(void)
{
}

CImageUtility::~CImageUtility(void)
{
}

int CImageUtility::GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
   UINT  num = 0;          // number of image encoders
   UINT  size = 0;         // size of the image encoder array in bytes

   ImageCodecInfo* pImageCodecInfo = NULL;

   GetImageEncodersSize(&num, &size);
   if(size == 0)
      return -1;  // Failure

   pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
   if(pImageCodecInfo == NULL)
      return -1;  // Failure

   GetImageEncoders(num, size, pImageCodecInfo);

   for(UINT j = 0; j < num; ++j)
   {
      if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
      {
         *pClsid = pImageCodecInfo[j].Clsid;
         free(pImageCodecInfo);
         return j;  // Success
      }    
   }

   free(pImageCodecInfo);
   return -1;  // Failure
}

void CImageUtility::GetJpgCompressionEncoderParameter(int* quality, Gdiplus::EncoderParameters* encoderParameters)
{
	encoderParameters->Count = 1;
	encoderParameters->Parameter[0].Guid = EncoderQuality;
	encoderParameters->Parameter[0].Type = EncoderParameterValueTypeLong;
	encoderParameters->Parameter[0].NumberOfValues = 1;

	encoderParameters->Parameter[0].Value = quality;
}

void CImageUtility::CopyMetaData(Image* from, Image* to)
{
	ASSERT(from && to);
	ASSERT(from != to);

	// Find out how many property items are in the image, and find out the
	// required size of the buffer that will receive those property items.
	UINT totalBufferSize;
	UINT numProperties;
	from->GetPropertySize(&totalBufferSize, &numProperties);

	// Allocate the buffer that will receive the property items.
	PropertyItem* pAllItems = (PropertyItem*)malloc(totalBufferSize);

	// Fill the buffer.
	from->GetAllPropertyItems(totalBufferSize, numProperties, pAllItems);

	for(UINT i=0;i<numProperties;i++)
		to->SetPropertyItem(&pAllItems[i]);
}

Image* CImageUtility::ShrinkImage(Image* image, CSize maxSize)
{
	ASSERT(image);
	ASSERT(maxSize.cx >= 0 && maxSize.cy >= 0);
	ASSERT(maxSize.cx >= maxSize.cy);

	if(!maxSize.cx || !maxSize.cy)
		return image;

	CSize size(image->GetWidth(), image->GetHeight());
	
	if(size.cx < size.cy) {
		int n = maxSize.cx;
		maxSize.cx = maxSize.cy;
		maxSize.cy = n;
	}

	double ratio	= size.cx/(float)size.cy;
	double resizeX	= (float)maxSize.cx/(float)size.cx;
	double resizeY	= (float)maxSize.cy/(float)size.cy;

	if(resizeX < 1.0f || resizeY < 1.0f)
	{
		if(resizeX < resizeY)
			size = CSize((int)(size.cx*resizeX), (int)(size.cy*resizeX));
		else
			size = CSize((int)(size.cx*resizeY), (int)(size.cy*resizeY));
	}
	else
		return image;


#undef new
	Bitmap* b = new Bitmap(size.cx, size.cy, image->GetPixelFormat());
#define new DEBUG_NEW
	if(b->GetLastStatus() != Gdiplus::Ok) {
		delete b;
		return NULL;
	}
	Graphics g(b);
	g.DrawImage(image, Rect(0,0,size.cx,size.cy));
	if(b->GetLastStatus() != Gdiplus::Ok) {
		delete b;
		return NULL;
	}

	CopyMetaData(image, b);

	return b;
}

UINT CImageUtility::SaveImage(Image* image, HGLOBAL& hMemory, LPCTSTR encoder, UINT quality)
{
	ASSERT(hMemory==0);
	ASSERT(quality == (UINT)-1 || (quality>=1 && quality <= 100));

	Status stat;
	

	//target stream
	hMemory = ::GlobalAlloc(GHND , 0);
	IStream* data=0;
	::CreateStreamOnHGlobal(hMemory, FALSE, &data);

	//encoding
	CLSID codecClsid;
	Gdiplus::EncoderParameters encoderParameters;
	GetEncoderClsid(CT2W(encoder), &codecClsid);
	int q = (int)quality;
	GetJpgCompressionEncoderParameter(&q, &encoderParameters);
	stat = image->Save(data, &codecClsid, quality==-1?null:&encoderParameters);

	data->Release();

	return (UINT)::GlobalSize(hMemory);
}


