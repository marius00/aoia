// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define STRICT
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _WTL_USE_CSTRING

#include "resource.h"

#define _WIN32_WINNT 0x0501
#define WINVER 0x0501
#include <Windows.h>

#include <atlbase.h>        // Base ATL classes
#include <atlapp.h>

extern CAppModule _Module;  // Global _Module

#include <atlwin.h>         // ATL windowing classes
#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlw.h>
#include <atlmisc.h>
#include <atlcrack.h>
//#include <atltypes.h>

#include <atlcoll.h>
#include "TabbingFramework/atlgdix.h"
#include "TabbingFramework/CustomTabCtrl.h"
#include "TabbingFramework/DotNetTabCtrl.h"
#include "TabbingFramework/TabbedMDI.h"

#include <stdio.h>

#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>

#include <shared/UnicodeSupport.h>

#include "sqlite3.h"

//#include "DBManager.h"
class DBManager;
extern DBManager g_DBManager; // Global DB manager

#include <shared/Singleton.h>

class Application;
typedef Shared::Singleton<Application> App;

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <ItemAssistantCore/Logger.h>
