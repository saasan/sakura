// sakuraOptions.h

#ifndef __SAKURAOPTIONS_H__
#define __SAKURAOPTIONS_H__

#include "resource.h"
#include "sakura.h"

#define OPTION_MAIN_OPEN		0x00000001L
#define OPTION_MAIN_EXPLORER	0x00000002L
#define OPTION_MAIN_RECENT		0x00000004L
#define OPTION_MAIN_OPTION		0x00000008L
#define OPTION_MAIN_SUBFOLDER	0x00000010L
#define OPTION_MAIN_MASK		0x0000FFFFL

// レジストリ関係
static const TCHAR regMainStyles[]	= _T("mainStyles");

/////////////////////////////////////////////////////////////////////////////
// CMainOption

class CMainOption
{
public:
	static DWORD mainStyles;	// メインオプション

	static VOID LoadProfile()
	{
		CString key, privateKey;
		CRegKey reg;

		key.LoadString(IDR_MAINFRAME);
		privateKey.LoadString(IDS_REG_PRIVATEKEY);
		key = privateKey + key;

		if (reg.Create(HKEY_CURRENT_USER, key) == ERROR_SUCCESS)
		{
			reg.QueryValue(mainStyles, regMainStyles);
		}
	}

	static VOID SaveProfile()
	{
		CString key, privateKey;
		CRegKey reg;

		key.LoadString(IDR_MAINFRAME);
		privateKey.LoadString(IDS_REG_PRIVATEKEY);
		key = privateKey + key;

		if (reg.Open(HKEY_CURRENT_USER, key) == ERROR_SUCCESS)
		{
			reg.SetValue(mainStyles, regMainStyles);
		}
	}
};

/////////////////////////////////////////////////////////////////////////////
// CPropertySheetNoHelp

class CPropertySheetNoHelp : public CPropertySheetImpl<CPropertySheetNoHelp>
{
public:
	CPropertySheetNoHelp(_U_STRINGorID title = (LPCTSTR)NULL, UINT uStartPage = 0, HWND hWndParent = NULL)
		: CPropertySheetImpl<CPropertySheetNoHelp>(title, uStartPage, hWndParent)
	{
	}

	BEGIN_MSG_MAP(CPropertySheetNoHelp)
		MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
	END_MSG_MAP()

	LRESULT OnShowWindow(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);
};

/////////////////////////////////////////////////////////////////////////////
// CPropertyPageMain

class CPropertyPageMain : public CPropertyPageImpl<CPropertyPageMain>,
	public CWinDataExchange<CPropertyPageMain>
{
public:
	enum { IDD = IDD_PROPPAGE_MAIN };

// Data members
	BOOL optionOpen, optionExplorer, optionRecent, optionOption, optionSubfolder;
	INT optionFolderCount;

	BEGIN_MSG_MAP(CPropertyPageMain)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(CPropertyPageImpl<CPropertyPageMain>)
	END_MSG_MAP()

// DDX map
	BEGIN_DDX_MAP(CPropertyPageMain)
		DDX_INT_RANGE(IDC_MAIN_FOLDER_COUNT_EDIT, optionFolderCount, FOLDER_COUNT_MIN, FOLDER_COUNT_MAX)
		DDX_CHECK(IDC_MAIN_OPEN, optionOpen)
		DDX_CHECK(IDC_MAIN_EXPLORER, optionExplorer)
		DDX_CHECK(IDC_MAIN_RECENT, optionRecent)
		DDX_CHECK(IDC_MAIN_OPTION, optionOption)
		DDX_CHECK(IDC_MAIN_SUBFOLDER, optionSubfolder)
	END_DDX_MAP()

	CPropertyPageMain() { LoadData(); }
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	// Overrides
	BOOL OnSetActive();
	BOOL OnKillActive();
	void OnDataExchangeError(UINT /*nCtrlID*/, BOOL /*bSave*/);
	void OnDataValidateError(UINT /*nCtrlID*/, BOOL /*bSave*/, _XData& /*data*/);

protected:
	VOID LoadData();
	VOID SaveData();
};

/////////////////////////////////////////////////////////////////////////////
// CPropertyPagePath

class CPropertyPagePath : public CPropertyPageImpl<CPropertyPagePath>
{
public:
	enum { IDD = IDD_PROPPAGE_PATH };
};

/////////////////////////////////////////////////////////////////////////////
// CPropertyPageShell

class CPropertyPageShell : public CPropertyPageImpl<CPropertyPageShell>,
	public CWinDataExchange<CPropertyPageShell>
{
public:
	enum { IDD = IDD_PROPPAGE_SHELL };

// Data members
	CCheckListViewCtrl clv;
	BOOL shortcutDesktop, shortcutSendto, shortcutPrograms, shortcutQuicklaunch;

	BEGIN_MSG_MAP(CPropertyPageShell)
		COMMAND_ID_HANDLER(IDC_SHELL_ALL, OnSellAll)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(CPropertyPageImpl<CPropertyPageShell>)
	END_MSG_MAP()

// DDX map
	BEGIN_DDX_MAP(CPropertyPageShell)
		DDX_CHECK(IDC_SHELL_DESKTOP, shortcutDesktop)
		DDX_CHECK(IDC_SHELL_SENDTO, shortcutSendto)
		DDX_CHECK(IDC_SHELL_PROGRAMS, shortcutPrograms)
		DDX_CHECK(IDC_SHELL_QUICKLAUNCH, shortcutQuicklaunch)
	END_DDX_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	LRESULT OnSellAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

// Overrides
	BOOL OnApply();
	BOOL OnSetActive();
	BOOL OnKillActive();
	void OnDataExchangeError(UINT /*nCtrlID*/, BOOL /*bSave*/);
	void OnDataValidateError(UINT /*nCtrlID*/, BOOL /*bSave*/, _XData& /*data*/);

private:
	INT GetImageIndexFromExtension(LPCTSTR extension);
	HIMAGELIST GetSystemImageList(BOOL large);
	BOOL ShortcutExists(int nFolder, LPCTSTR filePath = NULL);
	VOID ManageShortcut(int nFolder, BOOL create, LPCTSTR filePath = NULL);
	BOOL CreateShortcut(LPCTSTR shortcut, LPCTSTR source, LPCTSTR comment);

	BOOL GetAssociation(LPCTSTR lpExtension);
	VOID SetAssociation(BOOL bAssociate, LPCTSTR lpExtension);
};

/////////////////////////////////////////////////////////////////////////////
// CPropertyPageAbout

class CPropertyPageAbout : public CPropertyPageImpl<CPropertyPageAbout>
{
public:
	enum { IDD = IDD_PROPPAGE_ABOUT };

// Data members
	BEGIN_MSG_MAP(CPropertyPageAbout)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(CPropertyPageImpl<CPropertyPageAbout>)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

private:
	CHyperLink hyperLink;
	HICON GetIconFromExtension(LPCTSTR extension);
	VOID GetFileVersionString(LPCTSTR file, CString &verstr, BOOL detail = FALSE);
};

#endif // __SAKURAOPTIONS_H__