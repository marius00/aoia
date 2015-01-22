#ifndef IDENTIFYVIEW_H
#define IDENTIFYVIEW_H

#include <PluginSDK/ItemAssistView.h>
#include <PluginSDK/IGuiServices.h>
#include "datagrid/DataGridControl.h"
#include "IdentifyListDataModel.h"
#include <atlsplit.h>
#include <Shared/IDB.h>
#include <PluginSDK/IContainerManager.h>


class IdentifyView
    : public ItemAssistView<IdentifyView>
    , public WTL::CDialogResize<IdentifyView>
{
    typedef ItemAssistView<IdentifyView> inherited;

public:
    DECLARE_WND_CLASS(NULL)

    IdentifyView(sqlite::IDBPtr db, aoia::IContainerManagerPtr containerManager, aoia::IGuiServicesPtr gui);
    virtual ~IdentifyView();

    BEGIN_MSG_MAP_EX(IdentifyView)
        MSG_WM_CREATE(onCreate)
        NOTIFY_CODE_HANDLER_EX(LVN_ITEMCHANGED, onListItemChanged)
        NOTIFY_CODE_HANDLER_EX(LVN_ODSTATECHANGED, onListItemStateChanged)
        NOTIFY_CODE_HANDLER_EX(LVN_COLUMNCLICK, onColumnClick)
        COMMAND_ID_HANDLER(ID_HELP, OnHelp)
        CHAIN_MSG_MAP(inherited)
        CHAIN_MSG_MAP(WTL::CDialogResize<IdentifyView>)
        REFLECT_NOTIFICATIONS()
        DEFAULT_REFLECTION_HANDLER()
    END_MSG_MAP()

    enum ChildIDs
    {
        IDW_SPLITTER = 1,
        IDW_IDENTLIST,
        IDW_DATAGRID,
    };

    BEGIN_DLGRESIZE_MAP(IdentifyView)
        DLGRESIZE_CONTROL(IDW_SPLITTER, DLSZ_SIZE_X | DLSZ_SIZE_Y)
    END_DLGRESIZE_MAP()

protected:
    LRESULT onCreate(LPCREATESTRUCT createStruct);
    LRESULT onListItemChanged(LPNMHDR lParam);
    LRESULT onListItemStateChanged(LPNMHDR lParam);
    LRESULT onColumnClick(LPNMHDR lParam);
    LRESULT OnHelp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
    sqlite::IDBPtr m_db;
    aoia::IContainerManagerPtr m_containerManager;
    aoia::IGuiServicesPtr m_gui;
    WTL::CSplitterWindow m_splitter;
    aoia::IdentifyListDataModelPtr m_identifyListModel;
    aoia::DataGridControlPtr m_identifyList;
    aoia::DataGridControlPtr m_datagrid;
    int m_sortColumn;
    bool m_sortAscending;
};

#endif // IDENTIFYVIEW_H
