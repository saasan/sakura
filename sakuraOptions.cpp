#include "stdafx.h"
#include "resource.h"

#include "sakura.h"
#include "sakuraOptions.h"
#include "converter.h"

HINSTANCE CMainOption::hCCLDll	= NULL;
DWORD CMainOption::mainStyles	= 0;
DWORD CMainOption::snapWidth	= SNAP_WIDTH_DEFAULT;
DWORD CMainOption::transValue	= TRANS_VALUE_DEFAULT;

/////////////////////////////////////////////////////////////////////////////
// CHoverHyperLink

BOOL CHoverHyperLink::SubclassWindow(HWND hWnd)
{
	BOOL bRet = CHyperLinkImpl<CHoverHyperLink>::SubclassWindow(hWnd);
	if(bRet)
		Init();
	return bRet;
}

LRESULT CHoverHyperLink::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LRESULT ret = CHyperLinkImpl<CHoverHyperLink>::OnCreate(uMsg, wParam, lParam, bHandled);
	Init();
	return ret;
}

LRESULT CHoverHyperLink::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	if(m_lpstrHyperLink != NULL && ::PtInRect(&m_rcLink, pt))
	{
		// �}�E�X��������
		if (m_bHover == false)
		{
			// ��Ԃ��ς�����Ƃ������ĕ`��
			m_bHover = true;
			RedrawWindow();
		}
		SetCapture();
	}
	else
	{
		// �}�E�X���o��
		if (m_bHover == true)
		{
			// ��Ԃ��ς�����Ƃ������ĕ`��
			m_bHover = false;
			RedrawWindow();
		}
		ReleaseCapture();
	}
	return CHyperLinkImpl<CHoverHyperLink>::OnMouseMove(uMsg, wParam, lParam, bHandled);
}

void CHoverHyperLink::Init()
{
	if(m_bPaintLabel)
	{
		CWindow wnd = GetParent();
		CFontHandle font = wnd.GetFont();
		if(font.m_hFont != NULL)
		{
			LOGFONT lf;
			font.GetLogFont(&lf);

			if(m_hFont != NULL)
				::DeleteObject(m_hFont);

			lf.lfUnderline = FALSE;
			m_hFont = ::CreateFontIndirect(&lf);

			lf.lfUnderline = TRUE;
			m_hHoverFont = ::CreateFontIndirect(&lf);
		}
	}
}

void CHoverHyperLink::DoPaint(CDCHandle dc)
{
	dc.SetBkMode(TRANSPARENT);
	dc.SetTextColor(m_bVisited ? m_clrVisited : m_clrLink);
	if(m_hFont != NULL && m_hHoverFont != NULL)
	{
		dc.SelectFont((m_bHover ? m_hHoverFont : m_hFont));
	}
	LPTSTR lpstrText = (m_lpstrLabel != NULL) ? m_lpstrLabel : m_lpstrHyperLink;
	DWORD dwStyle = GetStyle();
	int nDrawStyle = DT_LEFT;
	if (dwStyle & SS_CENTER)
	{
		nDrawStyle = DT_CENTER;
	}
	else if (dwStyle & SS_RIGHT)
	{
		nDrawStyle = DT_RIGHT;
	}
	dc.DrawText(lpstrText, -1, &m_rcLink, nDrawStyle | DT_WORDBREAK);
	if(GetFocus() == m_hWnd)
	{
		dc.DrawFocusRect(&m_rcLink);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CPropertySheetNoHelp

LRESULT CPropertySheetNoHelp::OnShowWindow(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
	BOOL fShow = (BOOL) wParam;
	if (fShow)
	{
		// �E�C���h�E�̔�������
		SetTransparent();
		// �E���[?]�{�^��������
		LONG style = ::GetWindowLong(m_hWnd, GWL_EXSTYLE);
		style &= ~(WS_EX_CONTEXTHELP);
		::SetWindowLong(m_hWnd, GWL_EXSTYLE, style);
	}
	bHandled = FALSE;
	return 1;
}

// �E�C���h�E�̔�������
VOID CPropertySheetNoHelp::SetTransparent()
{
	LONG exstyle = GetWindowLong(GWL_EXSTYLE);
	if (trans)
	{
		exstyle |= WS_EX_LAYERED;
		SetWindowLong(GWL_EXSTYLE, exstyle);

		BYTE bAlpha = (BYTE)(255 * transValue / 100);
		SetLayeredWindowAttributes(m_hWnd, NULL, bAlpha, LWA_ALPHA);
	}
	else
	{
		exstyle &= ~(WS_EX_LAYERED);
		SetWindowLong(GWL_EXSTYLE, exstyle);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CPropertyPageMain

LRESULT CPropertyPageMain::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// �W�J�旚�𐔌�����(3)
	SendMessage(GetDlgItem(IDC_MAIN_FOLDER_COUNT_EDIT),
		EM_SETLIMITTEXT, (WPARAM) 3, (LPARAM) 0);
	// �W�J�旚�𐔔͈͐���
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
	optionArc		= ((CMainOption::mainStyles & OPTION_MAIN_ARC      ) ? TRUE : FALSE);
	optionSpi		= ((CMainOption::mainStyles & OPTION_MAIN_SPI      ) ? TRUE : FALSE);
	optionExe		= ((CMainOption::mainStyles & OPTION_MAIN_EXE      ) ? TRUE : FALSE);
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
	if (optionArc)
	{
		CMainOption::mainStyles |= OPTION_MAIN_ARC;
	}
	if (optionSpi)
	{
		CMainOption::mainStyles |= OPTION_MAIN_SPI;
	}
	if (optionExe)
	{
		CMainOption::mainStyles |= OPTION_MAIN_EXE;
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
// CPropertyPageWindows

LRESULT CPropertyPageWindows::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// �X�i�b�v������(2)
	SendMessage(GetDlgItem(IDC_WINDOW_SNAP_WIDTH_EDIT),
		EM_SETLIMITTEXT, (WPARAM) 2, (LPARAM) 0);
	// �X�i�b�v�͈͐���
	SendMessage(GetDlgItem(IDC_WINDOW_SNAP_WIDTH_UPDOWN),
		UDM_SETRANGE, (WPARAM) 0, (LPARAM) MAKELONG(SNAP_WIDTH_MAX, SNAP_WIDTH_MIN));

	// �����x������(3)
	SendMessage(GetDlgItem(IDC_WINDOW_TRANS_VALUE_EDIT),
		EM_SETLIMITTEXT, (WPARAM) 3, (LPARAM) 0);
	// �����x�͈͐���
	SendMessage(GetDlgItem(IDC_WINDOW_TRANS_VALUE_UPDOWN),
		UDM_SETRANGE, (WPARAM) 0, (LPARAM) MAKELONG(TRANS_VALUE_MAX, TRANS_VALUE_MIN));
	return TRUE;
}

BOOL CPropertyPageWindows::OnSetActive()
{
	return DoDataExchange(DDX_LOAD);
}

BOOL CPropertyPageWindows::OnKillActive()
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

void CPropertyPageWindows::OnDataExchangeError(UINT nCtrlID, BOOL /*bSave*/)
{
	::SetFocus(GetDlgItem(nCtrlID));

	CString message;
	message.LoadString(IDS_OPTION_WARNING_DEFAULT);
	MessageBox(message, NULL, MB_ICONWARNING);
}

void CPropertyPageWindows::OnDataValidateError(UINT nCtrlID, BOOL /*bSave*/, _XData& /*data*/)
{
	::SetFocus(GetDlgItem(nCtrlID));

	switch (nCtrlID)
	{
		case IDC_WINDOW_SNAP_WIDTH_EDIT:
		{
			CString message, format;
			format.LoadString(IDS_OPTION_WARNING_MAXCOUNT);
			message.Format(format, SNAP_WIDTH_MIN, SNAP_WIDTH_MAX);
			MessageBox(message, NULL, MB_ICONWARNING);
			break;
		}
		case IDC_WINDOW_TRANS_VALUE_EDIT:
		{
			CString message, format;
			format.LoadString(IDS_OPTION_WARNING_MAXCOUNT);
			message.Format(format, TRANS_VALUE_MIN, TRANS_VALUE_MAX);
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

void CPropertyPageWindows::LoadData()
{
	optionTop	= ((CMainOption::mainStyles & OPTION_WINDOW_TOP) ? TRUE : FALSE);
	optionSnap	= ((CMainOption::mainStyles & OPTION_WINDOW_SNAP) ? TRUE : FALSE);
	optionTrans	= ((CMainOption::mainStyles & OPTION_WINDOW_TRANS) ? TRUE : FALSE);

	optionSnapWidth = CMainOption::snapWidth;
	optionTransValue = CMainOption::transValue;
}

void CPropertyPageWindows::SaveData()
{
	CMainOption::mainStyles &= (~OPTION_WINDOW_MASK);

	if (optionTop)
	{
		CMainOption::mainStyles |= OPTION_WINDOW_TOP;
	}
	if (optionSnap)
	{
		CMainOption::mainStyles |= OPTION_WINDOW_SNAP;
	}
	if (optionTrans)
	{
		CMainOption::mainStyles |= OPTION_WINDOW_TRANS;
	}

	CMainOption::snapWidth = optionSnapWidth;
	CMainOption::transValue = optionTransValue;
}

/////////////////////////////////////////////////////////////////////////////
// CPropertyPageShell

LRESULT CPropertyPageShell::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	clv.SubclassWindow(GetDlgItem(IDC_SHELL_LIST));

	// �֘A�t��
	{
		// �C���[�W���X�g�̏���
		HIMAGELIST image;
		image = GetSystemImageList(FALSE);
		clv.SetImageList(image, LVSIL_SMALL);
		image = GetSystemImageList(TRUE);
		clv.SetImageList(image, LVSIL_NORMAL);

		// �w�b�_�̃Z�b�g
		LVCOLUMN col;
		ZeroMemory(&col, sizeof(col));
		col.mask = LVCF_TEXT | LVCF_WIDTH;
		col.cx = 96;
		col.pszText = _T("�g���q");
		clv.InsertColumn(0, &col);

#if 0
		col.cx = 192;
		col.pszText = _T("���݂̃A�v���P�[�V����");
		clv.InsertColumn(1, &col);
#endif

		// �A�C�e���̏���
		LVITEM item;
		ZeroMemory(&item, sizeof(item));
		item.mask = LVIF_TEXT | LVIF_IMAGE;


		// �R���o�[�^���X�g
		list<CConverter *> convList;

		// �R���o�[�^�����[�h
		CConverterManager manager(m_hWnd);
		manager.Load();

		// �R���o�[�^�𓾂�
		manager.GetConverters(convList);

		list<CConverter *>::iterator it, end;
		it = convList.begin();
		end = convList.end();

		// ���ׂẴR���o�[�^�́A���ׂĂ̑Ή��g���q��o�^
		for (; it != end; it++)
		{
			// �t�@�C�������݂��Ȃ�������X�L�b�v
			if (!CPathString::SystemFileExists((*it)->file))
			{
				continue;
			}

			list<CString> extList;

			// �Ή��g���q���X�g�𓾂�
			(*it)->GetSupportedExtensions(extList);

			list<CString>::iterator eit, eend;
			eit = extList.begin();
			eend = extList.end();
			for (; eit != eend; eit++)
			{
				if (!(*eit).IsEmpty())
				{
					int index = -1;
					int len = (*eit).GetLength();
					len++;
					TCHAR *text = new TCHAR[len];
					_tcsncpy(text, (LPCTSTR)(*eit), len);

					// ���X�g�ɓo�^
					item.pszText = text;
					item.iImage = GetImageIndexFromExtension(item.pszText);
					index = clv.InsertItem(&item);
					// ���X�g�o�^������
					if (index >= 0)
					{
						// �֘A�t���̃`�F�b�N
						clv.SetCheckState(index, GetAssociation(text));
					}

					DELETE_ARRAY_PTR(text);
				}
			}
		}
	}


	// �V���[�g�J�b�g
	{
		// �f�X�N�g�b�v
		shortcutDesktop = ShortcutExists(CSIDL_DESKTOP);
		// ����
		shortcutSendto = ShortcutExists(CSIDL_SENDTO);
		// �v���O����
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
		// ���ׂă`�F�b�N����Ă��邩���ׂ�
		for (int i = 0; i < count; i++)
		{
			all  = (all && clv.GetCheckState(i));
			if (!all)
			{
				break;
			}
		}

		// ���ׂĕύX
		clv.SetCheckState(-1, !all);
	}
	return 0;
}

BOOL CPropertyPageShell::OnApply()
{
	// �֘A�t��
	{
		TCHAR ext[MAX_PATH];
		int count = clv.GetItemCount();

		if (count > 0)
		{
			// ���ׂă`�F�b�N����Ă��邩���ׂ�
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

	// �V���[�g�J�b�g
	{
		// �f�X�N�g�b�v
		ManageShortcut(CSIDL_DESKTOP, shortcutDesktop);
		// ����
		ManageShortcut(CSIDL_SENDTO, shortcutSendto);
		// �v���O����
		ManageShortcut(CSIDL_PROGRAMS, shortcutPrograms);
		// �v���O����
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

//�g���q�����̃A�v���P�[�V�����Ɋ֘A�t������Ă��邩���ׂ�
BOOL CPropertyPageShell::GetAssociation(LPCTSTR lpExtension)
{
	BOOL bRet = FALSE;
	CRegKey reg;
	CString extKey, shellKey, commandKey, command;
	TCHAR *temp = NULL;
	DWORD len = 0;

	extKey = _T(".");
	extKey += lpExtension;

	// �t�@�C���^�C�v�ɑΉ�����L�[�𓾂�
	if (reg.Open(HKEY_CLASSES_ROOT, extKey) == ERROR_SUCCESS)
	{
		// �����𓾂�
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
				DELETE_ARRAY_PTR(temp);
				len = 0;
			}
		}
		reg.Close();
	}

	// �W���̊֘A�t���𓾂�
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
					DELETE_ARRAY_PTR(temp);
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

	// �R�}���h�𓾂�
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
					DELETE_ARRAY_PTR(temp);
					len = 0;
				}
			}
			reg.Close();
		}
	}

	// �R�}���h��r
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

// �g���q�����̃A�v���P�[�V�����Ɋ֘A�t����
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
		//���̃A�v���P�[�V�����̃p�X�����擾
		if (!GetModuleFileName(NULL, appPath, sizeof(appPath)/sizeof(TCHAR)))
		{
			return;
		}

		PathQuoteSpaces(appPath);

		// �t�@�C���^�C�v�ɑΉ�����L�[�𓾂�
		if (reg.Create(HKEY_CLASSES_ROOT, extKey) == ERROR_SUCCESS)
		{
			// �����𓾂�
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
					DELETE_ARRAY_PTR(temp);
					len = 0;
				}
			}
			reg.Close();
		}

		// �V�L�[���쐬
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

		// �V�L�[�쐬
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
			// �V�L�[��������
			reg.SetValue(typeKey);
			// ���L�[���o�b�N�A�b�v
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
			// �t�@�C���^�C�v�ɑΉ�����L�[�𓾂�
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
					DELETE_ARRAY_PTR(temp);
					len = 0;
				}
			}

			// �o�b�N�A�b�v���ꂽ�L�[���𓾂�
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
					DELETE_ARRAY_PTR(temp);
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
// �֐��� : CPropertyPageShell::GetImageIndexFromExtension
// ����   : �g���q�ɑΉ������A�C�R���̃C���f�b�N�X�𓾂�
// �߂�l : INT : �g���q�ɑΉ������A�C�R���̃C���f�b�N�X
// ����   : LPCTSTR extension	: �g���q
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
// �֐��� : CPropertyPageShell::GetSystemImageList
// ����   : �V�X�e���C���[�W���X�g�𓾂�
// �߂�l : HIMAGELIST : �C���[�W���X�g�̃n���h��
// ����   : BOOL large	: TRUE:�傫���A�C�R���AFALSE:�������A�C�R��
HIMAGELIST CPropertyPageShell::GetSystemImageList(BOOL large)
{
    SHFILEINFO info;
    UINT uFlags;
	
	ZeroMemory(&info, sizeof(SHFILEINFO));
    uFlags = SHGFI_SYSICONINDEX | (large ? SHGFI_LARGEICON : SHGFI_SMALLICON);
    return (HIMAGELIST)SHGetFileInfo(_T(""), 0, &info, sizeof(SHFILEINFO), uFlags);
}

/////////////////////////////////////////////////////////////////////////////
// �֐��� : CPropertyPageShell::ShortcutExists
// ����   : �V���[�g�J�b�g�t�@�C�������݂��邩���ׂ�
// �߂�l : BOOL : TRUE:���݂���AFALSE:���݂��Ȃ�
// ����   : int nFolder			: ����t�H���_��CSIDL_*
//          LPCTSTR filePath	: ����t�H���_����̃p�X
BOOL CPropertyPageShell::ShortcutExists(int nFolder, LPCTSTR filePath)
{
	TCHAR specialFolderPath[MAX_PATH];	// ����t�H���_�̃p�X
	CPathString name;	// �V���[�g�J�b�g�t�@�C���̖��O
	CPathString path;	// ����t�H���_����V���[�g�J�b�g�t�@�C���܂ł̃p�X

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
// �֐��� : CPropertyPageShell::ManageShortcut
// ����   : �V���[�g�J�b�g�̍쐬/�폜
// �߂�l : VOID : �Ȃ�
// ����   : int nFolder			: ����t�H���_��CSIDL_*
//          BOOL create			: TRUE:�쐬�AFALSE:�폜
//          LPCTSTR filePath	: ����t�H���_����̃p�X
VOID CPropertyPageShell::ManageShortcut(int nFolder, BOOL create, LPCTSTR filePath)
{
	TCHAR specialFolderPath[MAX_PATH];	// ����t�H���_�̃p�X
	CPathString name;	// �V���[�g�J�b�g�t�@�C���̖��O
	CPathString path;	// ����t�H���_����V���[�g�J�b�g�t�@�C���܂ł̃p�X

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
// �֐��� : CPropertyPageShell::CreateShortcut
// ����   : �V���[�g�J�b�g�̍쐬
// �߂�l : BOOL : TRUE:�����AFALSE:���s
// ����   : LPCTSTR shortcut	: �쐬����V���[�g�J�b�g�̃t���p�X��
//          LPCTSTR source		: �����N���̃t�@�C���̃t���p�X��
//          LPCTSTR comment		: �V���[�g�J�b�g�̐���
BOOL CPropertyPageShell::CreateShortcut(LPCTSTR shortcut, LPCTSTR source, LPCTSTR comment)
{
	BOOL bRet = FALSE;
	HRESULT hResult;
	IShellLink* psl;

	// COM�̏�����
	//CoInitialize(NULL);
	// IShellLink �C���^�[�t�F�[�X�̎擾
	hResult = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void **)&psl);
	if (SUCCEEDED(hResult))
	{
		IPersistFile* ppf;

		// IPersistFile�C���^�[�t�F�[�X�̎擾
		hResult = psl->QueryInterface(IID_IPersistFile, (void **)&ppf);
		if (SUCCEEDED(hResult))
		{
			// �����N���̃t�@�C���̃p�X���Z�b�g����
			hResult = psl->SetPath(source);
			if (SUCCEEDED(hResult))
			{
				// �V���[�g�J�b�g�t�@�C���̃R�����g���Z�b�g
				hResult = psl->SetDescription(comment);
				if (SUCCEEDED(hResult))
				{
#ifndef UNICODE
					WCHAR wszShortcutFile[MAX_PATH]; // ���j�R�[�h�p�̃o�b�t�@
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
	// ���̃A�v���P�[�V�����̃o�[�W���������\�[�X����擾���ĕ\��
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

	// CPU���
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

	// Windows �̃o�[�W����
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

	// InternetExplorer �̃o�[�W����
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

	// �V�X�e��DLL�̃o�[�W����
	{
		// �t�@�C����
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

	// DLL�̃A�C�R��
	{
		HICON hIcon = GetIconFromExtension(_T("*.dll"));
		if (hIcon)
		{
			SendMessage(GetDlgItem(IDC_ABOUT_DLLICON), STM_SETICON, (WPARAM)hIcon, 0);
		}
	}

	// �n�C�p�[�����N
	{
		hyperLink.SubclassWindow(GetDlgItem(IDC_ABOUT_URL));
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// �֐��� : CPropertyPageAbout::GetIconFromExtension
// ����   : �g���q�ɑΉ������A�C�R���̃n���h���𓾂�
// �߂�l : HICON : �g���q�ɑΉ������A�C�R���̃n���h��
// ����   : LPCTSTR extension	: �g���q
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
// �֐��� : CPropertyPageAbout::GetFileVersionString
// ����   : �t�@�C���̃o�[�W������\��������𓾂�
// �߂�l : VOID : �Ȃ�
// ����   : LPCTSTR file	: �o�[�W�����𓾂�t�@�C���̃t���p�X��
//          CString &verstr	: �o�[�W������\�������������CString�ϐ��ւ̎Q��
//          BOOL detail		: �o�[�W�������ڂ����\�����邩
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
