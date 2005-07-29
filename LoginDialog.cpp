// LoginDialog.cpp : implementation file
//

#include "stdafx.h"
#include "MinakortControls.h"
#include "LoginDialog.h"
#include ".\logindialog.h"
#include ".\defines.h"


// LoginDialog dialog

IMPLEMENT_DYNAMIC(LoginDialog, CDialog)
LoginDialog::LoginDialog(CWnd* pParent /*=NULL*/)
	: CDialog(LoginDialog::IDD, pParent)
{
	icon			= 0;
	previousResult	= null;
}

LoginDialog::~LoginDialog()
{
//should not close a std icon:	SAFE_CLOSE(icon);
}

void LoginDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(LoginDialog, CDialog)
END_MESSAGE_MAP()


// LoginDialog message handlers

BOOL LoginDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	//set password style
	this->GetDlgItem(IDC_PASSWORD)->ModifyStyle(0, ES_PASSWORD);

	if(previousResult) {
		if(previousResult->failed) {
			SAFE_CLOSE(icon);
			icon = AfxGetApp()->LoadStandardIcon(MAKEINTRESOURCE(IDI_HAND));
		}
		this->GetDlgItem(IDC_RESULT)->SetWindowText(previousResult->message);
	}

	//load icon
	if(icon)
		SendDlgItemMessage(
			IDC_DLGICON,			// icon identifier 
			STM_SETIMAGE,			// message to send 
			(WPARAM) IMAGE_ICON,	// image type 
			(LPARAM) icon);			// icon handle 
   
	//load saved data.
	//password is never saved, but auth token can be saved in cookie.
	save		= AfxGetApp()->GetProfileInt(_T("login"), _T("save"), 1)?true:false;
	username	= AfxGetApp()->GetProfileString(_T("login"), _T("username"));
	this->GetDlgItem(IDC_USERNAME)->SetWindowText(username);
	this->GetDlgItem(IDC_CHK_SAVE)->SendMessage(BM_SETCHECK, this->save?BST_CHECKED:BST_UNCHECKED, 0);

	//if we have a username focus password, else username
	this->GetDlgItem(username.GetLength()>0?IDC_PASSWORD:IDC_USERNAME)->SetFocus();

	USAGE("InitDialog: Login");

	//return FALSE since we focused controls
	return FALSE;
}

void LoginDialog::OnOK()
{
	this->GetDlgItem(IDC_USERNAME)->GetWindowText(username);
	this->GetDlgItem(IDC_PASSWORD)->GetWindowText(password);
	username.Trim();
	password.Trim();

	this->save = BST_CHECKED == this->GetDlgItem(IDC_CHK_SAVE)->SendMessage(BM_GETCHECK, 0, 0);

	if(username.GetLength()<1 || password.GetLength()<1) {
		USAGE("Login: Invalid input.");
		MessageBox(R2T(IDS_LOGIN_INVALID_ENTRY), R2T(IDS_LOGIN_CAPTION), MB_ICONSTOP);
		return;
	}

	AfxGetApp()->WriteProfileInt(_T("login"), _T("save"), save?1:0);
	AfxGetApp()->WriteProfileString(_T("login"), _T("username"), save?username:_T(""));

	USAGE("Provided login input.");

	CDialog::OnOK();
}

void LoginDialog::SetResult(const UploadManager::LoginResult* result)
{
	this->previousResult = result;
}


