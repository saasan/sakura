// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__4579BCE9_5C37_41A3_8AE0_2CF1664CBB1A__INCLUDED_)
#define AFX_STDAFX_H__4579BCE9_5C37_41A3_8AE0_2CF1664CBB1A__INCLUDED_

// Change these values to use different versions
//#define WINVER		0x0400
//#define _WIN32_WINNT	0x0400
//#define _WIN32_IE	0x0400
//#define _RICHEDIT_VER	0x0100
#define _WIN32_WINNT	0x0501
#define _WIN32_IE		0x0600

#include <atlbase.h>
#include <atlapp.h>

extern CAppModule _Module;

#include <atlwin.h>

#include <atlmisc.h>
#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlw.h>
#include <atlddx.h>
#include <atlgdi.h>
#include <atlctrlx.h>

#include "s2tlmisc.h"

#pragma comment(lib, "shlwapi.lib")
#include <shlwapi.h>
#pragma comment(lib, "comctl32.lib")
#include <commctrl.h>
#pragma comment(lib, "version.lib")
#include <version.h>
#pragma comment(lib, "user32.lib")
#include <winuser.h>

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_STDAFX_H__4579BCE9_5C37_41A3_8AE0_2CF1664CBB1A__INCLUDED_)
