#include "stdafx.h"
#include "aboutdlg.h"
#include "InventoryView.h"
#include "MainFrm.h"
#include "shared/aopackets.h"
#include "InjectionSupport.h"
#include "ntray.h"
#include "Version.h"
#include "GuiServices.h"
#include "ContainerManager.h"
#include "Desktop.h"

#include <boost/filesystem.hpp>
#include <Shared/UnicodeSupport.h>
#include <Shared/FileUtils.h>
#include <Shlobj.h>
#include <ItemAssistantCore/AOManager.h>


using namespace aoia;
using namespace Parsers;


// Delay loaded function definition
typedef  BOOL (WINAPI *ChangeWindowMessageFilterFunc)(UINT message, DWORD dwFlag);
#define MSGFLT_ADD 1
#define MSGFLT_REMOVE 2


CMainFrame::CMainFrame(aoia::ISettingsPtr settings)
    : m_settings(settings)
{
    assert(settings);

    try
    {
        m_windowRect.left = boost::lexical_cast<int>(m_settings->getValue(_T("Window.Left")));
        m_windowRect.top = boost::lexical_cast<int>(m_settings->getValue(_T("Window.Top")));
        m_windowRect.right = m_windowRect.left + boost::lexical_cast<unsigned int>(m_settings->getValue(_T("Window.Width")));
        m_windowRect.bottom = m_windowRect.top + boost::lexical_cast<unsigned int>(m_settings->getValue(_T("Window.Height")));

        Desktop desktop;
        if (!desktop.IntersectsRect(m_windowRect))
        {
            LOG("Previous window not inside current desktop. Using default.");
            m_windowRect.SetRectEmpty();
        }
    }
    catch(boost::bad_lexical_cast &/*e*/)
    {
        LOG("Could not load previous window position and size. Using default.");
        m_windowRect.SetRectEmpty();
    }
}


BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
    IPluginView *plugin = m_tabbedChildWindow->GetActivePluginView();
    if (plugin != NULL)
    {
        if (plugin->PreTranslateMsg(pMsg))
        {
            return TRUE;
        }
    }

    if(CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
    {
        return TRUE;
    }

    return FALSE;
}


BOOL CMainFrame::OnIdle()
{
    UIUpdateToolBar();
    return FALSE;
}


LRESULT CMainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    m_minimized = false;

    if (!m_windowRect.IsRectEmpty())
    {
        this->SetWindowPos(NULL, &m_windowRect, SWP_NOZORDER);
    }

    // create command bar window
    HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
    // attach menu
    m_CmdBar.AttachMenu(GetMenu());
    // load command bar images
    m_CmdBar.LoadImages(IDR_MAINFRAME);
    // remove old menu
    SetMenu(NULL);

    //HWND hWndToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);

    CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
    AddSimpleReBarBand(hWndCmdBar);
    //AddSimpleReBarBand(hWndToolBar, NULL, TRUE);
    {
        CReBarCtrl rebar(m_hWndToolBar);
        rebar.LockBands(true);
    }

    CreateSimpleStatusBar();

    // Create tray icon
    HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
        IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
    m_trayIcon = boost::shared_ptr<CTrayNotifyIcon>(new CTrayNotifyIcon());
    m_trayIcon->Create(this, IDR_TRAY_POPUP, _T("AO Item Assistant +"), hIconSmall, WM_TRAYICON);

    // Create common services
    m_containerManager.reset(new ContainerManager(g_DBManager.GetDatabase()));
    m_guiServices.reset(new aoia::GuiServices(m_trayIcon));

    DWORD style = WS_CHILD | /*WS_VISIBLE |*/ WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

    m_tabbedChildWindow.reset(new TabFrame(g_DBManager.GetDatabase(), m_containerManager, m_guiServices, m_settings));
    m_tabbedChildWindow->SetToolBarPanel(m_hWndToolBar);
    m_tabbedChildWindow->SetStatusBar(m_hWndStatusBar);
    m_tabbedChildWindow->SetTabStyles(CTCS_TOOLTIPS | CTCS_DRAGREARRANGE);
    m_hWndClient = m_tabbedChildWindow->Create(m_hWnd, rcDefault, 0, style | WS_VISIBLE);
    m_tabbedChildWindow->SetToolbarVisibility(true);
    
    UISetCheck(ID_VIEW_STATUS_BAR, true);
    UISetCheck(ID_VIEW_TOOLBAR, true);

	bool bAutoPrefs = AOManager::instance().getAutoPrefs();
	UISetCheck(ID_OPTIONS_AUTOMATICPREFS, bAutoPrefs);
	UISetCheck(ID_OPTIONS_MANUALPREFS, !bAutoPrefs);

	bool bShowCreds = AOManager::instance().getShowCreds();
	UISetCheck(ID_OPTIONS_SHOWCREDS, bShowCreds);
	
	bool bMinToTaskbar = AOManager::instance().getMinToTaskbar();
	UISetCheck(ID_OPTIONS_MINIMISETOTASKBAR, bMinToTaskbar);


    UpdateLayout();

    // register object for message filtering and idle updates
    CMessageLoop* pLoop = _Module.GetMessageLoop();
    ATLASSERT(pLoop != NULL);
    pLoop->AddMessageFilter(this);
    pLoop->AddIdleHandler(this);

    // To allow incoming messages from lower level applications like the AO Client we need to add the
    // message we use to the exception list.
    // http://blogs.msdn.com/vishalsi/archive/2006/11/30/what-is-user-interface-privilege-isolation-uipi-on-vista.aspx
    // It is however not available in all versions of windows, so we need to check for it existance 
    // before we try to call it.
    {
        ChangeWindowMessageFilterFunc f = (ChangeWindowMessageFilterFunc)GetProcAddress(LoadLibrary(_T("user32.dll")), "ChangeWindowMessageFilter");
        if (f) {
            (f)(WM_COPYDATA, MSGFLT_ADD);
        }
    }

    // This is the injection timer. We delay 1 sec initially so the GUI has time to be set up completely.
    SetTimer(1, 1000);

    return 0;
}


LRESULT CMainFrame::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    LOG("OnFileExit()");
    if (!m_minimized)
    {
        LOG("Recording window rectangle.");
        GetWindowRect(&m_windowRect);
    }
    if (m_trayIcon)
    {
        m_trayIcon->RemoveIcon();
        m_trayIcon.reset();
    }

    if (!m_windowRect.IsRectEmpty())
    {
        LOG("Storing window rectangle.");
        m_settings->setValue(_T("Window.Left"), STREAM2STR(m_windowRect.left));
        m_settings->setValue(_T("Window.Top"), STREAM2STR(m_windowRect.top));
        m_settings->setValue(_T("Window.Width"), STREAM2STR(m_windowRect.Width()));
        m_settings->setValue(_T("Window.Height"), STREAM2STR(m_windowRect.Height()));
    }

    LOG("Posting close command.");
    PostMessage(WM_CLOSE);
    return 0;
}


LRESULT CMainFrame::OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    static bool bVisible = true;	// initially state
    bVisible = !bVisible;
    m_tabbedChildWindow->SetToolbarVisibility(bVisible);
    UISetCheck(ID_VIEW_TOOLBAR, bVisible);
    UpdateLayout();
    return 0;
}


LRESULT CMainFrame::OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    BOOL bVisible = !::IsWindowVisible(m_hWndStatusBar);
    ::ShowWindow(m_hWndStatusBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
    UISetCheck(ID_VIEW_STATUS_BAR, bVisible);
    UpdateLayout();
    return 0;
}


LRESULT CMainFrame::OnOptionsManual(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	std::tstring path = AOManager::instance().getAOPrefsFolderManual();
	bool bAutoPrefs = AOManager::instance().getAutoPrefs();
	UISetCheck(ID_OPTIONS_AUTOMATICPREFS, bAutoPrefs);
	UISetCheck(ID_OPTIONS_MANUALPREFS, !bAutoPrefs);
    UpdateLayout();
    return 0;
}


LRESULT CMainFrame::OnOptionsAuto(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	bool bAutoPrefs = AOManager::instance().getAutoPrefs();
	if (!bAutoPrefs)
	{
		AOManager::instance().setAutoPrefs(true);
		UISetCheck(ID_OPTIONS_AUTOMATICPREFS, true);
		UISetCheck(ID_OPTIONS_MANUALPREFS, false);
		UpdateLayout();
	}
    return 0;
}


LRESULT CMainFrame::OnOptionsShowCreds(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	bool bShowCreds = AOManager::instance().getShowCreds();
	bShowCreds = !bShowCreds;
	AOManager::instance().setShowCreds(bShowCreds);
	UISetCheck(ID_OPTIONS_SHOWCREDS, bShowCreds);
    return 0;
}


LRESULT CMainFrame::OnOptionsMinToTaskbar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	bool bMinToTaskbar = AOManager::instance().getMinToTaskbar();
	bMinToTaskbar = !bMinToTaskbar;
	AOManager::instance().setMinToTaskbar(bMinToTaskbar);
	UISetCheck(ID_OPTIONS_MINIMISETOTASKBAR, bMinToTaskbar);
    return 0;
}


LRESULT CMainFrame::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    static bool isInside = false;

    if (!isInside) {
        isInside = true;
        CAboutDlg dlg;
        dlg.DoModal();
        isInside = false;
    }

    return 0;
}


LRESULT CMainFrame::OnTrayShow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    if (m_minimized)
	{
        SendMessage(WM_SYSCOMMAND, SC_RESTORE, NULL);
    }
    else
	{
        SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, NULL);
    }
    return 0;
}


LRESULT CMainFrame::OnHelp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    m_guiServices->ShowHelp(_T(""));
    return 0;
}


LRESULT CMainFrame::OnCheckForUpdates(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    m_guiServices->OpenURL(STREAM2STR(_T("http://www.ao-universe.com/index.php?id=29&mid=1&site=AO-Universe%2FMultimedia%2FDownloads%2FTools%2F")));
    return 0;
}


LRESULT CMainFrame::OnSupportForum(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    m_guiServices->OpenURL(_T("http://apps.sourceforge.net/phpbb/aoia/"));
    return 0;
}


LRESULT CMainFrame::OnAOMessage(HWND wnd, PCOPYDATASTRUCT pData)
{
    char* datablock = (char*)(pData->lpData);
    unsigned int datasize = pData->cbData;

    if (pData->dwData == 1) {
        AOMessageBase msg(datablock, datasize);
        m_tabbedChildWindow->OnAOServerMessage(msg);
    }
    else if (pData->dwData == 2) {
        AOClientMessageBase msg(datablock, datasize);
        m_tabbedChildWindow->OnAOClientMessage(msg);
    }

    return 0;
}


LRESULT CMainFrame::OnTimer(UINT wParam)
{
    static bool first_injection_try = true;

    if (wParam == 1)
    {
        bool injected = Inject();
        if (injected && first_injection_try) {
            // Show warning about AO being started before AOIA
            m_guiServices->ShowTrayIconBalloon(_T("Anarchy Online was started before Item Assistant +.\nThe database might be out of sync for the current toon."));
        }
        first_injection_try = false;
        SetTimer(1, 10000);
    }
    return 0;
}


void CMainFrame::OnSysCommand(UINT wParam, CPoint mousePos)
{
    switch (wParam)
	{
    case SC_CLOSE:
    case SC_MINIMIZE:
    {
        GetWindowRect(&m_windowRect);
        DefWindowProc(WM_SYSCOMMAND, SC_MINIMIZE, NULL);
        if (!AOManager::instance().getMinToTaskbar())
		{
			ShowWindow(SW_HIDE); // Hides the task bar button.
		}
        m_minimized = true;
    }
    break;
    case SC_RESTORE:
    {
        if (m_minimized)
		{
            ShowWindow(SW_SHOW);
            m_minimized = false;
        }
        DefWindowProc(WM_SYSCOMMAND, wParam, NULL);
    }
    break;
    default:
        SetMsgHandled(FALSE);
        break;
    }
}


bool CMainFrame::Inject()
{
    HWND AOWnd = FindWindow(_T("Anarchy client"), NULL);

    if(!AOWnd)
    {
        Logger::instance().log(_T("Could not locate Anarchy Online window."));
        return false;
    }

    // Get process id
    DWORD AOProcessId;
    GetWindowThreadProcessId( AOWnd, &AOProcessId );

    TCHAR CurrDir[MAX_PATH];
    GetCurrentDirectory( MAX_PATH, CurrDir );

    // Inject the dll into client process
    std::tstringstream temp;
#ifdef DEBUG
	temp << CurrDir << _T("\\..\\Debug\\ItemAssistantHook.dll");
#else
    temp << CurrDir << _T("\\ItemAssistantHook.dll");
#endif
	Logger::instance().log(temp.str());
    return InjectDLL(AOProcessId, temp.str());
}


LRESULT CMainFrame::OnTrayIcon(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
    if (m_trayIcon) {
        return m_trayIcon->OnTrayNotification(wParam, lParam);
    }
    return 0;
}
