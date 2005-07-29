
#pragma once


struct MemberInfo
{
	GUID id;
	CString username;
	CString email;
	CString name;
};

struct GroupInfo
{
	GUID id;
	CString name;
	CString owner;
	int memberCount;
};

typedef CArray<MemberInfo> MemberList;

struct ShareSettings
{
	ShareSettings()
	{
		ZeroMemory(&categoryId, sizeof(categoryId)); 
		shareToFriends = sendEmail = letFriendsAdd = checkedOnline = false;
	}
	ShareSettings(const ShareSettings& cpy)
	{
		Copy(cpy);
	}

	void Copy(const ShareSettings& cpy)
	{
		checkedOnline	= cpy.checkedOnline;
		categoryId		= cpy.categoryId;
		categoryName	= cpy.categoryName;
		sendEmail		= cpy.sendEmail;
		letFriendsAdd	= cpy.letFriendsAdd;
		shareToFriends	= cpy.shareToFriends;
		sharedMembers.Copy(cpy.sharedMembers);
	}
	GUID		categoryId;
	CString		categoryName;
	MemberList	sharedMembers;
	bool		shareToFriends;
	bool		sendEmail;
	bool		letFriendsAdd;
	bool		checkedOnline;
};
