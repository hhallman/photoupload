// ShareDialog.cpp : implementation file
//

#include "stdafx.h"
#include "MinakortControls.h"
#include "ShareDialog.h"
#include ".\sharedialog.h"
#include ".\defines.h"
#include ".\utility.h"
#include ".\HttpStream.h"
#include "InPlaceList.h"
#include ".\UploadManager.h"

// ShareDialog dialog

#define S_CANCELLED 200

IMPLEMENT_DYNAMIC(ShareDialog, CDialog)
ShareDialog::ShareDialog(const ShareSettings& settings, CWnd* pParent /*=NULL*/)
	: CDialog(ShareDialog::IDD, pParent), loadSettings(settings)
{
}

ShareDialog::~ShareDialog()
{
}

void ShareDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SHARELIST, shareList);
}


BEGIN_MESSAGE_MAP(ShareDialog, CDialog)
//	ON_NOTIFY(NM_DBLCLK, IDC_SHARELIST, OnNMDblclkSharelist)
//	ON_NOTIFY(NM_CLICK, IDC_SHARELIST, OnNMClickSharelist)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_SHARELIST, OnLvnEndlabeleditSharelist)
	ON_NOTIFY(LVN_ITEMACTIVATE, IDC_SHARELIST, OnLvnItemActivateSharelist)
	ON_BN_CLICKED(IDC_SHARETOFRIENDS, OnBnClickedSharetofriends)
	ON_EN_KILLFOCUS(IDC_CATEGORYNAME, OnEnKillfocusCategoryname)
	ON_NOTIFY(LVN_DELETEITEM, IDC_SHARELIST, OnLvnDeleteitemSharelist)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()

static const TCHAR LOOKUP_QUERY_STRING[]	= _T("public/controlCommand.aspx?uploadctrl=y&v=1&action=userlookup");
static const TCHAR CATEGORY_QUERY_STRING[]	= _T("public/controlCommand.aspx?uploadctrl=y&v=1&action=categorylookup");



HRESULT LookupMember(int item, MemberList* members, LPCTSTR query)
{
	WaitCursor waitCursor;

	try
	{
		CString searchQuery = query;
		if(searchQuery.Trim().GetLength() == 0)
			return S_FALSE;

		CInternetSession session(HTTP_AGENT);
		CString url = CString(APP_DIR) + LOOKUP_QUERY_STRING + CString(_T("&userquery="))+EscapeUrl(searchQuery);
		CHttpConnection* connection = session.GetHttpConnection(SERVER_ADDRESS, 0, 80, NULL, NULL);
		HttpStream s(connection, url);
		s.StartReading();

		int version		= s.Read<int>();
		UINT resultCode	= s.Read<UINT>();

		if(version != 1)
			return E_FAIL;

		if(resultCode < 0)
			return E_FAIL;

		int resultcount	= s.Read<int>();

		for(int i=0;i<resultcount;i++)
		{
			//Group or user?
			if(s.Read<char>())
			{
				GroupInfo info;
				info.id			= s.Read<GUID>();
				info.memberCount= s.Read<UINT>();
				info.name		= s.ReadStringW();
				info.owner		= s.ReadStringW();
			}
			else
			{
				MemberInfo info;
				info.id			= s.Read<GUID>();
				info.username	= s.ReadStringW();
				info.email		= s.ReadStringW();
				info.name		= s.ReadStringW();

				members->Add(info);
			}
		}
	}
	catch(...)
	{
		return E_FAIL;
	}
	return S_OK;
}

HRESULT LookupCategory(ShareDialog& dlg, CString& categoryName, GUID& categoryId, bool& hasAddAccess, bool& hasSecurityAccess, MemberList* members)
{
	ASSERT(members);
	hasAddAccess = hasSecurityAccess = false;


	WaitCursor cursor;
	try
	{
		categoryName.Trim();
		if(categoryName.GetLength() == 0)
			return S_FALSE;

		bool forceLogin = false;
retry:

		CInternetSession session(HTTP_AGENT);
		if(!UploadManager::Instance().InitSession(&session, forceLogin))
			return S_CANCELLED;
		CString url = CString(APP_DIR) + CATEGORY_QUERY_STRING + CString(_T("&categoryName="))+EscapeUrl(categoryName);
		CHttpConnection* connection = session.GetHttpConnection(SERVER_ADDRESS, 0, 80, NULL, NULL);
		HttpStream s(connection, url);
		s.StartReading();


		int version				= s.Read<int>();
		if(version != 1)
			return E_FAIL;

		int resultCode			= s.Read<int>();
		if(resultCode == -1) {
			forceLogin = true;
			goto retry;
		}
		if(resultCode == -2) {
			hasAddAccess = true;
			categoryName = _T("");
			return S_FALSE;
		}
		//-1 = not logged in, -2=invalid name.
		if(resultCode < 0)
			return E_FAIL;

		bool existed			= s.Read<byte>()?true:false;
		categoryId				= s.Read<GUID>();
		hasAddAccess			= s.Read<byte>()?true:false;
		hasSecurityAccess		= s.Read<byte>()?true:false;

		if(!existed)
			hasAddAccess = hasSecurityAccess = true;

		categoryName			= s.ReadStringW();

		bool canSendEmail		= s.Read<byte>()?true:false;
		bool canShareFriends	= s.Read<byte>()?true:false;
		if(!canSendEmail) {
			//static_cast<CButton*>(dlg.GetDlgItem(IDC_SEND_EMAIL))->SetCheck(BST_UNCHECKED);
			//dlg.GetDlgItem(IDC_SEND_EMAIL)->ModifyStyle(WS_VISIBLE,0);
		}
		if(!canShareFriends) {
			static_cast<CButton*>(dlg.GetDlgItem(IDC_SHARETOFRIENDS))->SetCheck(BST_UNCHECKED);
			dlg.GetDlgItem(IDC_SHARETOFRIENDS)->ModifyStyle(WS_VISIBLE,0);
		}

		UINT securityCount		= s.Read<UINT>();
		for(UINT i=0;i<securityCount;i++)
		{
			//Group or user?
			if(s.Read<char>())
			{
				GroupInfo info;
				info.id			= s.Read<GUID>();
				info.memberCount= s.Read<UINT>();
				info.name		= s.ReadStringW();
				info.owner		= s.ReadStringW();

			}
			else
			{
				MemberInfo info;
				info.id			= s.Read<GUID>();
				info.username	= s.ReadStringW();
				info.email		= s.ReadStringW();
				info.name		= s.ReadStringW();

				members->Add(info);
			}

			UINT reserved1	= s.Read<UINT>();
			UINT reserved2	= s.Read<UINT>();
			UINT reserved3	= s.Read<UINT>();
			UINT reserved4	= s.Read<UINT>();
		}
	}
	catch(...)
	{
		return E_FAIL;
	}
	return S_OK;
}

void ItemResolved(int item, UserListControl& shareList, MemberList* members, int index)
{
	MemberInfo info = members->GetAt(index);
	members->RemoveAll();
	members->Add(info);
	if(item == -1)
		item = shareList.InsertItem(shareList.GetItemCount(), _T(""));
	shareList.SetItemText(item, 0, info.name);
	shareList.SetItemText(item, 1, info.email);
	shareList.SetItemData(item, reinterpret_cast<DWORD_PTR>(members));
	
	//now, see if it's a duplicate.
	int i = -1;
	bool duplicate = false;
	while(true)
	{
		i = shareList.GetNextItem(i, LVNI_ALL);
		if(i == -1)
			break;
		if(i == item)
			continue;
		MemberList* list = reinterpret_cast<MemberList*>(shareList.GetItemData(i));
		if(list->GetCount() == 1 && list->GetAt(0).id == info.id)
		{
			duplicate = true;
			break;
		}
	}
	if(duplicate)
	{
		shareList.MessageBox(R2T(IDS_SHARE_DOUBLENAME), R2T(IDS_SHARE_DOUBLENAME_CAPTION), MB_OK);
		shareList.DeleteItem(item);
	}
}

void ShareDialog::SaveShareSettings(ShareSettings& settings) const
{
	settings.categoryName	= this->loadSettings.categoryName;
	settings.categoryId		= this->loadSettings.categoryId;
	settings.sendEmail		= false;
	settings.letFriendsAdd	= ((CButton*)GetDlgItem(IDC_LETFRIENDS_ADD))->GetCheck()==BST_CHECKED;
	settings.shareToFriends	= ((CButton*)GetDlgItem(IDC_SHARETOFRIENDS))->GetCheck()==BST_CHECKED;
	
	settings.sharedMembers.RemoveAll();
	int item=-1;
	while(true)
	{
		item = shareList.GetNextItem(item, LVNI_ALL);
		if(item == -1)
			break;
		MemberList* members = reinterpret_cast<MemberList*>(shareList.GetItemData(item));
		if(!members)
			continue;
		if(members->GetCount() != 1)
			continue;
		settings.sharedMembers.Add(members->GetAt(0));
	}
}


bool UpdateRow(int item, UserListControl& shareList, LPCTSTR searchName=0)
{
	CString name = searchName?searchName:shareList.GetItemText(item, 0);

	CString displayText;
	MemberList* members = new MemberList();
	if(FAILED(LookupMember(item, members, name)))
	{
        displayText = R2T(IDS_SHARE_NAMELOOKUPFAIL);
	}
	else
	{
		if(members->GetCount() > 1)
		{
			displayText.FormatMessage(IDS_SHARE_NAMELOOKUP_FOUND_N, members->GetCount());
		}
		if(members->GetCount() == 0)
		{
			displayText = R2T(IDS_SHARE_NAMELOOKUP_FOUND_0);
		}
		for(int i=0;i<members->GetCount();i++)
		{
			displayText += CString(i>0?_T(", "):_T(" ")) + (*members)[i].name;
		}
	}
	shareList.SetItem(item,1,LVIF_TEXT, displayText, 0, 0, 0, null);
	shareList.SetItemData(item, reinterpret_cast<DWORD_PTR>(members));
	if(members->GetCount() == 1) {
		ItemResolved(item, shareList, members, 0);
		return true;
	}
	return false;
}

// ShareDialog message handlers

BOOL ShareDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	shareList.InsertColumn(0, R2T(IDS_SHARE_CLM_NAME));
	shareList.InsertColumn(1, R2T(IDS_SHARE_CLM_DETAILS));
	const UINT nameWidth = 120;
	shareList.SetColumnWidth(0, nameWidth);
	CRect listSize;
	shareList.GetWindowRect(&listSize);
	shareList.SetColumnWidth(1, (listSize.Width() - nameWidth) - 10);


	//TODO: make this work with masking.
	Gdiplus::Bitmap topImage(::AfxGetInstanceHandle(), MAKEINTRESOURCEW(IDB_GROUPICON));
	HBITMAP htopbm;
	Gdiplus::Color c(0,0xff,0x00,0xff);
	((Gdiplus::Bitmap*)&topImage)->GetHBITMAP(c, &htopbm); //MAGENTA,RGB(0xFF, 0xFF, 0xFF)
	((CStatic*)GetDlgItem(IDC_SHARING_ICON))->SetBitmap(htopbm);

//	HBITMAP bm = ::LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_GROUPICON));
//	((CStatic*)GetDlgItem(IDC_SHARING_ICON))->SetBitmap(bm);

	//TODO: delete the resources from bitmaps etc.

	static_cast<CButton*>(GetDlgItem(IDC_SHARETOFRIENDS))->SetCheck(loadSettings.shareToFriends?BST_CHECKED:BST_UNCHECKED);
	static_cast<CButton*>(GetDlgItem(IDC_LETFRIENDS_ADD))->SetCheck(loadSettings.letFriendsAdd ?BST_CHECKED:BST_UNCHECKED);
	loadSettings.sendEmail;
	for(int i=0;i<loadSettings.sharedMembers.GetCount();i++)
	{
		MemberList* m = new MemberList();
		m->Add(loadSettings.sharedMembers[i]);
		ItemResolved(-1, shareList, m, 0);
	}

	GetDlgItem(IDC_CATEGORYNAME)->SetFocus();
	GetDlgItem(IDC_CATEGORYNAME)->SetWindowText(loadSettings.categoryName);
	if(!loadSettings.checkedOnline)
		UpdateCategoryName();

	return FALSE;  // return TRUE unless you set the focus to a control
}


void ShareDialog::OnEnKillfocusCategoryname()
{
	CString newName;
	GetDlgItem(IDC_CATEGORYNAME)->GetWindowText(newName);
	newName.Trim();
	if(newName != loadSettings.categoryName)
	{
		USAGE("Chose category");
		loadSettings.categoryName = newName;
		UpdateCategoryName();
	}
	else
		loadSettings.categoryName = newName;
}
void ShareDialog::UpdateCategoryName()
{
	loadSettings.categoryName.Trim();

	if(loadSettings.categoryName.GetLength() == 0)
	{
		//TODO: disable everything, and set a note.
	}
	else
	{
		bool hasAddAccess		= false;
		bool hasSecurityAccess	= false;
		HRESULT hr				= S_OK;
		GUID existingId;
		MemberList members;
		if(FAILED(hr = LookupCategory(*this, loadSettings.categoryName, existingId, hasAddAccess, hasSecurityAccess, &members)))
		{
			MessageBox(R2T(IDS_SHARE_NOCONTACT), R2T(IDS_NETWORKPROBLEM_CAPTION), MB_OK|MB_ICONERROR);
			this->OnCancel();
			return;
		}
		else if(hr == S_CANCELLED)
		{
			this->OnCancel();
			return;
		}
		else
			loadSettings.checkedOnline = true;
		if(!hasSecurityAccess)
		{
			MessageBox(R2T(IDS_SHARE_NOSECURITYPERMISSION), R2T(IDS_SHARE_CHOOSING_CAPTION), MB_OK|MB_ICONHAND);
			//TODO: disable security stuff.
			GetDlgItem(IDC_SHARELIST)->ModifyStyle(0, WS_DISABLED);
			GetDlgItem(IDC_SHARETOFRIENDS)->ModifyStyle(0, WS_DISABLED);
		}
		else
		{
			GetDlgItem(IDC_SHARELIST)->ModifyStyle(WS_DISABLED, 0);
			GetDlgItem(IDC_SHARETOFRIENDS)->ModifyStyle(WS_DISABLED, 0);
		}
		if(!hasAddAccess)
		{
			MessageBox(R2T(IDS_SHARE_NOADDPERMISSION), R2T(IDS_SHARE_CHOOSING_CAPTION), MB_OK|MB_ICONHAND);

			GetDlgItem(IDOK)->ModifyStyle(0, WS_DISABLED);
		}
		else
		{
			GetDlgItem(IDOK)->ModifyStyle(WS_DISABLED, 0);
			loadSettings.categoryId = existingId;
			if(hasSecurityAccess)
			{
				for(int i=0;i<members.GetCount();i++)
				{
					MemberList* m = new MemberList();
					m->Add(members[i]);
					ItemResolved(-1, shareList, m, 0);
				}
			}
		}
	}

	if(GetDlgItem(IDC_CATEGORYNAME))
		GetDlgItem(IDC_CATEGORYNAME)->SetWindowText(loadSettings.categoryName);
}

void ShareDialog::OnNMDblclkSharelist(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

void ShareDialog::OnNMClickSharelist(NMHDR *pNMHDR, LRESULT *pResult)
{
//	CEdit* edit = shareList.EditLabel(0);
	*pResult = 0;
}

void ShareDialog::OnLvnEndlabeleditSharelist(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	*pResult = TRUE;
	if(pDispInfo->item.iSubItem == 0)
	{
		if(pDispInfo->item.pszText)
		{
			CString s(pDispInfo->item.pszText);
			s.Trim();
			if(s.GetLength() == 0) {
				shareList.DeleteItem(pDispInfo->item.iItem);
				*pResult = FALSE;
			}
			//UpdateRow returns true if it edits the text, so then cancel.
			if(UpdateRow(pDispInfo->item.iItem, shareList, s))
				*pResult = FALSE;
		}
		else
		{
			//If an item was just created and not given a text, delete it.
			if(shareList.GetItemText(pDispInfo->item.iItem,0) .Trim().GetLength() == 0)
			{
				shareList.DeleteItem(pDispInfo->item.iItem);
				*pResult = FALSE;
			}
		}

		USAGE("Searched for a member");
	}
	else if(pDispInfo->item.iSubItem == 1)
	{
		if(_tcscmp(R2T(IDS_SHARE_ACT_REMOVE), pDispInfo->item.pszText) == 0)
		{
			shareList.DeleteItem(pDispInfo->item.iItem);
			USAGE("Deleted a member");
		}
		else if(shareList.GetItemData(pDispInfo->item.iItem))
		{
			MemberList* members = reinterpret_cast<MemberList*>(shareList.GetItemData(pDispInfo->item.iItem));
			int theIndex = -1;
			for(int i=0;i<members->GetCount();i++)
			{
				if((*members)[i].name == pDispInfo->item.pszText)
				{
					theIndex = i;
					break;
				}
			}
			if(theIndex >= 0)
			{
				ItemResolved(pDispInfo->item.iItem, shareList, members, theIndex);
			}
			USAGE("Chose a member among results.");
		}
	}
}

void ShareDialog::OnLvnItemActivateSharelist(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	shareList.EditLabel(pNMIA->iItem);
	*pResult = 0;
}

void ShareDialog::OnBnClickedSharetofriends()
{
	if(((CButton*)GetDlgItem(IDC_SHARETOFRIENDS))->GetCheck()==BST_CHECKED)
	{
		shareList.ModifyStyle(0, WS_DISABLED);
		shareList.GetHeaderCtrl()->ModifyStyle(0, WS_DISABLED);
		shareList.SetBkColor(RGB(0xCC,0xCC,0xCC));
	}
	else
	{
		shareList.ModifyStyle(WS_DISABLED, 0);
		shareList.GetHeaderCtrl()->ModifyStyle(WS_DISABLED, 0);
		shareList.SetBkColor(RGB(0xFF,0xFF,0xFF));//TODO: GET REAL COLOR
	}
	shareList.Invalidate();
//	GetDlgItem(IDOK)->ModifyStyle(0, WS_DISABLED);
}

BEGIN_MESSAGE_MAP(UserListControl, CListCtrl)
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_CHAR()
END_MESSAGE_MAP()

void UserListControl::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if(nChar == VK_BACK)
	{
		DeleteSelected();
	}

	CListCtrl::OnChar(nChar, nRepCnt, nFlags);
}

void UserListControl::DeleteSelected()
{
	if(1<GetSelectedCount() && IDYES != MessageBox(R2T(IDS_SHARE_REMOVEALLNAMES), R2T(IDS_CLEARLIST_CAPTION), MB_ICONQUESTION|MB_YESNO))
		return;
	int item=0;
	while(item = GetNextItem(-1, LVNI_SELECTED), item != -1)
		DeleteItem(item);
}


int UserListControl::GetColumnCount() const
{
	CHeaderCtrl* pHeader = (CHeaderCtrl*) GetDlgItem(0);
	return pHeader ? pHeader->GetItemCount() : 0;
}

int UserListControl::HitTestEx(CPoint & point, int * col) const
{
	int colnum = 0;
	int row = HitTest(point, NULL);
	
	if (col) *col = 0;
	if ((GetWindowLong(m_hWnd, GWL_STYLE) & LVS_TYPEMASK) != LVS_REPORT) return row;
	row = GetTopIndex();
	int bottom = row + GetCountPerPage();
	if (bottom > GetItemCount()) bottom = GetItemCount();
	int nColumnCount = GetColumnCount();
	for( ;row <=bottom;row++)
	{
		CRect rect;
		GetItemRect(row, &rect, LVIR_BOUNDS);
		if (rect.PtInRect(point))
		{
			for (colnum = 0; colnum < nColumnCount; colnum++)
			{
				int colwidth = GetColumnWidth(colnum);
				if (point.x >= rect.left && point.x <= (rect.left + colwidth))
				{
					if (col) *col = colnum;
					return row;
				}
				rect.left += colwidth;
			}
		}
	}
	return -1;
}

void UserListControl::OnLButtonDown(UINT nFlags, CPoint point)
{
	CListCtrl::OnLButtonDown(nFlags, point);

	int colnum, index;
	if( ( index = HitTestEx( point, &colnum )) != -1 && colnum==1 )
	{
		UINT flag = LVIS_FOCUSED;
		if( (GetItemState( index, flag ) & flag) == flag )
		{
			// Add check for LVS_EDITLABELS
			if( GetWindowLong(m_hWnd, GWL_STYLE) & LVS_EDITLABELS )
			{
				CStringList lstItems;
				lstItems.AddTail(R2T(IDS_SHARE_ACT_REMOVE));

				MemberList* members = reinterpret_cast<MemberList*>(GetItemData(index));
				if(members && members->GetCount() > 1)
				{
					for(int i=0;i<members->GetCount();i++)
						lstItems.AddTail((*members)[i].name);
				}
				ShowInPlaceList( index, colnum, lstItems, 2 );
			}
		}
		else
			SetItemState( index, LVIS_SELECTED | LVIS_FOCUSED ,
					LVIS_SELECTED | LVIS_FOCUSED);
	}
}

void UserListControl::OnLButtonUp(UINT nFlags, CPoint point)
{
	if(-1 == HitTest(point))
	{
		int at = this->InsertItem(GetItemCount(), _T(""));
		EditLabel(at);
	}

	CListCtrl::OnLButtonUp(nFlags, point);
}

// ShowInPlaceList		- Creates an in-place drop down list for any 
//				- cell in the list view control
// Returns			- A temporary pointer to the combo-box control
// nItem			- The row index of the cell
// nCol				- The column index of the cell
// lstItems			- A list of strings to populate the control with
// nSel				- Index of the initial selection in the drop down list
CComboBox* UserListControl::ShowInPlaceList( int nItem, int nCol, CStringList &lstItems, int nSel )
{
	// The returned pointer should not be saved

	// Make sure that the item is visible
	if( !EnsureVisible( nItem, TRUE ) ) return NULL;

	// Make sure that nCol is valid 
	CHeaderCtrl* pHeader = (CHeaderCtrl*)GetDlgItem(0);
	int nColumnCount = pHeader->GetItemCount();
	if( nCol >= nColumnCount || GetColumnWidth(nCol) < 10 )
		return NULL;

	// Get the column offset
	int offset = 0;
	for( int i = 0; i < nCol; i++ )
		offset += GetColumnWidth( i );

	CRect rect;
	GetItemRect( nItem, &rect, LVIR_BOUNDS );

	// Now scroll if we need to expose the column
	CRect rcClient;
	GetClientRect( &rcClient );
	if( offset + rect.left < 0 || offset + rect.left > rcClient.right )
	{
		CSize size;
		size.cx = offset + rect.left;
		size.cy = 0;
		Scroll( size );
		rect.left -= size.cx;
	}

	rect.left += offset+4;
	rect.right = rect.left + GetColumnWidth( nCol ) - 3 ;
	int height = rect.bottom-rect.top;
	rect.bottom += 5*height;
	if( rect.right > rcClient.right) rect.right = rcClient.right;

	DWORD dwStyle = WS_BORDER|WS_CHILD|WS_VISIBLE|WS_VSCROLL|WS_HSCROLL
					|CBS_DROPDOWNLIST|CBS_DISABLENOSCROLL;
	CComboBox *pList = new CInPlaceList(nItem, nCol, &lstItems, nSel);
	pList->Create( dwStyle, rect, this, 666 );
	pList->SetItemHeight( -1, height);
	pList->SetHorizontalExtent( GetColumnWidth( nCol ));


	return pList;
}

void ShareDialog::OnLvnDeleteitemSharelist(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	
	MemberList* data = reinterpret_cast<MemberList*>(shareList.GetItemData(pNMLV->iItem));
	if(data)
		delete data;

	*pResult = 0;
}

void ShareDialog::OnBnClickedOk()
{
	SaveShareSettings(loadSettings);
	OnOK();
	USAGE("Okayed the share dialog.");
}
