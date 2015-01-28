#include "StdAfx.h"
#include "SummaryView.h"
#include "DBManager.h"
#include "DataModel.h"
#include "CharacterSummaryFormatter.h"
#include <boost\algorithm\string\replace.hpp>


namespace aoia { namespace sv {

    SummaryView::SummaryView(sqlite::IDBPtr db, aoia::IGuiServicesPtr gui)
        : m_db(db)
        , m_webview(_T(""))
        , m_gui(gui)
    {
    }

    SummaryView::~SummaryView()
    {
    }

    LRESULT SummaryView::OnCreate( LPCREATESTRUCT createStruct )
    {
        m_webview.Create(m_hWnd);

        // Build the toolbar
        TBBUTTON buttons[3];
        buttons[0].iBitmap = 0;
        buttons[0].idCommand = ID_REFRESH;
        buttons[0].fsState = TBSTATE_ENABLED;
        buttons[0].fsStyle = TBSTYLE_BUTTON | BTNS_SHOWTEXT | BTNS_AUTOSIZE;
        buttons[0].dwData = NULL;
        buttons[0].iString = (INT_PTR)_T("Refresh");
        buttons[1].iBitmap = 3;
        buttons[1].idCommand = 0;
        buttons[1].fsState = 0;
        buttons[1].fsStyle = BTNS_SEP;
        buttons[1].dwData = NULL;
        buttons[1].iString = NULL;
        buttons[2].iBitmap = 1;
        buttons[2].idCommand = ID_HELP;
        buttons[2].fsState = TBSTATE_ENABLED;
        buttons[2].fsStyle = TBSTYLE_BUTTON | BTNS_SHOWTEXT | BTNS_AUTOSIZE;
        buttons[2].dwData = NULL;
        buttons[2].iString = (INT_PTR)_T("Help");

        CImageList imageList;
        imageList.CreateFromImage(IDB_SUMMARY_VIEW, 16, 1, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_DEFAULTSIZE);

        m_toolbar.Create(GetTopLevelWindow(), NULL, _T("SummaryViewToolBar"), 
            ATL_SIMPLE_TOOLBAR_PANE_STYLE | TBSTYLE_LIST, 
            TBSTYLE_EX_MIXEDBUTTONS);
        m_toolbar.SetButtonStructSize();
        m_toolbar.SetImageList(imageList);
        m_toolbar.AddButtons(ARRAYSIZE(buttons), buttons);
        m_toolbar.AutoSize();

        return 0;
    }

    LRESULT SummaryView::OnSize( UINT wParam, CSize newSize )
    {
        CRect r( CPoint( 0, 0 ), newSize );
        m_webview.SetWindowPos(NULL, r.left, r.top, r.Width(), r.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_DEFERERASE | SWP_NOSENDCHANGING);

        return 0;
    }

    LRESULT SummaryView::OnRefresh( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
    {
        UpdateSummary();
        return 0;
    }

    LRESULT SummaryView::OnHelp( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
    {
        m_gui->ShowHelp(_T("summary"));
        return 0;
    }

    void SummaryView::OnActive( bool doActivation )
    {
        if (doActivation) {
            UpdateSummary();
        }
    }

    void SummaryView::UpdateSummary()
    {
        DataModelPtr model(DataModelPtr(new DataModel(m_db)));

        std::tstringstream contentHtml;

        if (model->getItemCount() != 0)
        {
            CharacterSummaryFormatter formatter(model);
            contentHtml << formatter.toString();
        }

        std::tstring htmlTemplate = GetHtmlTemplate();
        boost::replace_all(htmlTemplate, _T("%SUMMARY%"), contentHtml.str());

        m_webview.SetHTML(htmlTemplate);
    }

    std::tstring SummaryView::GetHtmlTemplate()
    {
        if (!m_template.empty()) {
            return m_template;
        }

        HRSRC hrsrc = ::FindResource(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_TEMPLATE), RT_HTML);
        DWORD size = ::SizeofResource(_Module.GetResourceInstance(), hrsrc);
        HGLOBAL hGlobal = ::LoadResource(_Module.GetResourceInstance(), hrsrc);
        void * pData = ::LockResource(hGlobal);

        if (pData) {
            std::string raw((char*)pData, size);
            m_template = from_ascii_copy(raw);
        }

        return m_template;
    }

}}  // end of namespace
