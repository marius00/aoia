#ifndef INFOPANEL_H
#define INFOPANEL_H

#include <shared/IDB.h>

// Forward declarations
class InventoryView;


class InfoView
    : public CDialogImpl<InfoView>
    , public CDialogResize<InfoView>
{
public:
    enum { IDD = IDD_ITEM_INFO };

    InfoView(sqlite::IDBPtr db);

    void SetParent(InventoryView* parent);
    BOOL PreTranslateMsg(MSG* pMsg);

    BEGIN_MSG_MAP(InfoView)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_FORWARDMSG, OnForwardMsg)
        COMMAND_CODE_HANDLER(BN_CLICKED, OnButtonClicked)
        CHAIN_MSG_MAP(CDialogResize<InfoView>)
        DEFAULT_REFLECTION_HANDLER()
    END_MSG_MAP()

    BEGIN_DLGRESIZE_MAP(InfoView)
        DLGRESIZE_CONTROL(IDC_LISTVIEW, DLSZ_SIZE_X | DLSZ_SIZE_Y)
        DLGRESIZE_CONTROL(IDC_BUTTON_LABEL, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_AUNO, DLSZ_MOVE_X | DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_JAYDEE, DLSZ_MOVE_X | DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_XYPHOS, DLSZ_MOVE_X | DLSZ_MOVE_Y)
    END_DLGRESIZE_MAP()

    void SetCurrentItem(unsigned int item);

protected:
    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnButtonClicked(WORD commandID, WORD buttonID, HWND hButton, BOOL &bHandled);

private:
    sqlite::IDBPtr m_db;
    InventoryView* m_pParent;
    unsigned int m_currentItem;
};


#endif // INFOPANEL_H
