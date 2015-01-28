#include "stdafx.h"
#include "InfoPanel.h"
#include "InventoryView.h"
#include <boost/algorithm/string/replace.hpp>
#include <ShellAPI.h>


InfoView::InfoView(sqlite::IDBPtr db)
    : m_db(db)
    , m_pParent(NULL) {}


void InfoView::SetParent(InventoryView* parent)
{
    m_pParent = parent;
}


BOOL InfoView::PreTranslateMsg(MSG* pMsg)
{
    return IsDialogMessage(pMsg);
}


LRESULT InfoView::OnInitDialog(UINT/*uMsg*/, WPARAM/*wParam*/, LPARAM/*lParam*/, BOOL&/*bHandled*/)
{
    this->SetWindowText(_T("Info View"));

    DlgResize_Init(false, true, WS_CLIPCHILDREN);
    return 0;
}


LRESULT InfoView::OnForwardMsg(UINT/*uMsg*/, WPARAM/*wParam*/, LPARAM lParam, BOOL&/*bHandled*/)
{
    LPMSG pMsg = (LPMSG)lParam;
    return this->PreTranslateMsg(pMsg);
}


LRESULT InfoView::OnButtonClicked(WORD commandID, WORD buttonID, HWND hButton, BOOL& bHandled)
{
    if (m_currentItem == 0)
    {
        return 0;
    }

    InventoryView::ItemServer server;
    switch (buttonID)
    {
    case IDC_JAYDEE:
        server = InventoryView::SERVER_JAYDEE;
        break;

    case IDC_AUNO:
        server = InventoryView::SERVER_AUNO;
        break;

    case IDC_XYPHOS:
        server = InventoryView::SERVER_XYPHOS;
        break;

    default:
        assert(false);  // Wrong button ID?
        return 0;
    }
    std::tstring url = InventoryView::GetServerItemURLTemplate(server);

    g_DBManager.Lock();
    OwnedItemInfoPtr pItemInfo = g_DBManager.GetOwnedItemInfo(m_currentItem);
    g_DBManager.UnLock();

    boost::replace_all(url, _T("%lowid%"), pItemInfo->itemloid);
    boost::replace_all(url, _T("%hiid%"), pItemInfo->itemhiid);
    boost::replace_all(url, _T("%ql%"), pItemInfo->itemql);

    ShellExecute(NULL, _T("open"), url.c_str(), NULL, NULL, SW_SHOWDEFAULT);

    return 0;
}


void InfoView::SetCurrentItem(unsigned int item)
{
    m_currentItem = item;

    WTL::CListViewCtrl lv(GetDlgItem(IDC_LISTVIEW));

    lv.DeleteAllItems();

    std::tstringstream sql;
    sql << _T("SELECT * FROM tItems WHERE itemidx = ") << item;

    g_DBManager.Lock();
    sqlite::ITablePtr pT = m_db->ExecTable(sql.str());
    g_DBManager.UnLock();

    if (pT)
    {
        if (pT->Rows() > 0)
        {
            // Backup previous column widths
            std::vector<int> columnWidthList;
            while (lv.GetHeader().GetItemCount() > 0)
            {
                columnWidthList.push_back(lv.GetColumnWidth(0));
                lv.DeleteColumn(0);
            }

            // Rebuild columns with new labels
            lv.AddColumn(_T("Property"), 0);
            lv.AddColumn(_T("Value"), 1);

            int width = columnWidthList.size() > 0 ? columnWidthList[0] : 75;
            lv.SetColumnWidth(0, width);
            width = columnWidthList.size() > 1 ? columnWidthList[1] : 120;
            lv.SetColumnWidth(1, width);

            // Fill content
            int indx = INT_MAX - 1;
            for (unsigned int col = 0; col < pT->Columns(); col++)
            {
                indx = lv.AddItem(indx, 0, from_ascii_copy(pT->Headers(col)).c_str());
                lv.AddItem(indx, 1, from_ascii_copy(pT->Data(0, col)).c_str());
            }
        }
    }
}
