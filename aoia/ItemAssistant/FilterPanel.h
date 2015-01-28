#ifndef FILTERPANEL
#define FILTERPANEL

#include <boost/signal.hpp>
#include <shared/IDB.h>
#include <settings/ISettings.h>


namespace PatternMatcher
{

    /**
     * This class implements a GUI dialog for filtering which character(s) 
     * should be searched for pattern pieces.
     *
     * This class emits SettingsChangedSignal whenever the user makes a change 
     * in the GUI.
     */
    class FilterPanel
        : public CDialogImpl<FilterPanel>
    {
    public:
        enum
        {
            IDD = IDD_PATTERN_MATCHER
        };

		enum PbMode {
			REGULAR, INF_CLAN, INF_OMNI
		};

        typedef boost::signal<void ()> SettingsChangedSignal;
        typedef boost::signals::connection Connection;

        FilterPanel(sqlite::IDBPtr db, aoia::ISettingsPtr settings);
        virtual ~FilterPanel();

        // Access to properties in the filter GUI.
        unsigned int getCharId() const;
        float getAvailFilter() const;
		PbMode FilterPanel::getMode() const;
        bool getExcludeAssembled() const;

        BOOL PreTranslateMsg(MSG* pMsg);

        void setProgress(unsigned short percent);
        void updateCharList();

        BEGIN_MSG_MAP(FilterView)
            MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
            MESSAGE_HANDLER(WM_FORWARDMSG, onForwardMsg)
            COMMAND_HANDLER(IDC_SHOW_ALL, BN_CLICKED, onBnClickedShowAll)
			COMMAND_HANDLER(IDC_SHOW_PARTIALS, BN_CLICKED, onBnClickedShowPartials)
			COMMAND_HANDLER(IDC_COMPLETABLE, BN_CLICKED, onBnClickedCompletable)
			COMMAND_HANDLER(IDC_SHOW_INFERNO_CLAN, BN_CLICKED, onBnClickedInfClan)
			COMMAND_HANDLER(IDC_SHOW_INFERNO_OMNI, BN_CLICKED, onBnClickedInfOmni)
            COMMAND_HANDLER(IDC_CHARCOMBO, CBN_SELCHANGE, onCbnSelchangeCharcombo)
            COMMAND_HANDLER(IDC_CHARCOMBO, CBN_DROPDOWN, OnCbnDropdown)
            COMMAND_HANDLER(IDC_EXCLUDE_ASSEMBLED, BN_CLICKED, onExcludeAssembledPatternsClicked)
            DEFAULT_REFLECTION_HANDLER()
        END_MSG_MAP()

        void disconnect(Connection subscriber);
        Connection connectSettingsChanged(SettingsChangedSignal::slot_function_type subscriber);

    protected:
        void signalSettingsChanged();

        LRESULT onInitDialog(UINT/*uMsg*/, WPARAM/*wParam*/, LPARAM/*lParam*/, BOOL&/*bHandled*/);
        LRESULT onForwardMsg(UINT/*uMsg*/, WPARAM/*wParam*/, LPARAM/*lParam*/, BOOL&/*bHandled*/);
        LRESULT onCbnSelchangeCharcombo(WORD/*wNotifyCode*/, WORD/*wID*/, HWND/*hWndCtl*/, BOOL&/*bHandled*/);
        LRESULT onBnClickedShowAll(WORD/*wNotifyCode*/, WORD/*wID*/, HWND/*hWndCtl*/, BOOL&/*bHandled*/);
		LRESULT onBnClickedShowPartials(WORD/*wNotifyCode*/, WORD/*wID*/, HWND/*hWndCtl*/, BOOL&/*bHandled*/);
		LRESULT onBnClickedCompletable(WORD/*wNotifyCode*/, WORD/*wID*/, HWND/*hWndCtl*/, BOOL&/*bHandled*/);
		LRESULT onBnClickedInfClan(WORD/*wNotifyCode*/, WORD/*wID*/, HWND/*hWndCtl*/, BOOL&/*bHandled*/);
		LRESULT onBnClickedInfOmni(WORD/*wNotifyCode*/, WORD/*wID*/, HWND/*hWndCtl*/, BOOL&/*bHandled*/);
        LRESULT OnCbnDropdown(WORD/*wNotifyCode*/, WORD/*wID*/, HWND/*hWndCtl*/, BOOL&/*bHandled*/);
        LRESULT onExcludeAssembledPatternsClicked(WORD/*wNotifyCode*/, WORD/*wID*/, HWND/*hWndCtl*/, BOOL&/*bHandled*/);

    private:
        sqlite::IDBPtr m_db;
        aoia::ISettingsPtr m_settings;
        SettingsChangedSignal m_settingsChangedSignal;
        std::tstring m_basequery;
    };

}

#endif // FILTERPANEL
