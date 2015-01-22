#ifndef FINDPANEL_H
#define FINDPANEL_H

#include <shared/IDB.h>
#include <settings/ISettings.h>


// Forward declarations
class InventoryView;

class FindView
    : public CDialogImpl<FindView>
    , public CDialogResize<FindView>
{
public:
    enum
    {
        IDD = IDD_INV_FIND
    };

    FindView(sqlite::IDBPtr db, aoia::ISettingsPtr settings);

    void SetParent(InventoryView* parent);
    BOOL PreTranslateMsg(MSG* pMsg);

    BEGIN_MSG_MAP(FindView)
        MSG_WM_TIMER(OnTimer)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_FORWARDMSG, OnForwardMsg)
        COMMAND_HANDLER(IDC_ITEMTEXT, EN_CHANGE, OnEnChangeItemtext)
        COMMAND_HANDLER(IDC_QLMIN, EN_CHANGE, OnEnChangeItemtext)
        COMMAND_HANDLER(IDC_QLMAX, EN_CHANGE, OnEnChangeItemtext)
        COMMAND_HANDLER(IDC_CHARCOMBO, CBN_SELCHANGE, OnCbnSelChangeCharcombo)
        COMMAND_HANDLER(IDC_CHARCOMBO, CBN_DROPDOWN, OnCbnDropdown)
        CHAIN_MSG_MAP(CDialogResize<FindView>)
        DEFAULT_REFLECTION_HANDLER()
    END_MSG_MAP()

    BEGIN_DLGRESIZE_MAP(FindView)
        DLGRESIZE_CONTROL(IDC_ITEMTEXT, DLSZ_SIZE_X)
        DLGRESIZE_CONTROL(IDC_STATIC_TOON, DLSZ_MOVE_X)
        DLGRESIZE_CONTROL(IDC_CHARCOMBO, DLSZ_MOVE_X)
    END_DLGRESIZE_MAP()

    LRESULT OnInitDialog(UINT/*uMsg*/, WPARAM/*wParam*/, LPARAM/*lParam*/, BOOL&/*bHandled*/);
    LRESULT OnForwardMsg(UINT/*uMsg*/, WPARAM/*wParam*/, LPARAM/*lParam*/, BOOL&/*bHandled*/);
    LRESULT OnEnChangeItemtext(WORD/*wNotifyCode*/, WORD/*wID*/, HWND/*hWndCtl*/, BOOL&/*bHandled*/);
    LRESULT OnCbnSelChangeCharcombo(WORD/*wNotifyCode*/, WORD/*wID*/, HWND/*hWndCtl*/, BOOL&/*bHandled*/);
    LRESULT OnCbnDropdown(WORD/*wNotifyCode*/, WORD/*wID*/, HWND/*hWndCtl*/, BOOL&/*bHandled*/);
    LRESULT OnTimer(UINT wParam);

protected:
    virtual void UpdateFindQuery();
    void updateCharList();

private:
    sqlite::IDBPtr m_db;
    aoia::ISettingsPtr m_settings;
    InventoryView* m_pParent;
    std::tstring m_lastQueryText;
    int m_lastQueryChar;
    int m_lastQueryQlMin;
    int m_lastQueryQlMax;
};

#endif // FINDPANEL_H
