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
		// バッファの必要サイズを得る
		size = DragQueryFile(hdrop, i, NULL, 0);
		// null文字を含まない文字数が返るのでnull文字分+1
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

	// リスト以外の場所をクリックした場合は選択解除
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

	// 選択されていない場所を右クリックした場合は選択
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

// 項目をすべて選択する
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

// 項目を削除する
VOID CListFile::Remove()
{
	INT count, *selected;

	count = GetSelCount();
	selected = new INT[count];
	if (GetSelItems(count, selected) != LB_ERR)
	{
		// 後ろの方から削除
		for (int i = count-1; i>=0; i--)
		{
			DeleteString(*(selected+i));
		}
	}
	delete [] selected;
	SetHorizontalScrollBar();
}

// 項目を追加する
VOID CListFile::Add(LPCTSTR str)
{
	// 存在しなければ追加
	if (FindStringExact(-1, str) == LB_ERR)
	{
		AddString(str);
	}
}

VOID CListFile::AddFile(LPCTSTR strFilePath)
{
	CPathString str(strFilePath);

	// フォルダの場合はフォルダ内を検索
	if (str.IsDirectory())
	{
		str.AddBackslash();

		// 検索
		CFindFile ff;
		BOOL flag;
		for (flag = ff.FindFile(str + _T('*')); flag; flag = ff.FindNextFile())
		{
			// 登録
			if (ff.IsDirectory())
			{
				// サブディレクトリ登録
				if (CMainOption::mainStyles & OPTION_MAIN_SUBFOLDER)
				{
					// カレントディレクトリ、親ディレクトリは除く
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
	// ファイルの場合は追加  
	else
	{
		Add(str);
	}
}

// 水平幅を設定(水平スクロールバー表示)
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

		// 一番長い文字列を探す
		for (int i = 0; (UINT)i < count; i++)
		{
			INT len = GetTextLen(i);
			if (len > maxLength)
			{
				maxLength = len;
				maxIndex = i;
			}
		}

		// 文字列の幅を計算
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

