#pragma once
#include "resource.h"
#include "afxcmn.h"
#include "afxwin.h"

#include ".\progressbar.h"
#include ".\infopanel.h"
#include ".\imagepreview.h"
#include ".\detailsEditDialog.h"
#include ".\prettybutton.h"
#include ".\PhotoReference.h"
#include ".\DialogControlTarget.h"
#include ".\ShareDialog.h"

// CUploadDialog dialog


class CUploadDialog : public CDialog
{
	DECLARE_DYNAMIC(CUploadDialog)


	class MenuListView : public CListCtrl
	{
	public:
		MenuListView()
		{
			menu = NULL;
			menuHandler = NULL;
		}
		DECLARE_MESSAGE_MAP()
		afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	public:
		CPoint mousePos;
		CMenu* menu;
		CWnd* menuHandler;
		int clickedItem;
		afx_msg void OnNMRclick(NMHDR *pNMHDR, LRESULT *pResult);
	};

public:
	CUploadDialog(CWnd* pParent = NULL);
	virtual ~CUploadDialog();

// Dialog Data
	enum { IDD = IDD_UPLOAD_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

private:

	ULONG_PTR		m_gdiplusToken;
	MenuListView	m_FolderImages;
	MenuListView	queuedImages;
	CImageList		imageList;
	bool			m_bFinished;
	bool			m_bUploadInProgress;
	bool			closing; //set when the user wants to close the window, and it is waiting for UploadManager.Abort to finish.
	bool			updatingLast;
	CStatic			m_TopImage;
	CProgressBar	m_ProgressBar;
	int				logoheight;
	int				logowidth;
	CInfoPanel		infoPanel;
	CInfoPanel		infoGetStartedTip;
	CInfoPanel		infoPanelMisc;
	CImagePreview	imagePreview;
	CPrettyButton	m_btnFound2Queue;
	CPrettyButton	m_btnFound2QueueAll;
	CPrettyButton	m_btnQueue2Found;
	CPrettyButton	m_btnQueue2FoundAll;
	CPrettyButton	m_btnQueueHelp;
	CPrettyButton	btnShare;
	DetailsEditDialog detailsEditDialog;
	CMapStringToPtr loadedPaths;
	CMenu			mainContextMenu;
	CMenu			folderContextMenu;
	CMenu			queueContextMenu;
	HACCEL			accel;
	CDialogControlTarget controlTarget;
	ShareSettings	shareSettings;

	static const int PreviewSize = 128;

public:
	bool IsFinished() { return m_bFinished; }
	int AddFile(CString path, PhotoReference** ref=0);

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnBnClickedFound2queue();
	afx_msg void OnBnClickedQueue2found();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetItemDetails(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnItemDetailsUpdated(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnItemDetailsScroll(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkFolderImages(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedFound2queueAll();
	afx_msg void OnBnClickedQueue2foundAll();
	afx_msg LRESULT OnUploadProgressed(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUploadFinished(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUploadLock(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUploadUnlock(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUploadItemFinished(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUploadGetDirtyPhotos(WPARAM wParam, LPARAM);
	afx_msg void OnLvnKeydownQueuedImages(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnStyleGreen();
	afx_msg void OnStyleBlue();
	afx_msg LRESULT OnStyleInstalled(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedQueuehelp();
	afx_msg void OnLvnBegindragFolderImages(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg LRESULT OnRequireLogin(WPARAM, LPARAM);
	afx_msg LRESULT OnOldClient(WPARAM, LPARAM);
	afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);

private:
	void OnAbortUploadClicked();
	void OnOpenMyPhotosFolder();
	void OnGetStartedTipOk();
	void OnProgressBarClicked();
	void StartUpload();
	void ResetUploadState();
	void MoveSelected(CListCtrl& from, CListCtrl& to, BOOL all=FALSE);
	void OnEscape();
	void OnReturn();
	void GetDirtyPhotos(CArray<PhotoReference*>* dirtyPhotos, bool lock=true);
	void Save();
	void Load();

public:
	CString	instanceName;
	static bool StaticSetTargetCategory(const CString& instanceName, const CString& categoryName);
	static bool StaticSetQuota(const CString& instanceName, UINT quota);
	static bool StaticSetUsedQuota(const CString& instanceName, UINT usedQuota);
	static bool StaticBringWindowToTop(const CString& instanceName);
	afx_msg void OnClearFolderlist();
	afx_msg void OnRemoveFolderitem();
	afx_msg void OnClearFinished();
	afx_msg void OnClearQueue();
	afx_msg void OnQueueDeleteitem();
	afx_msg void OnBnClickedBtnsharesettings();
};
