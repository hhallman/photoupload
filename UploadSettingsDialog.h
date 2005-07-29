#pragma once
#include "afxwin.h"


// UploadSettingsDialog dialog

class UploadSettingsDialog : public CDialog
{
	DECLARE_DYNAMIC(UploadSettingsDialog)

public:
	UploadSettingsDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~UploadSettingsDialog();

// Dialog Data
	enum { IDD = IDD_UPLOADSETTINGSDIALOG };
private:
	CStatic icon;
	CButton rdNoChange;
	CButton rd1024;
	CButton rd800;
	CButton rd400;
	CSize size;
	CButton chkReduceQuality;
	UINT quality;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	virtual void OnOK();

public:
	CSize	GetSize()		{ return size; }
	UINT	GetQuality()	{ return quality; }
};
