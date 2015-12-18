// sakuraOptions.h

#ifndef __SAKURAOPTIONS_H__
#define __SAKURAOPTIONS_H__

#include "resource.h"
#include "s2tlframe.h"
#include "sakura.h"

#define OPTION_MAIN_OPEN		0x00000001L
#define OPTION_MAIN_EXPLORER	0x00000002L
#define OPTION_MAIN_ARC			0x00000004L
#define OPTION_MAIN_SPI			0x00000008L
#define OPTION_MAIN_EXE			0x00000010L
#define OPTION_MAIN_RECENT		0x00000020L
#define OPTION_MAIN_OPTION		0x00000040L
#define OPTION_MAIN_SUBFOLDER	0x00000080L
#define OPTION_MAIN_MASK		0x0000FFFFL

#define OPTION_WINDOW_TOP		0x00010000L
#define OPTION_WINDOW_SNAP		0x00020000L
#define OPTION_WINDOW_TRANS		0x00040000L
#define OPTION_WINDOW_MASK		0x000F0000L

// レジストリ関係
static const TCHAR regMainStyles[]	= _T("mainStyles");
static const TCHAR regSnapWidth[]	= _T("snapWidth");
static const TCHAR regTransValue[]	= _T("transValue");

/////////////////////////////////////////////////////////////////////////////
// CMainOption

class CMainOption
{
public:
	static HINSTANCE hCCLDll;	// CCL.DLLのハンドル
	static DWORD mainStyles;	// メインオプション
	static DWORD snapWidth;		// スナップする幅
	static DWORD transValue;	// 透明度[%]

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
			reg.QueryValue(snapWidth, regSnapWidth);
			reg.QueryValue(transValue, regTransValue);
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
			reg.SetValue(snapWidth, regSnapWidth);
			reg.SetValue(transValue, regTransValue);
		}
	}
};

/////////////////////////////////////////////////////////////////////////////
// CHoverHyperLink

class CHoverHyperLink : public CHyperLinkImpl<CHoverHyperLink>
{
public:
	HFONT m_hHoverFont;
	bool m_bHover;

	CHoverHyperLink() : m_hHoverFont(NULL), m_bHover(false)
	{
	}

	~CHoverHyperLink()
	{
		if(m_hHoverFont != NULL)
			::DeleteObject(m_hHoverFont);
	}

	BOOL SubclassWindow(HWND hWnd);

	BEGIN_MSG_MAP(CHyperLinkImpl)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		CHAIN_MSG_MAP(CHyperLinkImpl<CHoverHyperLink>)
	END_MSG_MAP()

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	void Init();
	void DoPaint(CDCHandle dc);
};

/////////////////////////////////////////////////////////////////////////////
// CPropertySheetNoHelp

class CPropertySheetNoHelp : public CPropertySheetImpl<CPropertySheetNoHelp>
{
public:
	CPropertySheetNoHelp(_U_STRINGorID title = (LPCTSTR)NULL, UINT uStartPage = 0, HWND hWndParent = NULL, BOOL bTrans = FALSE, INT iTransValue = TRANS_VALUE_MAX)
		: CPropertySheetImpl<CPropertySheetNoHelp>(title, uStartPage, hWndParent)
	{
		trans = bTrans;
		transValue = iTransValue;
	}

	BEGIN_MSG_MAP(CPropertySheetNoHelp)
		MESSAGE_HANDLER(WM_COMMAND, CPropertySheetImpl<CPropertySheetNoHelp>::OnCommand)
		MESSAGE_HANDLER(WM_SYSCOMMAND, CPropertySheetImpl<CPropertySheetNoHelp>::OnSysCommand)
		MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
	END_MSG_MAP()

	LRESULT OnShowWindow(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);

private:
	BOOL trans;
	INT transValue;

	VOID SetTransparent();
};

/////////////////////////////////////////////////////////////////////////////
// CPropertyPageMain

class CPropertyPageMain : public CPropertyPageImpl<CPropertyPageMain>,
	public CWinDataExchange<CPropertyPageMain>
{
public:
	enum { IDD = IDD_PROPPAGE_MAIN };

// Data members
	BOOL optionOpen, optionExplorer, optionArc, optionSpi,
		optionExe, optionRecent, optionOption, optionSubfolder;
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
		DDX_CHECK(IDC_MAIN_ARC, optionArc)
		DDX_CHECK(IDC_MAIN_SPI, optionSpi)
		DDX_CHECK(IDC_MAIN_EXE, optionExe)
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
// CPropertyPageWindows

class CPropertyPageWindows : public CPropertyPageImpl<CPropertyPageWindows>,
	public CWinDataExchange<CPropertyPageWindows>
{
public:
	enum { IDD = IDD_PROPPAGE_WINDOW };

// Data members
	BOOL optionTop, optionSnap, optionTrans;
	INT optionSnapWidth, optionTransValue;

	BEGIN_MSG_MAP(CPropertyPageWindows)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(CPropertyPageImpl<CPropertyPageWindows>)
	END_MSG_MAP()

// DDX map
	BEGIN_DDX_MAP(CPropertyPageWindows)
		DDX_INT_RANGE(IDC_WINDOW_SNAP_WIDTH_EDIT, optionSnapWidth, SNAP_WIDTH_MIN, SNAP_WIDTH_MAX)
		DDX_INT_RANGE(IDC_WINDOW_TRANS_VALUE_EDIT, optionTransValue, TRANS_VALUE_MIN, TRANS_VALUE_MAX)
		DDX_CHECK(IDC_WINDOW_TOP, optionTop)
		DDX_CHECK(IDC_WINDOW_SNAP, optionSnap)
		DDX_CHECK(IDC_WINDOW_TRANS, optionTrans)
	END_DDX_MAP()

	CPropertyPageWindows() { LoadData(); }
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	// Overrides
	BOOL OnSetActive();
	BOOL OnKillActive();
	void OnDataExchangeError(UINT /*nCtrlID*/, BOOL /*bSave*/);
	void OnDataValidateError(UINT /*nCtrlID*/, BOOL /*bSave*/, _XData& /*data*/);

protected:
	void LoadData();
	void SaveData();
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
	CHoverHyperLink hyperLink;
	HICON GetIconFromExtension(LPCTSTR extension);
	VOID GetFileVersionString(LPCTSTR file, CString &verstr, BOOL detail = FALSE);
};

#endif // __SAKURAOPTIONS_H__