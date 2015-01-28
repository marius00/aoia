#ifndef WEBVIEW_H
#define WEBVIEW_H

namespace aoia {

    class WebView
        : public CDialogImpl<WebView>
    {
    public:
        enum { IDD = IDD_WEB };

        WebView(std::tstring const& initialURL);

        //void SetParent(PatternMatchView* parent);
        BOOL PreTranslateMsg(MSG* pMsg);

        BEGIN_MSG_MAP(WebView)
            MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
            MESSAGE_HANDLER(WM_FORWARDMSG, OnForwardMsg)
            MSG_WM_SIZE(OnSize)
            DEFAULT_REFLECTION_HANDLER()
        END_MSG_MAP()

        LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
        LRESULT OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
        LRESULT OnSize(UINT wParam, CSize newSize);

        void Navigate(std::tstring const& url);
        void SetHTML(std::tstring const& html);

    private:
        CAxWindow m_wndIE;
        CComQIPtr<IWebBrowser2> m_pWB2;
        std::tstring m_initialURL;
    };

}   // namespace

#endif // WEBVIEW_H
