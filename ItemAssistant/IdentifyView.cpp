#include "StdAfx.h"
#include "IdentifyView.h"
#include <Shared/SQLite.h>
#include "DBManager.h"
#include "IdentifyListDataModel.h"
#include "ItemListDataModel.h"


using namespace sqlite;
using namespace aoia;


IdentifyView::IdentifyView(sqlite::IDBPtr db, aoia::IContainerManagerPtr containerManager, aoia::IGuiServicesPtr gui)
    : m_db(db)
    , m_containerManager(containerManager)
    , m_gui(gui)
    , m_datagrid(new DataGridControl())
    , m_identifyList(new DataGridControl())
    , m_sortColumn(-1)
    , m_sortAscending(true)
{
}


IdentifyView::~IdentifyView()
{
}


LRESULT IdentifyView::onCreate(LPCREATESTRUCT createStruct)
{
    RECT myRect = { 0, 0, createStruct->cx, createStruct->cy };
    //RECT identRect = { 0, 0, 250, createStruct->cy };
    //RECT gridRect = { 250, 0, createStruct->cx, createStruct->cy };

    DWORD splitterStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

    // Create child windows
    m_splitter.Create(m_hWnd, myRect, NULL, splitterStyle);
    m_splitter.SetSplitterExtendedStyle(0);
    m_splitter.SetDlgCtrlID(IDW_SPLITTER);

    DWORD gridStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_OWNERDATA;
    m_identifyList->Create(m_splitter.m_hWnd, rcDefault, NULL, gridStyle, WS_EX_CLIENTEDGE);
    m_identifyList->SetDlgCtrlID(IDW_IDENTLIST);

    m_datagrid->Create(m_splitter.m_hWnd, rcDefault, NULL, gridStyle, WS_EX_CLIENTEDGE);
    m_datagrid->SetDlgCtrlID(IDW_DATAGRID);

    m_splitter.SetSplitterPanes(m_identifyList->m_hWnd, m_datagrid->m_hWnd);
    m_splitter.SetActivePane(SPLIT_PANE_LEFT);
    m_splitter.SetSplitterPos(580);

    // Assign a datamodel to the list of identifyables.
    m_identifyListModel.reset(new IdentifyListDataModel(m_db));
    m_identifyList->setModel(m_identifyListModel);
    m_identifyList->autosizeColumnsUseData(false);

    DlgResize_Init(false, false);

    // Build the toolbar
    TBBUTTON buttons[1];
    buttons[0].iBitmap = 0;
    buttons[0].idCommand = ID_HELP;
    buttons[0].fsState = TBSTATE_ENABLED;
    buttons[0].fsStyle = TBSTYLE_BUTTON | BTNS_SHOWTEXT | BTNS_AUTOSIZE;
    buttons[0].dwData = NULL;
    buttons[0].iString = (INT_PTR)_T("Help");

    CImageList imageList;
    imageList.CreateFromImage(IDB_IDENTIFY_VIEW, 16, 1, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_DEFAULTSIZE);

    m_toolbar.Create(GetTopLevelWindow(), NULL, _T("IdentifyViewToolBar"), 
        ATL_SIMPLE_TOOLBAR_PANE_STYLE | TBSTYLE_LIST, 
        TBSTYLE_EX_MIXEDBUTTONS);
    m_toolbar.SetButtonStructSize();
    m_toolbar.SetImageList(imageList);
    m_toolbar.AddButtons(ARRAYSIZE(buttons), buttons);
    m_toolbar.AutoSize();

    return 0;
}


// Normal single select change notifications
LRESULT IdentifyView::onListItemChanged(LPNMHDR lParam)
{
    // Check that it is an event from the correct child window.
    if (lParam->hwndFrom == m_identifyList->m_hWnd)
    {
        LPNMLISTVIEW pItem = (LPNMLISTVIEW)lParam;

        // Check to see if the change is a selection event.
        if ( !(pItem->uOldState & LVIS_SELECTED) && (pItem->uNewState & LVIS_SELECTED) )
        {
            std::set<unsigned int> aoids;
            aoids.insert(m_identifyListModel->getItemId(pItem->iItem));
            aoids.insert(m_identifyListModel->getItemLowId(pItem->iItem));
            aoids.insert(m_identifyListModel->getItemHighId(pItem->iItem));

            ItemListDataModelPtr data(new ItemListDataModel(m_db, m_containerManager, aoids));

            m_datagrid->setModel(data);
            m_datagrid->autosizeColumnsUseData();
        }
    }

    return FALSE;
}


// Owner data (virtual lists) range selection changes
LRESULT IdentifyView::onListItemStateChanged(LPNMHDR lParam)
{
    LPNMLVODSTATECHANGE lStateChange = (LPNMLVODSTATECHANGE)lParam;

    // Check that it is an event from the correct child window.
    if (lStateChange->hdr.hwndFrom == m_identifyList->m_hWnd)
    {
        // Check to see if the change is a selection event.
        if ( !(lStateChange->uOldState & LVIS_SELECTED) && (lStateChange->uNewState & LVIS_SELECTED) )
        {
            std::set<unsigned int> aoids;
            aoids.insert(m_identifyListModel->getItemId(lStateChange->iFrom));
            aoids.insert(m_identifyListModel->getItemLowId(lStateChange->iFrom));
            aoids.insert(m_identifyListModel->getItemHighId(lStateChange->iFrom));

            ItemListDataModelPtr data(new ItemListDataModel(m_db, m_containerManager, aoids));

            m_datagrid->setModel(data);
            m_datagrid->autosizeColumnsUseData();
        }
    }

    return 0;
}


LRESULT IdentifyView::onColumnClick(LPNMHDR lParam)
{
    LPNMLISTVIEW pnmv = (LPNMLISTVIEW) lParam;

    if (m_sortColumn != pnmv->iSubItem)
    {
        m_sortColumn = pnmv->iSubItem;
        m_sortAscending = true;
    }
    else
    {
        m_sortAscending = !m_sortAscending;
    }

	if (pnmv->hdr.idFrom == IDW_DATAGRID)
	{
		ItemListDataModelPtr data_model = boost::static_pointer_cast<ItemListDataModel>(m_datagrid->getModel());
		data_model->sortData(m_sortColumn, m_sortAscending);
	}
	else if (pnmv->hdr.idFrom == IDW_IDENTLIST)
	{
		//IdentifyListDataModelPtr data_model = boost::static_pointer_cast<IdentifyListDataModel>(m_identifyList->getModel());
		//data_model->sortData(m_sortColumn, m_sortAscending);
	}

    return 0;
}


LRESULT IdentifyView::OnHelp( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
{
    m_gui->ShowHelp(_T("identify"));
    return 0;
}
