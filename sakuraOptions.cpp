#include "stdafx.h"
#include "resource.h"

#include "sakura.h"
#include "sakuraOptions.h"

DWORD CMainOption::mainStyles = 0;

/////////////////////////////////////////////////////////////////////////////
// CPropertySheetNoHelp

LRESULT CPropertySheetNoHelp::OnShowWindow(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
	BOOL fShow = (BOOL) wParam;
	if (fShow)
	{
		// 右上の[?]ボタンを消す
		LONG style = ::GetWindowLong(m_hWnd, GWL_EXSTYLE);
		style &= ~(WS_EX_CONTEXTHELP);
		::SetWindowLong(m_hWnd, GWL_EXSTYLE, style);
	}
	bHandled = FALSE;
	return 1;
}

/////////////////////////////////////////////////////////////////////////////
// CPropertyPageMain

LRESULT CPropertyPageMain::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// 展開先履歴数桁制限(3)
	SendMessage(GetDlgItem(IDC_MAIN_FOLDER_COUNT_EDIT),
		EM_SETLIMITTEXT, (WPARAM) 3, (LPARAM) 0);
	// 展開先履歴数範囲制限
	SendMessage(GetDlgItem(IDC_MAIN_FOLDER_COUNT_UPDOWN),
		UDM_SETRANGE, (WPARAM) 0, (LPARAM) MAKELONG(FOLDER_COUNT_MAX, FOLDER_COUNT_MIN));
	return TRUE;
}

BOOL CPropertyPageMain::OnSetActive()
{
	return DoDataExchange(DDX_LOAD);
}

BOOL CPropertyPageMain::OnKillActive()
{
	if (DoDataExchange(DDX_SAVE))
	{
		SaveData();
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void CPropertyPageMain::OnDataExchangeError(UINT nCtrlID, BOOL /*bSave*/)
{
	::SetFocus(GetDlgItem(nCtrlID));

	CString message;
	message.LoadString(IDS_OPTION_WARNING_DEFAULT);
	MessageBox(message, NULL, MB_ICONWARNING);
}

void CPropertyPageMain::OnDataValidateError(UINT nCtrlID, BOOL /*bSave*/, _XData& /*data*/)
{
	::SetFocus(GetDlgItem(nCtrlID));

	switch (nCtrlID)
	{
		case IDC_MAIN_FOLDER_COUNT_EDIT:
		{
			CString message, format;
			format.LoadString(IDS_OPTION_WARNING_MAXCOUNT);
			message.Format(format, FOLDER_COUNT_MIN, FOLDER_COUNT_MAX);
			MessageBox(message, NULL, MB_ICONWARNING);
			break;
		}
		default:
		{
			CString message;
			message.LoadString(IDS_OPTION_WARNING_DEFAULT);
			MessageBox(message, NULL, MB_ICONWARNING);
			break;
		}
	}
}

VOID CPropertyPageMain::LoadData()
{
	optionOpen		= ((CMainOption::mainStyles & OPTION_MAIN_OPEN     ) ? TRUE : FALSE);
	optionExplorer	= ((CMainOption::mainStyles & OPTION_MAIN_EXPLORER ) ? TRUE : FALSE);
	optionRecent	= ((CMainOption::mainStyles & OPTION_MAIN_RECENT   ) ? TRUE : FALSE);
	optionOption	= ((CMainOption::mainStyles & OPTION_MAIN_OPTION   ) ? TRUE : FALSE);
	optionSubfolder	= ((CMainOption::mainStyles & OPTION_MAIN_SUBFOLDER) ? TRUE : FALSE);
}

VOID CPropertyPageMain::SaveData()
{
	CMainOption::mainStyles &= (~OPTION_MAIN_MASK);

	if (optionOpen)
	{
		CMainOption::mainStyles |= OPTION_MAIN_OPEN;
	}
	if (optionExplorer)
	{
		CMainOption::mainStyles |= OPTION_MAIN_EXPLORER;
	}
	if (optionRecent)
	{
		CMainOption::mainStyles |= OPTION_MAIN_RECENT;
	}
	if (optionOption)
	{
		CMainOption::mainStyles |= OPTION_MAIN_OPTION;
	}
	if (optionSubfolder)
	{
		CMainOption::mainStyles |= OPTION_MAIN_SUBFOLDER;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CPropertyPageShell

LRESULT CPropertyPageShell::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	clv.SubclassWindow(GetDlgItem(IDC_SHELL_LIST));

	// 関連付け
	{
		// イメージリストの準備
		HIMAGELIST image;
		image = GetSystemImageList(FALSE);
		clv.SetImageList(image, LVSIL_SMALL);
		image = GetSystemImageList(TRUE);
		clv.SetImageList(image, LVSIL_NORMAL);

		// ヘッダのセット
		LVCOLUMN col;
		ZeroMemory(&col, sizeof(col));
		col.mask = LVCF_TEXT | LVCF_WIDTH;
		col.cx = 96;
		col.pszText = _T("拡張子");
		clv.InsertColumn(0, &col);

#if 0
		col.cx = 192;
		col.pszText = _T("現在のアプリケーション");
		clv.InsertColumn(1, &col);
#endif

		// アイテムの準備
		LVITEM item;
		ZeroMemory(&item, sizeof(item));
		item.mask = LVIF_TEXT | LVIF_IMAGE;
	}


	// ショートカット
	{
		// デスクトップ
		shortcutDesktop = ShortcutExists(CSIDL_DESKTOP);
		// 送る
		shortcutSendto = ShortcutExists(CSIDL_SENDTO);
		// プログラム
		shortcutPrograms = ShortcutExists(CSIDL_PROGRAMS);
		// Quick Launch
		shortcutQuicklaunch = ShortcutExists(CSIDL_APPDATA, _T("Microsoft\\Internet Explorer\\Quick Launch"));
	}

	return TRUE;
}

LRESULT CPropertyPageShell::OnSellAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int count = clv.GetItemCount();
	BOOL all = TRUE;

	if (count > 0)
	{
		// すべてチェックされているか調べる
		for (int i = 0; i < count; i++)
		{
			all  = (all && clv.GetCheckState(i));
			if (!all)
			{
				break;
			}
		}

		// すべて変更
		clv.SetCheckState(-1, !all);
	}
	return 0;
}

BOOL CPropertyPageShell::OnApply()
{
	// 関連付け
	{
		TCHAR ext[MAX_PATH];
		int count = clv.GetItemCount();

		if (count > 0)
		{
			// すべてチェックされているか調べる
			for (int i = 0; i < count; i++)
			{
				if (clv.GetItemText(i, 0, ext, MAX_PATH) > 0)
				{
					SetAssociation(clv.GetCheckState(i), ext);
				}
			}
			SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST | SHCNF_FLUSHNOWAIT, NULL, NULL);
		}
	}

	// ショートカット
	{
		// デスクトップ
		ManageShortcut(CSIDL_DESKTOP, shortcutDesktop);
		// 送る
		ManageShortcut(CSIDL_SENDTO, shortcutSendto);
		// プログラム
		ManageShortcut(CSIDL_PROGRAMS, shortcutPrograms);
		// プログラム
		ManageShortcut(CSIDL_APPDATA, shortcutQuicklaunch, _T("Microsoft\\Internet Explorer\\Quick Launch"));
	}

	return TRUE;
}

BOOL CPropertyPageShell::OnSetActive()
{
	return DoDataExchange(DDX_LOAD);
}

BOOL CPropertyPageShell::OnKillActive()
{
	return DoDataExchange(DDX_SAVE);
}

void CPropertyPageShell::OnDataExchangeError(UINT nCtrlID, BOOL /*bSave*/)
{
	::SetFocus(GetDlgItem(nCtrlID));

	CString message;
	message.LoadString(IDS_OPTION_WARNING_DEFAULT);
	MessageBox(message, NULL, MB_ICONWARNING);
}

void CPropertyPageShell::OnDataValidateError(UINT nCtrlID, BOOL /*bSave*/, _XData& /*data*/)
{
	::SetFocus(GetDlgItem(nCtrlID));

	CString message;
	message.LoadString(IDS_OPTION_WARNING_DEFAULT);
	MessageBox(message, NULL, MB_ICONWARNING);
}

//拡張子がこのアプリケーションに関連付けされているか調べる
BOOL CPropertyPageShell::GetAssociation(LPCTSTR lpExtension)
{
	BOOL bRet = FALSE;
	CRegKey reg;
	CString extKey, shellKey, commandKey, command;
	TCHAR *temp = NULL;
	DWORD len = 0;

	extKey = _T(".");
	extKey += lpExtension;

	// ファイルタイプに対応するキーを得る
	if (reg.Open(HKEY_CLASSES_ROOT, extKey) == ERROR_SUCCESS)
	{
		// 長さを得る
		if (reg.QueryValue(NULL, NULL, &len) == ERROR_SUCCESS)
		{
			temp = new TCHAR[len/sizeof(TCHAR)];
			if (temp)
			{
				ZeroMemory(temp, len);
				if (reg.QueryValue(temp, NULL, &len) == ERROR_SUCCESS)
				{
					shellKey = temp;
					shellKey += _T("\\shell");
				}
				delete [] temp;
				len = 0;
			}
		}
		reg.Close();
	}

	// 標準の関連付けを得る
	if (!shellKey.IsEmpty())
	{
		if (reg.Open(HKEY_CLASSES_ROOT, shellKey) == ERROR_SUCCESS)
		{
			if (reg.QueryValue(NULL, NULL, &len) == ERROR_SUCCESS)
			{
				temp = new TCHAR[len/sizeof(TCHAR)];
				if (temp)
				{
					ZeroMemory(temp, len);
					if (reg.QueryValue(temp, NULL, &len) == ERROR_SUCCESS)
					{
						commandKey = shellKey;
						commandKey += _T("\\");
						commandKey += temp;
						commandKey += _T("\\command");
					}
					delete [] temp;
					len = 0;
				}
			}
			else
			{
				commandKey = shellKey;
				commandKey += _T("\\open\\command");
			}
			reg.Close();
		}
	}

	// コマンドを得る
	if (!commandKey.IsEmpty())
	{
		if (reg.Open(HKEY_CLASSES_ROOT, commandKey) == ERROR_SUCCESS)
		{
			if (reg.QueryValue(NULL, NULL, &len) == ERROR_SUCCESS)
			{
				temp = new TCHAR[len/sizeof(TCHAR)];
				if (temp)
				{
					ZeroMemory(temp, len);
					if (reg.QueryValue(temp, NULL, &len) == ERROR_SUCCESS)
					{
						command = temp;
					}
					delete [] temp;
					len = 0;
				}
			}
			reg.Close();
		}
	}

	// コマンド比較
	if (!command.IsEmpty())
	{
		TCHAR appPath[MAX_PATH];
		if (GetModuleFileName(NULL, appPath, sizeof(appPath)/sizeof(TCHAR)))
		{
			PathQuoteSpaces(appPath);
			if (StrCmpNI(command, appPath, lstrlen(appPath)) == 0)
			{
				bRet = TRUE;
			}
		}
	}

	return bRet;
}

// 拡張子をこのアプリケーションに関連付ける
VOID CPropertyPageShell::SetAssociation(BOOL bAssociate, LPCTSTR lpExtension)
{
	const TCHAR backupValue[] = _T("sakura_backup");
	TCHAR appPath[MAX_PATH];
	CRegKey reg;
	CString extKey, typeKey, oldTypeKey, value;
	TCHAR *temp = NULL;
	DWORD len = 0;

	if (bAssociate == GetAssociation(lpExtension))
	{
		return;
	}

	extKey = _T(".");
	extKey += lpExtension;

	if (bAssociate)
	{
		//このアプリケーションのパス名を取得
		if (!GetModuleFileName(NULL, appPath, sizeof(appPath)/sizeof(TCHAR)))
		{
			return;
		}

		PathQuoteSpaces(appPath);

		// ファイルタイプに対応するキーを得る
		if (reg.Create(HKEY_CLASSES_ROOT, extKey) == ERROR_SUCCESS)
		{
			// 長さを得る
			if (reg.QueryValue(NULL, NULL, &len) == ERROR_SUCCESS)
			{
				temp = new TCHAR[len/sizeof(TCHAR)];
				if (temp)
				{
					ZeroMemory(temp, len);
					if (reg.QueryValue(temp, NULL, &len) == ERROR_SUCCESS)
					{
						oldTypeKey = temp;
					}
					delete [] temp;
					len = 0;
				}
			}
			reg.Close();
		}

		// 新キー名作成
		typeKey = lpExtension;
		typeKey += _T("file");

		if (typeKey.CompareNoCase(oldTypeKey) == 0)
		{
			const CString tmplt = typeKey + _T("file%03d");
			int i = 0;

			typeKey.Format(tmplt, i);
			while (reg.Open(HKEY_CLASSES_ROOT, typeKey) == ERROR_SUCCESS)
			{
				reg.Close();
				i++;
				typeKey.Format(tmplt, i);
			}
		}

		// 新キー作成
		if (reg.Create(HKEY_CLASSES_ROOT, typeKey) == ERROR_SUCCESS)
		{
			CRegKey sub;

			if (sub.Create(reg, _T("DefaultIcon")) == ERROR_SUCCESS)
			{
				value = appPath;
				value += _T(",1");
				sub.SetValue(value);
				sub.Close();
			}

			if (sub.Create(reg, _T("shell\\open\\command")) == ERROR_SUCCESS)
			{
				value = appPath;
				value += _T(" \"%1\"");
				sub.SetValue(value);
				sub.Close();
			}
			
			reg.Close();
		}

		if (reg.Create(HKEY_CLASSES_ROOT, extKey) == ERROR_SUCCESS)
		{
			// 新キー書き込み
			reg.SetValue(typeKey);
			// 旧キー名バックアップ
			if (!oldTypeKey.IsEmpty())
			{
				reg.SetValue(oldTypeKey, backupValue);
			}
			reg.Close();
		}
	}
	else
	{
		if (reg.Open(HKEY_CLASSES_ROOT, extKey) == ERROR_SUCCESS)
		{
			// ファイルタイプに対応するキーを得る
			if (reg.QueryValue(NULL, NULL, &len) == ERROR_SUCCESS)
			{
				temp = new TCHAR[len/sizeof(TCHAR)];
				if (temp)
				{
					ZeroMemory(temp, len);
					if (reg.QueryValue(temp, NULL, &len) == ERROR_SUCCESS)
					{
						oldTypeKey = temp;
					}
					delete [] temp;
					len = 0;
				}
			}

			// バックアップされたキー名を得る
			if (reg.QueryValue(NULL, backupValue, &len) == ERROR_SUCCESS)
			{
				temp = new TCHAR[len/sizeof(TCHAR)];
				if (temp)
				{
					ZeroMemory(temp, len);
					if (reg.QueryValue(temp, backupValue, &len) == ERROR_SUCCESS)
					{
						reg.SetValue(temp);
						reg.DeleteValue(backupValue);
					}
					delete [] temp;
					len = 0;
				}
			}
			else
			{
				reg.DeleteValue(NULL);
			}
			reg.Close();
		}

		if (!oldTypeKey.IsEmpty())
		{
			if (reg.Open(HKEY_CLASSES_ROOT, NULL) == ERROR_SUCCESS)
			{
				reg.RecurseDeleteKey(oldTypeKey);
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// 関数名 : CPropertyPageShell::GetImageIndexFromExtension
// 説明   : 拡張子に対応したアイコンのインデックスを得る
// 戻り値 : INT : 拡張子に対応したアイコンのインデックス
// 引数   : LPCTSTR extension	: 拡張子
INT CPropertyPageShell::GetImageIndexFromExtension(LPCTSTR extension)
{
    SHFILEINFO info;
	CString wildcards(_T("*."));

	ZeroMemory(&info, sizeof(SHFILEINFO));
	wildcards += extension;
    SHGetFileInfo(wildcards, 0, &info, sizeof(SHFILEINFO), 
		SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);
	return info.iIcon;
}

/////////////////////////////////////////////////////////////////////////////
// 関数名 : CPropertyPageShell::GetSystemImageList
// 説明   : システムイメージリストを得る
// 戻り値 : HIMAGELIST : イメージリストのハンドル
// 引数   : BOOL large	: TRUE:大きいアイコン、FALSE:小さいアイコン
HIMAGELIST CPropertyPageShell::GetSystemImageList(BOOL large)
{
    SHFILEINFO info;
    UINT uFlags;
	
	ZeroMemory(&info, sizeof(SHFILEINFO));
    uFlags = SHGFI_SYSICONINDEX | (large ? SHGFI_LARGEICON : SHGFI_SMALLICON);
    return (HIMAGELIST)SHGetFileInfo(_T(""), 0, &info, sizeof(SHFILEINFO), uFlags);
}

/////////////////////////////////////////////////////////////////////////////
// 関数名 : CPropertyPageShell::ShortcutExists
// 説明   : ショートカットファイルが存在するか調べる
// 戻り値 : BOOL : TRUE:存在する、FALSE:存在しない
// 引数   : int nFolder			: 特殊フォルダのCSIDL_*
//          LPCTSTR filePath	: 特殊フォルダからのパス
BOOL CPropertyPageShell::ShortcutExists(int nFolder, LPCTSTR filePath)
{
	TCHAR specialFolderPath[MAX_PATH];	// 特殊フォルダのパス
	CPathString name;	// ショートカットファイルの名前
	CPathString path;	// 特殊フォルダからショートカットファイルまでのパス

	if (filePath)
	{
		path = filePath;
	}
	else
	{
		path = _T("");
	}

	name.LoadString(IDR_MAINFRAME);
	name += _T(".lnk");

	if (SUCCEEDED(SHGetSpecialFolderPath(m_hWnd, specialFolderPath, nFolder, FALSE)))
	{
		PathAddBackslash(specialFolderPath);
		path.AddBackslash();
		name = specialFolderPath + path + name;
		return name.FileExists();
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// 関数名 : CPropertyPageShell::ManageShortcut
// 説明   : ショートカットの作成/削除
// 戻り値 : VOID : なし
// 引数   : int nFolder			: 特殊フォルダのCSIDL_*
//          BOOL create			: TRUE:作成、FALSE:削除
//          LPCTSTR filePath	: 特殊フォルダからのパス
VOID CPropertyPageShell::ManageShortcut(int nFolder, BOOL create, LPCTSTR filePath)
{
	TCHAR specialFolderPath[MAX_PATH];	// 特殊フォルダのパス
	CPathString name;	// ショートカットファイルの名前
	CPathString path;	// 特殊フォルダからショートカットファイルまでのパス

	if (filePath)
	{
		path = filePath;
	}
	else
	{
		path = _T("");
	}

	name.LoadString(IDR_MAINFRAME);
	name += _T(".lnk");

	if (SUCCEEDED(SHGetSpecialFolderPath(m_hWnd, specialFolderPath, nFolder, FALSE)))
	{
		PathAddBackslash(specialFolderPath);
		if (path.GetLength() > 0)
		{
			if (path.GetAt(path.GetLength() - 1) != _T('\\'))
			{
				path += _T('\\');
			}
		}
		name = specialFolderPath + path + name;

		BOOL exists = name.FileExists();
		if (create)
		{
			if (!exists)
			{
				TCHAR appPath[MAX_PATH];
				if (GetModuleFileName(NULL, appPath, sizeof(appPath)/sizeof(TCHAR)))
				{
					CString comment;
					comment.LoadString(IDS_SHORTCUT_COMMENT);
					CreateShortcut(name, appPath, comment);
					SHChangeNotify(SHCNE_CREATE, SHCNF_PATH, name, NULL);
				}
			}
		}
		else
		{
			if (exists)
			{
				DeleteFile(name);
				SHChangeNotify(SHCNE_DELETE, SHCNF_PATH, name, NULL);
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// 関数名 : CPropertyPageShell::CreateShortcut
// 説明   : ショートカットの作成
// 戻り値 : BOOL : TRUE:成功、FALSE:失敗
// 引数   : LPCTSTR shortcut	: 作成するショートカットのフルパス名
//          LPCTSTR source		: リンク元のファイルのフルパス名
//          LPCTSTR comment		: ショートカットの説明
BOOL CPropertyPageShell::CreateShortcut(LPCTSTR shortcut, LPCTSTR source, LPCTSTR comment)
{
	BOOL bRet = FALSE;
	HRESULT hResult;
	IShellLink* psl;

	// COMの初期化
	//CoInitialize(NULL);
	// IShellLink インターフェースの取得
	hResult = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void **)&psl);
	if (SUCCEEDED(hResult))
	{
		IPersistFile* ppf;

		// IPersistFileインターフェースの取得
		hResult = psl->QueryInterface(IID_IPersistFile, (void **)&ppf);
		if (SUCCEEDED(hResult))
		{
			// リンク元のファイルのパスをセットする
			hResult = psl->SetPath(source);
			if (SUCCEEDED(hResult))
			{
				// ショートカットファイルのコメントをセット
				hResult = psl->SetDescription(comment);
				if (SUCCEEDED(hResult))
				{
#ifndef UNICODE
					WCHAR wszShortcutFile[MAX_PATH]; // ユニコード用のバッファ
					MultiByteToWideChar(CP_ACP, 0, shortcut, -1, wszShortcutFile, sizeof(wszShortcutFile)/sizeof(WCHAR));
					hResult = ppf->Save(wszShortcutFile, TRUE);
#else
					hResult = ppf->Save(shortcut, TRUE);
#endif
					if (SUCCEEDED(hResult))
					{
						bRet = TRUE;
					}
				}
			}
			ppf->Release();
		}
		psl->Release();
	}

	//CoUninitialize();
	return bRet;
}

/////////////////////////////////////////////////////////////////////////////
// CPropertyPageAbout

LRESULT CPropertyPageAbout::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// このアプリケーションのバージョンをリソースから取得して表示
	{
		TCHAR appPath[MAX_PATH];
		if (GetModuleFileName(NULL, appPath, sizeof(appPath)/sizeof(TCHAR)))
		{
			CString appName, verstr, title;

			appName.LoadString(IDR_MAINFRAME);
			GetFileVersionString(appPath, verstr);
			title.Format(_T("%s version %s"), appName, verstr);
			::SetWindowText(GetDlgItem(IDC_ABOUT_APPTITLE), title);
		}
	}

	// CPU情報
	{
		CString name;
		CRegKey reg;
		if (reg.Open(HKEY_LOCAL_MACHINE, _T("Hardware\\Description\\System\\CentralProcessor\\0")) == ERROR_SUCCESS)
		{
			DWORD size = 0;
			if (reg.QueryValue(NULL, _T("ProcessorNameString"), &size) == ERROR_SUCCESS)
			{
				TCHAR *regname = new TCHAR[size/sizeof(TCHAR)];
				ZeroMemory(regname, sizeof(regname));
				if (reg.QueryValue(regname, _T("ProcessorNameString"), &size) == ERROR_SUCCESS)
				{
					name = regname;
					name.TrimLeft();
					name = _T("CPU : ") + name;
					::SetWindowText(GetDlgItem(IDC_ABOUT_CPU), name);
				}
				delete [] regname;
			}
		}
	}

	// Windows のバージョン
	{
		OSVERSIONINFO ovi;
		ovi.dwOSVersionInfoSize = sizeof(ovi);
		if (GetVersionEx(&ovi))
		{
			CString version;
			switch (ovi.dwPlatformId)
			{
				case VER_PLATFORM_WIN32_WINDOWS:
				{
					CString os;
					DWORD dwMajorVersion = HIBYTE(HIWORD(ovi.dwBuildNumber));
					DWORD dwMinorVersion = LOBYTE(HIWORD(ovi.dwBuildNumber));
					DWORD dwBuildNumber = LOWORD(ovi.dwBuildNumber);

					switch (dwMinorVersion)
					{
						case 0:
						{
							os = _T("95");
							break;
						}
						case 10:
						{
							os = _T("98");
							break;
						}
						case 90:
						{
							os = _T("Me");
							break;
						}
						default:
						{
							os = _T("");
							break;
						}
					}
					version.Format(_T("OS : Microsoft Windows %s %d.%02d.%04d"), os, dwMajorVersion, dwMinorVersion, dwBuildNumber);
					break;
				}
				case VER_PLATFORM_WIN32_NT:
				{
					CString os;
					switch (ovi.dwMajorVersion)
					{
						case 3:
						{
							os = _T("NT 3.51");
							break;
						}
						case 4:
						{
							os = _T("NT 4.0");
							break;
						}
						case 5:
						{
							switch (ovi.dwMinorVersion)
							{
								case 0:
								{
									os = _T("2000");
									break;
								}
								case 1:
								{
									os = _T("XP");
									break;
								}
								case 2:
								{
									os = _T("Server 2003");
									break;
								}
							}
							break;
						}
						default:
						{
							os = _T("");
							break;
						}
					}
					version.Format(_T("OS : Microsoft Windows %s %d.%02d.%04d %s"), os, ovi.dwMajorVersion, ovi.dwMinorVersion, ovi.dwBuildNumber, ovi.szCSDVersion);
					break;
				}
				default:
				version =  _T("OS : Unknown");
				break;
			}
			::SetWindowText(GetDlgItem(IDC_ABOUT_OS), version);
		}
	}

	// InternetExplorer のバージョン
	{
		CString version;
		CRegKey reg;
		if (reg.Open(HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\Internet Explorer")) == ERROR_SUCCESS)
		{
			DWORD size = 0;
			if (reg.QueryValue(NULL, _T("Version"), &size) == ERROR_SUCCESS)
			{
				TCHAR *regver = new TCHAR[size/sizeof(TCHAR)];
				if (reg.QueryValue(regver, _T("Version"), &size) == ERROR_SUCCESS)
				{
					version.Format(_T("Internet Explorer : %s"), regver);
					::SetWindowText(GetDlgItem(IDC_ABOUT_IE), version);
				}
				delete [] regver;
			}
		}
	}

	// システムDLLのバージョン
	{
		// ファイル名
		TCHAR *file[] = {
			_T("shell32.dll"),
			_T("shlwapi.dll"),
			_T("comctl32.dll"),
			_T("user32.dll")
		};
		INT count = sizeof(file)/sizeof(TCHAR *);
		HWND hList = GetDlgItem(IDC_ABOUT_DLLVERSION_LIST);

		if (hList)
		{
			for (INT i = 0; i < count; i++)
			{
				CString verstr, title;

				GetFileVersionString(file[i], verstr, TRUE);
				title.Format(_T("%s : %s"), file[i], verstr);
				::SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)(LPCTSTR)title);
			}
		}
	}

	// DLLのアイコン
	{
		HICON hIcon = GetIconFromExtension(_T("*.dll"));
		if (hIcon)
		{
			SendMessage(GetDlgItem(IDC_ABOUT_DLLICON), STM_SETICON, (WPARAM)hIcon, 0);
		}
	}

	// ハイパーリンク
	{
		hyperLink.SubclassWindow(GetDlgItem(IDC_ABOUT_URL));
		hyperLink.SetHyperLinkExtendedStyle(HLINK_UNDERLINEHOVER, HLINK_UNDERLINEHOVER);
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// 関数名 : CPropertyPageAbout::GetIconFromExtension
// 説明   : 拡張子に対応したアイコンのハンドルを得る
// 戻り値 : HICON : 拡張子に対応したアイコンのハンドル
// 引数   : LPCTSTR extension	: 拡張子
HICON CPropertyPageAbout::GetIconFromExtension(LPCTSTR extension)
{
    SHFILEINFO info;
	CString wildcards(_T("*."));

	ZeroMemory(&info, sizeof(SHFILEINFO));
	wildcards += extension;
    SHGetFileInfo(wildcards, 0, &info, sizeof(SHFILEINFO), 
		SHGFI_ICON | SHGFI_LARGEICON | SHGFI_USEFILEATTRIBUTES);
	return info.hIcon;
}

/////////////////////////////////////////////////////////////////////////////
// 関数名 : CPropertyPageAbout::GetFileVersionString
// 説明   : ファイルのバージョンを表す文字列を得る
// 戻り値 : VOID : なし
// 引数   : LPCTSTR file	: バージョンを得るファイルのフルパス名
//          CString &verstr	: バージョンを表す文字列を入れるCString変数への参照
//          BOOL detail		: バージョンを詳しく表示するか
VOID CPropertyPageAbout::GetFileVersionString(LPCTSTR file, CString &verstr, BOOL detail)
{
	DWORD handle;
	DWORD size = GetFileVersionInfoSize(file, &handle);

	LPVOID version = malloc(size);
	if (size > 0 && version != NULL)
	{
		UINT usize = size;
		if (GetFileVersionInfo(file, handle, size, version))
		{
			VS_FIXEDFILEINFO *pvffi;
			if (VerQueryValue(version, _T("\\"), (LPVOID *)&pvffi, &usize))
			{
				if (detail)
				{
					verstr.Format(_T("%d.%02d.%04d.%04d"), HIWORD(pvffi->dwFileVersionMS), LOWORD(pvffi->dwFileVersionMS), HIWORD(pvffi->dwFileVersionLS), LOWORD(pvffi->dwFileVersionLS));
				}
				else
				{
					verstr.Format(_T("%d.%02d"), HIWORD(pvffi->dwFileVersionMS), LOWORD(pvffi->dwFileVersionMS));
				}
			}
		}
	}
	free(version);
}
