#ifndef INVENTORYVIEW_H
#define INVENTORYVIEW_H

#include <vector>
#include <atlsplit.h>
#include <boost/signal.hpp>

#include <PluginSDK/IGuiServices.h>
#include <PluginSDK/ItemAssistView.h>
#include <datagrid/DataGridControl.h>
#include <PluginSDK/IContainerManager.h>
#include <shared/aopackets.h>
#include <settings/ISettings.h>

#include "CharacterParserDumper.h"
#include "FindPanel.h"
#include "InfoPanel.h"
#include "InvTreeItems.h"
#include "MFTreeView.h"
#include "TempContainerCache.h"
#include "ItemListDataModel.h"


class InventoryView
    : public ItemAssistView<InventoryView>
{
    typedef ItemAssistView<InventoryView> inherited;
public:
    DECLARE_WND_CLASS(NULL)

    InventoryView(sqlite::IDBPtr db, aoia::IContainerManagerPtr containerManager, aoia::IGuiServicesPtr gui, aoia::ISettingsPtr settings);
    virtual ~InventoryView();

    enum
    {
        WM_POSTCREATE = WM_APP + 1,
    };

    // Id numbers for child windows.
    enum {
        IDW_LISTVIEW = 1,
        IDW_TREEVIEW,
        IDW_FINDVIEW,
        IDW_INFOVIEW
    };

    BEGIN_MSG_MAP_EX(InventoryView)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_SIZE(OnSize)
        //MSG_WM_NOTIFY(OnNotify)
        MESSAGE_HANDLER(WM_POSTCREATE, OnPostCreate)
        COMMAND_ID_HANDLER(ID_INV_FIND_TOGGLE, OnFindToggle)
        COMMAND_ID_HANDLER(ID_INV_FIND, OnFind)
        COMMAND_ID_HANDLER(ID_INV_FIND_HIDE, OnFindHide)
        COMMAND_ID_HANDLER(ID_INFO, OnInfo)
        COMMAND_ID_HANDLER(ID_HELP, OnHelp)
        COMMAND_ID_HANDLER(ID_COPY_ITEMNAME, OnCopyItemName)
        COMMAND_ID_HANDLER(ID_COPY_AUNO_ITEMREF, OnCopyItemRef)
        COMMAND_ID_HANDLER(ID_COPY_AUNO_ITEMREF_VBB, OnCopyItemRef)
        COMMAND_ID_HANDLER(ID_COPY_JAYDEE_ITEMREF, OnCopyItemRef)
        COMMAND_ID_HANDLER(ID_COPY_JAYDEE_ITEMREF_VBB, OnCopyItemRef)
        COMMAND_ID_HANDLER(ID_COPY_AO_ITEMREF, OnCopyItemRef)
        COMMAND_ID_HANDLER(ID_VIEW_ITEMSTATS_AUNO, OnShowItemRef)
        COMMAND_ID_HANDLER(ID_VIEW_ITEMSTATS_JAYDEE, OnShowItemRef)
        COMMAND_ID_HANDLER(ID_EXPORTTOCSV_AUNO, OnExportToCSV)
        COMMAND_ID_HANDLER(ID_EXPORTTOCSV_JAYDEE, OnExportToCSV)
        COMMAND_ID_HANDLER(ID_COPY_XYPHOS_ITEMREF, OnCopyItemRef)
        COMMAND_ID_HANDLER(ID_COPY_XYPHOS_ITEMREF_VBB, OnCopyItemRef)
        COMMAND_ID_HANDLER(ID_VIEW_ITEMSTATS_XYPHOS, OnShowItemRef)
        COMMAND_ID_HANDLER(ID_EXPORTTOCSV_XYPHOS, OnExportToCSV)
        COMMAND_ID_HANDLER(ID_RECORD_STATS_TOGGLE, OnRecordStatsToggle)
        COMMAND_ID_HANDLER(ID_DELETE, OnDelete)
		NOTIFY_CODE_HANDLER_EX(LVN_COLUMNCLICK, OnColumnClick)
		NOTIFY_CODE_HANDLER_EX(NM_DBLCLK, onDoubleClick)
        //NOTIFY_CODE_HANDLER_EX(LVN_ITEMACTIVATE, OnItemActivate)
        NOTIFY_CODE_HANDLER_EX(LVN_ITEMCHANGED, OnItemChanged)
        NOTIFY_HANDLER_EX(IDW_LISTVIEW, NM_RCLICK, OnItemContextMenu)
        CHAIN_MSG_MAP(inherited)
        REFLECT_NOTIFICATIONS()
        DEFAULT_REFLECTION_HANDLER()
    END_MSG_MAP()

    LRESULT OnCreate(LPCREATESTRUCT createStruct);
    LRESULT OnSize(UINT wParam, CSize newSize);
    LRESULT OnPostCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnFind(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnFindToggle(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnFindHide(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnInfo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnHelp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnColumnClick(LPNMHDR lParam);
	LRESULT onDoubleClick(LPNMHDR lParam);
    LRESULT OnItemChanged(LPNMHDR lParam);
    LRESULT OnItemContextMenu(LPNMHDR lParam);
    LRESULT OnCopyItemName(WORD FromAccelerator, WORD CommandId, HWND hWndCtrl, BOOL& bHandled);
    LRESULT OnCopyItemRef(WORD FromAccelerator, WORD CommandId, HWND hWndCtrl, BOOL& bHandled);
    LRESULT OnShowItemRef(WORD FromAccelerator, WORD CommandId, HWND hWndCtrl, BOOL& bHandled);
    LRESULT OnExportToCSV(WORD FromAccelerator, WORD CommandId, HWND hWndCtrl, BOOL& bHandled);
    LRESULT OnRecordStatsToggle(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnDelete(WORD FromAccelerator, WORD CommandId, HWND hWndCtrl, BOOL& bHandled);

	virtual void OnAOClientMessage(Parsers::AOClientMessageBase &msg);
    virtual void OnAOServerMessage(Parsers::AOMessageBase &msg);
    virtual bool PreTranslateMsg(MSG* pMsg);
	

    void HideFindWindow();
    void UpdateListView(std::tstring const& where);
    void exportToCSV(std::tstring const& where);
    void OnSelectionChanged();

    enum ItemServer {
        SERVER_AUNO = 1,
        SERVER_JAYDEE,
        SERVER_AO,
        SERVER_XYPHOS,
    };

    static std::tstring GetServerItemURLTemplate( ItemServer server );

protected:
    enum ExportFormat {
        FORMAT_HTML = 1,
        FORMAT_VBB,
        FORMAT_AO,
        FORMAT_CSV,
    };

    std::tstring GetContainerNameForItem( OwnedItemInfoPtr pItemInfo ) const;
    //void AddItemToView(Native::DbKey const& key, Native::DbItem const& item);
    void AddToTreeView(unsigned int charId, unsigned int contId);
    void CleanupDB(unsigned int charid);
    void UpdateLayout(CSize newSize);
    std::vector<std::tstring> GetAccountNames();
    std::tstring ensureEncapsulation(std::tstring const& field);
    void UpdateStatusText();

    // Data model slots
    void onAllItemsUpdated();
    void onItemAdded(unsigned int index);
    void onItemRemoved(unsigned int index);
    void onAllItemsRemoved();

    bool SetClipboardText(std::tstring const& text);
    void GetSelectedItemIds( std::set<unsigned int> &ids );

private:
    sqlite::IDBPtr m_db;
    aoia::IContainerManagerPtr m_containerManager;
    aoia::IGuiServicesPtr m_gui;
    aoia::ISettingsPtr m_settings;
    aoia::inv::TempContainerCachePtr m_tempContainers;

    CharacterParserDumper m_characterParserDumper;
    bool m_enableCharacterParserDumper;

    CSplitterWindow   m_splitter;
    MFTreeView        m_treeview;
    FindView          m_findview;
    InfoView          m_infoview;
    aoia::DataGridControlPtr m_datagrid;

    CTreeItem   m_inventory;
    CTreeItem   m_bank;
    CTreeItem   m_unknown;

    CAccelerator m_accelerators;

    boost::shared_ptr<AccountRootTreeViewItem> m_rootNode;

    bool  m_sortDesc;
    int   m_sortColumn;

	unsigned int GetFromContainerId(unsigned int charId, unsigned short fromType, unsigned short fromSlotId);

    boost::signals::connection m_addSignalConnection;
    boost::signals::connection m_removeSignalConnection;
    boost::signals::connection m_clearSignalConnection;
    boost::signals::connection m_updateSignalConnection;

	aoia::ItemListDataModelPtr m_datagridmodel;

	struct CharacterPosition {
		int zoneid;
		float x;
		float z;
		DWORD lastSeen;
		std::tstring characterName;
	};
	std::map<unsigned int, CharacterPosition> m_characterPositionMap;
	void WriteWaypointsScript();
	DWORD m_lastUpdateCharpos;

	std::set<unsigned int> m_zones_sl;
	std::set<unsigned int> m_zones_rk;
};


#endif // INVENTORYVIEW_H
