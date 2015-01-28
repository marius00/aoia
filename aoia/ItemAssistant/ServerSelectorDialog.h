#ifndef SERVERSELECTORDIALOG_H
#define SERVERSELECTORDIALOG_H

#include "resource.h"


class ServerSelectorDialog :
    public CDialogImpl<ServerSelectorDialog>
{
public:
    enum { IDD = IDD_SERVER_SELECTOR };

    ServerSelectorDialog();

    BEGIN_MSG_MAP(ServerSelectorDialog)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
        COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

    unsigned int getChoice() const;

private:
    unsigned int m_choice;
};

#endif // SERVERSELECTORDIALOG_H
