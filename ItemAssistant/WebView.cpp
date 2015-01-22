#include "stdafx.h"
#include "WebView.h"

namespace aoia {

    WebView::WebView(std::tstring const& initialURL)
        : m_initialURL(initialURL)
    {
        if (m_initialURL.empty()) {
            m_initialURL = _T("about:blank");
        }

    }


    LRESULT WebView::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        CWindow wndPlaceholder = GetDlgItem ( IDC_IE_PLACEHOLDER );
        CRect rc;

        // Get the rect of the placeholder group box, then destroy 
        // that window because we don't need it anymore.
        wndPlaceholder.GetWindowRect ( rc );
        ScreenToClient ( rc );
        wndPlaceholder.DestroyWindow();

        // Create the AX host window.
        m_wndIE.Create ( *this, rc, _T(""), WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN );

        CComPtr<IUnknown> punkCtrl;
        CComVariant v;    // empty VARIANT

        // Create the browser control using its GUID.
        m_wndIE.CreateControlEx ( L"Shell.Explorer", NULL, NULL, &punkCtrl );

        // Get an IWebBrowser2 interface on the control and navigate to a page.
        m_pWB2 = punkCtrl;
        if (m_pWB2 && !m_initialURL.empty())
        {
            m_pWB2->Navigate(CComBSTR(m_initialURL.c_str()), &v, &v, &v, &v);
        }
        m_pWB2->put_Visible(VARIANT_TRUE);

        return 0;
    }


    LRESULT WebView::OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
    {
        LPMSG pMsg = (LPMSG)lParam;
        return this->PreTranslateMsg(pMsg);
    }


    LRESULT WebView::OnSize(UINT wParam, CSize newSize)
    {
        m_wndIE.SetWindowPos(NULL, 0, 0, newSize.cx, newSize.cy, SWP_NOZORDER | SWP_NOACTIVATE | SWP_DEFERERASE | SWP_NOSENDCHANGING);
        return 0;
    }


    BOOL WebView::PreTranslateMsg(MSG* pMsg)
    {
        return IsDialogMessage(pMsg);
    }


    void WebView::Navigate(std::tstring const& url)
    {
        if (!m_pWB2) {
            return;
        }

        CComVariant v;    // empty VARIANT
        m_pWB2->Navigate( CComBSTR(url.c_str()), &v, &v, &v, &v );
    }


    void WebView::SetHTML(std::tstring const& html)
    {
        if (!m_pWB2) {
            return;
        }

        LPDISPATCH pDisp = NULL;
        HRESULT hr = m_pWB2->get_Document(&pDisp);
        if (hr != S_OK)
        {
            LOG("Unable to retrieve document. Error code: " << std::hex << hr);
            return;
        }
        if (!pDisp)
        {
            LOG("Unable to retrieve document.");
            return;
        }

        IHTMLDocument2* pIDoc = NULL;
        pDisp->QueryInterface(IID_IHTMLDocument2, (void**)&pIDoc);
        if (!pIDoc)
        {
            LOG("IE document doesn't implement IHTMLDocument2.");
            return;
        }
        pIDoc->clear();

        SAFEARRAY *sfArray = SafeArrayCreateVector(VT_VARIANT, 0, 1);
        if (sfArray)
        {
            CComBSTR bstr( html.c_str() );
            VARIANT *param;
            SafeArrayAccessData(sfArray,(LPVOID*) & param);
            param->vt = VT_BSTR;
            param->bstrVal = bstr;
            SafeArrayUnaccessData(sfArray);
            pIDoc->writeln(sfArray);
        }

        SafeArrayDestroy(sfArray);
        pIDoc->close();
        pIDoc->Release();
        pDisp->Release();
    }

}   // namespace
