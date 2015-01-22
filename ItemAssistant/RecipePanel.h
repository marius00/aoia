#pragma once

#include <boost/signal.hpp>
#include <shared/IDB.h>
#include <settings/ISettings.h>
#include "RecipeLoader.h"
#include "RecipeManageDlg.h"


class RecipePanel : public CDialogImpl<RecipePanel> {
public:
    enum
    {
        IDD = IDD_RECIPES
    };


    typedef boost::signal<void ()> SettingsChangedSignal;
    typedef boost::signals::connection Connection;

    RecipePanel(sqlite::IDBPtr db, aoia::ISettingsPtr settings);
    virtual ~RecipePanel();

    // Access to properties in the GUI.
    unsigned int getCharId() const;

    BOOL PreTranslateMsg(MSG* pMsg);

    void setProgress(unsigned short percent);
    void updateCharList();

    BEGIN_MSG_MAP(FilterView)
        MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
        MESSAGE_HANDLER(WM_FORWARDMSG, onForwardMsg)
		COMMAND_HANDLER(IDC_CHARACTERCOMBO, CBN_SELCHANGE, onCbnSelchangeCharcombo)
		COMMAND_HANDLER(IDC_RECIPECOMBO, CBN_SELCHANGE, onCbnSelchangeCharcombo)
        COMMAND_HANDLER(IDC_CHARACTERCOMBO, CBN_DROPDOWN, OnCbnDropdown)
		COMMAND_ID_HANDLER(IDC_BUTTONMANAGE, OnManageCmd)
        DEFAULT_REFLECTION_HANDLER()
    END_MSG_MAP()

    void disconnect(Connection subscriber);
    Connection connectSettingsChanged(SettingsChangedSignal::slot_function_type subscriber);


	std::list<unsigned int> getDesiredItemRefs() const;
	std::set<Item> getItemList(const std::list<Item>& itemsInDatabase) const;
	std::tstring getRecipeName() const;
protected:
    void signalSettingsChanged();

    LRESULT onInitDialog(UINT/*uMsg*/, WPARAM/*wParam*/, LPARAM/*lParam*/, BOOL&/*bHandled*/);
    LRESULT onForwardMsg(UINT/*uMsg*/, WPARAM/*wParam*/, LPARAM/*lParam*/, BOOL&/*bHandled*/);
    LRESULT onCbnSelchangeCharcombo(WORD/*wNotifyCode*/, WORD/*wID*/, HWND/*hWndCtl*/, BOOL&/*bHandled*/);
    LRESULT OnCbnDropdown(WORD/*wNotifyCode*/, WORD/*wID*/, HWND/*hWndCtl*/, BOOL&/*bHandled*/);

	LRESULT OnManageCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
    sqlite::IDBPtr m_db;
    aoia::ISettingsPtr m_settings;
    SettingsChangedSignal m_settingsChangedSignal;
	std::tstring m_basequery;
	RecipeLoader m_recipeLoader;
};


