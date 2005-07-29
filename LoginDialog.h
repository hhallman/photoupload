#pragma once

#include ".\uploadmanager.h"

// LoginDialog dialog

class LoginDialog : public CDialog
{
	DECLARE_DYNAMIC(LoginDialog)

public:
	LoginDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~LoginDialog();

// Dialog Data
	enum { IDD = IDD_LOGINDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

private:
	CString	username;
	CString	password;
	HICON	icon;
	bool	save;
	const UploadManager::LoginResult* previousResult;

protected:
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
protected:
	virtual void OnOK();

public:
	void SetResult(const UploadManager::LoginResult* result);
	const CString GetUsername() const { return this->username; }
	const CString GetPassword() const { return this->password; }
	bool GetPersistLogin() const { return this->save; }
};
