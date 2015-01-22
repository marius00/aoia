// ItemAssistant.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "ItemAssistant.h"
#include "DBManager.h"
#include <shlobj.h>
#include "Application.h"
#include "mdump.h"

#define MAX_LOADSTRING 100


// Global Variables:
CAppModule _Module;
DBManager g_DBManager;
MiniDumper _crashdumper(_T("AOIA"));


int APIENTRY _tWinMain(HINSTANCE hInstance,
                       HINSTANCE hPrevInstance,
                       LPTSTR    lpCmdLine,
                       int       nCmdShow)
{
    HRESULT hRes = ::CoInitialize(NULL);
    // If you are running on NT 4.0 or higher you can use the following call instead to 
    // make the EXE free threaded. This means that calls come in on a random RPC thread.
    // HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
    ATLASSERT(SUCCEEDED(hRes));

    // this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
    ::DefWindowProc(NULL, 0, 0, 0L);

    AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);	// add flags to support other controls

    hRes = _Module.Init(NULL, hInstance, &LIBID_ATLLib);
    ATLASSERT(SUCCEEDED(hRes));

    AtlAxWinInit();

    int nRet = 0;

    try
    {
        if (!App::Instance()->init( lpCmdLine )) {
            MessageBox( NULL, _T("Unable to start the application from the current location!"), 
                _T("Error - AO Item Assistant +"), MB_OK | MB_ICONERROR);
            return 0;
        }

        nRet = App::Instance()->run(lpCmdLine, nCmdShow);

        App::Instance()->destroy();
    }
    catch (std::exception& e)
    {
        Logger::instance().log(_T("Unhandled exception caught:"));
        Logger::instance().log(from_ascii_copy(e.what()));
    }
    catch (...)
    {
        Logger::instance().log(_T("Unhandled exception caught."));
    }

    _Module.Term();
    ::CoUninitialize();

    return nRet;
}
