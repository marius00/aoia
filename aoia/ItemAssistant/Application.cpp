#include "Stdafx.h"
#include "Application.h"
#include "Version.h"
#include <ItemAssistantCore/PluginManager.h>
#include <ItemAssistantCore/AOManager.h>
#include "MainFrm.h"


Application::Application()
{
    m_settings.reset(new aoia::SettingsManager());
    AOManager::instance().SettingsManager(m_settings);
}


Application::~Application()
{
}


bool Application::init(std::tstring const& cmdLine)
{
    TCHAR tmpfolder[MAX_PATH];
    GetTempPath(MAX_PATH, tmpfolder);
    std::tstring logfile(tmpfolder);
    logfile += _T("aoia.log");
	//logfile = _T("C:\\Users\\Andrew\\Desktop\\aoia.log");

    // Read stored settings from file.
    m_settings->readSettings(_T("ItemAssistant.conf"));

    // Check to see if logging should be enabled
    Logger::instance().init(logfile, g_versionNumber);

    LOG(_T("Using AO Folder at: ") << AOManager::instance().getAOFolder());
    LOG(_T("Using AO Prefs at: ") << AOManager::instance().getAOPrefsFolder());

    std::tstring dbfile;
    std::tstring::size_type argPos = cmdLine.find(_T("-db"));
    if (argPos != std::tstring::npos)
    {
        dbfile = cmdLine.substr(argPos + 4, cmdLine.find_first_of(_T(" "), argPos + 4) - argPos - 4);
    }

    if (!g_DBManager.init(dbfile))
    {
        LOG(_T("Failed to initialize DB Manager. Aborting!"));
        return false;
    }

    PluginManager::instance().AddLibraries(_T("./Plugins"));

    return true;
}


void Application::destroy()
{
    m_mainWindow.reset();

    // Save user settings.
    m_settings->writeSettings(_T("ItemAssistant.conf"));

    g_DBManager.Lock();
    g_DBManager.destroy();
    g_DBManager.UnLock();

    Logger::instance().destroy();
}


int Application::run(LPTSTR /*lpstrCmdLine*/, int nCmdShow)
{
    CMessageLoop theLoop;
    _Module.AddMessageLoop(&theLoop);

    // Create the GUI
    m_mainWindow.reset(new CMainFrame(m_settings));
    if (m_mainWindow->CreateEx() == NULL)
    {
        assert(false);  // Could not create main window for some reason.
        return 0;
    }
    m_mainWindow->ShowWindow(nCmdShow);

    int nRet = theLoop.Run();

    _Module.RemoveMessageLoop();
    return nRet;
}
