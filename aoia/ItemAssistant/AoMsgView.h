#pragma once

#include <set>
#include <PluginSDK/ItemAssistView.h>
#include "shared/aopackets.h"


class DlgView
    : public CDialogImpl<DlgView>
    , public CDialogResize<DlgView>
{
public:
    enum { IDD = IDD_MSG_FORM };

    BOOL PreTranslateMessage(MSG* pMsg);

    BEGIN_MSG_MAP(DlgView)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_FORWARDMSG, OnForwardMsg)
        NOTIFY_HANDLER(IDC_LIST1, NM_CLICK, OnNMClickList1)
        CHAIN_MSG_MAP(CDialogResize<DlgView>)
        DEFAULT_REFLECTION_HANDLER()
    END_MSG_MAP()

    BEGIN_DLGRESIZE_MAP(DlgView)
        DLGRESIZE_CONTROL(IDC_LIST1,     (DLSZ_SIZE_Y))
        DLGRESIZE_CONTROL(IDC_EDIT2,     (DLSZ_SIZE_X | DLSZ_SIZE_Y))
        DLGRESIZE_CONTROL(IDC_LIST2,     (DLSZ_SIZE_X | DLSZ_MOVE_Y))
        DLGRESIZE_CONTROL(IDC_SQL_EDIT,  (DLSZ_SIZE_X | DLSZ_MOVE_Y))
    END_DLGRESIZE_MAP()

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnNMClickList1(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
};


class AoMsgView
    : public ItemAssistView<AoMsgView>
{
    typedef ItemAssistView<AoMsgView> inherited;
public:
    DECLARE_WND_CLASS(NULL)

    AoMsgView(void);
    virtual ~AoMsgView(void);

    BOOL PreTranslateMessage(MSG* pMsg);

    BEGIN_MSG_MAP_EX(AoMsgView)
        MSG_WM_CREATE(OnCreate)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MSG_WM_SIZE(OnSize)
        COMMAND_ID_HANDLER(ID_EDIT_CLEAR, OnClear)
        COMMAND_ID_HANDLER(ID_FILE_SAVE, OnSave)
        CHAIN_MSG_MAP(inherited)
        DEFAULT_REFLECTION_HANDLER()
    END_MSG_MAP()

    LRESULT OnCreate(LPCREATESTRUCT createStruct);
    LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnClear(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnSave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnSize(UINT wParam, CSize newSize);

    virtual void OnAOServerMessage(Parsers::AOMessageBase &msg);
    virtual void OnAOClientMessage(Parsers::AOClientMessageBase &msg);

protected:
	void DumpMessageToTextStream(std::basic_ofstream<TCHAR> &out, Parser &msg);
    void DumpMessageToStream(std::ostream &out, Parser &msg);
    bool SearchMessageForBinarySequence(Parsers::AOMessageBase &msg, unsigned char* pArray, unsigned int arraySize);

private:
    WTL::CListViewCtrl m_listview;
    DlgView m_dlgview;

    std::set<unsigned int> m_mask;
};
