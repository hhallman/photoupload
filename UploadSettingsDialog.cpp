// UploadSettingsDialog.cpp : implementation file
//
#include "stdafx.h"
#include ".\Defines.h"
#include ".\MinakortControls.h"
#include ".\UploadSettingsDialog.h"
#include ".\uploadsettingsdialog.h"
#include ".\UploadManager.h"

#define REDUCED_QUALITY 60

// UploadSettingsDialog dialog

IMPLEMENT_DYNAMIC(UploadSettingsDialog, CDialog)
UploadSettingsDialog::UploadSettingsDialog(CWnd* pParent /*=NULL*/)
	: CDialog(UploadSettingsDialog::IDD, pParent)
{
}

UploadSettingsDialog::~UploadSettingsDialog()
{
}

void UploadSettingsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DLGICON, icon);
	DDX_Control(pDX, IDC_CHOOSESIZE_NOCHANGE,	rdNoChange);
	DDX_Control(pDX, IDC_CHOOSESIZE_LARGE,		rd1024);
	DDX_Control(pDX, IDC_CHOOSESIZE_MEDIUM,		rd800);
	DDX_Control(pDX, IDC_CHOOSESIZE_SMALL,		rd400);
	DDX_Control(pDX, IDC_CHKREDUCEQUALITY, chkReduceQuality);
}


BEGIN_MESSAGE_MAP(UploadSettingsDialog, CDialog)
END_MESSAGE_MAP()

// UploadSettingsDialog message handlers

BOOL UploadSettingsDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	icon.SetIcon(::LoadIcon(NULL, IDI_INFORMATION));

	UINT n = AfxGetApp()->GetProfileInt(_T("PhotoSettings"), _T("shrinkSize"), 3);
	
	if(n==3)
		rd1024.SetCheck(BST_CHECKED);
	else if(n==2)
		rd800.SetCheck(BST_CHECKED);
	else if(n==1)
		rd400.SetCheck(BST_CHECKED);
	else
		rdNoChange.SetCheck(BST_CHECKED);
	
	quality = AfxGetApp()->GetProfileInt(_T("PhotoSettings"), _T("quality"), (UINT)-1);

	chkReduceQuality.SetCheck(quality==-1?BST_UNCHECKED:BST_CHECKED);


    CString recommendation;
	UINT quota		= UploadManager::Instance().Quota;
	UINT usedQuota	= UploadManager::Instance().UsedQuota;
	if(usedQuota != (UINT)-1) {
		UploadManager::UploadStatus	status;
		UINT left					= quota - usedQuota;
		UploadManager::Instance()	.GetStatus(&status);
		if(status.queuedItems) {
		UINT kbAvailablePerPhoto	= left/status.queuedItems;
		if(kbAvailablePerPhoto > 1000)
			recommendation = R2T(IDS_QUOTARECOMMENDATION_ANY);
		else if(kbAvailablePerPhoto > 150)
			recommendation = R2T(IDS_QUOTARECOMMENDATION_LL);
		else if(kbAvailablePerPhoto > 120)
			recommendation = R2T(IDS_QUOTARECOMMENDATION_LS);
		else if(kbAvailablePerPhoto > 100)
			recommendation = R2T(IDS_QUOTARECOMMENDATION_ML);
		else if(kbAvailablePerPhoto > 80)
			recommendation = R2T(IDS_QUOTARECOMMENDATION_MS);
		else if(kbAvailablePerPhoto > 40)
			recommendation = R2T(IDS_QUOTARECOMMENDATION_SL);
		else if(kbAvailablePerPhoto > 30)
			recommendation = R2T(IDS_QUOTARECOMMENDATION_SS);
		else 
			recommendation = R2T(IDS_QUOTARECOMMENDATION_NONE);
		}
	}
	GetDlgItem(IDC_COMPRESSIONRECOMMEDATION)->SetWindowText(recommendation);

	return TRUE;  // return TRUE unless you set the focus to a control
}


void UploadSettingsDialog::OnOK()
{
	int n = 0;
	if(BST_CHECKED == rd1024.GetCheck()) {
		size = CSize(1024,768);
		n=3;
	}
	if(BST_CHECKED == rd800.GetCheck()) {
		n = 2;
		size = CSize(800, 600);
	}
	if(BST_CHECKED == rd400.GetCheck()) {
		n = 1;
		size = CSize(400, 300);
	}
	if(BST_CHECKED == rdNoChange.GetCheck())
		size = CSize(0);

	AfxGetApp()->WriteProfileInt(_T("PhotoSettings"), _T("shrinkSize"), n);
	
	quality = chkReduceQuality.GetCheck()==BST_CHECKED?REDUCED_QUALITY:-1;
	
	AfxGetApp()->WriteProfileInt(_T("PhotoSettings"), _T("quality"), quality);

	CDialog::OnOK();
}
