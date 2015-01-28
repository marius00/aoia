#include "StdAfx.h"
#include "RecipesView.h"
#include "DBManager.h"
#include "PatternReport.h"
#include "Version.h"
#include <sstream>
#include <map>
#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>
#include <Mmsystem.h>

RecipesView::RecipesView(sqlite::IDBPtr db, aoia::IContainerManagerPtr containerManager, aoia::IGuiServicesPtr gui, aoia::ISettingsPtr settings)
    : m_db(db)
    , m_containerManager(containerManager)
    , m_gui(gui)
    , m_toonid(0)
    , m_sortDesc(true)
    , m_sortColumn(1)
	, m_lastView(0)
	, m_recipePanel(db, settings)
    , m_webview( _T("") )
{
}


RecipesView::~RecipesView() {}

void RecipesView::onFilterSettingsChanged() {
	//PbListMutex().MutexOn();


	sqlite::ITablePtr pIDs;
	g_DBManager.Lock();
	unsigned int charid = m_recipePanel.getCharId();
	std::list<unsigned int> refs = m_recipePanel.getDesiredItemRefs();
	if (refs.size() == 0)
		return;

	// RecipePanel.getIds
	{
		bool first = true;
		std::tstringstream sql;
		sql << _T("SELECT keyhigh, ql, stack FROM tItems WHERE keyhigh in (");
		for (auto it : refs) {
			if (!first)
				sql << _T(",") << it;
			else  {
				sql << it;
				first = false;
			}
			
		}
		sql <<_T(") ");
		if (charid > 0)
			sql << _T(" AND owner = ") << charid;

		pIDs = m_db->ExecTable(sql.str());
	}
	g_DBManager.UnLock();

	if (pIDs == NULL) {
		return;
	}

	std::list<Item> items;
	// Loop all the pattern pieces and searches for recorded items.
	for (unsigned int idIdx = 0; idIdx < pIDs->Rows(); idIdx++) {
		
		std::tstring id = from_ascii_copy(pIDs->Data(idIdx, 0));
		std::tstring ql = from_ascii_copy(pIDs->Data(idIdx, 1));
		std::tstring stack = from_ascii_copy(pIDs->Data(idIdx, 2));

		Item item(boost::lexical_cast<unsigned int>(id));
		item.minql = boost::lexical_cast<unsigned int>(ql);
		item.maxql = boost::lexical_cast<unsigned int>(ql);
		item.count = boost::lexical_cast<unsigned int>(stack);
		items.push_back(item);
	}

	std::set<Item> itemsToShow = m_recipePanel.getItemList(items);


	// Update view


	std::tstringstream out;
	out << _T("<tr><th>QL</th><th>Item</th><th>Comments</th><th>Count</th><th>Required</th></tr>\n");
	for (Item item : itemsToShow) {
			if (item.nCountInDb >= item.count) out << _T("<tr style=\"color:red;\">");
			else out << _T("<tr>");
			out << _T("<th>") << item.minql << "-" << item.maxql 
			<< _T("</th><th>") << item.name
			<< _T("</th><th>") << item.comments
			<< _T("</th><th>") << item.nCountInDb
			<< _T("</th><th>") << item.count
			<< _T("</th></tr>\n");
	}
	HRSRC hrsrc = ::FindResource(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_HTML3), RT_HTML);
	DWORD size = ::SizeofResource(_Module.GetResourceInstance(), hrsrc);
	HGLOBAL hGlobal = ::LoadResource(_Module.GetResourceInstance(), hrsrc);
	void* pData = ::LockResource(hGlobal);
	if (pData != NULL) {
		std::string raw((char*)pData, size);
		std::tstring html = from_ascii_copy(raw);
		boost::algorithm::replace_all(html, _T("%RESULT_TABLE%"), out.str());
		boost::algorithm::replace_all(html, _T("%TITLE%"), m_recipePanel.getRecipeName());
		m_webview.SetHTML(html);
	}

	//PbListMutex().MutexOff();
}

LRESULT RecipesView::OnCreate(LPCREATESTRUCT createStruct)
{
    DWORD style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	/*
    m_listview.Create(m_hWnd, rcDefault, NULL, style | LVS_REPORT | LVS_SINGLESEL, WS_EX_CLIENTEDGE);*/
	

	m_recipePanel.Create(m_hWnd);
	m_recipePanel.ShowWindow(SW_SHOWNOACTIVATE);
	m_filterConnection = m_recipePanel.connectSettingsChanged(boost::bind(&RecipesView::onFilterSettingsChanged, this));
	/*
	m_buttonRefresh.Create(m_hWnd);
	m_buttonRefresh.ShowWindow(SW_SHOWNOACTIVATE);*/

    m_webview.Create(m_hWnd);
   // m_webview.ShowWindow(SW_SHOWNOACTIVATE);
	m_webview.SetHTML(_T("<html><body></body></html>"));


    CImageList imageList;
    imageList.CreateFromImage(IDB_PATTERNMATCH_VIEW, 16, 1, CLR_DEFAULT, IMAGE_BITMAP,
        LR_CREATEDIBSECTION | LR_DEFAULTSIZE);

    m_toolbar.Create(GetTopLevelWindow(), NULL, _T("RecipesViewToolBar"),
        ATL_SIMPLE_TOOLBAR_PANE_STYLE | TBSTYLE_LIST,
        TBSTYLE_EX_MIXEDBUTTONS);
    m_toolbar.SetButtonStructSize();
    m_toolbar.SetImageList(imageList);
    m_toolbar.AutoSize();

    return 0;
}


void RecipesView::OnDestroy()
{
	m_recipePanel.disconnect(m_filterConnection);
}





void RecipesView::UpdateFilterProgress(unsigned short percent)
{
}


void RecipesView::OnActive(bool doActivation) {
	m_recipePanel.updateCharList();
}





void RecipesView::OnFocus(CWindow wndOld) {
	if (timeGetTime() > m_lastView + 3000) {
		onFilterSettingsChanged();
		m_lastView = timeGetTime();
	}
}
LRESULT RecipesView::OnSize(UINT wParam, CSize newSize)
{
    UpdateLayout(newSize);
    return 0;
}


LRESULT RecipesView::OnRecalculate(WORD/*wNotifyCode*/, WORD/*wID*/, HWND/*hWndCtl*/, BOOL&/*bHandled*/)
{


    return 0;
}


LRESULT RecipesView::OnHelp(WORD/*wNotifyCode*/, WORD/*wID*/, HWND/*hWndCtl*/, BOOL&/*bHandled*/)
{
    m_gui->ShowHelp(_T("patternmatcher"));
    return 0;
}



void RecipesView::UpdateLayout(CSize newSize)
{
    int left = 0;
    int top = 0;

	if (::IsWindowVisible(m_recipePanel))
	{
		RECT fvRect;
		m_recipePanel.GetWindowRect(&fvRect);
		int width = newSize.cy;
		int height = fvRect.bottom - fvRect.top;
		::SetWindowPos(m_recipePanel, 0, 0, 0, newSize.cx, height,
			SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE | SWP_DEFERERASE | SWP_NOSENDCHANGING);
		//left += width;
		top += height;
	}


    m_webview.SetWindowPos(NULL, left, top, newSize.cx - left, newSize.cy,
        SWP_NOZORDER | SWP_NOACTIVATE | SWP_DEFERERASE | SWP_NOSENDCHANGING);
		
}


bool RecipesView::PreTranslateMsg(MSG* pMsg)
{


    return false;
}


