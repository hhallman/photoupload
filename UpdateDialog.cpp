// UpdateDialog.cpp : implementation file
//

#include "stdafx.h"
#include "MinakortControls.h"
#include ".\updatedialog.h"
#include ".\defines.h"
#include ".\Utility.h"


// UpdateDialog dialog

IMPLEMENT_DYNAMIC(UpdateDialog, CDialog)
UpdateDialog::UpdateDialog(CWnd* pParent /*=NULL*/)
	: CDialog(UpdateDialog::IDD, pParent)
{
	this->jobBytes		= 0;
	this->finishedBytes = 0;
}

UpdateDialog::~UpdateDialog()
{
}

void UpdateDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(UpdateDialog, CDialog)
END_MESSAGE_MAP()



BOOL UpdateDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	GetDlgItem(IDC_UPDATEPROGRESSTEXT)->SetWindowText(R2T(IDS_UPDATE_STARTING));
	
	//static_cast<CProgressBar*>(GetDlgItem(IDC_UPDATEPROGRESS))->SetCurrentValue(0);
	GetDlgItem(IDC_UPDATEPROGRESS)->SendMessage((UINT)PBM_SETPOS);
	GetDlgItem(IDC_UPDATEPROGRESS)->SendMessage((UINT)PBM_SETPOS, 0);
	GetDlgItem(IDC_UPDATEPROGRESS)->SendMessage((UINT)PBM_SETRANGE, 0, MAKELPARAM(0, 100));

	if(!Utility::IsUserAdmin())
	{
		MessageBox(R2T(IDS_INSTALLNOADMIN), R2T(IDS_INSTALLNOADMINCAPTION), MB_OK|MB_ICONINFORMATION);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
}

void UpdateDialog::SetJobBytes(UINT bytes)
{
	this->jobBytes = bytes;
	UINT jobKBytes = bytes/1024;
	GetDlgItem(IDC_UPDATEPROGRESS)->SendMessage((UINT)PBM_SETRANGE, 0, MAKELPARAM(0, jobKBytes));
	UpdateTexts();
}

void UpdateDialog::SetFinishedBytes(UINT bytes)
{
	this->finishedBytes = bytes;
	GetDlgItem(IDC_UPDATEPROGRESS)->SendMessage((UINT)PBM_SETPOS, bytes/1024);
	UpdateTexts();
}

void UpdateDialog::UpdateTexts()
{
	CString text;
	text.Format(_T("%ukB / %ukB"), this->finishedBytes/1024, this->jobBytes/1024);
	GetDlgItem(IDC_UPDATEPROGRESSTEXT)->SetWindowText(text);
}

