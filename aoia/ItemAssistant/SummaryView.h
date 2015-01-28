#ifndef SUMMARYVIEW_H
#define SUMMARYVIEW_H

#include <shared/IDB.h>
#include <PluginSDK/ItemAssistView.h>
#include <PluginSDK/IGuiServices.h>
#include "WebView.h"


namespace aoia
{
    namespace sv
    {

        class SummaryView
            : public ItemAssistView<SummaryView>
        {
        public:
            SummaryView(sqlite::IDBPtr db, aoia::IGuiServicesPtr gui);
            virtual ~SummaryView();

            BEGIN_MSG_MAP_EX(SummaryView)
                MSG_WM_CREATE(OnCreate)
                MSG_WM_SIZE(OnSize)
                COMMAND_ID_HANDLER(ID_REFRESH, OnRefresh)
                COMMAND_ID_HANDLER(ID_HELP, OnHelp)
                REFLECT_NOTIFICATIONS()
                DEFAULT_REFLECTION_HANDLER()
            END_MSG_MAP();

            // ItemAssistView overrides
            virtual void OnActive(bool doActivation);

        protected:
            LRESULT OnCreate(LPCREATESTRUCT createStruct);
            LRESULT OnSize(UINT wParam, CSize newSize);
            LRESULT OnRefresh(WORD/*wNotifyCode*/, WORD/*wID*/, HWND/*hWndCtl*/, BOOL&/*bHandled*/);
            LRESULT OnHelp(WORD/*wNotifyCode*/, WORD/*wID*/, HWND/*hWndCtl*/, BOOL&/*bHandled*/);

            void UpdateSummary();
            std::tstring SummaryView::GetHtmlTemplate();

        private:
            sqlite::IDBPtr m_db;
            aoia::IGuiServicesPtr m_gui;
            aoia::WebView m_webview;
            std::tstring m_template;
        };

    }
}  // end of namespace

#endif // SUMMARYVIEW_H
