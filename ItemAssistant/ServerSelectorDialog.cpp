#include "StdAfx.h"
#include "ServerSelectorDialog.h"

ServerSelectorDialog::ServerSelectorDialog()
    : m_choice(1)
{
}


LRESULT ServerSelectorDialog::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    CenterWindow(GetParent());
    CheckDlgButton(IDC_AUNO, 1);
    return TRUE;
}


LRESULT ServerSelectorDialog::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    if (IsDlgButtonChecked(IDC_AUNO))
    {
        m_choice = 1;
    }
    else if (IsDlgButtonChecked(IDC_JAYDEE))
    {
        m_choice = 2;
    }
    else if (IsDlgButtonChecked(IDC_XYPHOS))
    {
        m_choice = 3;
    }
    else 
    {
        m_choice = 0;
    }

    EndDialog(wID);
    return 0;
}


unsigned int ServerSelectorDialog::getChoice() const
{
    return m_choice;
}