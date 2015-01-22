#ifndef PLAYERSHOPVIEW_H
#define PLAYERSHOPVIEW_H

#include <PluginSDK/ItemAssistView.h>
#include <PluginSDK/IGuiServices.h>
#include <boost/filesystem/path.hpp>
#include "shared/Thread.h"
#include "shared/Mutex.h"
#include <atlsplit.h>
#include "MFTreeView.h"
#include "PsmTreeItems.h"


#define WM_PSM_UPDATE WM_USER+1


class WatchDirectoryThread
    : public Thread
{
public:
    WatchDirectoryThread(HANDLE hWakeupEvent, PlayershopView* owner);
    virtual ~WatchDirectoryThread();

    virtual DWORD ThreadProc();

protected:
    void WatchDirectory(LPTSTR lpDir);

private:
    HANDLE m_hWakeupEvent;
    PlayershopView* m_pOwner;
};


class PlayershopView
    : public ItemAssistView<PlayershopView>
{
    typedef ItemAssistView<PlayershopView> inherited;
public:
    //enum { IDD = IDD_PLAYERSHOP };
    DECLARE_WND_CLASS(NULL)

    PlayershopView(aoia::IGuiServicesPtr gui);
    virtual ~PlayershopView();

    enum
    {
        WM_POSTCREATE = WM_APP + 1,
    };

    enum
    {
        IDW_PSMLISTVIEW = 1,
        IDW_PSMTREEVIEW
    };

    BEGIN_MSG_MAP_EX(PlayershopView)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_SIZE(OnSize)
        MESSAGE_HANDLER(WM_POSTCREATE, OnPostCreate)
        MESSAGE_HANDLER(WM_PSM_UPDATE, OnContentUpdate)
        COMMAND_ID_HANDLER(ID_HELP, OnHelp)
        COMMAND_ID_HANDLER(ID_PAUSE_TOGGLE, OnPause)
    /*      COMMAND_ID_HANDLER(ID_INV_FIND, OnFind)
        COMMAND_ID_HANDLER(ID_INV_FIND_HIDE, OnFindHide)
        COMMAND_ID_HANDLER(ID_INFO, OnInfo)
        NOTIFY_CODE_HANDLER_EX(LVN_COLUMNCLICK, OnColumnClick)
        NOTIFY_CODE_HANDLER_EX(LVN_ITEMACTIVATE, OnItemActivate)
        CHAIN_MSG_MAP(inherited)*/
        REFLECT_NOTIFICATIONS()
        DEFAULT_REFLECTION_HANDLER()
    END_MSG_MAP();

    LRESULT OnCreate(LPCREATESTRUCT createStruct);
    LRESULT OnSize(UINT wParam, CSize newSize);
    LRESULT OnPostCreate(UINT/*uMsg*/, WPARAM/*wParam*/, LPARAM/*lParam*/, BOOL&/*bHandled*/);
    LRESULT OnContentUpdate(UINT/*uMsg*/, WPARAM/*wParam*/, LPARAM/*lParam*/, BOOL&/*bHandled*/);
    LRESULT OnHelp(WORD/*wNotifyCode*/, WORD/*wID*/, HWND/*hWndCtl*/, BOOL&/*bHandled*/);
    LRESULT OnPause(WORD/*wNotifyCode*/, WORD/*wID*/, HWND/*hWndCtl*/, BOOL&/*bHandled*/);
    LRESULT OnColumnClick(LPNMHDR lParam);
    LRESULT OnItemActivate(LPNMHDR lParam);
    void StartMonitoring();
    void StopMonitoring();

    void UpdateListView(std::tstring const& where);
    void UpdateListView(std::vector<std::tstring> v);

    // ItemAssistView overrides
    virtual void OnActive(bool doActivation);

    // Add files to the list of changed log files.
    void PushChangedFile( boost::filesystem::path p );

protected:
    void UpdateLayout(CSize newSize);
    static int CALLBACK CompareStr(LPARAM param1, LPARAM param2, LPARAM sort);
    void ShowSalesBaloon();

private:
    aoia::IGuiServicesPtr m_gui;
    CSplitterWindow m_splitter;
    PsmTreeView m_treeview;
    CListViewCtrl m_listview;

    PlayershopTreeRoot m_treeRoot;
    boost::shared_ptr<WatchDirectoryThread> m_directoryWatch;
    HANDLE m_hWakeupEvent;

    bool m_sortDesc;
    int m_sortColumn;

    std::vector<boost::filesystem::path> m_changedFiles;
    Mutex m_changedFilesMutex;
};


#endif // PLAYERSHOPVIEW_H
