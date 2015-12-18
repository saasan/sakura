// combofolder.cpp : implementation of the CComboFolder class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "combofolder.h"
#include "sakuraOptions.h"

// レジストリ関係
const TCHAR regRecentFolderMax[] = _T("recentFolderMax");
const TCHAR regRecentFolderCount[] = _T("recentFolderCount");
const TCHAR regRecentFolder[] = _T("recentFolder%.03d");

LRESULT CComboFolder::OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
	UINT vk = (UINT)wParam;

	bHandled = FALSE;

	switch (vk)
	{
		case VK_DELETE:
		{
			Remove();
			break;
		}
	}
	return 0;
}

LRESULT CComboFolder::OnDropFiles(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	HDROP hdrop = (HDROP)wParam;
	UINT num;
	CString file;

	num = DragQueryFile(hdrop, 0xFFFFFFFF, NULL, 0);
	for (UINT i = 0; i < num; i++)
	{
		DragQueryFile(hdrop, i, file.GetBuffer(MAX_PATH), MAX_PATH);
		file.ReleaseBuffer(-1);
		Add(file);
	}
	DragFinish(hdrop);
	return 0;
}

// 選択正常終了時
VOID CComboFolder::OnSelEndOk()
{
	INT index = GetCurSel();

	if (index > RecentFolderItemFolder)
	{
		// 選択時に入れ替え

		TCHAR str[MAX_PATH];
		INT data;

		GetLBText(index, str);
		data = GetItemData(index);

		DeleteItem(index);

		COMBOBOXEXITEM cbei;
		ZeroMemory(&cbei, sizeof(cbei));

		cbei.mask = CBEIF_TEXT;
		cbei.pszText = str;
		cbei.cchTextMax = MAX_PATH;
		cbei.iItem = RecentFolderItemFolder;

		index = InsertItem(&cbei);
		if (index >= 0)
		{
			SetItemData(index, data);
		}

		SetCurSel(RecentFolderItemFolder);
	}
	else if (index == RecentFolderItemBrowse)
	{
		// フォルダの参照

		CString title;
		title.LoadString(IDS_BROWSE_TITLE);

		CFolderDialog cFolderDlg(
			m_hWnd,
			title,
			BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT
		);

		// フォルダ選択ダイアログ表示
		if (cFolderDlg.DoModal() == IDOK)
		{
			Add(cFolderDlg.GetFolderPath());
		}
		else
		{
			if (GetCount() >= RecentFolderItemFolder)
			{
				SetCurSel(RecentFolderItemFolder);
			}
		}
	}
	else if (index == RecentFolderItemSame)
	{
	}
}

// 選択キャンセル終了時
VOID CComboFolder::OnSelEndCancel()
{
	int count = GetCount();
	if (count >= RecentFolderItemFolder)
	{
		SetCurSel(RecentFolderItemFolder);
	}
	else if (count == RecentFolderItemSame)
	{
		SetCurSel(RecentFolderItemSame);
	}
}

VOID CComboFolder::OnGetDispInfo(PNMCOMBOBOXEX pDispInfo)
{
	INT data = GetItemData(pDispInfo->ceItem.iItem);

	pDispInfo->ceItem.mask = CBEIF_IMAGE | CBEIF_SELECTEDIMAGE;
	switch (data)
	{
		case RecentFolderItemBrowse:
		{
			pDispInfo->ceItem.iImage = RecentFolderItemBrowse;
			pDispInfo->ceItem.iSelectedImage = pDispInfo->ceItem.iImage;
			break;
		}
		case RecentFolderItemSame:
		{
			pDispInfo->ceItem.iImage = RecentFolderItemSame;
			pDispInfo->ceItem.iSelectedImage = pDispInfo->ceItem.iImage;
			break;
		}
		default:
		{
			pDispInfo->ceItem.iImage = RecentFolderItemFolder;
			pDispInfo->ceItem.iSelectedImage = RecentFolderItemFolderOpen;
			break;
		}
	}
}

// 項目を追加する
VOID CComboFolder::Add(LPCTSTR str, INT_PTR iItem)
{
	TCHAR folder[MAX_PATH];
	INT index;
	COMBOBOXEXITEM cbei;

	// 初期化
	ZeroMemory(folder, sizeof(folder));
	ZeroMemory(&cbei, sizeof(cbei));

	// コピー
	_tcsncpy(folder, str, sizeof(folder)/sizeof(TCHAR));

	// 存在するか確認
	if (!PathFileExists(folder))
	{
		return;
	}

	// ファイルの場合はファイルのあるフォルダに
	if (!PathIsDirectory(folder))
	{
		PathRemoveFileSpec(folder);
	}

	// アイテム設定
	cbei.mask = CBEIF_TEXT;
	cbei.iItem = iItem;
	cbei.pszText = folder;
	cbei.cchTextMax = sizeof(folder)/sizeof(TCHAR);

	// 検索
	index = FindStringExact(-1, folder);
	if (index == CB_ERR)
	{
		// 存在しなければ追加
		index = InsertItem(&cbei);

		// (maxCount + RecentFolderItemFolder)個に減らす
		while(GetCount() > (int)(maxCount + RecentFolderItemFolder))
			DeleteItem(maxCount + RecentFolderItemFolder);
	}
	else
	{
		// 既に存在していれば入れ替え
		DeleteItem(index);
		index = InsertItem(&cbei);
	}

	if (index >= 0)
	{
		SetItemData(index, RecentFolderItemFolder);
	}

	SetCurSel(RecentFolderItemFolder);
}

// 項目の最大数を設定
VOID CComboFolder::MaxCount(INT count)
{
	if (count >= FOLDER_COUNT_MIN && count <= FOLDER_COUNT_MAX)
	{
		maxCount = count;
	}
}

// 項目の最大数を取得
INT CComboFolder::MaxCount()
{
	return maxCount;
}

// 初期化処理
VOID CComboFolder::Initialize()
{
	INT index;

	// イメージリストの準備
	SetImageList(folderImageList);

	TCHAR name[MAX_PATH];
	COMBOBOXEXITEM cbei;

	// 初期化
	ZeroMemory(name, sizeof(name));
	ZeroMemory(&cbei, sizeof(cbei));

	// コピー
	_tcsncpy(name, _T("[ 参照... ]"), sizeof(name)/sizeof(TCHAR));

	// アイテム設定
	cbei.mask = CBEIF_TEXT;
	cbei.iItem = -1;
	cbei.pszText = name;
	cbei.cchTextMax = sizeof(name)/sizeof(TCHAR);

	index = InsertItem(&cbei);
	if (index >= 0)
	{
		SetItemData(index, RecentFolderItemBrowse);
	}

	// 初期化
	ZeroMemory(name, sizeof(name));

	_tcsncpy(name, _T("[ ファイルと同じフォルダ ]"), sizeof(name)/sizeof(TCHAR));

	index = InsertItem(&cbei);
	if (index >= 0)
	{
		SetItemData(index, RecentFolderItemSame);
	}

	comboChild.SubclassWindow(GetComboCtrl());
	SHAutoComplete(GetEditCtrl(), SHACF_FILESYS_ONLY | SHACF_FILESYS_DIRS);

	// 設定読み込み
	LoadProfile();
}

VOID CComboFolder::LoadProfile()
{
	CString key, privateKey;
	CRegKey reg;

	key.LoadString(IDR_MAINFRAME);
	privateKey.LoadString(IDS_REG_PRIVATEKEY);
	key = privateKey + key;

	if (reg.Open(HKEY_CURRENT_USER, key) == ERROR_SUCCESS)
	{
		// 最大数
		reg.QueryValue(maxCount, regRecentFolderMax);

		// 現在数
		DWORD count = 0;
		reg.QueryValue(count, regRecentFolderCount);

		// 最大数と現在数の小さい方
		if (maxCount < count)
		{
			count = maxCount;
		}

		UINT i = 0;
		TCHAR item[MAX_PATH];
		DWORD len = sizeof(item); 
		CString name;

		name.Format(regRecentFolder, i);
		while (i < count && reg.QueryValue(item, name, &len) == ERROR_SUCCESS)
		{
			Add(item, -1);
			i++;
			name.Format(regRecentFolder, i);
			len = sizeof(item);
		}

		if (i > 0)
		{
			SetCurSel(0);
		}
	}

	// 無いときは追加
	if (GetCount() <= RecentFolderItemFolder)
	{
		TCHAR path[MAX_PATH];

		// Program Files
		if (SUCCEEDED(SHGetSpecialFolderPath(m_hWnd, path, CSIDL_PROGRAM_FILES, FALSE)))
		{
			Add(path);
		}

		// Temp
		if (GetTempPath(MAX_PATH, path) > 0)
		{
			TCHAR longPath[MAX_PATH];
			GetLongPathName(path, longPath, MAX_PATH);
			PathRemoveBackslash(longPath);
			Add(longPath);
		}

		// MyDocuments
		if (SUCCEEDED(SHGetSpecialFolderPath(m_hWnd, path, CSIDL_PERSONAL, FALSE)))
		{
			Add(path);
		}

		// Desktop
		if (SUCCEEDED(SHGetSpecialFolderPath(m_hWnd, path, CSIDL_DESKTOP, FALSE)))
		{
			Add(path);
		}
	}

	// アイテムがあれば選択
	if (GetCount() >= RecentFolderItemFolder)
	{
		SetCurSel(RecentFolderItemFolder);
	}
}

VOID CComboFolder::SaveProfile()
{
	CString key, privateKey;
	CRegKey reg;

	key.LoadString(IDR_MAINFRAME);
	privateKey.LoadString(IDS_REG_PRIVATEKEY);
	key = privateKey + key;

	if (reg.Create(HKEY_CURRENT_USER, key) == ERROR_SUCCESS)
	{
		INT i, count = GetCount() - RecentFolderItemFolder;
		TCHAR item[MAX_PATH];
		CString name;

		// GetCount()かMaxCount()の小さい方
		count = (count < MaxCount() ? count : MaxCount());

		for (i = 0; i < count; i++)
		{
			GetLBText(i + RecentFolderItemFolder, item);
			name.Format(regRecentFolder, i);
			reg.SetValue(item, name);
		}
		// 現在数
		reg.SetValue(count, regRecentFolderCount);
		// 最大数
		reg.SetValue(MaxCount(), regRecentFolderMax);
	}
}

// 項目を削除する
VOID CComboFolder::Remove()
{
	INT index, count;

	index = GetCurSel();
	if (index >= RecentFolderItemFolder)
	{
		DeleteItem(index);
	}

	count = GetCount();
	if (count != CB_ERR)
	{
		count--;
		if (count >= index)
		{
			SetCurSel(index);
		}
		else
		{
			SetCurSel(count);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
LRESULT CComboChild::OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
	UINT vk = (UINT)wParam;

	bHandled = FALSE;

	switch (vk)
	{
		case VK_DELETE:
		{
			HWND hWndParent = GetParent();
			::SendMessage(hWndParent, WM_KEYDOWN, VK_DELETE, 0);
			break;
		}

		// キーボードでも入れ替えできるように
		case VK_PRIOR:
		case VK_NEXT:
		case VK_END:
		case VK_HOME:
		case VK_LEFT:
		case VK_UP:
		case VK_RIGHT:
		case VK_DOWN:
		{
			if (!GetDroppedState())
			{
				ShowDropDown();
			}
			break;
		}
	}
	return 0;
}
