// UploadDialog.cpp : implementation file
//

#include "stdafx.h"
#include ".\MinakortControls.h"
#include ".\UploadDialog.h"
#include ".\uploaddialog.h"
#include ".\imageutility.h"
#include ".\infopanel.h"
#include ".\defines.h"
#include ".\Style.h"
#include ".\defines.h"
#include ".\utility.h"
#include ".\UploadSettingsDialog.h"
#include ".\UploadManager.h"
#include ".\LoginDialog.h"
#include ".\ShareDialog.h"

using namespace Gdiplus;

//afxtempl.h
// CUploadDialog dialog

#ifdef DEBUG
void WINAPI GdiplusDebugEvent(DebugEventLevel level, CHAR* message)
{
	int a = 3;
	::MessageBox(NULL, CA2T(message), _T("gdi error"), MB_ICONERROR);
}
#endif

IMPLEMENT_DYNAMIC(CUploadDialog, CDialog)
CUploadDialog::CUploadDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CUploadDialog::IDD, pParent)
{
	m_bFinished				= false;
	m_bUploadInProgress		= false;
	closing					= false;
	updatingLast			= false;
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
#ifdef DEBUG
	gdiplusStartupInput.DebugEventCallback = GdiplusDebugEvent;
#endif
	Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
	CProgressBar::RegisterRuntimeClass();
}

CUploadDialog::~CUploadDialog()
{
	Gdiplus::GdiplusShutdown(m_gdiplusToken);
}


int CUploadDialog::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

BOOL CUploadDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	controlTarget.Create(this->GetSafeHwnd(), instanceName);

	UINT theme = AfxGetApp()->GetProfileInt(SECTION_UISETTINGS, _T("Theme"), 0);
	if(theme == 1)
		BlueStyle::Install();
	else
		Style::Install();

	this->SetIcon(AfxGetApp()->LoadIcon(IDI_MINAKORTLOGO), false);
	this->SetIcon(AfxGetApp()->LoadIcon(IDI_MINAKORTLOGO), true);

	imageList.Create(PreviewSize,PreviewSize,ILC_COLOR32|ILC_MASK,32,32);
	//load overlays:
	{
		CBitmap finishedOverlayBitmap;
		finishedOverlayBitmap.LoadBitmap(IDB_OVERLAY_FINISHED);
		int pos = imageList.Add(&finishedOverlayBitmap, MAGENTA);
		imageList.SetOverlayImage(pos, FINISHED_OVERLAY);
		finishedOverlayBitmap.DeleteObject();
		finishedOverlayBitmap.LoadBitmap(IDB_OVERLAY_RUNNING);
		pos = imageList.Add(&finishedOverlayBitmap, MAGENTA);
		imageList.SetOverlayImage(pos, RUNNING_OVERLAY);
	}
	m_FolderImages.SetImageList(&imageList, LVSIL_NORMAL);
	queuedImages.SetImageList(&imageList, LVSIL_NORMAL);
	RECT rect;
	this->GetClientRect(&rect);

	//set the top logo:
	Gdiplus::Bitmap topImage(::AfxGetInstanceHandle(), Style::Instance().GetTopImageResourceName());
	HBITMAP htopbm;
	((Gdiplus::Bitmap*)&topImage)->GetHBITMAP(MAGENTA, &htopbm);
	m_TopImage.SetBitmap(htopbm);

	logowidth	= topImage.GetWidth();
	logoheight	= topImage.GetHeight();
	SIZE logoSize = { logowidth, logoheight };

	m_TopImage.SetWindowPos(&wndTop, rect.right-logoSize.cx, 0, logoSize.cx, logoSize.cy, SWP_NOZORDER);

	m_FolderImages.ModifyStyle(0, WS_CLIPSIBLINGS);
	queuedImages.ModifyStyle(0, WS_CLIPSIBLINGS);

	infoPanel.Create(this,IDD_INFO_UPLOADSTATUS);

	infoPanelMisc.Create(this);

	detailsEditDialog.CreateEx(WS_EX_TOPMOST, NULL, _T("Details"), WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, CRect(100,100,600,600), this, IDD_EDITDETAILSDLG);
	detailsEditDialog.SetImageList(&imageList);

	CRect folderImagesRect;
	m_FolderImages.GetWindowRect(folderImagesRect);
	ScreenToClient(folderImagesRect);
	infoGetStartedTip.Create(this, IDD_INFO_ADDPHOTOSTIP);
	infoGetStartedTip.SetText(R2T(IDS_TIP_START_TITLE),R2T(IDS_TIP_START_TEXT));
	infoGetStartedTip.SetButtonText(R2T(IDS_TIP_START_BTN1), R2T(IDS_TIP_START_BTN2));
	infoGetStartedTip.Show(folderImagesRect.TopLeft() + CPoint(5,5), CInfoPanel::DirectionMode::Top, false);
	::SetWindowPos(m_FolderImages.m_hWnd,HWND_BOTTOM,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
	::SetWindowPos(queuedImages.m_hWnd,HWND_BOTTOM,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);

	imagePreview.Create(this);

	UploadManager::Instance().SetNotify(*this);
	ResetUploadState();

	mainContextMenu.LoadMenu(IDR_MAIN_MENU);
	accel = LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINACC));
	ASSERT(accel);

	folderContextMenu.LoadMenu(IDR_FOLDERMENU);
	m_FolderImages.menu = folderContextMenu.GetSubMenu(0);
	m_FolderImages.menuHandler = this;
	queueContextMenu.LoadMenu(IDR_QUEUEMENU);
	queuedImages.menu = queueContextMenu.GetSubMenu(0);
	queuedImages.menuHandler = this;

//	OnSize(SIZE_RESTORED, rect.right, rect.bottom);

//	CWnd* p = GetDlgItem(IDC_TEST_LIST1);
//	if(p)
//		BOOL b = list.SubclassWindow(*p);


	Load();


	return TRUE;  // return TRUE unless you set the focus to a control
}

void CUploadDialog::Save()
{
	const static int ArchiveVersion = 1;

	try
	{
		CString fileName = Utility::GetLocalSettingsPath() + _T("savedSelections.dat");
		CFile file(fileName, CFile::modeCreate|CFile::modeWrite);
		CArchive ar(&file, CArchive::store);
		CArray<PhotoReference*> jobs;
		LVITEM item;
		for(int i=queuedImages.GetItemCount()-1;i>=0;i--) {
			item.iItem = i;
			item.mask = LVIF_PARAM;
			queuedImages.GetItem(&item);
			PhotoReference* job = reinterpret_cast<PhotoReference*>(item.lParam);
			//TODO: maybe we should remain able to set names for finished items later?
			if(job->Finished())
				continue;
			jobs.Add(job);
		}


		ar << ArchiveVersion;
		ar << (int)jobs.GetCount();
		for(int i=0;i<jobs.GetCount();i++)
		{
			PhotoReference* job = jobs[i];
			CStringW ws;
			ws = job->GetPath();	ar << ws;
			ws = job->GetTitle();	ar << ws;
			ws = job->GetText();	ar << ws;
		//		ar << job->getkeywords();
		//		ar << job->GetRemoteId();
		}
	}
	catch(CException* e)
	{
		CString msg;
		e->GetErrorMessage(msg.GetBuffer(1000), 1000);
		msg.ReleaseBuffer();
		e->Delete();
	}
}

void CUploadDialog::Load()
{
	try
	{
		CString fileName = Utility::GetLocalSettingsPath() + _T("savedSelections.dat");
		if(!::PathFileExists(fileName))
			return;
		CFile file(fileName, CFile::modeRead);
		CArchive ar(&file, CArchive::load);
		
		int version;
		ar >> version;
		if(version != 1)
			return;

		int jobCount=0, loadedJobs=0;
		ar >> jobCount;
		for(int i=0;i<jobCount;i++)
		{
			CStringW path;
			ar >> path;

			if(!::PathFileExists(CW2T(path)))
				continue;            			
			PhotoReference* ref=0;
			AddFile(CString(CW2T(path)), &ref);
			if(!path)
				continue;
			loadedJobs++;
			CStringW s;
			ar >> s;
			ref->SetTitle(CW2T(s));

			ar >> s;
			ref->SetText(CW2T(s));
		}

		MoveSelected(m_FolderImages, queuedImages, TRUE);
		//HACK: This is an ugly hack:
		for(int i=0;i<queuedImages.GetItemCount();i++)
		{
			LRESULT r=0;
			DetailsEditDialog::NMGetItem nm;
			nm.index = i;
			OnItemDetailsUpdated((NMHDR*)&nm, &r);
		}

		if(loadedJobs<jobCount)
			MessageBox(R2T(IDS_STARTUP_JOBSMISSING), R2T(IDS_STARTUP_RESTORINGJOBS), MB_ICONINFORMATION);
	}
	catch(...)
	{
	}

}

void CUploadDialog::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

//	m_TopImage.SetWindowPos(&wndTop, cx-logowidth, 0, logowidth, logoheight,SWP_NOZORDER);
}

void CUploadDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FOLDER_IMAGES, m_FolderImages);
	DDX_Control(pDX, IDC_TOP_IMAGE, m_TopImage);
	DDX_Control(pDX, IDC_QUEUED_IMAGES, queuedImages);
	DDX_Control(pDX, IDC_PROGRESSBAR, m_ProgressBar);
	DDX_Control(pDX, IDC_FOUND2QUEUE_ALL, m_btnFound2QueueAll);
	DDX_Control(pDX, IDC_FOUND2QUEUE, m_btnFound2Queue);
	DDX_Control(pDX, IDC_QUEUE2FOUND, m_btnQueue2Found);
	DDX_Control(pDX, IDC_QUEUE2FOUND_ALL, m_btnQueue2FoundAll);
	DDX_Control(pDX, IDC_QUEUEHELP, m_btnQueueHelp);
	DDX_Control(pDX, IDC_BTNSHARESETTINGS, btnShare);
}


BEGIN_MESSAGE_MAP(CUploadDialog, CDialog)
	ON_WM_CLOSE()
	ON_WM_DROPFILES()
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_CONTROL(INFOCOMMAND_BUTTON2, IDD_INFO_UPLOADSTATUS, OnAbortUploadClicked)
	ON_CONTROL(INFOCOMMAND_BUTTON1, IDD_INFO_ADDPHOTOSTIP, OnGetStartedTipOk)
	ON_CONTROL(INFOCOMMAND_BUTTON2, IDD_INFO_ADDPHOTOSTIP, OnOpenMyPhotosFolder)
	ON_NOTIFY(DETAILS_NOTIFY_GETITEM, IDD_EDITDETAILSDLG, OnGetItemDetails)
	ON_NOTIFY(DETAILS_NOTIFY_ITEMUPDATED, IDD_EDITDETAILSDLG, OnItemDetailsUpdated)
	ON_NOTIFY(DETAILS_NOTIFY_SCROLL, IDD_EDITDETAILSDLG, OnItemDetailsScroll)
	ON_BN_CLICKED(IDC_FOUND2QUEUE, OnBnClickedFound2queue)
	ON_BN_CLICKED(IDC_QUEUE2FOUND, OnBnClickedQueue2found)
	ON_BN_CLICKED(IDC_FOUND2QUEUE_ALL, OnBnClickedFound2queueAll)
	ON_BN_CLICKED(IDC_QUEUE2FOUND_ALL, OnBnClickedQueue2foundAll)
	ON_BN_CLICKED(IDC_PROGRESSBAR, OnProgressBarClicked)
	ON_NOTIFY(NM_DBLCLK, IDC_FOLDER_IMAGES, OnNMDblclkFolderImages)
	ON_NOTIFY(NM_DBLCLK, IDC_QUEUED_IMAGES, OnNMDblclkFolderImages)
	ON_NOTIFY(LVN_KEYDOWN, IDC_QUEUED_IMAGES, OnLvnKeydownQueuedImages)
	ON_REGISTERED_MESSAGE(wm_style_installed_message, OnStyleInstalled)
	
	// job processing:
	ON_MESSAGE(WM_UPLOAD_FINISH,		OnUploadFinished)
	ON_MESSAGE(WM_UPLOAD_PROGRESSED,	OnUploadProgressed)
	ON_MESSAGE(WM_UPLOAD_TRYLOCKITEM,	OnUploadLock)
	ON_MESSAGE(WM_UPLOAD_UNLOCKITEM,	OnUploadUnlock)
	ON_MESSAGE(WM_UPLOAD_ITEMFINISHED,	OnUploadItemFinished)
	ON_MESSAGE(WM_UPLOAD_REQUIRELOGIN,	OnRequireLogin)
	ON_MESSAGE(WM_UPLOAD_GETDIRTYPHOTOTEXT, OnUploadGetDirtyPhotos)
	ON_MESSAGE(WM_UPLOAD_OLDCLIENT,		OnOldClient)

	ON_WM_RBUTTONUP()
	ON_COMMAND(ID_STYLE_GREEN, OnStyleGreen)
	ON_COMMAND(ID_STYLE_BLUE, OnStyleBlue)

	ON_COMMAND(ID_ESCAPE, OnEscape)
	ON_COMMAND(ID_RETURN, OnReturn)
	ON_BN_CLICKED(IDC_QUEUEHELP, OnBnClickedQueuehelp)
	ON_NOTIFY(LVN_BEGINDRAG, IDC_FOLDER_IMAGES, OnLvnBegindragFolderImages)
	ON_WM_COPYDATA()
	ON_COMMAND(ID_CLEAR_FOLDERLIST, OnClearFolderlist)
	ON_COMMAND(ID_REMOVE_FOLDERITEM, OnRemoveFolderitem)
	ON_COMMAND(ID_CLEAR_FINISHED, OnClearFinished)
	ON_COMMAND(ID_CLEAR_QUEUE, OnClearQueue)
	ON_COMMAND(ID_QUEUE_DELETEITEM, OnQueueDeleteitem)
	ON_BN_CLICKED(IDC_BTNSHARESETTINGS, OnBnClickedBtnsharesettings)
END_MESSAGE_MAP()


// CUploadDialog message handlers

void CUploadDialog::OnClose()
{

	if(!closing && m_bUploadInProgress)
	{
		if(IDYES != MessageBox(R2T(IDS_MAIN_ABORTQUESTION), R2T(IDS_MAIN_UPLOADINGCAPTION), MB_YESNO|MB_ICONWARNING|MB_DEFBUTTON2))
			return;
		this->closing = true;
		this->CloseWindow();
		UploadManager::Instance().Abort();
		return;
	}
	else
	{
		CArray<PhotoReference*> dirtyPhotos;
		GetDirtyPhotos(&dirtyPhotos, false);
		if(dirtyPhotos.GetSize() > 0)
		{
			updatingLast = true;
			UploadManager::Instance().RunDetailsUpdate();//&dirtyPhotos);
			return;
		}
	}

	m_bFinished = true;
	CDialog::OnClose();
}
void CUploadDialog::OnDestroy()
{
	Save();

	POSITION pos;
	for( pos = loadedPaths.GetStartPosition(); pos != NULL; )
	{
		CString key;
		PhotoReference* ref;
		loadedPaths.GetNextAssoc(pos, key, reinterpret_cast<void*&>(ref) );

		delete ref;
	}

	SAFE_DELETEOBJECT_NOL(m_TopImage.GetBitmap());

	CDialog::OnDestroy();
}

void CUploadDialog::OnDropFiles(HDROP hDropInfo)
{
	infoGetStartedTip.Hide();
	infoGetStartedTip.DestroyWindow();
	BeginWaitCursor();
	int num = ::DragQueryFile(hDropInfo,-1,0,0);
	int total = 0;
	for(int i=0;i<num;i++)
	{
		CString file;
		::DragQueryFile(hDropInfo, i, file.GetBuffer(MAX_PATH), MAX_PATH);
		file.ReleaseBuffer();

		total += AddFile(file);
	}

	CDialog::OnDropFiles(hDropInfo);
	EndWaitCursor();

	if(total == 0)
		MessageBox(R2T(IDS_MAIN_NONEFOUND), R2T(IDS_MAIN_ADDINGCAPTION), MB_ICONWARNING);
}

int CUploadDialog::AddFile(CString file, PhotoReference** ref)
{
	if(ref) *ref = 0;
	if(::PathIsDirectory(file)) {
		int num = 0;
		WIN32_FIND_DATA data;
		HANDLE find = ::FindFirstFile(file+_T("\\*"), &data);
		while(::FindNextFile(find, &data)) {
			if(_tcsncmp(_T(".."), data.cFileName, 2) == 0)
				continue;
			num += AddFile(file+_T('\\')+data.cFileName);
		}
		::FindClose(find);
		return num;
	}

	//Don't add twice:
	PhotoReference* foundRef = NULL;
	if(loadedPaths.Lookup(file, (void*&)foundRef))
		return 0;

	int imageIndex;
	Gdiplus::Bitmap* tn=0;
	Gdiplus::Image imgref(CT2W(file), FALSE);
	if(Gdiplus::Ok != imgref.GetLastStatus())
		return 0;
	tn = (Gdiplus::Bitmap*)imgref.GetThumbnailImage(PreviewSize,PreviewSize,0,0);
	if(tn == NULL || Ok != tn->GetLastStatus())
		return 0;
	HBITMAP hthumbnail;//todo: close handle
	Gdiplus::Status s = tn->GetHBITMAP(MAGENTA, &hthumbnail);
	if(s != Gdiplus::Ok) {
		delete tn;
		return 0;
	}
	CBitmap* bitmap = CBitmap::FromHandle(hthumbnail); //autodeleted
	imageIndex = imageList.Add(bitmap, MAGENTA);
	delete tn;
	PhotoReference* state = new PhotoReference(file);
	if(ref)
		*ref = state;
	state->imageIndex = imageIndex;
	int position;
	position = m_FolderImages.InsertItem(LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM, 0, state->GetTitle(), LVIS_SELECTED, LVIS_SELECTED, imageIndex, (LPARAM)state);

	loadedPaths.SetAt(file, state);

	return 1;
}


void CUploadDialog::StartUpload()
{
	if(m_bUploadInProgress)
		return;

	if(!UploadManager::Instance().HasJobs())
	{
		MessageBox(R2T(IDS_MAIN_STARTINGWITHNOTHING), R2T(IDS_MAIN_STARTINGCAPTION), MB_ICONINFORMATION);
		return;
	}

	UploadSettingsDialog dlg(this);
	if(IDOK != dlg.DoModal())
		return;

	m_bUploadInProgress = true;


	m_ProgressBar.SetCaption(R2T(IDS_MAIN_STARTINGUPLOAD));

	UploadManager::Instance().Start(shareSettings, dlg.GetSize(), dlg.GetQuality());
}



void CUploadDialog::MoveSelected(CListCtrl& from, CListCtrl& to, BOOL all)
{
	int  nItem		= -2;
	bool movable	= false;
	bool stop		= false;
	bool hasMoved	= false;
	TCHAR name[PhotoReference::MAX_PHOTO_NAME_LEN+1];
	
	while(true || nItem != -1)
	{
		if(stop&&hasMoved)
			break;
		nItem = from.GetNextItem(nItem==-2?-1:nItem, all?LVNI_ALL:LVNI_SELECTED);
		if(!all && nItem == -1) {
			nItem	= hasMoved?-1:from.GetItemCount()==0?-1:0;
			stop	= true;
			all		= TRUE;
		}
		if(nItem == -1)
			break;

		LVITEM item;
		ZeroMemory(&item,sizeof(item));
		item.mask			= LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM|LVIF_STATE;
		item.stateMask		= -1;
		item.iItem			= nItem;
		item.pszText		= name;
		item.cchTextMax		= PhotoReference::MAX_PHOTO_NAME_LEN;
		from.GetItem(&item);

		//check if we can move it:
		PhotoReference* ref = reinterpret_cast<PhotoReference*>(item.lParam);
		if(ref->LockedStatus() || ref->Finished())
		{
			movable = false;
		}
		else
		{
			if(to == this->queuedImages)
			{
				UploadManager::Instance().Enqueue(ref);
				movable = true;
			}
			else
			{
				movable = UploadManager::Instance().Unenqueue(ref);
			}
		}
		if(movable)
		{
			hasMoved = true;
			from.DeleteItem(item.iItem);
			--nItem;
			item.iItem = to.GetItemCount();
			to.InsertItem(&item);
		}

	} 
}


void CUploadDialog::OnBnClickedFound2queue()
{
	MoveSelected(m_FolderImages, queuedImages);
}

void CUploadDialog::OnBnClickedQueue2found()
{
	MoveSelected(queuedImages, m_FolderImages);
}

void CUploadDialog::OnBnClickedFound2queueAll()
{
	MoveSelected(m_FolderImages, queuedImages, TRUE);
}

void CUploadDialog::OnBnClickedQueue2foundAll()
{
	MoveSelected(queuedImages, m_FolderImages, TRUE);
}


void CUploadDialog::OnPaint()
{
	CPaintDC	tempDC			(this);
	Graphics	g				(tempDC.m_hDC);
	CRect		window			;GetClientRect(&window);
	
	
	//Draw gradient body:
	StyleObject<Brush>	bodyBrush	= Style::Instance().GetDefaultBodyBrush(window);
	g.FillRectangle(bodyBrush,0,0,window.right,window.bottom);

	/////////////////////////////////////////////////////////////////////////////
	// Draw top
	
	//area
	StyleObject<Brush>	topBrush	= Style::Instance().GetMainTopBrush();
	g.FillRectangle(topBrush, 0,0,window.right, logoheight);

	StyleObject<Pen>	topPen		= Style::Instance().GetMainTopLine();
	g.DrawLine(topPen, 0, logoheight+1, window.right, logoheight+1);

 	g.SetSmoothingMode(SmoothingModeHighQuality);
	g.SetTextRenderingHint(TextRenderingHintAntiAlias);

	//title
	StyleObject<Brush>	textBrush	= Style::Instance().GetDefaultLabelBrush();
	StyleObject<Font>	titleFont	= Style::Instance().GetMainTitleFont();
	g.DrawString(R2W(IDS_MAIN_TITLE),-1,titleFont,PointF(14,5),textBrush);

	/////////////////////////////////////////////////////////////////////////////
	// Draw text
	CRect pos;
	StyleObject<Font>	headerFont	= Style::Instance().GetDefaultHeaderFont();

	m_FolderImages.GetWindowRect(&pos);
	ScreenToClient(&pos);
	g.DrawString(R2W(IDS_MAIN_FOUNDPHOTOS), -1,headerFont,PointF((float)pos.left+10,90),textBrush);	
	queuedImages.GetWindowRect(&pos);
	ScreenToClient(&pos);
	g.DrawString(R2W(IDS_MAIN_QUEUEDPHOTOS),-1,headerFont,PointF((float)pos.left+10,90),textBrush);
}



void CUploadDialog::OnNMDblclkFolderImages(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE arg	= reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	CListCtrl* list			= static_cast<CListCtrl*>(CWnd::FromHandle(arg->hdr.hwndFrom));
	LVITEM item;
	ZeroMemory(&item, sizeof(item));
	item.iItem = arg->iItem;
	item.mask = LVIF_PARAM;
	BOOL b = list->GetItem(&item);
	if(!b) return;

	PhotoReference* ref = reinterpret_cast<PhotoReference*>(item.lParam);

	imagePreview.SetImage(ref->GetPath());
	imagePreview.Show();

	*pResult = 0;
}


void SetOverlay(CListCtrl& view, const PhotoReference* ref, int overlay)
{
	int item = Utility::FindItem(view, reinterpret_cast<LPARAM>(ref));
	if(item >= 0)
		Utility::SetOverlay(view, item, overlay);
}

LRESULT CUploadDialog::OnRequireLogin(WPARAM wParam, LPARAM lParam)
{
	UploadManager::LoginResult*			result		= reinterpret_cast<UploadManager::LoginResult*>(wParam);
	UploadManager::LoginCredentials*	credentials	= reinterpret_cast<UploadManager::LoginCredentials*>(lParam);

	LoginDialog dlg(this);
	if(result)
	{
		dlg.SetResult(result);
	}
	if(IDOK != dlg.DoModal())
		return IDCANCEL;
	else
	{
		credentials->persistLogin	= dlg.GetPersistLogin();
		credentials->username		= dlg.GetUsername();
		credentials->password		= dlg.GetPassword();
		return IDOK;
	}
}

LRESULT CUploadDialog::OnUploadFinished(WPARAM wParam, LPARAM lParam)
{
	UploadManager::UploadStatus*		status = reinterpret_cast<UploadManager::UploadStatus*>(wParam);
	UploadManager::UploadEndedResult	result = (UploadManager::UploadEndedResult)(lParam);
	m_bUploadInProgress = false;
	if(closing) {

		CArray<PhotoReference*> dirtyPhotos;
		GetDirtyPhotos(&dirtyPhotos, false);
		if(dirtyPhotos.GetSize() > 0)
		{
			updatingLast = true;
			UploadManager::Instance().RunDetailsUpdate();//&dirtyPhotos);
			return 0;
		}
		else
			CDialog::OnOK();
	}
	else {
		switch(result) {
			case UploadManager::UploadEndedResult::UploadCancelled:
				break;

			case UploadManager::UploadEndedResult::UploadAborted:
				MessageBox(
					R2T(IDS_MAIN_UPLOADRESULT_ABORTED),
					R2T(IDS_MAIN_UPLOADINGCAPTION),
					MB_OK|MB_ICONINFORMATION
					);
				break;

			case UploadManager::UploadEndedResult::UploadNetworkError:
				MessageBox(
					R2T(IDS_MAIN_UPLOADRESULT_NETWORKERROR),
					R2T(IDS_MAIN_UPLOADINGCAPTION),
					MB_OK|MB_ICONERROR
					);
				break;

			case UploadManager::UploadEndedResult::UploadPermanentError:
				MessageBox(
					R2T(IDS_MAIN_UPLOADRESULT_GENERALERROR),
					R2T(IDS_MAIN_UPLOADINGCAPTION),
					MB_OK|MB_ICONERROR
					);
				m_ProgressBar.ShowWindow(SW_HIDE);
				//TODO: disable further uploads.
				break;

			case UploadManager::UploadEndedResult::UploadEndSuccess:
				MessageBox(
					R2T(IDS_MAIN_UPLOADRESULT_FINISHED),
					R2T(IDS_MAIN_UPLOADINGCAPTION),
					MB_OK|MB_ICONINFORMATION
					);
				break;
			default:
				ASSERT(FALSE);
		}
	}
	ResetUploadState();
	return 0;
}

LRESULT CUploadDialog::OnUploadProgressed(WPARAM wParam, LPARAM lParam)
{
	UploadManager::UploadStatus* status = reinterpret_cast<UploadManager::UploadStatus*>(wParam);
	m_ProgressBar.SetCurrentValue(status->percentEstimate);

	CString caption;
	int secondsLeft = status->remainingTime/1000;
	if(secondsLeft == 0)
		caption = R2T(IDS_PROGRESSBAR_INDETERMINATE);
	else if(secondsLeft < 60)
		caption =  R2T(IDS_PROGRESSBAR_1);
	else
	{
		int minutesLeft = secondsLeft/60;
		caption.FormatMessage(minutesLeft == 1?IDS_PROGRESSBAR_1P:IDS_PROGRESSBAR_N
			,minutesLeft
			);
	}

	m_ProgressBar.SetCaption(caption);

	return 0;
}

LRESULT CUploadDialog::OnUploadLock(WPARAM wParam, LPARAM lParam)
{
	PhotoReference* ref = reinterpret_cast<PhotoReference*>(wParam);

	ref->Lock();
	ASSERT(ref->ownerThread = UploadManager::Instance().GetThreadId());
	int item = Utility::FindItem(queuedImages, reinterpret_cast<LPARAM>(ref));
	if(item >= 0)
		Utility::SetOverlay(queuedImages, item, RUNNING_OVERLAY);

	return UploadManager::LockResult_Ok;
}

LRESULT CUploadDialog::OnUploadUnlock(WPARAM wParam, LPARAM lParam)
{
	PhotoReference* ref = reinterpret_cast<PhotoReference*>(wParam);
	ASSERT(ref->ownerThread = ::GetCurrentThreadId());
	ref->Unlock();
	SetOverlay(queuedImages, ref, 0);
	return 0;
}

LRESULT CUploadDialog::OnUploadItemFinished(WPARAM wParam, LPARAM lParam)
{
	PhotoReference* ref = reinterpret_cast<PhotoReference*>(wParam);

	ASSERT(!ref->LockedStatus());
	if(lParam == 0) {
		SetOverlay(queuedImages, ref, FINISHED_OVERLAY);
	}
	else {
		SetOverlay(queuedImages, ref, ERROR_OVERLAY);
	}

	return 0;
}

void CUploadDialog::GetDirtyPhotos(CArray<PhotoReference*>* dirtyPhotos, bool lock)
{
	POSITION pos;
	for( pos = loadedPaths.GetStartPosition(); pos != NULL; )
	{
		CString key;
		PhotoReference* ref;
		loadedPaths.GetNextAssoc(pos, key, reinterpret_cast<void*&>(ref) );

		if(ref->Finished() && ref->GetDirty() && !ref->LockedStatus()) {
			if(lock) {		
				ref->Lock();
				ASSERT(ref->ownerThread = UploadManager::Instance().GetThreadId());
			}
			dirtyPhotos->Add(ref);
		}
	}
}

LRESULT CUploadDialog::OnUploadGetDirtyPhotos(WPARAM wParam, LPARAM lParam)
{
	CArray<PhotoReference*>* dirtyPhotos = reinterpret_cast<CArray<PhotoReference*>*>(wParam);

	if(lParam == 0)
	{
		ASSERT(dirtyPhotos->GetCount() == 0);
		GetDirtyPhotos(dirtyPhotos);
	}
	else
	{
		for(int i=0;i<dirtyPhotos->GetCount();i++)
		{
			PhotoReference* ref = dirtyPhotos->GetAt(i);
			ASSERT(ref->ownerThread = ::GetCurrentThreadId());
			ref->Unlock();
		}

		if(updatingLast)
			CDialog::OnOK();
	}

	return 0;
}


void CUploadDialog::OnAbortUploadClicked()
{
	if(IDYES != MessageBox(R2T(IDS_MAIN_ABORTUPLOADQUESTION), R2T(IDS_MAIN_UPLOADINGCAPTION), MB_ICONQUESTION|MB_YESNO))
		return;
	UploadManager::Instance().Abort();
}
void CUploadDialog::OnOpenMyPhotosFolder()
{
	infoGetStartedTip.Hide();
	Utility::OpenMyPictures();
}
void CUploadDialog::OnGetStartedTipOk()
{
	infoGetStartedTip.Hide();
}

CString Value2Str(int val)
{
	if(val == 0)
		return R2T(IDS_VALUE2STR_L1);
	CString f;
	f.Format(R2T(IDS_VALUE2STR_V), val);
	return f;
}

void CUploadDialog::OnProgressBarClicked()
{
	if(!m_bUploadInProgress)
	{
		StartUpload();
	}
	else
	{
		UploadManager::UploadStatus status;
		UploadManager::Instance().GetStatus(&status);

		CString title;
		CString text;
		CString size;
		CString speed;

		title.Format(status.queuedItems==1?IDS_STATUS_TITLE_1:IDS_STATUS_TITLE_N, 
 			status.queuedItems);

		size.FormatMessage(IDS_STATUS_SIZE
			,Value2Str((status.queuedBytesEstimate/1024)/1024)
			,Value2Str((status.finishedBytes/1024)/1024)
			);

		speed.FormatMessage(IDS_STATUS_SPEED
			,Value2Str(status.kBpS)
			,Value2Str(status.MBpM)
			);
		
		text.Format(
				R2T(status.finishedItems<2
					?IDS_STATUS_REMAINING_1N
					:IDS_STATUS_REMAINING_NN
					),
			status.finishedItems, 
			status.queuedItems-status.finishedItems, 
			status.minutesRemainingEstimate
			);

		infoPanel.SetButtonText(null,R2T(IDS_MAIN_INPROGRESS_ABORTBTN)); //TODO: IDS_MAIN_INPROGRESS_RUNINBAKGROUNDBTN
		infoPanel.SetText(title, size+_T("\r\n\r\n")+speed+_T("\r\n\r\n")+text);

		CRect rect;
		m_ProgressBar.GetWindowRect(rect);
		ScreenToClient(rect);
		infoPanel.Show(rect.TopLeft());
	}
}

void CUploadDialog::ResetUploadState()
{
	m_bUploadInProgress = false;
	m_ProgressBar.SetCaption(R2T(IDS_MAIN_PROGRESSBARSTART));
	m_ProgressBar.SetCurrentValue(0);
}

void CUploadDialog::OnLvnKeydownQueuedImages(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);

	if(false
		|| pLVKeyDow->wVKey == VK_UP 
		|| pLVKeyDow->wVKey == VK_DOWN
		|| pLVKeyDow->wVKey == VK_LEFT
		|| pLVKeyDow->wVKey == VK_RIGHT
		|| pLVKeyDow->wVKey == VK_ESCAPE
		|| pLVKeyDow->wVKey == 18 //alt
		|| (pLVKeyDow->wVKey >= VK_F1 && pLVKeyDow->wVKey <= VK_F24)
		)

		return;

	CString n;
	if(::IsCharAlphaNumeric((TCHAR)pLVKeyDow->wVKey))
		n.AppendChar( (TCHAR)pLVKeyDow->wVKey);
	else
		return;

	int selected = queuedImages.GetNextItem(-1, LVNI_FOCUSED);
	if(selected == -1)
		MessageBeep(-1);
	else
	{
		detailsEditDialog.SetItemIndex(selected);
		detailsEditDialog.Show(n);
		queuedImages.EnsureVisible(selected, FALSE);
	}

	*pResult = 0;
}

void CUploadDialog::OnGetItemDetails(NMHDR *pNMHDR, LRESULT *pResult)
{
	DetailsEditDialog::NMGetItem* nm = reinterpret_cast<DetailsEditDialog::NMGetItem*>(pNMHDR);
	// negative value signals next from 0-index
	int a = nm->index & GETDETAILS_NEXTFLAG;
	if(nm->index & ~GETDETAILS_VALUEMASK)
	{
		int indexValue	=    nm->index & GETDETAILS_VALUEMASK;
		bool previous	= 0!=(nm->index & GETDETAILS_PREVIOUSFLAG);
		bool next		= 0!=(nm->index & GETDETAILS_NEXTFLAG);
		int n = queuedImages.GetNextItem(indexValue, previous?LVNI_ABOVE:LVNI_BELOW);
		if(n == -1 && previous)
			nm->index = indexValue;
		else { if(n == indexValue)
			nm->index = -1;
		else
			nm->index = n;
		}
	}
	// if not found, return null
	if(nm->index < 0) {
		nm->reference = NULL;
		return;
	}
	LVITEM item;
	ZeroMemory(&item, sizeof(item));
	item.iItem		= nm->index;
	item.mask		= LVIF_PARAM;
	BOOL b = queuedImages.GetItem(&item);
	if(!b)
		nm->reference = NULL;
	else
		nm->reference = reinterpret_cast<PhotoReference*>(item.lParam);

	*pResult = 0;
}

void CUploadDialog::OnItemDetailsUpdated(NMHDR *pNMHDR, LRESULT *pResult)
{
	DetailsEditDialog::NMGetItem* nm = reinterpret_cast<DetailsEditDialog::NMGetItem*>(pNMHDR);
	
	LVITEM item;
	ZeroMemory(&item, sizeof(item));
	item.iItem		= nm->index;
	item.mask		= LVIF_PARAM;
	BOOL b = queuedImages.GetItem(&item);
	if(b)
	{
		PhotoReference* ref = reinterpret_cast<PhotoReference*>(item.lParam);
		if(!ref)
			return;
		item.mask = LVIF_TEXT;
		CString title = ref->GetTitle();
		item.pszText = title.GetBuffer();
		queuedImages.SetItem(&item);
		title.ReleaseBuffer();
	}
}

void CUploadDialog::OnItemDetailsScroll(NMHDR *pNMHDR, LRESULT *pResult)
{
	DetailsEditDialog::NMGetItem* nm = reinterpret_cast<DetailsEditDialog::NMGetItem*>(pNMHDR);
	ASSERT(nm->index >= 0);
	queuedImages.SetSelectionMark(nm->index);
	queuedImages.EnsureVisible(nm->index, FALSE);
}

void CUploadDialog::OnOK()
{
	//Don't just close the window when press enter or escape:
	//CDialog::OnOK();
}

void CUploadDialog::OnCancel()
{
	CDialog::OnCancel();
}

/*

void CUploadDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	const MSG* msg = GetCurrentMessage();
	this->SendMessage(WM_NCMOUSEMOVE, msg->wParam, msg->lParam);
	CDialog::OnMouseMove(nFlags, point);
}

void CUploadDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	const MSG* msg = GetCurrentMessage();
	this->SendMessage(WM_NCLBUTTONDOWN, msg->wParam, msg->lParam);
}

void CUploadDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	const MSG* msg = GetCurrentMessage();
	this->SendMessage(WM_NCLBUTTONUP, msg->wParam, msg->lParam);
}
*/

void CUploadDialog::OnRButtonUp(UINT nFlags, CPoint point)
{
	int currentStyle = BlueStyle::IsInstalled()?ID_STYLE_BLUE:Style::IsInstalled()?ID_STYLE_GREEN:0;
	mainContextMenu.CheckMenuRadioItem(ID_STYLE_GREEN, ID_STYLE_BLUE, currentStyle, MF_BYCOMMAND);
	ClientToScreen(&point);
	CMenu* p = mainContextMenu.GetSubMenu(0);
	if(p)
		p->TrackPopupMenu(TPM_LEFTBUTTON, point.x, point.y, this);

	CDialog::OnRButtonUp(nFlags, point);
}

void CUploadDialog::OnStyleGreen()
{
	Style::Install();
	Style::Instance().UpdateDisplay(this->GetSafeHwnd());
	AfxGetApp()->WriteProfileInt(SECTION_UISETTINGS, _T("Theme"), 0);
	USAGE("Style set to green");
}

void CUploadDialog::OnStyleBlue()
{
	BlueStyle::Install();
	Style::Instance().UpdateDisplay(this->GetSafeHwnd());
	AfxGetApp()->WriteProfileInt(SECTION_UISETTINGS, _T("Theme"), 1);
	USAGE("Style set to blue");
}

LRESULT CUploadDialog::OnStyleInstalled(WPARAM wParam, LPARAM lParam)
{
	Style* style = reinterpret_cast<Style*>(wParam);

	Gdiplus::Bitmap topImage(::AfxGetInstanceHandle(), style->GetTopImageResourceName());
	HBITMAP htopbm;
	((Gdiplus::Bitmap*)&topImage)->GetHBITMAP(MAGENTA, &htopbm);
	SAFE_DELETEOBJECT_NOL(m_TopImage.GetBitmap());
	m_TopImage.SetBitmap(htopbm);

	return 0;
}

BOOL CUploadDialog::PreTranslateMessage(MSG* pMsg)
{
	do
	{
		if(accel && pMsg->message == WM_KEYDOWN)
		{
			if(0==(pMsg->lParam&0x20000000))
			{
				if(pMsg->wParam == VK_RETURN && detailsEditDialog.IsWindowVisible())
					break;
				if(TranslateAccelerator(this->m_hWnd, accel, pMsg))
					return TRUE;
			}
		}
	}
	while(false);

	return CDialog::PreTranslateMessage(pMsg);
}

void CUploadDialog::OnEscape()
{
	if(detailsEditDialog.IsWindowVisible())
		detailsEditDialog.Hide();
	else if(imagePreview.IsWindowVisible())
		imagePreview.Hide();
}

void CUploadDialog::OnReturn()
{
	if(imagePreview.IsWindowVisible()) {
		imagePreview.Hide();
		return;
	}

	if(&queuedImages == GetFocus())
	{
		POSITION pos = queuedImages.GetFirstSelectedItemPosition();
		if(!pos)
			return;

		int n = queuedImages.GetNextSelectedItem(pos);

		LVITEM item;
		ZeroMemory(&item, sizeof(item));
		item.iItem = n;
		item.mask = LVIF_PARAM;
		BOOL b = queuedImages.GetItem(&item);
		if(!b) return;

		PhotoReference* ref = reinterpret_cast<PhotoReference*>(item.lParam);

		imagePreview.SetImage(ref->GetPath());
		imagePreview.Show();

	}
}

void CUploadDialog::OnBnClickedQueuehelp()
{
	CRect rect;
	this->GetDlgItem(IDC_QUEUEHELP)->GetWindowRect(rect);
	ScreenToClient(rect);
	infoPanelMisc.SetText(
		R2T(IDS_MAIN_QUEUEHELP_TITLE),
		R2T(IDS_MAIN_QUEUEHELP_TEXT)
		);

	infoPanelMisc.Show(CPoint(rect.left,rect.top), CInfoPanel::DirectionMode::Top);
}

void CUploadDialog::OnLvnBegindragFolderImages(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	//imageList.BeginDrag(0, CPoint(50, 50));
	//imageList.DragEnter(&m_FolderImages, CPoint(0,0));
	*pResult = 0;
}


//static, sends a wm_copydata message.
bool CUploadDialog::StaticSetTargetCategory(const CString& instanceName, const CString& categoryName)
{
	CWnd* wnd = CDialogControlTarget::FindInstance(instanceName);
	if(!wnd)
		return false;

	COPYDATASTRUCT copy;
	copy.dwData = 1;
	copy.cbData = sizeof TCHAR * (1+categoryName.GetLength());
	copy.lpData = (LPVOID)(LPCTSTR)categoryName;

	LRESULT res = wnd->SendMessage(WM_COPYDATA, 0, reinterpret_cast<LPARAM>(&copy));
	return res?true:false;
}

bool CUploadDialog::StaticBringWindowToTop(const CString& instanceName)
{
	CWnd* wnd = CDialogControlTarget::FindInstance(instanceName);
	if(!wnd)
		return false;

	COPYDATASTRUCT copy;
	copy.dwData = 4;
	copy.cbData = 0;
	copy.lpData = 0;

	LRESULT res = wnd->SendMessage(WM_COPYDATA, 0, reinterpret_cast<LPARAM>(&copy));
	return res?true:false;
}

bool CUploadDialog::StaticSetQuota(const CString& instanceName, UINT quota)
{
	CWnd* wnd = CDialogControlTarget::FindInstance(instanceName);
	if(!wnd)
		return false;

	COPYDATASTRUCT copy;
	copy.dwData = 2;
	copy.cbData = 4;
	copy.lpData = &quota;

	LRESULT res = wnd->SendMessage(WM_COPYDATA, 0, reinterpret_cast<LPARAM>(&copy));
	return res?true:false;
}

bool CUploadDialog::StaticSetUsedQuota(const CString& instanceName, UINT quota)
{
	CWnd* wnd = CDialogControlTarget::FindInstance(instanceName);
	if(!wnd)
		return false;

	COPYDATASTRUCT copy;
	copy.dwData = 3;
	copy.cbData = 4;
	copy.lpData = &quota;

	LRESULT res = wnd->SendMessage(WM_COPYDATA, 0, reinterpret_cast<LPARAM>(&copy));
	return res?true:false;
}

BOOL CUploadDialog::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct)
{
	if(pCopyDataStruct->dwData == 1)
	{
		CString path;
		path = static_cast<LPCTSTR>(pCopyDataStruct->lpData);
		if(path[0] == _T('{'))
		{
			GUID id;
			ZeroMemory(&id, sizeof(id));
			if(SUCCEEDED(CLSIDFromString(CT2W(path), &id)))
				this->shareSettings.categoryId = id;
			path = path.Right(path.GetLength() - 38);
		}
		this->shareSettings.categoryName = path;
	}
	else if(pCopyDataStruct->dwData == 2)
	{
		UINT quota = 0;
		ASSERT(pCopyDataStruct->cbData == sizeof quota);
		quota = *static_cast<UINT*>(pCopyDataStruct->lpData);
		UploadManager::Instance().Quota = quota;
	}
	else if(pCopyDataStruct->dwData == 3)
	{
		UINT usedQuota = 0;
		ASSERT(pCopyDataStruct->cbData == sizeof usedQuota);
		usedQuota = *static_cast<UINT*>(pCopyDataStruct->lpData);
		UploadManager::Instance().UsedQuota = usedQuota;
	}
	else if(pCopyDataStruct->dwData == 4)
	{
		CWnd *pWndChild = null;
	
		// do we have any popups?
		pWndChild = GetLastActivePopup();

		// If iconic, restore the main window
		if (IsIconic())
			ShowWindow(SW_RESTORE);

		// Bring the main window or its popup to the foreground
		pWndChild->SetForegroundWindow();
	}

	return TRUE;
}

LRESULT CUploadDialog::OnOldClient(WPARAM, LPARAM)
{
	MessageBox(
		R2T(IDS_MAIN_CLIENTOLD_TEXT),
		R2T(IDS_MAIN_ERROR),
		MB_OK|MB_ICONERROR);
	return 0;
}

BEGIN_MESSAGE_MAP(CUploadDialog::MenuListView, CListCtrl)
	ON_WM_MOUSEMOVE()
	ON_NOTIFY_REFLECT(NM_RCLICK, OnNMRclick)
END_MESSAGE_MAP()


void CUploadDialog::MenuListView::OnMouseMove(UINT nFlags, CPoint point)
{
	this->mousePos = point;

	CListCtrl::OnMouseMove(nFlags, point);
}


void CUploadDialog::OnClearFolderlist()
{
	if(IDYES == MessageBox(R2T(IDS_CLEAR_LISTQUESTION), R2T(IDS_CLEAR_LIST), MB_YESNO|MB_ICONQUESTION))
	{
		for(int i=0;i<m_FolderImages.GetItemCount();i++)
		{
			LVITEM item;
			item.iItem = i;
			item.mask = LVIF_PARAM;
			m_FolderImages.GetItem(&item);
			PhotoReference* job = reinterpret_cast<PhotoReference*>(item.lParam);
			loadedPaths.RemoveKey( job->GetPath() );
			delete job;
		}
		m_FolderImages.DeleteAllItems();
	}
}


void CUploadDialog::OnRemoveFolderitem()
{
	if(m_FolderImages.clickedItem != -1)
	{
		LVITEM item;
		item.iItem = m_FolderImages.clickedItem;
		item.mask = LVIF_PARAM;
		m_FolderImages.GetItem(&item);
		PhotoReference* job = reinterpret_cast<PhotoReference*>(item.lParam);
		loadedPaths.RemoveKey( job->GetPath() );
		delete job;

		m_FolderImages.DeleteItem(m_FolderImages.clickedItem);
	}
}


void CUploadDialog::MenuListView::OnNMRclick(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE p = (LPNMITEMACTIVATE)pNMHDR;
	
	this->clickedItem = p->iItem;
	CPoint pt = this->mousePos;
	this->ClientToScreen(&pt);

	menu->EnableMenuItem(ID_CLEAR_FOLDERLIST,	MF_BYCOMMAND|(GetItemCount()?MF_ENABLED:(MF_GRAYED|MF_DISABLED)));
	menu->EnableMenuItem(ID_REMOVE_FOLDERITEM,	MF_BYCOMMAND|(clickedItem==-1?(MF_GRAYED|MF_DISABLED):MF_ENABLED));

	bool hasUnfinishedJobs		= false;
	bool hasFinishedJobs		= false;
	bool selectedInProgress		= true;
	if(clickedItem != -1)
	{
		LVITEM item;
		item.iItem = clickedItem;
		item.mask = LVIF_PARAM;
		GetItem(&item);
		PhotoReference* job = reinterpret_cast<PhotoReference*>(item.lParam);
		selectedInProgress = job->LockedStatus();
	}
	for(int i=0;i<GetItemCount();i++)
	{
		LVITEM item;
		item.iItem = i;
		item.mask = LVIF_PARAM;
		GetItem(&item);
		PhotoReference* job = reinterpret_cast<PhotoReference*>(item.lParam);
		if(job->LockedStatus())
			continue;
		if(job->Finished()) {
			hasFinishedJobs = true;
		}
		else
			hasUnfinishedJobs = true;
	}
	menu->EnableMenuItem(ID_CLEAR_QUEUE,	MF_BYCOMMAND|hasUnfinishedJobs?MF_ENABLED:(MF_GRAYED|MF_DISABLED));
	menu->EnableMenuItem(ID_CLEAR_FINISHED,	MF_BYCOMMAND|hasFinishedJobs?MF_ENABLED:(MF_GRAYED|MF_DISABLED));
	menu->EnableMenuItem(ID_QUEUE_DELETEITEM,	MF_BYCOMMAND|(selectedInProgress||clickedItem==-1)?(MF_GRAYED|MF_DISABLED):MF_ENABLED);

	if(this->menu)
		this->menu->TrackPopupMenuEx(0,pt.x,pt.y,menuHandler?menuHandler:this,NULL);

	*pResult = 0;
}


void CUploadDialog::OnClearFinished()
{
	if(IDYES != MessageBox(R2T(IDS_CLEAR_FINISHEDLIST), R2T(IDS_CLEARLIST), MB_ICONQUESTION|MB_YESNO))
		return;
	for(int i=0;i<queuedImages.GetItemCount();i++)
	{
		LVITEM item;
		item.iItem = i;
		item.mask = LVIF_PARAM;
		queuedImages.GetItem(&item);
		PhotoReference* job = reinterpret_cast<PhotoReference*>(item.lParam);
		if(job->LockedStatus())
			continue;
		if(job->Finished()) {
			loadedPaths.RemoveKey( job->GetPath() );
			delete job;
			queuedImages.DeleteItem(i--);
		}
	}
}

void CUploadDialog::OnClearQueue()
{
	if(IDYES != MessageBox(R2T(IDS_CLEAR_QUEUE), R2T(IDS_CLEARLIST), MB_ICONQUESTION|MB_YESNO))
		return;

	for(int i=0;i<queuedImages.GetItemCount();i++)
	{
		LVITEM item;
		item.iItem = i;
		item.mask = LVIF_PARAM;
		queuedImages.GetItem(&item);
		PhotoReference* job = reinterpret_cast<PhotoReference*>(item.lParam);
		if(job->LockedStatus())
			continue;
		if(!job->Finished()) {
			if(!UploadManager::Instance().Unenqueue(job))
				continue;
			loadedPaths.RemoveKey( job->GetPath() );
			delete job;
			queuedImages.DeleteItem(i--);
		}
	}
}

void CUploadDialog::OnQueueDeleteitem()
{
	if(-1 == queuedImages.clickedItem)
		return;

	LVITEM item;
	item.iItem = queuedImages.clickedItem;
	item.mask = LVIF_PARAM;
	queuedImages.GetItem(&item);
	PhotoReference* job = reinterpret_cast<PhotoReference*>(item.lParam);
	if(job->LockedStatus())
		return;
	if(!UploadManager::Instance().Unenqueue(job))
		return;
	loadedPaths.RemoveKey( job->GetPath() );
	delete job;
	queuedImages.DeleteItem(queuedImages.clickedItem);
}

void CUploadDialog::OnBnClickedBtnsharesettings()
{
	if(UploadManager::Instance().UploadState)
	{
		MessageBox(R2T(IDS_MAIN_CANTEDITSHAREWHILEUPLOADING), R2T(IDS_MAIN_UPLOADINPROGRES), MB_OK|MB_ICONINFORMATION);
		return;
	}

	ShareDialog shareDialog(shareSettings);
	if(IDOK == shareDialog.DoModal())
		 shareDialog.GetShareSettings(this->shareSettings);
}
