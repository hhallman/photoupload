#pragma once

// UpdateDialog dialog

class UpdateDialog : public CDialog
{
	DECLARE_DYNAMIC(UpdateDialog)

public:
	UpdateDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~UpdateDialog();

// Dialog Data
	enum { IDD = IDD_UPDATEDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

public:
	void SetJobBytes(UINT bytes);
	void SetFinishedBytes(UINT bytes);

private:
	UINT jobBytes;
	UINT finishedBytes;
	void UpdateTexts();
};
