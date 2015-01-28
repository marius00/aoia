#include "StdAfx.h"
#include "FindPanel.h"
#include "InventoryView.h"
#include <boost/algorithm/string.hpp>

namespace ba = boost::algorithm;
using namespace aoia;

FindView::FindView(sqlite::IDBPtr db, aoia::ISettingsPtr settings)
    : m_db(db)
    , m_settings(settings)
    , m_lastQueryChar(-1)
    , m_lastQueryQlMin(-1)
    , m_lastQueryQlMax(-1)
    , m_pParent(NULL) {}


void FindView::SetParent(InventoryView* parent)
{
    m_pParent = parent;
}


LRESULT FindView::OnInitDialog(UINT/*uMsg*/, WPARAM/*wParam*/, LPARAM/*lParam*/, BOOL&/*bHandled*/)
{
    SetWindowText(_T("Find View"));
    updateCharList();
    DlgResize_Init(false, true, WS_CLIPCHILDREN);
    return 0;
}


LRESULT FindView::OnForwardMsg(UINT/*uMsg*/, WPARAM/*wParam*/, LPARAM lParam, BOOL&/*bHandled*/)
{
    LPMSG pMsg = (LPMSG)lParam;
    return this->PreTranslateMsg(pMsg);
}


BOOL FindView::PreTranslateMsg(MSG* pMsg)
{
    return IsDialogMessage(pMsg);
}


LRESULT FindView::OnEnChangeItemtext(WORD/*wNotifyCode*/, WORD/*wID*/, HWND/*hWndCtl*/, BOOL&/*bHandled*/)
{
    SetTimer(1, 1500);
    return 0;
}


LRESULT FindView::OnCbnSelChangeCharcombo(WORD/*wNotifyCode*/, WORD/*wID*/, HWND/*hWndCtl*/, BOOL&/*bHandled*/)
{
    UpdateFindQuery();
    return 0;
}


LRESULT FindView::OnCbnDropdown(WORD/*wNotifyCode*/, WORD/*wID*/, HWND/*hWndCtl*/, BOOL&/*bHandled*/)
{
    CComboBox toon_combo = GetDlgItem(IDC_CHARCOMBO);
    unsigned int char_id = toon_combo.GetItemData(toon_combo.GetCurSel());

    updateCharList();

    bool found = false;
    for (int i = 0; i < toon_combo.GetCount(); ++i)
    {
        unsigned int data = toon_combo.GetItemData(i);
        if (data == char_id)
        {
            toon_combo.SetCurSel(i);
            found = true;
            break;
        }
    }

    if (!found)
    {
        toon_combo.SetCurSel(0);
    }
    return 0;
}


LRESULT FindView::OnTimer(UINT wParam)
{
    if (wParam == 1)
    {
        UpdateFindQuery();
        KillTimer(1);
    }
    return 0;
}


void FindView::UpdateFindQuery()
{
    KillTimer(1);

    int item = -1;
    CComboBox cb = GetDlgItem(IDC_CHARCOMBO);
    CEdit eb = GetDlgItem(IDC_ITEMTEXT);
    CEdit qlmin = GetDlgItem(IDC_QLMIN);
    CEdit qlmax = GetDlgItem(IDC_QLMAX);

    unsigned int charid = 0;
    item = -1;
    if ((item = cb.GetCurSel()) != CB_ERR)
    {
        charid = (unsigned int)cb.GetItemData(item);
    }

    TCHAR buffer[MAX_PATH];
    ZeroMemory(buffer, MAX_PATH);
    eb.GetWindowText(buffer, MAX_PATH);
    std::tstring text(buffer);
    ba::trim_if(text, ba::is_any_of(" \n\r\t"));

    int minql = -1;
    ZeroMemory(buffer, MAX_PATH);
    qlmin.GetWindowText(buffer, MAX_PATH);
    std::tstring qlminText(buffer);
    if (!qlminText.empty())
    {
        try
        {
            minql = boost::lexical_cast<int>(qlminText);
        }
        catch (boost::bad_lexical_cast&/*e*/)
        {
            // Go with the default value
        }
    }

    int maxql = -1;
    ZeroMemory(buffer, MAX_PATH);
    qlmax.GetWindowText(buffer, MAX_PATH);
    std::tstring qlmaxText(buffer);
    if (!qlmaxText.empty())
    {
        try
        {
            maxql = boost::lexical_cast<int>(qlmaxText);
        }
        catch (boost::bad_lexical_cast&/*e*/)
        {
            // Go with the default value
        }
    }

    if (text.size() > 2
        && (m_lastQueryText != text
        || m_lastQueryChar != charid
        || m_lastQueryQlMin != minql
        || m_lastQueryQlMax != maxql))
    {
        m_lastQueryText = text;
        m_lastQueryChar = charid;
        m_lastQueryQlMin = minql;
        m_lastQueryQlMax = maxql;
        std::tstringstream sql;

        if (charid > 0)
        {
            sql << _T("I.owner = ") << charid << _T(" AND ");
        }
        if (minql > -1)
        {
            sql << _T("I.ql >= ") << minql << _T(" AND ");
        }
        if (maxql > -1)
        {
            sql << _T("I.ql <= ") << maxql << _T(" AND ");
        }

        sql << _T("keylow IN (SELECT aoid FROM aodb.tblAO WHERE name LIKE \"%") << text << _T("%\")");

        m_pParent->UpdateListView(sql.str());
    }
}


void FindView::updateCharList()
{
    CComboBox cb = GetDlgItem(IDC_CHARCOMBO);

    cb.ResetContent();
    int item = cb.AddString(_T("-"));
    cb.SetItemData(item, 0);

    g_DBManager.Lock();
    sqlite::ITablePtr pT = m_db->ExecTable("SELECT DISTINCT charid FROM tToons T ORDER BY charname");
    g_DBManager.UnLock();

    if (pT != NULL)
    {
        for (unsigned int i = 0; i < pT->Rows(); i++)
        {
            try
            {
                unsigned int id = boost::lexical_cast<unsigned int>(pT->Data(i, 0));

                g_DBManager.Lock();
                std::tstring name = g_DBManager.GetToonName(id);
                g_DBManager.UnLock();

                if (name.empty())
                {
                    name = from_ascii_copy(pT->Data(i, 0));
                }

                if ((item = cb.AddString(name.c_str())) != CB_ERR)
                {
                    cb.SetItemData(item, id);
                }
            }
            catch (boost::bad_lexical_cast&/*e*/)
            {
                // This is here because of a wierd but that appears to be SQLite's fault.
                LOG("Error in updateCharList(). Bad lexical cast at row " << i << ".");
                continue;
            }
        }
    }
}
