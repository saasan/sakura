// maindlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "maindlg.h"
#include "sakuraOptions.h"

// レジストリ関係
const TCHAR regWindowLeft[]			= _T("windowLeft");
const TCHAR regWindowTop[]			= _T("windowTop");
const TCHAR regWindowRight[]		= _T("windowRight");
const TCHAR regWindowBottom[]		= _T("windowBottom");
const TCHAR regWindowMaximized[]	= _T("maximized");

BOOL CMainDlg::PreTranslateMessage(MSG* pMsg)
{
	if (m_hAccel != NULL && ::TranslateAccelerator(m_hWnd, m_hAccel, pMsg))
	{
		return TRUE;
	}
	return IsDialogMessage(pMsg);
}

BOOL CMainDlg::OnIdle()
{
	return FALSE;
}

LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// ツールチップの最大幅
	const INT MaxTipWidth = 640;

	// アクセラレータをロード
	if (m_hWnd != NULL)
	{
		m_hAccel = ::LoadAccelerators(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME));
	}

	// サブクラス化
	combofolder.SubclassWindow(GetDlgItem(ID_FOLDER));
	listfile.SubclassWindow(GetDlgItem(ID_FILE));

	// 初期化
	combofolder.Initialize();

	// ツールチップを作成
	{
		toolTip.Create(m_hWnd);
		toolTip.SetMaxTipWidth(MaxTipWidth);
		CToolInfo ti(TTF_IDISHWND | TTF_SUBCLASS, m_hWnd, (UINT)(HWND)combofolder.GetEditCtrl(), NULL, (LPTSTR)ID_FOLDER);
		toolTip.AddTool(ti);
		ti.Init(TTF_IDISHWND | TTF_SUBCLASS, m_hWnd, (UINT)listfile.m_hWnd, NULL, (LPTSTR)ID_FILE);
		toolTip.AddTool(ti);
	}

	// Init the CDialogResize code
	DlgResize_Init();

	// set icons
	HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	SetIcon(hIconSmall, FALSE);

	LoadProfile();

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	UIAddChildWindowContainer(m_hWnd);

	return TRUE;
}

LRESULT CMainDlg::OnNotify(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int idCtrl = (int)wParam; 
    LPNMHDR pnmh = (LPNMHDR)lParam;

	if (idCtrl == ID_FOLDER)
	{
		switch (pnmh->code)
		{
			case CBEN_GETDISPINFO:
			{
				PNMCOMBOBOXEX pDispInfo = (PNMCOMBOBOXEX)lParam;
				combofolder.OnGetDispInfo(pDispInfo);
				return 0;
			}
		}
	}
	bHandled = FALSE;
	return TRUE;
}

LRESULT CMainDlg::OnAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	TCHAR fileName[MAX_PATH * 10];

	CFileDialog cFileDlg(
		TRUE, NULL, NULL,
		 OFN_ALLOWMULTISELECT | OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
		 _T("すべてのファイル (*.*)\0*.*\0\0"),
		 m_hWnd
	);

	ZeroMemory(fileName, sizeof(fileName) / sizeof(TCHAR));

	cFileDlg.m_ofn.lpstrFile = fileName;
	cFileDlg.m_ofn.nMaxFile = sizeof(fileName) / sizeof(TCHAR);

	if (cFileDlg.DoModal() == IDOK)
	{
		CPathString filePath;
		TCHAR *c, *start;

		filePath = cFileDlg.m_ofn.lpstrFile;

		if (PathIsDirectory(filePath))
		{
			CString filePathName;
			filePath.AddBackslash();

			c = cFileDlg.m_ofn.lpstrFile + cFileDlg.m_ofn.nFileOffset;

			while (*c != _T('\0'))
			{
				start = c;
				while (*c != _T('\0'))
				{
					c = CharNext(c);
				}
				filePathName = filePath;
				filePathName += start;
				listfile.Add(filePathName);
				c++;
			}
		}
		else
		{
			listfile.Add(filePath);
		}
	}
	return 0;
}

LRESULT CMainDlg::OnOption(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CPropertySheetNoHelp sheet(IDS_OPTION_TITLE, 0, m_hWnd);
	CPropertyPageMain pageMain;
	CPropertyPagePath pagePath;
	CPropertyPageShell pageShell;
	CPropertyPageAbout pageAbout;

	// 適用を非表示
	sheet.m_psh.dwFlags |= PSH_NOAPPLYNOW;

	// データ設定
	pageMain.optionFolderCount = combofolder.MaxCount();

	// ページ追加
	sheet.AddPage(pageMain);
	sheet.AddPage(pagePath);
	sheet.AddPage(pageShell);
	sheet.AddPage(pageAbout);

	// 表示
	if (sheet.DoModal() == IDOK)
	{
		combofolder.MaxCount(pageMain.optionFolderCount);
	}
	return 0;
}

LRESULT CMainDlg::OnEditClear(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	listfile.Remove();
	return 0;
}

LRESULT CMainDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	Excute();
	return 0;
}

LRESULT CMainDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CloseDialog(wID);
	return 0;
}

LRESULT CMainDlg::OnFolder(WORD wNotifyCode, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled)
{
	bHandled = FALSE;

	switch (wNotifyCode)
	{
		case CBN_SELENDOK:
		{
			combofolder.OnSelEndOk();
			bHandled = TRUE;
			break;
		}
		case CBN_SELENDCANCEL:
		{
			combofolder.OnSelEndCancel();
			bHandled = TRUE;
			break;
		}
	}
	return 0;
}

LRESULT CMainDlg::OnEditSelectAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	listfile.SelectAll();
	listfile.SetFocus();
	return 0;
}

VOID CMainDlg::CloseDialog(int nVal)
{
	SaveProfile();
	combofolder.SaveProfile();

	DestroyWindow();
	::PostQuitMessage(nVal);
}

VOID CMainDlg::LoadProfile()
{
	CString key, privateKey;
	CRegKey reg;

	key.LoadString(IDR_MAINFRAME);
	privateKey.LoadString(IDS_REG_PRIVATEKEY);
	key = privateKey + key;

	if (reg.Open(HKEY_CURRENT_USER, key) == ERROR_SUCCESS)
	{
		// ウインドウ位置、サイズ
		BOOL flag = TRUE;
	
		DWORD maximized = 0, left = 0, top = 0, right = 0, bottom = 0;

		flag &= (reg.QueryValue(left, regWindowLeft) == ERROR_SUCCESS);
		flag &= (reg.QueryValue(top, regWindowTop) == ERROR_SUCCESS);
		flag &= (reg.QueryValue(right, regWindowRight) == ERROR_SUCCESS);
		flag &= (reg.QueryValue(bottom, regWindowBottom) == ERROR_SUCCESS);

		// すべて取得できたら設定
		if (flag)
		{
			MoveWindow(left, top, right - left, bottom - top);
		}
		else
		{
			CenterWindow();
		}

		if (reg.QueryValue(maximized, regWindowMaximized) == ERROR_SUCCESS)
		{
			if (maximized)
			{
				ShowWindow(SW_MAXIMIZE);
			}
		}
	}
	else
	{
		CenterWindow();
	}
}

VOID CMainDlg::SaveProfile()
{
	CString key, privateKey;
	CRegKey reg;

	key.LoadString(IDR_MAINFRAME);
	privateKey.LoadString(IDS_REG_PRIVATEKEY);
	key = privateKey + key;

	if (reg.Create(HKEY_CURRENT_USER, key) == ERROR_SUCCESS)
	{
		// ウインドウ位置、サイズ
		WINDOWPLACEMENT wp;

		ZeroMemory(&wp, sizeof(WINDOWPLACEMENT));
		wp.length = sizeof(WINDOWPLACEMENT);
		if (GetWindowPlacement(&wp))
		{
			DWORD maximized = (wp.showCmd ==SW_SHOWMAXIMIZED) ? 1 : 0;

			reg.SetValue(maximized, regWindowMaximized);
			reg.SetValue(wp.rcNormalPosition.left, regWindowLeft);
			reg.SetValue(wp.rcNormalPosition.top, regWindowTop);
			reg.SetValue(wp.rcNormalPosition.right, regWindowRight);
			reg.SetValue(wp.rcNormalPosition.bottom, regWindowBottom);
		}
	}
}

VOID CMainDlg::Excute()
{
	MessageBox(_T("まだできてませんー"));
}
