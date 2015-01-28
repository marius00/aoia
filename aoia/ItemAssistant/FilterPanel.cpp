#include "StdAfx.h"
#include "FilterPanel.h"
#include "DBManager.h"

using namespace aoia;

namespace PatternMatcher
{

    FilterPanel::FilterPanel(sqlite::IDBPtr db, aoia::ISettingsPtr settings)
        : m_db(db) 
        , m_settings(settings)
    {}


    FilterPanel::~FilterPanel() {}


    LRESULT FilterPanel::onInitDialog(UINT/*uMsg*/, WPARAM/*wParam*/, LPARAM/*lParam*/, BOOL&/*bHandled*/)
    {
        SetWindowText(_T("Filter View"));

        updateCharList();

        CheckDlgButton(IDC_SHOW_ALL, 1);

        WTL::CProgressBarCtrl progressbar = GetDlgItem(IDC_PROGRESS);
        progressbar.SetRange(0, 100);
        progressbar.SetPos(0);

        signalSettingsChanged();

        return 0;
    }


    LRESULT FilterPanel::onForwardMsg(UINT/*uMsg*/, WPARAM/*wParam*/, LPARAM lParam, BOOL&/*bHandled*/)
    {
        LPMSG pMsg = (LPMSG)lParam;
        return this->PreTranslateMsg(pMsg);
    }


    void FilterPanel::updateCharList()
    {
        CComboBox cb = GetDlgItem(IDC_CHARCOMBO);
        if (cb.GetCount() < 1)
        {
            WTL::CComboBox cb = GetDlgItem(IDC_CHARCOMBO);
            cb.ResetContent();
            int item = cb.AddString(_T("-"));
            cb.SetItemData(item, 0);
            cb.SetCurSel(0);
            return;
        }

        unsigned int prev_selected_toon = cb.GetItemData(cb.GetCurSel());

        cb.ResetContent();
        int item = cb.AddString(_T("-"));
        cb.SetItemData(item, 0);
        cb.SetCurSel(0);

        g_DBManager.Lock();
        sqlite::ITablePtr pT = m_db->ExecTable("SELECT DISTINCT charid FROM tToons ORDER BY charname");

        if (pT != NULL)
        {
            for (unsigned int i = 0; i < pT->Rows(); i++)
            {
                unsigned int id = boost::lexical_cast<unsigned int>(pT->Data(i, 0));

                std::tstring name = g_DBManager.GetToonName(id);
                if (name.empty())
                {
                    name = from_ascii_copy(pT->Data(i, 0));
                }

                if ((item = cb.AddString(name.c_str())) != CB_ERR)
                {
                    cb.SetItemData(item, id);
                }
            }
        }
        g_DBManager.UnLock();

        bool found = false;
        for (int i = 0; i < cb.GetCount(); ++i)
        {
            unsigned int data = cb.GetItemData(i);
            if (data == prev_selected_toon)
            {
                cb.SetCurSel(i);
                found = true;
                break;
            }
        }

        if (!found)
        {
            cb.SetCurSel(0);
        }
    }


    void FilterPanel::setProgress(unsigned short percent)
    {
        WTL::CProgressBarCtrl progressbar = GetDlgItem(IDC_PROGRESS);
        progressbar.SetPos(percent);
    }


    unsigned int FilterPanel::getCharId() const
    {
        unsigned int toonid = 0;

        WTL::CComboBox tooncb = GetDlgItem(IDC_CHARCOMBO);
        if (tooncb.GetCurSel() > 0)
        {
            toonid = (unsigned int)tooncb.GetItemData(tooncb.GetCurSel());
        }

        return toonid;
    }


	FilterPanel::PbMode FilterPanel::getMode() const {
		if (IsDlgButtonChecked(IDC_SHOW_INFERNO_CLAN)) {
			return PbMode::INF_CLAN;
		}
		else if (IsDlgButtonChecked(IDC_SHOW_INFERNO_OMNI)) {
			return PbMode::INF_OMNI;
		}
		else {
			return PbMode::REGULAR;
		}
	}

    float FilterPanel::getAvailFilter() const {
        float availfilter = -1.0f;

        if (IsDlgButtonChecked(IDC_SHOW_ALL))
        {
            availfilter = -1.0f;
        }
        else if (IsDlgButtonChecked(IDC_SHOW_PARTIALS))
        {
            availfilter = 0.25f;
        }
        else if (IsDlgButtonChecked(IDC_COMPLETABLE))
        {
            availfilter = 1.0f;
        }

        return availfilter;
    }


    bool FilterPanel::getExcludeAssembled() const {
        return IsDlgButtonChecked(IDC_EXCLUDE_ASSEMBLED) == TRUE ? true : false;
    }


    BOOL FilterPanel::PreTranslateMsg(MSG* pMsg)
    {
        return IsDialogMessage(pMsg);
    }


    LRESULT FilterPanel::onCbnSelchangeCharcombo(WORD/*wNotifyCode*/, WORD/*wID*/, HWND/*hWndCtl*/, BOOL&/*bHandled*/)
    {
        signalSettingsChanged();
        return 0;
    }


    LRESULT FilterPanel::onBnClickedShowAll(WORD/*wNotifyCode*/, WORD/*wID*/, HWND/*hWndCtl*/, BOOL&/*bHandled*/)
    {
        signalSettingsChanged();
        return 0;
    }


    LRESULT FilterPanel::onBnClickedShowPartials(WORD/*wNotifyCode*/, WORD/*wID*/, HWND/*hWndCtl*/, BOOL&/*bHandled*/)
    {
        signalSettingsChanged();
        return 0;
    }


	LRESULT FilterPanel::onBnClickedInfClan(WORD/*wNotifyCode*/, WORD/*wID*/, HWND/*hWndCtl*/, BOOL&/*bHandled*/) {
		signalSettingsChanged();
		return 0;
	}
	LRESULT FilterPanel::onBnClickedInfOmni(WORD/*wNotifyCode*/, WORD/*wID*/, HWND/*hWndCtl*/, BOOL&/*bHandled*/) {
		signalSettingsChanged();
		return 0;
	}
    LRESULT FilterPanel::onBnClickedCompletable(WORD/*wNotifyCode*/, WORD/*wID*/, HWND/*hWndCtl*/, BOOL&/*bHandled*/) {
        signalSettingsChanged();
        return 0;
    }


    LRESULT FilterPanel::OnCbnDropdown(WORD/*wNotifyCode*/, WORD/*wID*/, HWND/*hWndCtl*/, BOOL&/*bHandled*/)
    {
        updateCharList();
        return 0;
    }


    LRESULT FilterPanel::onExcludeAssembledPatternsClicked(WORD/*wNotifyCode*/, WORD/*wID*/, HWND/*hWndCtl*/, BOOL&/*bHandled*/)
    {
        signalSettingsChanged();
        return 0;
    }


    void FilterPanel::disconnect(Connection subscriber)
    {
        subscriber.disconnect();
    }


    FilterPanel::Connection FilterPanel::connectSettingsChanged(SettingsChangedSignal::slot_function_type subscriber)
    {
        return m_settingsChangedSignal.connect(subscriber);
    }


    void FilterPanel::signalSettingsChanged()
    {
        m_settingsChangedSignal();
    }

}   // namespace