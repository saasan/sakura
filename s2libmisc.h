#ifndef __S2LIBMISC_H__
#define __S2LIBMISC_H__

#pragma once

#ifndef __ATLMISC_H__
	#error s2libmisc.h requires atlmisc.h to be included first
#endif

#ifndef _INC_SHLWAPI
	#error s2libmisc.h requires shlwapi.h to be included first
#endif

/////////////////////////////////////////////////////////////////////////////
// Classes in this file
//
// CPathString


namespace WTL
{

/////////////////////////////////////////////////////////////////////////////
// CPathString - PathString class

#ifndef _WTL_NO_CSTRING

class CPathString : public CString
{
public:
// Constructors
	CPathString() : CString() {}
	CPathString(const CString& stringSrc) : CString(stringSrc) {}
	CPathString(TCHAR ch, int nRepeat = 1) : CString(ch, nRepeat) {}
	CPathString(LPCSTR lpsz) : CString(lpsz) {}
	CPathString(LPCWSTR lpsz) : CString(lpsz) {}
	CPathString(LPCSTR lpch, int nLength) : CString(lpch, nLength) {}
	CPathString(LPCWSTR lpch, int nLength) : CString(lpch, nLength) {}
	CPathString(const unsigned char* psz) : CString(psz) {}

// Operations
	inline const CString& operator=(const CString& stringSrc)
		{ return CString::operator=(stringSrc); }
	inline const CString& operator=(TCHAR ch)
		{ return CString::operator=(ch); }
#ifdef _UNICODE
	inline const CString& operator=(char ch)
		{ return CString::operator=(ch); }
#endif
	inline const CString& operator=(LPCSTR lpsz)
		{ return CString::operator=(lpsz); }
	inline const CString& operator=(LPCWSTR lpsz)
		{ return CString::operator=(lpsz); }
	inline const CString& operator=(const unsigned char* psz)
		{ return CString::operator=(psz); }

	VOID AddBackslash()
	{
		INT len = GetLength();
		if (len > 0)
		{
			len--;
			if (GetAt(len) != (TCHAR)_T('\\'))
			{
				operator+=((TCHAR)_T('\\'));
			}
		}
	}

	VOID RemoveBackslash()
	{
		INT len = GetLength();
		if (len > 0)
		{
			len--;
			if (GetAt(len) == (TCHAR)_T('\\'))
			{
				Delete(len, 1);
			}
		}
	}

	VOID QuoteSpaces()
	{
		INT len = GetLength();
		if (len > 0)
		{
			if (GetAt(0) != _T('"') && GetAt(len - 1)  != _T('"'))
			{
				if (Find(_T(' ')) > 0)
				{
					Insert(len, _T('"'));
					Insert(0, _T('"'));
				}
			}
		}
	}

	VOID UnquoteSpaces()
	{
		INT len = GetLength();
		if (len > 0)
		{
			len--;
			if (GetAt(0) == _T('"') && GetAt(len)  == _T('"'))
			{
				Delete(len, 1);
				Delete(0, 1);
			}
		}
	}

	BOOL FileExists()
	{
		return PathFileExists(m_pchData);
	}

	BOOL SystemFileExists()
	{
		return SystemFileExists(m_pchData);
	}
	
	BOOL IsDirectory()
	{
		return PathIsDirectory(m_pchData);
	}

	VOID RemoveFileSpec()
	{
		INT len = GetLength();
		INT pos = ReverseFind((TCHAR)_T('\\'));
		
		if (pos > 0)
		{
			Delete(pos, len - pos);
		}
	}
	
	//GetLongPathName;
	//GetModuleFileName;
	//GetTempPath;
	//SHGetSpecialFolderPath;

	/////////////////////////////////////////////////////////////////////////////
	// 関数名 : SystemFileExists
	// 説明   : パスが通った場所に指定されたファイルが存在するか調べる
	// 戻り値 : BOOL : TRUE:存在する、FALSE:存在しないorエラー
	// 引数   : LPCTSTR file : 調べるファイル名
	static BOOL SystemFileExists(LPCTSTR lpszFileName)
	{
		return SearchPath(NULL, lpszFileName, NULL, 0, NULL, NULL);
	}
};

#endif // !_WTL_NO_CSTRING

}; //namespace WTL

#endif // __S2LIBMISC_H__
