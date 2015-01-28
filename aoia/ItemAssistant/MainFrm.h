#ifndef MAINFRM_H
#define MAINFRM_H

#include <settings/ISettings.h>
#include <PluginSDK/IPluginView.h>
#include "TabFrame.h"
#include <vector>
#include <shellapi.h>

// Forward declarations
class CTrayNotifyIcon;
namespace aoia {
    class GuiServices;
}


class CMainFrame
    : public CFrameWindowImpl<CMainFrame>
    , public CUpdateUI<CMainFrame>
    , public CMessageFilter
    , public CIdleHandler
{
public:
    CMainFrame(aoia::ISettingsPtr settings);

    DECLARE_FRAME_WND_CLASS(_T("ItemAssistantWindowClass"), IDR_MAINFRAME)

    enum {
        WM_TRAYICON = WM_USER + 1
    };

    virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual BOOL OnIdle();

    BEGIN_UPDATE_UI_MAP(CMainFrame)
        UPDATE_ELEMENT(ID_VIEW_TOOLBAR, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_OPTIONS_AUTOMATICPREFS, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_OPTIONS_MANUALPREFS, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_OPTIONS_SHOWCREDS, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_OPTIONS_MINIMISETOTASKBAR, UPDUI_MENUPOPUP)
    END_UPDATE_UI_MAP()

    BEGIN_MSG_MAP_EX(CMainFrame)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
        COMMAND_ID_HANDLER(ID_VIEW_TOOLBAR, OnViewToolBar)
        COMMAND_ID_HANDLER(ID_VIEW_STATUS_BAR, OnViewStatusBar)
		COMMAND_ID_HANDLER(ID_OPTIONS_AUTOMATICPREFS, OnOptionsAuto)
		COMMAND_ID_HANDLER(ID_OPTIONS_MANUALPREFS, OnOptionsManual)
		COMMAND_ID_HANDLER(ID_OPTIONS_SHOWCREDS, OnOptionsShowCreds)
		COMMAND_ID_HANDLER(ID_OPTIONS_MINIMISETOTASKBAR, OnOptionsMinToTaskbar)
        COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
        COMMAND_ID_HANDLER(ID_TRAY_SHOW, OnTrayShow)
        COMMAND_ID_HANDLER(ID_HELP_INDEX, OnHelp)
        COMMAND_ID_HANDLER(ID_HELP_CHECKFORUPDATES, OnCheckForUpdates)
        COMMAND_ID_HANDLER(ID_HELP_SUPPORTFORUM, OnSupportForum)
        MSG_WM_COPYDATA(OnAOMessage)
        MSG_WM_TIMER(OnTimer)
        MSG_WM_ERASEBKGND(OnEraseBkgnd)
        MSG_WM_SYSCOMMAND(OnSysCommand)
        MESSAGE_HANDLER(WM_TRAYICON, OnTrayIcon)
        CHAIN_COMMANDS_MEMBER( (*m_tabbedChildWindow) )
        CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
        CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
        REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

    LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnOptionsManual(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnOptionsAuto(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnOptionsShowCreds(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnOptionsMinToTaskbar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnTrayShow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnHelp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnCheckForUpdates(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnSupportForum(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnAOMessage(HWND wnd, PCOPYDATASTRUCT pData);
    LRESULT OnTimer(UINT wParam);
    LRESULT OnEraseBkgnd(HDC dc) { return 1; }
    LRESULT OnTrayIcon(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
    void OnSysCommand(UINT wParam, CPoint mousePos);

    //boost::shared_ptr<CTrayNotifyIcon> GetTrayIcon() const { return m_trayIcon; }

private:
    boost::shared_ptr<CTrayNotifyIcon> m_trayIcon;
    boost::shared_ptr<TabFrame> m_tabbedChildWindow;

    bool Inject();

    CCommandBarCtrl m_CmdBar;
    bool m_minimized;
    CRect m_windowRect;
    boost::shared_ptr<aoia::GuiServices> m_guiServices;
    aoia::IContainerManagerPtr m_containerManager;
    aoia::ISettingsPtr m_settings;
};

#endif // MAINFRM_H
