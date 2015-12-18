// sakura.cpp : main source file for sakura.exe
#pragma warning(disable: 4786)

#include <string>
#include <vector>
#include "stdafx.h"
#include "arguments.h"
#include "sakuraOptions.h"
#include "maindlg.h"

using namespace std;

typedef BOOL (WINAPI *PCCL)();

CAppModule _Module;

bool PrivateInit(const CString &cmdLine)
{
	::CMainOption::LoadProfile();

	// CoolControlLibrary
	CMainOption::hCCLDll = LoadLibrary(_T("CCL.DLL"));
	if (CMainOption::hCCLDll)
	{
		PCCL fpInstall = (PCCL)GetProcAddress(CMainOption::hCCLDll, "InstallHook");
		if (fpInstall)
		{
			(*fpInstall)();
		}
	}

	return true;
}

void PrivateTerm()
{
	CMainOption::SaveProfile();

	if (CMainOption::mainStyles & OPTION_MAIN_OPTION)
	{
		CString key, privateKey;
		CRegKey reg;

		key.LoadString(IDR_MAINFRAME);
		privateKey.LoadString(IDS_REG_PRIVATEKEY);
		key = privateKey + key;

		TCHAR dir[MAX_PATH];
		CString param = _T("/E \"");

		GetModuleFileName(NULL, dir, sizeof(dir)/sizeof(TCHAR));
		PathRemoveExtension(dir);

		param += dir + CString(_T(".reg\" HKEY_CURRENT_USER\\")) + key;

		ShellExecute(NULL, _T("open"), _T("regedit.exe"), param, NULL, SW_HIDE);
	}

	// CoolControlLibrary Hookâèú
	if (CMainOption::hCCLDll)
	{
		PCCL fpUninstall = (PCCL)GetProcAddress(CMainOption::hCCLDll, "UninstallHook");
		if (fpUninstall)
		{
			(*fpUninstall)();
		}
		FreeLibrary(CMainOption::hCCLDll);
	}
}

int Run(LPTSTR lpstrCmdLine = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	CMainDlg dlgMain;

	if(dlgMain.Create(NULL) == NULL)
	{
		ATLTRACE(_T("Main dialog creation failed!\n"));
		return 0;
	}

	// à¯êîèàóù
	s2lib::arguments<TCHAR> arg(lpstrCmdLine);
	s2lib::arguments<TCHAR>::iterator it = arg.begin();
	s2lib::arguments<TCHAR>::iterator itend = arg.end();

	while (it != itend)
	{
		dlgMain.listfile.AddFile(it->c_str());
		it++;
	}

#if 0
	dlgMain.ShowWindow(nCmdShow);
#else
	//AnimateWindow(dlgMain.m_hWnd, 200, AW_ACTIVATE | AW_BLEND);
	//RedrawWindow(dlgMain.m_hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);
	dlgMain.ShowWindow(SW_SHOW);
#endif

	if (dlgMain.listfile.GetCount() > 0)
	{
		dlgMain.listfile.SetHorizontalScrollBar();
		dlgMain.Excute();
	}

	int nRet = theLoop.Run();

	_Module.RemoveMessageLoop();
	return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	HRESULT hRes = ::CoInitialize(NULL);
// If you are running on NT 4.0 or higher you can use the following call instead to 
// make the EXE free threaded. This means that calls come in on a random RPC thread.
//	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(hRes));

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES | ICC_USEREX_CLASSES);	// add flags to support other controls

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	if (!PrivateInit(lpstrCmdLine))
	{
		return 0;
	}

	int nRet = Run(lpstrCmdLine, nCmdShow);

	PrivateTerm();
	_Module.Term();
	::CoUninitialize();

	return nRet;
}
