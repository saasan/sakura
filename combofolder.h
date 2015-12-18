// combofolder.h : interface of the CComboFolder class
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __COMBOFOLDER_H__
#define __COMBOFOLDER_H__

#include "resource.h"
#include "sakura.h"

/////////////////////////////////////////////////////////////////////////////
class CComboChild : public CWindowImpl<CComboChild, CComboBox>
{
	BEGIN_MSG_MAP(CComboChild)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
	END_MSG_MAP()

	LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
};

/////////////////////////////////////////////////////////////////////////////
class CComboFolder : public CWindowImpl<CComboFolder, CComboBoxEx>
{
public:
	enum RecentFolderItem
	{
		RecentFolderItemBrowse = 0,
		RecentFolderItemSame,
		RecentFolderItemFolder,
		RecentFolderItemFolderOpen,
		RecentFolderItemEnd
	};

	BEGIN_MSG_MAP(CComboFolder)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_DROPFILES, OnDropFiles)
	END_MSG_MAP()

	CComboFolder()
	{
		maxCount = FOLDER_COUNT_DEFAULT;

		folderImageList.Create(16, 16, ILC_COLOR8 | ILC_MASK, 0, 5);
		hRecentBitmap = LoadBitmap(_Module.GetModuleInstance(), MAKEINTRESOURCE(IDB_FOLDER));
		folderImageList.Add(hRecentBitmap, RGB(0xFF, 0x00, 0xFF));
		ExtractIconEx(_T("shell32.dll"), 3, NULL, &hRecentIcon[0], 1);
		folderImageList.AddIcon(hRecentIcon[0]);
		ExtractIconEx(_T("shell32.dll"), 4, NULL, &hRecentIcon[1], 1);
		folderImageList.AddIcon(hRecentIcon[1]);
	}

	~CComboFolder()
	{
		DeleteObject(hRecentBitmap);
		DestroyIcon(hRecentIcon[0]);
		DestroyIcon(hRecentIcon[1]);
	}

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDropFiles(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	VOID OnSelEndOk();
	VOID OnSelEndCancel();
	VOID OnGetDispInfo(PNMCOMBOBOXEX pDispInfo);

	VOID Add(LPCTSTR str, INT_PTR iItem = RecentFolderItemFolder);
	VOID MaxCount(INT count);
	INT MaxCount();
	VOID Initialize();
	VOID LoadProfile();
	VOID SaveProfile();

private:
	HBITMAP hRecentBitmap;
	HICON hRecentIcon[2];
	DWORD maxCount;
	CImageList folderImageList;

	CComboChild comboChild;

	VOID Remove();
};

/////////////////////////////////////////////////////////////////////////////
// CTransFolderDialog - used for browsing for a folder
class CTransFolderDialog : public CFolderDialogImpl<CTransFolderDialog>
{
public:
	CTransFolderDialog(HWND hWndParent = NULL, LPCTSTR lpstrTitle = NULL, UINT uFlags = BIF_RETURNONLYFSDIRS, BOOL bTrans = FALSE, INT iTransValue = TRANS_VALUE_MAX)
		: CFolderDialogImpl<CTransFolderDialog>(hWndParent, lpstrTitle, uFlags)
	{
		trans = bTrans;
		transValue = iTransValue;
	}

	void OnInitialized();
	void OnSelChanged(LPITEMIDLIST /*pItemIDList*/);

	VOID SetTransparent();

private:
	BOOL trans;
	INT transValue;
};

#endif // __COMBOFOLDER_H__