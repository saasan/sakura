// listfile.cpp : implementation of the CListFile class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "listfile.h"
#include "sakuraOptions.h"

LRESULT CListFile::OnDropFiles(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	HDROP hdrop = (HDROP)wParam;
	UINT num;
	UINT size;

	ShowWindow(SW_HIDE);

	num = DragQueryFile(hdrop, 0xFFFFFFFF, NULL, 0);
	for (UINT i = 0; i < num; i++)
	{
		// �o�b�t�@�̕K�v�T�C�Y�𓾂�
		size = DragQueryFile(hdrop, i, NULL, 0);
		// null�������܂܂Ȃ����������Ԃ�̂�null������+1
		size++;

		TCHAR *lpszFile = new TCHAR[size];

		if (lpszFile)
		{
			DragQueryFile(hdrop, i, lpszFile, size);

			AddFile(lpszFile);
			delete [] lpszFile;
			lpszFile = NULL;
		}
	}
	DragFinish(hdrop);

	ShowWindow(SW_SHOW);
	SetHorizontalScrollBar();
	return 0;
}

LRESULT CListFile::OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
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

LRESULT CListFile::OnLButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	BOOL outside;
	POINT point;
	INT index;

	bHandled = FALSE;

	// ���X�g�ȊO�̏ꏊ���N���b�N�����ꍇ�͑I������
	point.x = LOWORD(lParam);
	point.y = HIWORD(lParam);
	index = ItemFromPoint(point, outside);
	if (outside)
	{
		if (GetSelCount() > 0)
		{
			INT count = GetCount();
			if (count > 0)
			{
				for (INT i = 0; i < count; i++)
				{
					SetSel(i, FALSE);
				}
				bHandled = TRUE;
			}
		}
	}
	return 0;
}

LRESULT CListFile::OnRButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	BOOL outside;
	POINT point;
	INT index;

	bHandled = FALSE;

	// �I������Ă��Ȃ��ꏊ���E�N���b�N�����ꍇ�͑I��
	point.x = LOWORD(lParam);
	point.y = HIWORD(lParam);
	index = ItemFromPoint(point, outside);
	if (!outside)
	{
		if (GetSel(index) < 1)
		{
			INT count = GetCount();
			if (count > 0)
			{
				for (INT i = 0; i < count; i++)
				{
					SetSel(i, (i == index));
				}
				bHandled = TRUE;
			}
		}
	}
	return 0;
}

LRESULT CListFile::OnRButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	HMENU hMenu = LoadMenu(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME));
	if (hMenu != NULL)
	{
		CMenu sub(::GetSubMenu(hMenu, 0));
		POINT point;
		point.x = LOWORD(lParam);
		point.y = HIWORD(lParam);
		ClientToScreen(&point);
		sub.TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN, point.x, point.y, GetParent());
	}
	return 0;
}

// ���ڂ����ׂđI������
VOID CListFile::SelectAll()
{
	INT count = GetCount();
	if (count > 0)
	{
		if (count == 1)
		{
			SetSel(0, TRUE);
		}
		else
		{
			SelItemRange(TRUE, 0, count-1);
		}
	}
}

// ���ڂ��폜����
VOID CListFile::Remove()
{
	INT count, *selected;

	count = GetSelCount();
	selected = new INT[count];
	if (GetSelItems(count, selected) != LB_ERR)
	{
		// ���̕�����폜
		for (int i = count-1; i>=0; i--)
		{
			DeleteString(*(selected+i));
		}
	}
	delete [] selected;
	SetHorizontalScrollBar();
}

// ���ڂ�ǉ�����
VOID CListFile::Add(LPCTSTR str)
{
	// ���݂��Ȃ���Βǉ�
	if (FindStringExact(-1, str) == LB_ERR)
	{
		AddString(str);
	}
}

VOID CListFile::AddFile(LPCTSTR strFilePath)
{
	CPathString str(strFilePath);

	// �t�H���_�̏ꍇ�̓t�H���_��������
	if (str.IsDirectory())
	{
		str.AddBackslash();

		// ����
		CFindFile ff;
		BOOL flag;
		for (flag = ff.FindFile(str + _T('*')); flag; flag = ff.FindNextFile())
		{
			// �o�^
			if (ff.IsDirectory())
			{
				// �T�u�f�B���N�g���o�^
				if (CMainOption::mainStyles & OPTION_MAIN_SUBFOLDER)
				{
					// �J�����g�f�B���N�g���A�e�f�B���N�g���͏���
					if (lstrcmp(ff.GetFileName(), _T(".")) && lstrcmp(ff.GetFileName(), _T("..")))
					{
						AddFile(str + ff.GetFileName());
					}
				}
			}
			else
			{
				Add(str + ff.GetFileName());
			}
		}
		ff.Close();
	}
	// �t�@�C���̏ꍇ�͒ǉ�  
	else
	{
		Add(str);
	}
}

// ��������ݒ�(�����X�N���[���o�[�\��)
VOID CListFile::SetHorizontalScrollBar()
{
	UINT count = GetCount();
	if (count > 0)
	{
		TCHAR item[MAX_PATH];
		INT maxLength = 0, maxIndex = 0, width = 0, iSave;
		SIZE size;
		HDC hDC;
		HFONT hFont;

		size.cx = size.cy = 0;

		// ��Ԓ����������T��
		for (int i = 0; (UINT)i < count; i++)
		{
			INT len = GetTextLen(i);
			if (len > maxLength)
			{
				maxLength = len;
				maxIndex = i;
			}
		}

		// ������̕����v�Z
		hDC = GetDC();
		{
			iSave = SaveDC(hDC);
			{
				hFont = GetFont();
				if (hFont != NULL)
					SelectObject(hDC, hFont);
				GetText(maxIndex, item);
				GetTextExtentPoint32(hDC, item, lstrlen(item), &size);
				width = size.cx + GetSystemMetrics(SM_CXVSCROLL);
				SetHorizontalExtent(width);
			}
			RestoreDC(hDC, iSave);
		}
        ReleaseDC(hDC);
	}
	else
	{
		SetHorizontalExtent(0);
	}
}

