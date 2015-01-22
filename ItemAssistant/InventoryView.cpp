#include "stdafx.h"
#include "DBManager.h"
#include "InventoryView.h"
#include "AOMessageParsers.h"
#include <shared/AODB.h>
#include <shared/FileUtils.h>
#include <ShellAPI.h>
#include <ItemAssistantCore/AOManager.h>
#include "ItemListDataModel.h"
#include "ServerSelectorDialog.h"
#include <Parsers/AOFullCharacterSync.h>
#include <Parsers/AOPlayerShopInfo.h>
#include <Parsers/AOPlayerShopContent.h>
#include "AOMessageIDs.h"
#include "AOCharacterActionIDs.h"
#include "InventoryIDs.h"
#include <csvexport/CSVDataModel.h>
#include <csvexport/CSVExporter.h>
#include <boost/filesystem.hpp>
#include <Mmsystem.h>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>


using namespace WTL;
using namespace aoia;
using namespace boost::algorithm;
using namespace Parsers;

namespace bfs = boost::filesystem;

struct LootInfo {
	LootInfo(unsigned int* arr) { lowid = arr[1]; highid = arr[0]; ql = arr[2]; }
	unsigned int lowid;
	unsigned int highid;
	unsigned int ql;
};

// [Server]Parses every highid/lowid/ql from a "open container" message
// Container id returned by reference, 0 if a body
std::vector<LootInfo> parseLootdata(Parser msg, unsigned int& containerID) {
	int i = 0;
	std::vector<LootInfo> output;

	// Ignore 11 bytes
	while (msg.remaining() && i < 11) {
		i++;
		msg.popChar();
	}

	int numItems = (msg.popChar() - 3)/4;
	if (numItems) {
		i = 0;
		while (i < 17 && msg.remaining()) { msg.popChar(); i++; }
	}

	for (int item = 0; item < numItems; item++) {
		i= 0;
		unsigned int data[3];
		while (i < 3 && msg.remaining()) data[i++] = msg.popInteger();
		output.push_back( LootInfo(data) );

		// Remove 20 byte padding behind
		i = 0;
		if (item != numItems-1)
			while (i < 20 && msg.remaining()) { i++; msg.popChar(); }
	}


	containerID = 0;
	while (msg.remaining()) {
		if (msg.popChar() != 199) continue;
		int x = msg.popChar();
		if (x == 73) // Store id only if a backpack
			containerID = msg.popInteger();
		break;
	}

	return output;
}

struct SMSG_MOVED_DATA {
	char unknown0;
	enum TYPE {
		START_FRONT = 1,		STOP_FRONT = 2, 
		START_BACK = 3,			STOP_BACK = 4, 
		START_SRIGHT = 5,		STOP_SRIGHT = 6, 
		START_SLEFT = 7,		STOP_SLEFT = 8,
		START_ROT_RIGHT = 10,	STOP_ROT_RIGHT = 11, 
		START_ROT_LEFT = 13,	STOP_ROT_LEFT = 14, 
		JUMP = 15, ROTATE_RELATED = 22};
	TYPE state;
	float x,z,y;

	// Angles?
	float unknown_d0;
	float unknown_d1;
	SMSG_MOVED_DATA(AOMessageBase msg) {
		unknown0 = msg.popChar();
		state = (TYPE)msg.popChar();
		
		// 4*4 bytes unknown ,first row
		float* d;
		int tmp[2] = {0};
		tmp[0] = msg.popInteger(); // 0
		tmp[1] = msg.popInteger(); // float/double? increments on look right, decrements on look left
		d = (float*)&tmp[1]; unknown_d0 = *d;

		tmp[0] = msg.popInteger(); // 0
		tmp[1] = msg.popInteger(); // float/double? increments on look left, decrements on look right
		d = (float*)&tmp[1]; unknown_d1 = *d;

		// Grab coordinates
		float* f;
		int ix = msg.popInteger();
		int iz = msg.popInteger();
		int iy = msg.popInteger();
		f = (float*)&ix; x = *f;
		f = (float*)&iz; z = *f;
		f = (float*)&iy; y = *f;
	}
	
};

InventoryView::InventoryView(sqlite::IDBPtr db, aoia::IContainerManagerPtr containerManager, aoia::IGuiServicesPtr gui, aoia::ISettingsPtr settings)
    : m_db(db)
    , m_containerManager(containerManager)
    , m_gui(gui)
    , m_settings(settings)
    , m_sortDesc(false)
    , m_sortColumn(0)
    , m_datagrid(new DataGridControl())
    , m_characterParserDumper(_T("binfiles"))
    , m_infoview(db)
	, m_lastUpdateCharpos(0)
    , m_findview(db, settings)
{
    assert(db);
    assert(containerManager);
    assert(gui);
    assert(settings);

    m_tempContainers.reset(new aoia::inv::TempContainerCache());

    m_enableCharacterParserDumper = true;
    if (boost::algorithm::iequals(m_settings->getValue(_T("Record Stats")), _T("no")))
    {
        m_enableCharacterParserDumper = false;
    }

	unsigned int zones_sl[] = {4336 , 4337 , 4374 , 4365 , 4366 , 4367 , 4370 , 4543 , 4676 , 4688 , 4686 , 4678 , 4684 , 4692 , 4694 , 4682 , 4696 , 4683 , 4680 , 4677 , 4697 , 4690 , 4329 , 4328 , 4331 , 4330 , 4006 , 4629 , 4320 , 4322 , 4321 , 4681 , 4540 , 4364 , 4368 , 4691 , 4541 , 4544 , 4698 , 4542 , 4679 , 4693 , 4695 , 4881 , 4005 , 4689 , 4687 , 4873 , 4872 , 4877 , 4685 , 4582 , 4605 , 690 , 4560 , 4531 , 4532 , 4530 , 4533 , 4001 , 4313 , 4310 , 4312 , 4311 , 4880 , 4699};
	unsigned int zones_rk[] = {760 , 585 , 655 , 550 , 505 , 660 , 605 , 800 , 510 , 665 , 120 , 590 , 670 , 556 , 656 , 595 , 555 , 620 , 685 , 687 , 765 , 770 , 717 , 647 , 152 , 791 , 775 , 695 , 625 , 560 , 696 , 780 , 567 , 566 , 565 , 540 , 705 , 716 , 700 , 710 , 570 , 720 , 575 , 630 , 725 , 785 , 735 , 740 , 730 , 520 , 745 , 610 , 615 , 525 , 1933 , 635 , 790 , 535 , 515 , 795 , 750 , 755 , 580 , 640 , 646 , 650 , 600 , 551 , 586 , 545};
	m_zones_sl.insert(zones_sl, zones_sl + (sizeof(zones_sl)/sizeof(unsigned int)));
	m_zones_rk.insert(zones_rk, zones_rk + (sizeof(zones_rk)/sizeof(unsigned int)));
}


InventoryView::~InventoryView() {}


LRESULT InventoryView::OnFind(WORD/*wNotifyCode*/, WORD/*wID*/, HWND/*hWndCtl*/, BOOL&/*bHandled*/)
{
    if (!::IsWindowVisible(m_findview))
    {
        m_findview.ShowWindow(SW_SHOW);
    }
    m_findview.SetFocus();

    RECT rect;
    GetClientRect(&rect);
    CSize newsize(rect.right, rect.bottom);
    UpdateLayout(newsize);

    m_toolbar.CheckButton(ID_INV_FIND_TOGGLE, TRUE);

    return 0;
}


LRESULT InventoryView::OnFindToggle(WORD/*wNotifyCode*/, WORD/*wID*/, HWND/*hWndCtl*/, BOOL&/*bHandled*/)
{
    if (!::IsWindowVisible(m_findview))
    {
        m_findview.ShowWindow(SW_SHOW);
        m_findview.SetFocus();
        RECT rect;
        GetClientRect(&rect);
        CSize newsize(rect.right, rect.bottom);
        UpdateLayout(newsize);
    }
    else
    {
        HideFindWindow();
    }

    m_toolbar.CheckButton(ID_INV_FIND_TOGGLE, m_findview.IsWindowVisible());

    return 0;
}


LRESULT InventoryView::OnFindHide(WORD/*wNotifyCode*/, WORD/*wID*/, HWND/*hWndCtl*/, BOOL&/*bHandled*/)
{
    HideFindWindow();
    return 0;
}


LRESULT InventoryView::OnInfo(WORD/*wNotifyCode*/, WORD/*wID*/, HWND/*hWndCtl*/, BOOL&/*bHandled*/)
{
    if (!::IsWindowVisible(m_infoview))
    {
        m_infoview.ShowWindow(SW_SHOW);
    }
    else
    {
        m_infoview.ShowWindow(SW_HIDE);
    }

    m_toolbar.CheckButton(ID_INFO, m_infoview.IsWindowVisible());

    RECT rect;
    GetClientRect(&rect);
    CSize newsize(rect.right, rect.bottom);
    UpdateLayout(newsize);

    return 0;
}


LRESULT InventoryView::OnHelp(WORD/*wNotifyCode*/, WORD/*wID*/, HWND/*hWndCtl*/, BOOL&/*bHandled*/)
{
    m_gui->ShowHelp(_T("inventory"));
    return 0;
}


LRESULT InventoryView::onDoubleClick(LPNMHDR lParam) {
	int x = 9;

	NMITEMACTIVATE *pdi = (NMITEMACTIVATE*)lParam;
	unsigned int containerid;
	unsigned int charid;


	ItemListDataModelPtr data_model = boost::static_pointer_cast<ItemListDataModel>(m_datagrid->getModel());
	if (data_model->getItemContainerId(pdi->iItem, charid, containerid)) {

		if (containerid >= 1024) {
			std::tstringstream sql;
			sql << _T("owner = ") << charid << _T(" AND parent ");
			sql << _T(" = ") << containerid;
			UpdateListView(sql.str());

		
			m_datagrid->setSelectedItems(containerid);
		}
	}

	return 0;
}
LRESULT InventoryView::OnColumnClick(LPNMHDR lParam)
{
    LPNMLISTVIEW pnmv = (LPNMLISTVIEW)lParam;
	if (pnmv->hdr.idFrom == IDW_LISTVIEW)
	{
		if (m_sortColumn != pnmv->iSubItem)
		{
		    m_sortColumn = pnmv->iSubItem;
		    m_sortDesc = false;
		}
		else
		{
		    m_sortDesc = !m_sortDesc;
		}

		ItemListDataModelPtr data_model = boost::static_pointer_cast<ItemListDataModel>(m_datagrid->getModel());
		data_model->sortData(m_sortColumn, !m_sortDesc);
	}

    return 0;
}


LRESULT InventoryView::OnItemChanged(LPNMHDR lParam)
{
    LPNMLISTVIEW pnmv = (LPNMLISTVIEW)lParam;

    if (pnmv->uChanged & LVIF_STATE)
    {
        OnSelectionChanged();
    }

    return 0;
}


LRESULT InventoryView::OnItemContextMenu(LPNMHDR lParam)
{
    LPNMITEMACTIVATE param = (LPNMITEMACTIVATE)lParam;

    int sel = param->iItem;
    if (sel >= 0)
    {
        WTL::CPoint pos;
        GetCursorPos(&pos);

        WTL::CMenu menu;
        if (m_datagrid->getSelectedCount() > 1)
        {
            menu.LoadMenu(IDR_ITEM_POPUP_MULTISEL);
        }
        else
        {
            menu.LoadMenu(IDR_ITEM_POPUP);
        }
        WTL::CMenuHandle popup = menu.GetSubMenu(0);
        int cmd = popup.TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN, pos.x, pos.y, m_hWnd);
    }

    return 0;
}


LRESULT InventoryView::OnCopyItemName(WORD FromAccelerator, WORD CommandId, HWND hWndCtrl, BOOL& bHandled)
{
    if (m_datagrid->getSelectedCount() < 1)
    {
        return 0;
    }

    std::tstring text;

    std::set<unsigned int> selectedIndexes = m_datagrid->getSelectedItems();
    for (std::set<unsigned int>::iterator it = selectedIndexes.begin(); it != selectedIndexes.end(); ++it)
    {
        text += m_datagrid->getModel()->getItemProperty(*it, 0); // column 0 should be the item name.
        text += _T("\n");
    }

    SetClipboardText(text);
    return 0;
}


LRESULT InventoryView::OnCopyItemRef(WORD FromAccelerator, WORD CommandId, HWND hWndCtrl, BOOL& bHandled)
{
    if (m_datagrid->getSelectedCount() < 1)
    {
        return 0;
    }

    std::set<unsigned int> selectedIndexes = m_datagrid->getSelectedItems();

    ItemServer server;
    ExportFormat format;
    bool scriptOutput = false;

    switch (CommandId)
    {
    case ID_COPY_AUNO_ITEMREF:
        server = SERVER_AUNO;
        format = FORMAT_HTML;
        break;

    case ID_COPY_AUNO_ITEMREF_VBB:
        server = SERVER_AUNO;
        format = FORMAT_VBB;
        break;

    case ID_COPY_JAYDEE_ITEMREF:
        server = SERVER_JAYDEE;
        format = FORMAT_HTML;
        break;

    case ID_COPY_JAYDEE_ITEMREF_VBB:
        server = SERVER_JAYDEE;
        format = FORMAT_VBB;
        break;

    case ID_COPY_AO_ITEMREF:
        server = SERVER_AO;
        format = FORMAT_AO;
        scriptOutput = true;
        break;

    case ID_COPY_XYPHOS_ITEMREF:
        server = SERVER_XYPHOS;
        format = FORMAT_HTML;
        break;

    case ID_COPY_XYPHOS_ITEMREF_VBB:
        server = SERVER_XYPHOS;
        format = FORMAT_VBB;
        break;

    default:
        assert(false);  // Unknown command ID?
        break;
    }

    std::tstring urlTemplate = GetServerItemURLTemplate(server);
    std::tstring itemTemplate;
    std::tstring prefix;
    std::tstring postfix;
    std::tstring separator = _T("\r\n");

    switch (format)
    {
    case FORMAT_HTML:
        itemTemplate = _T("<a href=\"%url%\">%itemname%</a> [QL %ql%] [%itemlocation%]");
        separator = _T("<br />\r\n");
        break;

    case FORMAT_VBB:
        itemTemplate = _T("[url=%url%]%itemname%[/url] {QL %ql%} {%itemlocation%}");
        break;

    case FORMAT_AO:
        itemTemplate = _T("<a href=%url%>%itemname%</a> [QL %ql%] [%itemlocation%]");
        prefix = _T("Item Assistant list :: <a href=\"text://");
        postfix =
            _T(
            "<p><p><center>Generated by <a href='chatcmd:///start http://ia.frellu.net'>AOIA</a></center>\">view</a>");
        separator = _T("<br>");
        break;

    default:
        assert(false);  // Unknown type?
        break;
    }

    boost::replace_all(itemTemplate, _T("%url%"), urlTemplate);

    ItemListDataModelPtr model = boost::static_pointer_cast<ItemListDataModel>(m_datagrid->getModel());

    g_DBManager.Lock();
    std::tstring output = prefix;
    for (std::set<unsigned int>::iterator it = selectedIndexes.begin(); it != selectedIndexes.end(); ++it)
    {
        if (it != selectedIndexes.begin())
        {
            output += separator;
        }

        OwnedItemInfoPtr pItemInfo = g_DBManager.GetOwnedItemInfo(model->getItemIndex(*it));
        std::tstring containername = GetContainerNameForItem(pItemInfo);

        std::tstring itemlocation = pItemInfo->ownername;
        itemlocation += _T(" -> ");
        itemlocation += containername;
        boost::replace_all(itemlocation, _T("\""), _T("&quot;"));

        std::tstring itemStr = itemTemplate;
        boost::replace_all(itemStr, _T("%lowid%"), pItemInfo->itemloid);
        boost::replace_all(itemStr, _T("%hiid%"), pItemInfo->itemhiid);
        boost::replace_all(itemStr, _T("%ql%"), pItemInfo->itemql);
        boost::replace_all(itemStr, _T("%itemname%"), pItemInfo->itemname);
        boost::replace_all(itemStr, _T("%itemlocation%"), itemlocation);

        output += itemStr;
    }
    output += postfix;
    g_DBManager.UnLock();

    SetClipboardText(output);

    // Output to the aoia script in the AO script folder.
    if (scriptOutput)
    {
        if (output.length() <= 4096)
        {
            bfs::tpath scriptFile = bfs::tpath(AOManager::instance().getAOPrefsFolder()).parent_path() / _T("scripts") / _T("aoia");
#ifdef UNICODE
            std::wofstream ofs(scriptFile.native());
#else
            std::ofstream ofs(scriptFile.string());
#endif  // UNICODE
            ofs << output;

            static bool showMessage = true;

            if (showMessage)
            {
                MessageBox(_T("All selected items were copied to the clipboard AND written to the AO script called 'aoia' \r\nYou can access it ingame by typing /aoia"), _T("INFORMATION"),
                           MB_OK | MB_ICONINFORMATION);
                showMessage = false;
            }
        }
        else
        {
            MessageBox(_T("All selected items were copied to the clipboard, however Anarchy Online does not support scripts of more than 4096 bytes.\r\n\r\nTry selecting fewer items if you want the script file updated."), _T("ERROR"),
                       MB_OK | MB_ICONERROR);
        }
    }

    return 0;
}


LRESULT InventoryView::OnShowItemRef(WORD FromAccelerator, WORD CommandId, HWND hWndCtrl, BOOL& bHandled)
{
    if (m_datagrid->getSelectedCount() != 1)
    {
        return 0;
    }

    int activeItemIdx = *(m_datagrid->getSelectedItems().begin());
    ItemListDataModelPtr model = boost::static_pointer_cast<ItemListDataModel>(m_datagrid->getModel());
    unsigned int ownedItemIndex = model->getItemIndex(activeItemIdx);

    g_DBManager.Lock();
    OwnedItemInfoPtr pItemInfo = g_DBManager.GetOwnedItemInfo(ownedItemIndex);
    g_DBManager.UnLock();
    
    std::tstring containername = GetContainerNameForItem(pItemInfo);

    std::tstring itemlocation = pItemInfo->ownername;
    itemlocation += _T(" -> ");
    itemlocation += containername;

    TCHAR buffer[1024];
    if (CommandId == ID_VIEW_ITEMSTATS_AUNO)
    {
        ATL::AtlLoadString(IDS_AUNO_ITEMREF_URL, buffer, 1024);
    }
    else if (CommandId == ID_VIEW_ITEMSTATS_JAYDEE)
    {
        ATL::AtlLoadString(IDS_JAYDEE_ITEMREF_URL, buffer, 1024);
    }
    else if (CommandId == ID_VIEW_ITEMSTATS_XYPHOS)
    {
        ATL::AtlLoadString(IDS_XYPHOS_ITEMREF_URL, buffer, 1024);
    }
    else
    {
        assert(false); // Looks like you forgot to add a section to load the proper url for your command.
    }
    std::tstring url(buffer);

    if (!url.empty())
    {
        boost::replace_all(url, _T("%lowid%"), pItemInfo->itemloid);
        boost::replace_all(url, _T("%hiid%"), pItemInfo->itemhiid);
        boost::replace_all(url, _T("%ql%"), pItemInfo->itemql);
    }

    ShellExecute(NULL, _T("open"), url.c_str(), NULL, NULL, SW_SHOWDEFAULT);

    return 0;
}


LRESULT InventoryView::OnExportToCSV(WORD FromAccelerator, WORD CommandId, HWND hWndCtrl, BOOL& bHandled)
{
    if (m_datagrid->getSelectedCount() < 1)
    {
        return 0;
    }

    // Ask user for name of CSV file
    std::tstring filename = BrowseForOutputFile(GetTopLevelWindow(), _T("Select Name of Output File"), _T("CSV Files (Comma-Separated Values)\0*.csv\0\0"), _T("csv"));
    if (filename.empty())
    {
        return 0;
    }

    // Output to the selected CSV file.
    std::tofstream ofs(filename.c_str());
    if (!ofs.is_open())
    {
        return 0;
    }

    ItemServer server = SERVER_AUNO;
    if (CommandId == ID_EXPORTTOCSV_JAYDEE)
    {
        server = SERVER_JAYDEE;
    }
    else if (CommandId == ID_EXPORTTOCSV_XYPHOS)
    {
        server = SERVER_XYPHOS;
    }

    std::set<unsigned int> ids;
    GetSelectedItemIds(ids);

    CSVDataModelPtr model(new CSVDataModel(m_db, m_containerManager, ids, GetServerItemURLTemplate(server)));
    CSVExporter exporter;
    exporter.DoExport(ofs, model);

    return 0;
}


LRESULT InventoryView::OnRecordStatsToggle(WORD/*wNotifyCode*/, WORD/*wID*/, HWND/*hWndCtl*/, BOOL&/*bHandled*/)
{
    m_enableCharacterParserDumper = !m_enableCharacterParserDumper;

    m_toolbar.CheckButton(ID_RECORD_STATS_TOGGLE, m_enableCharacterParserDumper ? TRUE : FALSE);
    m_settings->setValue(_T("Record Stats"), m_enableCharacterParserDumper ? _T("yes") : _T("no"));

    return 0;
}


void InventoryView::HideFindWindow()
{
    if (::IsWindowVisible(m_findview))
    {
        m_datagrid->SetFocus();
        m_findview.ShowWindow(SW_HIDE);
    }

    RECT rect;
    GetClientRect(&rect);
    CSize newsize(rect.right, rect.bottom);
    UpdateLayout(newsize);

    m_toolbar.CheckButton(ID_INV_FIND_TOGGLE, FALSE);
}


LRESULT InventoryView::OnCreate(LPCREATESTRUCT createStruct)
{
    m_findview.Create(m_hWnd);
    //m_findview.ShowWindow(SW_SHOWNOACTIVATE);
    m_findview.SetParent(this);
    m_findview.SetDlgCtrlID(IDW_FINDVIEW);

    m_infoview.Create(m_hWnd);
    m_infoview.SetParent(this);
    m_infoview.SetDlgCtrlID(IDW_INFOVIEW);

    DWORD style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	//RECT myRect = { 100, 20, 440, createStruct->cy };
    m_splitter.Create(m_hWnd, rcDefault, NULL, style);
    m_splitter.SetSplitterExtendedStyle(0);
	
    m_treeview.Create(m_splitter.m_hWnd, rcDefault, NULL,
                      style | TVS_SHOWSELALWAYS | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_EDITLABELS,
                      WS_EX_CLIENTEDGE);
    m_treeview.SetDlgCtrlID(IDW_TREEVIEW);

    m_datagrid->Create(m_splitter.m_hWnd, rcDefault, NULL,
                       style | LVS_REPORT /*| LVS_SINGLESEL*/ | LVS_SHOWSELALWAYS | LVS_OWNERDATA, WS_EX_CLIENTEDGE);
    m_datagrid->SetDlgCtrlID(IDW_LISTVIEW);

	m_rootNode.reset(new AccountRootTreeViewItem(m_db, m_containerManager, this));
    m_treeview.addRootItem(m_rootNode.get());

    m_splitter.SetSplitterPanes(m_treeview, m_datagrid->m_hWnd);
    m_splitter.SetActivePane(SPLIT_PANE_LEFT);

    m_accelerators.LoadAccelerators(IDR_INV_ACCEL);

    TBBUTTON buttons[5];
    buttons[0].iBitmap = 0;
    buttons[0].idCommand = ID_INV_FIND_TOGGLE;
    buttons[0].fsState = TBSTATE_ENABLED;
    buttons[0].fsStyle = TBSTYLE_BUTTON | BTNS_SHOWTEXT | BTNS_CHECK | BTNS_AUTOSIZE;
    buttons[0].dwData = NULL;
    buttons[0].iString = (INT_PTR)_T("Find Item");

    buttons[1].iBitmap = 1;
    buttons[1].idCommand = ID_INFO;
    buttons[1].fsState = TBSTATE_ENABLED;
    buttons[1].fsStyle = TBSTYLE_BUTTON | BTNS_SHOWTEXT | BTNS_CHECK | BTNS_AUTOSIZE;
    buttons[1].dwData = NULL;
    buttons[1].iString = (INT_PTR)_T("Item Info");

    buttons[2].iBitmap = 0;
    buttons[2].idCommand = 0;
    buttons[2].fsState = 0;
    buttons[2].fsStyle = BTNS_SEP;
    buttons[2].dwData = NULL;
    buttons[2].iString = NULL;

    buttons[3].iBitmap = 3;
    buttons[3].idCommand = ID_RECORD_STATS_TOGGLE;
    buttons[3].fsState = TBSTATE_ENABLED;
    buttons[3].fsStyle = TBSTYLE_BUTTON | BTNS_SHOWTEXT | BTNS_CHECK | BTNS_AUTOSIZE;
    buttons[3].dwData = NULL;
    buttons[3].iString = (INT_PTR)_T("Record Stats");

    buttons[4].iBitmap = 2;
    buttons[4].idCommand = ID_HELP;
    buttons[4].fsState = TBSTATE_ENABLED;
    buttons[4].fsStyle = TBSTYLE_BUTTON | BTNS_SHOWTEXT | BTNS_AUTOSIZE;
    buttons[4].dwData = NULL;
    buttons[4].iString = (INT_PTR)_T("Help");

    CImageList imageList;
    imageList.CreateFromImage(IDB_INVENTORY_VIEW, 16, 2, CLR_DEFAULT, IMAGE_BITMAP,
                              LR_CREATEDIBSECTION | LR_DEFAULTSIZE);

    m_toolbar.Create(GetTopLevelWindow(), NULL, _T("InventoryViewToolBar"),
                     ATL_SIMPLE_TOOLBAR_PANE_STYLE | TBSTYLE_LIST,
                     TBSTYLE_EX_MIXEDBUTTONS);
    m_toolbar.SetButtonStructSize();
    //m_toolbar.AddBitmap(2, IDB_INVENTORY_VIEW);
    m_toolbar.SetImageList(imageList);
    m_toolbar.AddButtons(ARRAYSIZE(buttons), buttons);
    m_toolbar.AutoSize();

    m_toolbar.CheckButton(ID_RECORD_STATS_TOGGLE, m_enableCharacterParserDumper ? TRUE : FALSE);

    PostMessage(WM_POSTCREATE);

    return 0;
}


void InventoryView::UpdateLayout(CSize newSize)
{
    CRect r(CPoint(0, 0), newSize);

    if (::IsWindowVisible(m_findview))
    {
        RECT fvRect;
        m_findview.GetWindowRect(&fvRect);
        int height = fvRect.bottom - fvRect.top;
        ::SetWindowPos(m_findview, 0, r.left, r.top, r.Width(), height,
                       SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE | SWP_DEFERERASE | SWP_NOSENDCHANGING);
        r.top += height;
    }

    if (::IsWindowVisible(m_infoview))
    {
        RECT fvRect;
        m_infoview.GetWindowRect(&fvRect);
        int width = fvRect.right - fvRect.left;
        ::SetWindowPos(m_infoview, 0, r.right - width, r.top, width, r.Height(),
                       SWP_NOZORDER | SWP_NOACTIVATE | SWP_DEFERERASE | SWP_NOSENDCHANGING);
        r.right -= width;
    }

    m_splitter.SetWindowPos(NULL, r.left, r.top, r.Width(), r.Height(),
                            SWP_NOZORDER | SWP_NOACTIVATE | SWP_DEFERERASE | SWP_NOSENDCHANGING);
}


LRESULT InventoryView::OnSize(UINT wParam, CSize newSize)
{
    UpdateLayout(newSize);
    return 0;
}


LRESULT InventoryView::OnPostCreate(UINT/*uMsg*/, WPARAM/*wParam*/, LPARAM/*lParam*/, BOOL&/*bHandled*/)
{
    //SetSplitterPos works best after the default WM_CREATE has been handled
    m_splitter.SetSplitterPos(296);
	
    CTreeItem item = m_treeview.GetRootItem();

	if (item.HasChildren())
	{
		item.Expand();
		CTreeItem cItem = item.GetChild();
		while (!cItem.IsNull())
		{
			if (cItem.HasChildren())
			{
				cItem.Expand();
			}
			cItem = cItem.GetNext(TVGN_NEXT);
		}
	}

	m_treeview.SelectItem(item);

	BOOL b = 1;
	OnFindToggle(0, 0, 0, b);
	m_toolbar.CheckButton(ID_INV_FIND_TOGGLE, TRUE);
    return 0;
}


void InventoryView::OnAOClientMessage(AOClientMessageBase& msg)
{
    switch (msg.messageId())
    {
    case AO::MSG_GIVE_TO_NPC:
        {
            LOG(_T("MSG_GIVE_TO_NPC"));
            Native::AOGiveToNPC npAction((AO::GiveToNPC*)msg.start(), false);

            TRACE(npAction.print());

            //if (npAction.fromId().High() == npAction.charid())
            {
                unsigned int fromContainerId = GetFromContainerId(msg.characterId(), npAction.fromType(),
                                                                  npAction.fromContainerTempId());
                unsigned int toContainer = aoia::INV_TRADE;

                if (npAction.direction() == 1)
                {
                    toContainer = aoia::INV_TOONINV;
                    assert(fromContainerId == aoia::INV_TRADE);
                }

                g_DBManager.Lock();
                m_db->Begin();

                unsigned int nextFreeInvSlot = g_DBManager.FindNextAvailableContainerSlot(npAction.charid(),
                                                                                          toContainer);

                {
                    std::tstringstream sql;

                    sql << _T("UPDATE tItems SET parent = ") << toContainer
                        << _T(", slot = ") << nextFreeInvSlot
                        << _T(" WHERE parent = ") << fromContainerId
                        << _T(" AND slot = ") << npAction.fromItemSlotId()
                        << _T(" AND owner = ") << msg.characterId(); //TODO: remove from other monitored char with fromId?

                    m_db->Exec(sql.str());

                    //OutputDebugString(sql.str().c_str());
                }

                m_db->Commit();
                g_DBManager.UnLock();
            }

        }
        break;

    case AO::MSG_END_NPC_TRADE:
        {
            LOG(_T("MSG_END_NPC_TRADE"));
            Native::AOEndNPCTrade npAction((AO::EndNPCTrade*)msg.start(), false);

            TRACE(npAction.print());

            //if (npAction.fromId().High() == npAction.charid())
            {

                if (npAction.operation() != 01) //01 is close
                {
                    if (npAction.operation() != 00) //00 is accept, handle on server message with more data.
                    {
                        TRACE(_T("Unknown MSG_END_NPC_TRADE Operation"));
                    }

                    return;
                }

                //move stuff back!
                unsigned int shopCapacity = 35;
                unsigned int newParent = aoia::INV_TOONINV;
                unsigned int shopContainer = aoia::INV_TRADE;

                g_DBManager.Lock();
                m_db->Begin();

                for (unsigned int i = 0; i <= shopCapacity; i++) //or is it the other direction?
                {
                    unsigned int nextFreeInvSlot = g_DBManager.FindNextAvailableContainerSlot(msg.characterId(),
                                                                                              newParent);
                    if (nextFreeInvSlot >= 94)
                    {
                        //if full, move to overflow. 
                        //This seems to crash with a MSG_CONTAINER on overflow before this,
                        //even if overflow already open.
                        //if we dont break here, we get duplicate items in IA.
                        break;
                        // We cant do this since it will dupe items in overflow:
                        //newParent = aoia::INV_OVERFLOW;
                        //nextFreeInvSlot = g_DBManager.findNextAvailableContainerSlot(item.charid(), aoia::INV_OVERFLOW);
                    }

                    std::tstringstream sqlMoveBack;
                    //move stuff back
                    sqlMoveBack << _T("UPDATE tItems SET parent = ") << newParent
                        << _T(", slot = ") << nextFreeInvSlot
                        << _T(" WHERE parent = ") << shopContainer
                        << _T(" AND slot = ") << i
                        << _T(" AND owner = ") << msg.characterId();
                    m_db->Exec(sqlMoveBack.str());
                }

                m_db->Commit();
                g_DBManager.UnLock();
            }
        }
        break;

    case AO::MSG_CHAR_OPERATION: //0x5e477770
        {
            LOG(_T("MSG_CHAR_OPERATION"));

            Native::AOCharacterAction itemOp((AO::CharacterAction*)msg.start(), false);

            unsigned int opId = itemOp.operationId();

            //TODO: switch statement
            if (opId == AO::CHAR_ACTION_SPLITSTACK)
            {
                LOG(_T("CHAR_ACTION_SPLITSTACK"));

                //split! itemHigh is the count of the new item. this gets a new slotId
                //the new item gets the count of the existing.
                //the one with 

                g_DBManager.Lock();
                m_db->Begin();
                unsigned int fromContainerId = GetFromContainerId(msg.characterId(), itemOp.fromType(),
                                                                  itemOp.fromContainerTempId());
                if (fromContainerId == 0)
                {
                    TRACE(_T("From container not found!"));
                    return; //we dont have the value cached, either a bug or ia was started after the bp was opened. Or unknown from type
                }

                unsigned int nextFreeInvSlot = g_DBManager.FindNextAvailableContainerSlot(msg.characterId(),
                                                                                          fromContainerId);

                std::tstringstream sqlInsert;
                sqlInsert << _T("INSERT INTO tItems (keylow, keyhigh, ql, stack, parent, slot, children, owner)")
                    << _T(" SELECT keylow, keyhigh, ql, ") << itemOp.itemId().High()
                    << _T(", parent, ") << nextFreeInvSlot << _T(", children, owner FROM tItems")
                    << _T(" WHERE owner = ") << msg.characterId()
                    << _T(" AND parent = ") << fromContainerId
                    << _T(" AND slot = ") << itemOp.fromItemSlotId();
                m_db->Exec(sqlInsert.str());

                {
                    std::tstringstream sql;
                    sql << _T("UPDATE tItems SET stack = stack - ") << itemOp.itemId().High()
                        << _T(" WHERE owner = ") << msg.characterId()
                        << _T(" AND parent = ") << fromContainerId
                        << _T(" AND slot = ") << itemOp.fromItemSlotId();
                    m_db->Exec(sql.str());
                }
                m_db->Commit();
                g_DBManager.UnLock();
            }
            else if (opId == AO::CHAR_ACTION_JOINSTACKS)
            {
                LOG(_T("CHAR_ACTION_JOINSTACKS"));
                return;//handled on server message!
            }
            else if (opId == AO::CHAR_ACTION_DELETEITEM)
            {
                LOG(_T("CHAR_ACTION_DELETEITEM"));
                //user deleted an item
                //handled on server side aswell (without keylow/high).

                g_DBManager.Lock();
                m_db->Begin();
                unsigned int fromContainerId = GetFromContainerId(msg.characterId(), itemOp.fromType(),
                                                                  itemOp.fromContainerTempId());
                if (fromContainerId == 0)
                {
                    TRACE(_T("From container not found!"));
                    return; //we dont have the value cached, either a bug or ia was started after the bp was opened. Or unknown from type
                }

                //user deleted an item!
                {
                    std::tstringstream sql;
                    sql << _T("DELETE FROM tItems WHERE owner = ") << msg.characterId()
                        << _T(" AND parent = ") << fromContainerId
                        << _T(" AND keylow = ") << itemOp.itemId().Low()
                        << _T(" AND keyhigh = ") << itemOp.itemId().High()
                        << _T(" AND slot = ") << itemOp.fromItemSlotId();
                    m_db->Exec(sql.str());
                }
                TRACE(itemOp.print());
                m_db->Commit();
                g_DBManager.UnLock();
            }
            else if (opId == AO::CHAR_ACTION_RUN_NANO)
            {
                LOG(_T("CHAR_ACTION_RUN_NANO"));
                //when you run a nano, this one fires
                return;
            }
            else if (opId == AO::CHAR_ACTION_RUN_PERK)
                //0x69  from = 0c350+ a char Id = ?
            {
                LOG(_T("CHAR_ACTION_RUN_PERK"));
                //When you hit a perk, this one fires
                return;
            }
            else if (opId == AO::CHAR_ACTION_UNKNOWN1) //0x69  from = 0c350+ a char Id
            {
                LOG(_T("CHAR_ACTION_UNKNOWN1"));
                //occationally, this one
                return;
            }
            else if (opId == AO::CHAR_ACTION_TRADESKILL)
            {
                LOG(_T("CHAR_ACTION_TRADESKILL"));
                //tradeskill. do nothing. we will get delete/trade operations for all.
            }
            else if (opId == AO::CHAR_ACTION_LOGOFF1 || opId == AO::CHAR_ACTION_LOGOFF2)
            {
                LOG(_T("CHAR_ACTION_LOGOFF"));
                //All zeroes, when you log off
                return;
            }
            else if (opId == AO::CHAR_ACTION_SNEAK)
            {
                LOG(_T("CHAR_ACTION_SNEAK"));
                return;
            }
            else if (opId == AO::CHAR_ACTION_STAND)
            {
                LOG(_T("CHAR_ACTION_STAND"));
                return;
            }
            else
            {
                LOG(_T("Unknown client operation Id: ") << std::hex << opId);
                LOG(itemOp.print());
                return;
            }
        }
        break;

    case 0x1b: //zone
        {
        }
        break;

    case AO::MSG_OPENBACKPACK:// 0x52526858://1196653092:
        {
            LOG(_T("MSG_OPENBACKPACK"));
            return;//handled on server msg. 
            //Since the server msg is only sent once, we might want to update tempId here, but it shouldnt be needeed.
            // Native::AOOpenBackpackOperation moveOp((AO::OpenBackpackOperation*)msg.start());
        }
        break;

    case AO::MSG_ITEM_MOVE: //0x47537a24://1196653092:
        {
            LOG(_T("MSG_ITEM_MOVE"));
            //we handle this on the server message.
            return;
        }
        break;

    default:
        break;
    }
}


unsigned int InventoryView::GetFromContainerId(unsigned int charId, unsigned short fromType, unsigned short fromSlotId)
{
    if (fromType == 0x006b) //backpack. fromSlotId contains a temp container id.
    {
        return m_tempContainers->GetContainerId(charId, fromSlotId);
    }
    else if (fromType == 0x0068//inv
        || fromType == 0x0065  //utils, hud, ncu, weap pane
        || fromType == 0x0073  //social pande
        || fromType == 0x0067  //implants
        || fromType == 0x0066) //wear neck,sleeve
    {
        return aoia::INV_TOONINV;
    }
    else if (fromType == 0x0069)//bank
    {
        return aoia::INV_BANK;
    }
    else if (fromType == 0x006f)//trade window
    {
        TRACE(_T("Move from trade remote party ignored"));
        return 0;
    }
    else if (fromType == 0xc767)//shop inventory
    {
        TRACE(_T("Move from shop inventory ignored"));//Handled in ItemBought message.
        return 0;
    }
    else if (fromType == 0xc790)//51088 player shop inventory
    {
        TRACE(_T("Move from player shop inventory ignored"));//Not handled currently!
        return 0;
    }
    else if (fromType == 0x006c)//shop/trade window.. could be remote or local. If needed, add an owner charId to compare with to func.
    {
        TRACE(_T("Move from trade win"));
        return aoia::INV_TRADE;
    }
    else if (fromType == 0x006e)//overflow or tradeskill temp window
    {
        TRACE(_T("Move from overflow"));
        return aoia::INV_OVERFLOW;
    }
    else
    {
        TRACE(_T("TODO: UNKNOWN FROM TYPE ") << std::hex << fromType);
        return 2; //we assume inventory o.O
    }
}

void InventoryView::WriteWaypointsScript() {
	if (timeGetTime() > m_lastUpdateCharpos + 3000) {
		std::tstring path = AOManager::instance().getAOPrefsFolder() + _T("\\..\\scripts\\wp");



		DWORD maxAllowedTime = timeGetTime() - (5*60*1000);
		std::tstring script = _T("<a href=\"text://");
		for (auto it : m_characterPositionMap) {
			if (it.second.characterName.empty()) {
				g_DBManager.Lock();
				it.second.characterName = g_DBManager.GetToonName(it.first);
				g_DBManager.UnLock();
			}

			if (it.second.lastSeen > maxAllowedTime) {
				std::tstringstream st;
				std::tstring zoneMap = _T("Unknown map");
				if (m_zones_sl.find(it.second.zoneid) != m_zones_sl.cend())
					zoneMap = _T("SL map");
				else if (m_zones_rk.find(it.second.zoneid) != m_zones_rk.cend())
					zoneMap = _T("RK map");

				st << _T("<a href='chatcmd:///waypoint ") << (int)it.second.x << _T(" ") << (int)it.second.z << _T(" ") << it.second.zoneid << _T("'>") 
					<< it.second.characterName << _T(": Upload waypoint to map</a> (") << zoneMap << _T(")<br>");
				script += st.str();
			}
		}
	
		script += _T("---<br><a href='chatcmd:///waypoint . . .'>Remove uploaded waypoint</a>\">Click here for waypoint</a>");
		m_lastUpdateCharpos = timeGetTime();

		std::tofstream outfile(path);
		outfile << script;
		outfile.close();
	}
}


void InventoryView::OnAOServerMessage(AOMessageBase& msg)
{
    if (m_enableCharacterParserDumper)
    {
        m_characterParserDumper.OnAOServerMessage(msg);
    }

	/*
	{
		AOMessageBase copy = msg;
		std::vector<char> buf;
		while (copy.remaining() > 0)
			buf.push_back(copy.popChar());

		char* name = "34-I Helper";
		int strle = sizeof(name) - 2;
		if (buf.size() >= strle) {
			for (int i = 0; i < buf.size()-strle-1; i++) {
				for (int j = 0; j < strle-1; j++) {
					if (buf[i+j] != name[j])
						break;
					if (j >= strle-2) {
						std::stringstream st;
						st << "Voila: " << msg.messageId() << "\n";
						OutputDebugStringA(st.str().c_str());
					}
				}
			}
		}
	}*/

    switch (msg.messageId())
    {
    case AO::MSG_ACCEPT_NPC_TRADE:
        {
            Native::AONPCTradeAccept npcAccept((AO::NPCTradeAcceptBase*)msg.start(), true);

            if (npcAccept.targetId() != npcAccept.charid())//skip messages not for me.
            {
                return;
            }

            TRACE(npcAccept.print());

            //move rejected stuff back to inv:
            unsigned int count = npcAccept.itemCount();
            unsigned int newParent = aoia::INV_TOONINV;
            unsigned int shopContainer = aoia::INV_TRADE;

            g_DBManager.Lock();
            m_db->Begin();

            for (unsigned int i = 0; i < count; i++) //or is it the other direction?
            {
                TRACE(npcAccept.rejectedItem(i).print());

                unsigned int nextFreeInvSlot = g_DBManager.FindNextAvailableContainerSlot(msg.characterId(),
                                                                                          newParent);

                if (nextFreeInvSlot >= 94)
                {
                    //if full, move to overflow. 
                    //This seems to crash with a MSG_CONTAINER on overflow before this,
                    //even if overflow already open.
                    //if we dont break here, we get duplicate items in IA.
                    break;
                    // We cant do this since it will dupe items in overflow:
                    //newParent = aoia::INV_OVERFLOW;
                    //nextFreeInvSlot = g_DBManager.findNextAvailableContainerSlot(item.charid(), aoia::INV_OVERFLOW);
                }

                std::tstringstream sqlMoveBack;
                //move stuff back
                //Todo: Move only the first one found to preserve order in case 2 of same ql are
                //returned with other stuff in between.??

                sqlMoveBack << _T("UPDATE tItems SET parent = ") << newParent
                    << _T(", slot = ") << nextFreeInvSlot
                    << _T(" WHERE parent = ") << shopContainer
                    << _T(" AND keylow = ") << npcAccept.rejectedItem(i).itemId().Low()
                    << _T(" AND keyhigh = ") << npcAccept.rejectedItem(i).itemId().High()
                    << _T(" AND ql = ") << npcAccept.rejectedItem(i).ql()
                    << _T(" AND owner = ") << msg.characterId();
                m_db->Exec(sqlMoveBack.str());
            }

            //and delete the stuff I gave away:
            {
                std::tstringstream sqlGiveAwayStuff;
                //move stuff back
                //Todo: Move only the first one found to preserve order in case 2 of same ql are
                //returned with other stuff in between.??

                sqlGiveAwayStuff << _T("DELETE FROM tItems WHERE owner = ") << npcAccept.charid()
                    << _T(" AND parent = ") << shopContainer;

                m_db->Exec(sqlGiveAwayStuff.str());
            }

            m_db->Commit();
            g_DBManager.UnLock();
        }
        break;

    case AO::MSG_CHAR_OPERATION: //0x5e477770
        {
            //    LOG(_T("MSG_CHAR_OPERATION"));

            Native::AOCharacterAction charOp((AO::CharacterAction*)msg.start(), true);

            if (charOp.targetId() != charOp.charid())//skip messages not for me.
            {
                return;
            }
            // TRACE(charOp.print());

            unsigned int opId = charOp.operationId();

            //TODO: switch statement
            if (opId == AO::CHAR_ACTION_DELETEITEM) //delete (user or tradeskill)
            {
                LOG(_T("CHAR_ACTION_DELETEITEM"));

                //user deleted an item
                g_DBManager.Lock();
                m_db->Begin();
                unsigned int fromContainerId = GetFromContainerId(charOp.charid(), charOp.fromType(),
                                                                  charOp.fromContainerTempId());
                if (fromContainerId == 0)
                {
                    TRACE(_T("From container not found!"));
                    return; //we dont have the value cached, either a bug or ia was started after the bp was opened. Or unknown from type
                }

                //server deleted an item!
                {
                    std::tstringstream sql;
                    sql << _T("DELETE FROM tItems WHERE owner = ") << charOp.charid()
                        << _T(" AND parent = ") << fromContainerId
                        //<< _T(" AND keylow = ") << itemOp.itemId().Low()
                        //<< _T(" AND keyhigh = ") << itemOp.itemId().High()
                        << _T(" AND slot = ") << charOp.fromItemSlotId();

                    m_db->Exec(sql.str());
                }

                m_db->Commit();
                g_DBManager.UnLock();
            }
            else if (opId == AO::CHAR_ACTION_DELETE_TEMP_ITEM)
            {
                LOG(_T("CHAR_ACTION_DELETE_TEMP_ITEM"));
                TRACE(charOp.print());

                //A quest item (key or use-item) gets deleted, only temp id sent in here.
                //we got this id from a MSG_SPAWN_REWARD before a full sync or when item was spawned

                //the id is stored in  charOp.fromContainerTempId() + charOp.fromItemSlotId()

                unsigned int fromContainerId = aoia::INV_TOONINV;
                unsigned int charId = charOp.charid();
                unsigned int itemTempId = (charOp.fromContainerTempId() << 16) + charOp.fromItemSlotId();
                unsigned int realSlotId = m_tempContainers->GetItemSlotId(charId, itemTempId);

                if (realSlotId > 0)
                {
                    std::tstringstream sql;
                    sql << _T("DELETE FROM tItems WHERE owner = ") << charId
                        << _T(" AND parent = ") << fromContainerId
                        << _T(" AND slot = ") << realSlotId;

                    m_db->Exec(sql.str());
                    //OutputDebugString(sql.str().c_str());
                }

                return;
            }
            else if (opId == AO::CHAR_ACTION_JOINSTACKS)
            {
                LOG(_T("CHAR_ACTION_JOINSTACKS"));
                /*
                [9352] from Server: CHAR_ACTION_JOINSTACKS
                [9352] AOCharacterAction:MsgId 5e477770 CharId 2568996070 Target 2568996070 
                [9352] operationId  0x35 
                [9352] unknown3  0x0 
                [9352] fromType  0x68 
                [9352] fromContainerTempId 0 
                [9352] fromItemSlotId 58 
                [9352] itemId [104|90]
                */

                TRACE(charOp.print());

                //user joined two stacks!
                //itemHigh is the container id and slot id of the other item (gets deleted)
                //itemLow is the fromType of the other item

                unsigned short removedItemContType = charOp.itemId().Low() & 0xff;
                unsigned short removedItemContTempId = charOp.itemId().High() >> 16;
                unsigned short removedItemSlotId = charOp.itemId().High() & 0xff;
                unsigned int removedItemContainerId = GetFromContainerId(charOp.charid(), removedItemContType,
                                                                         removedItemContTempId);
                unsigned int fromContainerId = GetFromContainerId(charOp.charid(), charOp.fromType(),
                                                                  charOp.fromContainerTempId());
                if (fromContainerId == 0)
                {
                    TRACE(_T("From container not found!"));
                    return; //we dont have the value cached, either a bug or ia was started after the bp was opened. Or unknown from type
                }

                g_DBManager.Lock();
                m_db->Begin();

                std::tstringstream sqlUpd;
                sqlUpd << _T("UPDATE tItems SET stack = ")
                    << _T(" (stack + (SELECT tItems2.stack FROM tItems tItems2 WHERE")
                    << _T(" tItems2.owner = ") << charOp.charid()
                    << _T(" AND tItems2.parent = ") << removedItemContainerId
                    << _T(" AND tItems2.slot = ") << removedItemSlotId << _T("))")
                    << _T(" WHERE owner = ") << charOp.charid()
                    << _T(" AND parent = ") << fromContainerId
                    << _T(" AND slot = ") << charOp.fromItemSlotId();

                //OutputDebugString(sqlUpd.str().c_str());
                m_db->Exec(sqlUpd.str());

                {
                    std::tstringstream sql;
                    sql << _T("DELETE FROM tItems WHERE owner = ") << charOp.charid()
                        << _T(" AND parent = ") << removedItemContainerId
                        << _T(" AND slot = ") << removedItemSlotId;//itemId().High();

                    //OutputDebugString(sql.str().c_str());
                    m_db->Exec(sql.str());
                }
                m_db->Commit();
                g_DBManager.UnLock();

            }
#ifdef DEBUG
            else if (opId == AO::CHAR_ACTION_RUN_NANO || opId == AO::CHAR_ACTION_NANO_RAN)
            {
                //when you run a nano, this one fires
                return;
            }
            else if (opId == AO::CHAR_ACTION_RUN_PERK || opId == AO::CHAR_ACTION_SKILL_AVAIL)
                //0x69  from = 0c350+ a char Id = ?
            {
                //When you hit a perk, this one fires
                return;
            }
            else if (opId == AO::CHAR_ACTION_UNKNOWN1 || opId == AO::CHAR_ACTION_UNKNOWN2 )
            {
                //occationally, this one
                return;
            }
            else if (opId == AO::CHAR_ACTION_TRADESKILL)
            {
                //tradeskill. do nothing. we will get delete/trade operations for all.
            }
            else if (opId == AO::CHAR_ACTION_LOGOFF1 ||opId == AO::CHAR_ACTION_LOGOFF2 || opId == AO::CHAR_ACTION_ZONE)
            {
                //All zeroes, when you log off
                return;
            }
            else if (opId == AO::CHAR_ACTION_STAND || opId == AO::CHAR_ACTION_SNEAK  || opId == AO::CHAR_ACTION_MOVED || opId == AO::CHAR_ACTION_JUMP)
            {
                return;
            }
            else
            {
                LOG(_T("From Server: Unknown CHAR_ACTION: (check if SPLITSTACKS CAN BE DETECTED"));
                TRACE(charOp.print());
            }
#endif;
        }
        break;
        //TODO: Trades with backpacks
        //TODO: Test MSG_CONTAINER and the new overflow container detection.
        //TODO: Tradeskills - test with and without overflow - and what happens to items that get deleted?
        //TODO: add/remove from playershop

        //TODO: item move to inventory when stackable exists joins to lowest slot id stack
        //TODO: item rewards (check how you receive key/item on rk mish)
        //TODO: item depletion (ammo, use stims etc (last stim important))
    case AO::MSG_PARTNER_TRADE://0x35505644
        {
            LOG(_T("MSG_PARTNER_TRADE"));

            //when trade partner adds/removes items to trade window
            Native::AOPartnerTradeItem item((AO::PartnerTradeItem*)msg.start());

            if (item.targetId() == msg.characterId()) //else we get other ppl using stims etc.. :)
            {
                unsigned int opId = item.operationId();
#ifdef DEBUG

                // if (opId == 0x07 || opId == 0x06) //vicinity item swaps!

                if (opId == 0x20)//when perking a mob
                {
                    return;
                }
                else
                {
                    //OutputDebugString(_T("Unknown trade operation id:"));
                    TRACE(item.print());
                }
#endif

                if (opId == 0x55) //trade partner adds an item
                {
                    unsigned int otherTradeContainer = aoia::INV_TRADEPARTNER;
                    g_DBManager.Lock();
                    m_db->Begin();

                    unsigned int nextFreeInvSlot = g_DBManager.FindNextAvailableContainerSlot(item.charid(),
                                                                                              otherTradeContainer);
                    g_DBManager.InsertItem(item.itemid().Low(),
                                           item.itemid().High(),
                                           item.ql(),
                                           item.flags(),
                                           item.stack(),
                                           otherTradeContainer,
                                           nextFreeInvSlot,
                                           0,
                                           item.charid());

                    m_db->Commit();
                    g_DBManager.UnLock();
                }
                else if (opId == 0x56) //trade partner removes an item
                {
                    if (item.partnerFromType() == 0x6c)//remove from trade window
                    {
                        unsigned int otherTradeContainer = aoia::INV_TRADEPARTNER;
                        g_DBManager.Lock();
                        m_db->Begin();

                        std::tstringstream sql;

                        sql << _T("DELETE FROM tItems")
                            << _T(" WHERE parent = ") << otherTradeContainer
                            << _T(" AND slot = ") << item.partnerFromItemSlotId()
                            << _T(" AND owner = ") << item.charid();
                        m_db->Exec(sql.str());
                        //OutputDebugString(sql.str().c_str());

                        m_db->Commit();
                        g_DBManager.UnLock();
                    }
                }
                else if (opId == 0x57)//tradeskill result added to overflow
                {
                    // if no overflow window open, it goes in there, and gets moved to inv at once (MSG_ITEM_MOVE).
                    // if overflow window open, it goes there and an MSG_CONTAINER msg follows
                    unsigned int otherTradeContainer = aoia::INV_OVERFLOW;

                    if (item.partnerFromType() != 0x6e)
                    {
#ifdef DEBUG
                        TRACE(_T("Unexpected partner from type!"));
                        TRACE(item.print());
#endif
                        return;
                    }

                    unsigned int slotId = item.partnerFromItemSlotId();

                    g_DBManager.Lock();
                    m_db->Begin();

                    if (slotId >= 0x6f)
                        slotId = g_DBManager.FindNextAvailableContainerSlot(item.charid(), otherTradeContainer);

                    g_DBManager.InsertItem(item.itemid().Low(),
                                           item.itemid().High(),
                                           item.ql(),
                                           item.flags(),
                                           item.stack(),
                                           otherTradeContainer,  // 2 = inventory
                                           slotId,
                                           0,
                                           item.charid());

                    /* {
                    std::tstringstream logS;
                    logS << _T("~INSERT (") << item.itemid().Low() << _T("/") << item.ql() << _T("/") << item.stack()
                    << _T(") WHERE parent = ") << otherTradeContainer
                    << _T(" AND slot = ") << slotId
                    << _T(" AND owner = ") << item.charid();
                    OutputDebugString(logS.str().c_str());
                    }*/

                    m_db->Commit();
                    g_DBManager.UnLock();

                }
                else if (opId == 0x03)//item used/deleted if stack=1 and depleteable
                {
                    //Checking deplete and delete if stack=0.

                    //TODO: Things that do not deplete by 1 each use. (ammo stacks)
                    //Box of ammo did not deplete

                    unsigned int fromContainerId = GetFromContainerId(item.charid(),
                                                                      item.partnerFromType(),
                                                                      item.partnerFromContainerTempId());

                    g_DBManager.Lock();
                    m_db->Begin();

                    std::tstringstream sqlDeplete;
                    sqlDeplete << _T("UPDATE tItems SET stack=(stack-1)")
                        << _T(" WHERE parent = ") << fromContainerId
                        << _T(" AND slot = ") << item.partnerFromItemSlotId()
                        << _T(" AND owner = ") << msg.characterId()
                        << _T(" AND keyhigh = ") << item.itemid().High()
                        << _T(" AND keylow = ") << item.itemid().Low()
                        << _T(" AND keylow IN (SELECT aoid FROM tblao WHERE aoid = ") << item.itemid().Low()
                        << _T(" AND (flags & 4196368))");//FLAG_USE_EMPTY_DESTRUCT+FLAG_HAS_ENERGY+FLAG_TURN_ON_USE

                    m_db->Exec(sqlDeplete.str());
#ifdef DEBUG
                    TRACE(sqlDeplete.str());
#endif
                    ////2048 == ItemFlag::FLAG_USE_EMPTY_DESTRUCT
                    //SELECT * from tblAO WHERE (flags & 2048) order by name
                    //OR FLAG_HAS_ENERGY         = 0x00400000,
                    //OR FLAG_TURN_ON_USE        = 0x00000010,
                    std::tstringstream sqlDelete;
                    /*sqlDelete << _T("DELETE FROM tItems WHERE owner = ") << msg.characterId() 
                    << _T(" AND stack = 0")
                    << _T(" AND parent = ") << fromContainerId
                    << _T(" AND keylow = ") << item.itemid().Low()
                    << _T(" AND keyhigh = ") << item.itemid().High()
                    << _T(" AND slot = ") << item.partnerFromItemSlotId();*/

                    sqlDelete << _T("DELETE FROM tItems WHERE owner = ") << msg.characterId()
                        << _T(" AND stack = 0")
                        << _T(" AND parent = ") << fromContainerId
                        << _T(" AND keylow = ") << item.itemid().Low()
                        << _T(" AND keyhigh = ") << item.itemid().High()
                        << _T(" AND slot = ") << item.partnerFromItemSlotId()
                        << _T(" AND keylow IN (SELECT aoid FROM tblao WHERE aoid = ") << item.itemid().Low()
                        << _T(" AND (flags & 4196368))");//FLAG_USE_EMPTY_DESTRUCT+FLAG_HAS_ENERGY+FLAG_TURN_ON_USE

                    m_db->Exec(sqlDelete.str());
#ifdef DEBUG
                    TRACE(sqlDelete.str());
#endif

                    m_db->Commit();
                    g_DBManager.UnLock();
                }
            }

        }
        break;

    case AO::MSG_SHOP_TRANSACTION: //0x36284f6e 
        {
            LOG(_T("MSG_SHOP_TRANSACTION"));

            Native::AOTradeTransaction item((AO::TradeTransaction*)msg.start(), true);

            if (item.targetId() != item.charid())//skip messages not for me.
            {
                return;
            }

#ifdef DEBUG
            TRACE(item.print());
#endif

            unsigned int shopContainer = aoia::INV_TRADE;
            unsigned int otherTradeContainer = aoia::INV_TRADEPARTNER;//trade partner

            //we move stuff to parent=4 when in a trade
            switch (item.operationId())
            {
                //00=start trade? 01=client accept, 02=decline,03=tempAccept, 04=commit,05=add item,06=remove item
            case 0x00:
                {
                    g_DBManager.Lock();
                    m_db->Begin();

                    std::tstringstream sql;

                    //start trade?
                    sql << _T("DELETE FROM tItems WHERE (parent = ") << shopContainer
                        << _T(" OR parent = ") << otherTradeContainer << _T(")")
                        << _T(" AND owner = ") << item.charid();

                    m_db->Exec(sql.str());

                    m_db->Commit();
                    g_DBManager.UnLock();
                }

            case 0x01:
                {
                    //01=client accept //do nothing
                }
                break;

            case 0x02:
                {
                    //02=decline or cancel//move my stuff back
                    g_DBManager.Lock();
                    m_db->Begin();

                    unsigned int shopCapacity = 35;
                    unsigned int newParent = aoia::INV_TOONINV;

                    for (unsigned int i = 0; i <= shopCapacity; i++) //or is it the other direction?
                    {
                        unsigned int nextFreeInvSlot = g_DBManager.FindNextAvailableContainerSlot(item.charid(),
                                                                                                  newParent);
                        if (nextFreeInvSlot >= 94)
                        {
                            //if full, move to overflow. 
                            //This seems to crash with a MSG_CONTAINER on overflow before this,
                            //even if overflow already open.
                            //if we dont break here, we get duplicate items in IA.
                            break;
                            // We cant do this since it will dupe items in overflow:
                            //newParent = aoia::INV_OVERFLOW;
                            //nextFreeInvSlot = g_DBManager.findNextAvailableContainerSlot(item.charid(), aoia::INV_OVERFLOW);
                        }

                        std::tstringstream sqlMoveBack;
                        //move stuff back
                        sqlMoveBack << _T("UPDATE tItems SET parent = ") << newParent
                            << _T(", slot = ") << nextFreeInvSlot
                            << _T(" WHERE parent = ") << shopContainer
                            << _T(" AND slot = ") << i
                            << _T(" AND owner = ") << item.charid();
                        m_db->Exec(sqlMoveBack.str());
                        //OutputDebugString(sqlMoveBack.str().c_str());
                    }

                    //delete trade partner added items:
                    std::tstringstream sql;
                    sql << _T("DELETE FROM tItems WHERE parent = ") << otherTradeContainer
                        << _T(" AND owner = ") << item.charid();

                    //TODO: What when this is a known player shop? Maybe we should duplicate the items instead from 

                    m_db->Exec(sql.str());
                    //OutputDebugString(sql.str().c_str());

                    m_db->Commit();
                    g_DBManager.UnLock();
                }
                break;

            case 0x03:
                {
                    //03=tempAccept //do nothing
                }
                break;

            case 0x04:
                {
                    if (item.fromId().Low() == 0xC790)//51088)//fromType+fromTempContId+fromSlotId = charId
                    {
                        //accept from playershop:
                        TRACE(_T("accept from a playershop:!"));

                        // We duplicate items from playershop to other trade as they are moved out of shop
                        // and move from to inv and delete the original when comitting. or delete the dupe when aborting.

                        /*[6628] AOTradeTransaction:MsgId 36284f6e CharId 2568996070 Target 2568996070 
                        [6628] unknown2 1 
                        [6628] operationId 4 
                        [6628] fromType c790 
                        [6628] fromContainerTempId 1b3e 
                        [6628] fromItemSlotId fbf6 
                        [6628] fromId [51088|457112566] 
                        */

                        //from a playershop. item.fromId().High() contains the shop id.
                        unsigned int owner = g_DBManager.GetShopOwner(item.fromId().High());
                        unsigned int container = aoia::INV_PLAYERSHOP;

                        if (owner == 0)
                        {
                            //INV_OTHER_PLAYERSHOP container should be filled with the shop contents in the MSG_SHOP_ITEMS
                            owner = item.charid();
                            container = aoia::INV_OTHER_PLAYERSHOP;
                        }

                        //from a known shop!
                        g_DBManager.Lock();
                        m_db->Begin();

                        //first, remove the duplicate from the player shop

                        std::tstringstream sqlRemoveFromShop;
                        //we must duplicate it since we cant detect trade in the cancel message. (where we wipe otherTradeContainer) 

                        sqlRemoveFromShop << _T("DELETE FROM tItems WHERE parent = ") << container
                            << _T(" AND slot IN (SELECT slot from tItems WHERE parent = ") << otherTradeContainer
                            << _T("              AND owner = ") << item.charid() << _T(")")
                            << _T(" AND owner = ") << owner;

                        m_db->Exec(sqlRemoveFromShop.str());

                        //OutputDebugString(sqlRemoveFromShop.str().c_str());

                        unsigned int shopCapacity = 35;

                        for (unsigned int i = 0; i <= shopCapacity; i++) //or is it the other direction?
                        {
                            // unsigned int otherTradeContainer = aoia::INV_TRADEPARTNER;
                            unsigned int nextFreeInvSlot = g_DBManager.FindNextAvailableContainerSlot(item.charid(),
                                                                                                      aoia::
                                                                                                      INV_TOONINV);
                            //we keep the slotId!
                            {
                                std::tstringstream sqlGetTheStuff;
                                //we must duplicate it since we cant detect trade in the cancel message. (where we wipe otherTradeContainer) 

                                sqlGetTheStuff << _T("UPDATE tItems SET parent = ") << aoia::INV_TOONINV
                                    << _T(", slot = ") << nextFreeInvSlot
                                    << _T(", owner = ") << item.charid()
                                    << _T(" WHERE parent = ") << otherTradeContainer
                                    << _T(" AND slot = ") << i
                                    << _T(" AND owner = ") << owner;

                                m_db->Exec(sqlGetTheStuff.str());

                                //OutputDebugString(sqlGetTheStuff.str().c_str());
                            }
                        }

                        //put the stuff I added into the shop!
                        for (unsigned int i = 0; i <= shopCapacity; i++) //or is it the other direction?
                        {
                            // unsigned int otherTradeContainer = aoia::INV_TRADEPARTNER;
                            unsigned int nextFreeInvSlot = g_DBManager.FindNextAvailableContainerSlot(item.charid(),
                                                                                                      container);
                            //we keep the slotId!
                            {
                                std::tstringstream sqlMoveStuffToShop;
                                //we must duplicate it since we cant detect trade in the cancel message. (where we wipe otherTradeContainer) 

                                sqlMoveStuffToShop << _T("UPDATE tItems SET parent = ") << container
                                    << _T(", slot = ") << nextFreeInvSlot
                                    << _T(", owner = ") << owner
                                    << _T(" WHERE parent = ") << shopContainer
                                    << _T(" AND slot = ") << i
                                    << _T(" AND owner = ") << item.charid();

                                m_db->Exec(sqlMoveStuffToShop.str());

                                //OutputDebugString(sqlMoveStuffToShop.str().c_str());
                            }
                        }

                        m_db->Commit();
                        g_DBManager.UnLock();
                    }

                    else //if (item.fromId().High() == item.charid())
                    {
                        //[13464] AOTradeTransaction:
                        //[13464] unknown2 1  operationId 4 
                        //[13464] fromType c75b 
                        //[13464] fromContainerTempId 2018  fromItemSlotId c404 
                        //[13464] fromId [51035|538493956] 

                        g_DBManager.Lock();
                        m_db->Begin();
                        //commit/server accept: (bye-bye stuff!)

                        std::tstringstream sqlEmptyBackpacks;
                        sqlEmptyBackpacks << _T("DELETE FROM tItems WHERE owner = ") << item.charid()
                            << _T(" AND parent in (SELECT children from tItems WHERE parent = ") << shopContainer
                            << _T(" AND children > 0 AND owner = ") << item.charid() << _T(")");
                        m_db->Exec(sqlEmptyBackpacks.str());

                        std::tstringstream sql;
                        sql << _T("DELETE FROM tItems WHERE parent = ") << shopContainer
                            << _T(" AND owner = ") << item.charid();
                        m_db->Exec(sql.str());

                        //OutputDebugString(sql.str().c_str());

                        //grab the new stuff!
                        unsigned int shopCapacity = 35;

                        for (unsigned int i = 0; i <= shopCapacity; i++) //or is it the other direction?
                        {
                            unsigned int nextFreeInvSlot = g_DBManager.FindNextAvailableContainerSlot(item.charid(),
                                                                                                      aoia::
                                                                                                      INV_TOONINV);
                            std::tstringstream sqlGrabStuff;

                            sqlGrabStuff << _T("UPDATE tItems SET parent = 2")
                                << _T(", slot = ") << nextFreeInvSlot
                                << _T(" WHERE parent = ") << otherTradeContainer
                                << _T(" AND slot = ") << i
                                << _T(" AND owner = ") << item.charid();
                            m_db->Exec(sqlGrabStuff.str());
                            // OutputDebugString(sqlGrabStuff.str().c_str());
                        }

                        m_db->Commit();
                        g_DBManager.UnLock();
                    }
                }
                break;

            case 0x05:
                {
                    //add item to trade window
                    //TODO : Add/remove/commit/cancel with stuff from inv/bp to player shop trade window
                    //fromId contains the owner
                    if (item.fromId().High() == item.charid())
                    {
                        unsigned int fromContainerId = GetFromContainerId(item.charid(), item.fromType(),
                                                                          item.fromContainerTempId());

                        g_DBManager.Lock();
                        m_db->Begin();

                        unsigned int nextFreeInvSlot = g_DBManager.FindNextAvailableContainerSlot(item.charid(),
                                                                                                  shopContainer);
                        {
                            std::tstringstream sql;

                            sql << _T("UPDATE tItems SET parent = ") << shopContainer
                                << _T(", slot = ") << nextFreeInvSlot
                                << _T(" WHERE parent = ") << fromContainerId
                                << _T(" AND slot = ") << item.fromItemSlotId()
                                << _T(" AND owner = ") << item.fromId().High(); //TODO: remove from other monitored char with fromId?

                            m_db->Exec(sql.str());

                            //OutputDebugString(sql.str().c_str());
                        }

                        m_db->Commit();
                        g_DBManager.UnLock();
                    }
                    else if (item.fromType() == 0x6b && item.fromId().Low() == 0xC790)//51088)
                    {
                        //This is TODO! Do the same for op 0x6 (remove from trade)
                        // This is for removing. fromItemSlotId is the original slot in the playershop, 
                        // so set this to be the new slotId! we keep this.

                        //TODO: when accepting, check that stuff is moved correctly!

                        /*[6628] AOTradeTransaction:MsgId 36284f6e CharId 2568996070 Target 457098300 
                        [6628] unknown2 1 
                        [6628] operationId 6 
                        [6628] fromType 6b 
                        [6628] fromContainerTempId 76 
                        [6628] fromItemSlotId 7 
                        [6628] fromId [51088|457098300] */

                        //from a playershop. item.fromId().High() contains the shop id.
                        unsigned int owner = g_DBManager.GetShopOwner(item.fromId().High());
                        unsigned int container = aoia::INV_PLAYERSHOP;

                        if (owner == 0)
                        {
                            //INV_OTHER_PLAYERSHOP container should be filled with the shop contents in the MSG_SHOP_ITEMS
                            owner = item.charid();
                            container = aoia::INV_OTHER_PLAYERSHOP;
                        }

                        g_DBManager.Lock();
                        m_db->Begin();

                        {
                            std::tstringstream sqlDuplicate;
                            //we must duplicate it since we cant detect trade in the cancel message. (where we wipe otherTradeContainer) 
                            sqlDuplicate <<
                                _T("INSERT INTO tItems (keylow, keyhigh, ql, stack, parent, slot, children, owner)")
                                << _T(" SELECT keylow, keyhigh, ql, stack, ") << otherTradeContainer
                                << _T(" , slot, children, ") << item.charid()
                                << _T(" FROM tItems WHERE owner = ") << owner
                                << _T(" AND parent = ") << container
                                << _T(" AND slot = ") << item.fromItemSlotId();


                            m_db->Exec(sqlDuplicate.str());

                            //OutputDebugString(sql.str().c_str());
                        }

                        m_db->Commit();
                        g_DBManager.UnLock();
                    }
                    else
                    {
                        TRACE(_T("Added by the other part"));
                        //handled by Native::AOPartnerTradeItem
                        return;
                    }
                }
                break;

            case 0x06:
                {
                    //item removed from trade window

                    if (item.fromId().High() == item.charid())
                    {
                        //an item I had added to a trade.

                        g_DBManager.Lock();
                        m_db->Begin();

                        unsigned int nextFreeInvSlot = g_DBManager.FindNextAvailableContainerSlot(item.charid(),
                                                                                                  aoia::INV_TOONINV);

                        std::tstringstream sql;
                        sql << _T("UPDATE tItems SET parent = 2, slot = ") << nextFreeInvSlot
                            << _T(" WHERE parent = ") << shopContainer
                            << _T(" AND slot = ") << item.fromItemSlotId()
                            << _T(" AND owner = ") << item.charid();
                        m_db->Exec(sql.str());
                        //OutputDebugString(sql.str().c_str());

                        m_db->Commit();
                        g_DBManager.UnLock();
                    }
                    else if (item.fromType() == 0x6b && item.fromId().Low() == 0xC790)//51088)
                    {
                        //remove from playershop trade (currently in the otherTradeContainer)

                        /*[6628] AOTradeTransaction:MsgId 36284f6e CharId 2568996070 Target 457098300 
                        [6628] unknown2 1 
                        [6628] operationId 6 
                        [6628] fromType 6b 
                        [6628] fromContainerTempId 76 
                        [6628] fromItemSlotId 7 
                        [6628] fromId [51088|457098300] */
                        // unsigned int owner = g_DBManager.getShopOwner(item.fromId().High());

                        g_DBManager.Lock();
                        m_db->Begin();

                        {
                            std::tstringstream sqlRemove;
                            //we delete it since we duplicate it in the 0x5 operation!

                            sqlRemove << _T("DELETE FROM tItems WHERE parent = ") << otherTradeContainer
                                << _T(" AND slot = ") << item.fromItemSlotId()
                                << _T(" AND owner = ") << item.charid();

                            m_db->Exec(sqlRemove.str());

                            //OutputDebugString(sql.str().c_str());
                        }

                        m_db->Commit();
                        g_DBManager.UnLock();
                    }
                    else
                    {
                        //OutputDebugString(_T("Removed by the other part"));
                        //handled by Native::AOPartnerTradeItem
                        return;
                    }
                }
                break;

            case 0x08:
                {
                    //partner added a backpack to the trade window, this does not take a slot id in the trade window.
                    //we will create the bp contents in the MSG_CONTAINER
                    /*[6884] unknown2 1 
                    [6884] operationId 8 
                    [6884] fromType c749 
                    [6884] fromContainerTempId 2b6f 
                    [6884] fromItemSlotId 5c2a 
                    [6884] fromId [50000|2568996070] 
                    [6884] Trade partner added a backpack to trade window, not supported yet.
                    */

                    unsigned int otherTradeContainer = aoia::INV_TRADEPARTNER;
                    unsigned int containerId = (item.fromContainerTempId() << 16) + item.fromItemSlotId();

                    g_DBManager.Lock();
                    m_db->Begin();

                    unsigned int nextFreeInvSlot = g_DBManager.FindNextAvailableContainerSlot(item.charid(),
                                                                                              otherTradeContainer);

                    g_DBManager.InsertItem(99228,//small backpack used as template. To fix this, I must cache all MSG_BACKPACKS msgtype 0x1
                                           99228,
                                           1,
                                           0,
                                           1,
                                           otherTradeContainer,
                                           nextFreeInvSlot,
                                           containerId,
                                           item.charid());

                    m_db->Commit();
                    g_DBManager.UnLock();

                    // OutputDebugString(_T("Trade partner added a backpack to trade window."));
                    // OutputDebugString(sql11.str().c_str());*/
                    return;
                }
                break;

            case 0x09:// remove a backpack from trade window, same msg
                {
                    unsigned int otherTradeContainer = aoia::INV_TRADEPARTNER;
                    unsigned int containerId = (item.fromContainerTempId() << 16) + item.fromItemSlotId();
                    g_DBManager.Lock();
                    m_db->Begin();

                    std::tstringstream sql;

                    sql << _T("DELETE FROM tItems")
                        << _T(" WHERE parent = ") << otherTradeContainer
                        << _T(" AND children = ") << containerId
                        << _T(" AND owner = ") << item.charid();

                    m_db->Exec(sql.str());
                    //OutputDebugString(sql.str().c_str());

                    m_db->Commit();
                    g_DBManager.UnLock();

                    //OutputDebugString(_T("Trade partner removed a backpack to trade window."));

                    return;
                }
                break;

            default:
                {
                    LOG(_T("Unknown trade operation: ") << std::hex << item.operationId());
                    return;
                }
                break;
            }
        }
        break;

    case AO::MSG_ITEM_BOUGHT: //0x052e2f0c: 
        {
            LOG(_T("MSG_ITEM_BOUGHT"));

            Native::AOBoughtItemFromShop item((AO::BoughtItemFromShop*)msg.start(), true);

            TRACE(item.print());

            g_DBManager.Lock();
            m_db->Begin();

            unsigned int nextFreeInvSlot = g_DBManager.FindNextAvailableContainerSlot(item.charid(),
                                                                                      aoia::INV_TOONINV);

            g_DBManager.InsertItem(item.itemid().Low(),
                                   item.itemid().High(),
                                   item.ql(),
                                   item.flags(),
                                   item.stack(),
                                   aoia::INV_TOONINV,
                                   nextFreeInvSlot,
                                   0,
                                   item.charid());

            m_db->Commit();
            g_DBManager.UnLock();
        }
        break;

    case AO::MSG_SPAWN_REWARD:
        {
            LOG(_T("MSG_SPAWN_REWARD"));

            Native::AOBackpack bp((AO::Backpack*)msg.start(), true);

            if (bp.owner().High() == msg.characterId())
            {
                TRACE(bp.print());

                if (bp.operationId() == 0x01)
                {
                    //TODO:
                    //this sets an item's temp id on zone!
                    //stored in bp.target().High() in this msg.
                    //also contains inv slot id. (plus keylow/ql/stack/flags)
                    //when mission deleted, this id is sent in a MSG_CHAR_OPERATION/op 0x2f
                    TRACE(_T("updating!"));
                    m_tempContainers->UpdateTempItemId(bp.charid(), bp.targetId(), bp.invSlot());
                }
                else if (bp.operationId() == 0x71) //I got an mission item
                {
                    unsigned int containerId = bp.targetId();
                    unsigned int newParent = aoia::INV_OVERFLOW;

                    g_DBManager.Lock();
                    m_db->Begin();

                    //find a free spot:
                    unsigned int slotId = bp.invSlot();
                    if (slotId >= 0x6f)
                    {
                        slotId = g_DBManager.FindNextAvailableContainerSlot(msg.characterId(), newParent);
                    }

                    // Add item
                    g_DBManager.InsertItem(bp.keyLow(),
                                           bp.keyHigh(),
                                           bp.ql(),
                                           bp.flags(),
                                           1,//stack
                                           newParent,
                                           slotId,//bp.invSlot(),
                                           0,
                                           msg.characterId());

                    TRACE(_T("updating!"));

                    //this will be moved to the next inv slot in the next msg.
                    unsigned int invSlotId = bp.invSlot();
                    if (invSlotId >= 0x6f)
                        invSlotId = g_DBManager.FindNextAvailableContainerSlot(msg.characterId(), aoia::INV_TOONINV);

                    m_tempContainers->UpdateTempItemId(bp.charid(), bp.targetId(), invSlotId);

                    m_db->Commit();
                    g_DBManager.UnLock();
                }
                else
                {
                    TRACE(_T("UNKNOWN MSG_SPAWN_REWARD operation Id: ") << std::hex << bp.operationId());
                }
            }
        }
        break;

    case AO::MSG_ITEM_MOVE: //0x47537A24:// what about 0x52526858:
        {
            LOG(_T("MSG_ITEM_MOVE"));

            //The ItemMoved is identical to the client send version except the header is server type.
            Native::AOItemMoved moveOp((AO::ItemMoved*)msg.start(), true);
            TRACE(moveOp.print());

            unsigned int fromContainerId = GetFromContainerId(moveOp.charid(), moveOp.fromType(),
                                                              moveOp.fromContainerTempId());
            unsigned int newParent;
            unsigned int toType = moveOp.toContainer().Low();

            if (fromContainerId == aoia::INV_OVERFLOW)
            {
                newParent = aoia::INV_TOONINV; //toType seems to be 0x006e sometimes (tradeskill) when moving from overflow
            }
            else if (toType == 0xc749) //to backpack
            {
                newParent = moveOp.toContainer().High();
            }
            else if (toType == 0xdead)
            {
                newParent = aoia::INV_BANK;
            }
            else if (toType == 0xc350)
            {
                newParent = aoia::INV_TOONINV;
            }
            else if (toType == 0x006e) //to overflow
            {
                newParent = aoia::INV_OVERFLOW;
            }
            else
            {
                TRACE(_T("TODO: MOVE/UNKNOWN TO TYPE ") << std::hex << toType);
                return;
            }

            if (fromContainerId == 0)
            {
                TRACE(_T("FromContainerId not found in cache "));
                return; //we dont have the value cached, either a bug or ia was started after the bp was opened.
            }

            unsigned short newSlot = moveOp.targetSlotId();

            if (newParent == aoia::INV_TOONINV)
            {
                if (newSlot < 64)//64=first inv slot!
                {
                    //we are moving to an equip panel!

                    //if anything there, we must hotswap!
                    //we move anything that was there to a temp hotswap inv, move the new item, then move the other back.

                    g_DBManager.Lock();
                    m_db->Begin();
                    {
                        std::tstringstream sqlMoveToTemp;
                        sqlMoveToTemp << _T("UPDATE tItems SET parent = ") << aoia::INV_HOTSWAPTEMP
                            << _T(", slot = ") << moveOp.fromItemSlotId()
                            << _T(" WHERE parent = ") << newParent
                            << _T(" AND slot = ") << newSlot
                            << _T(" AND owner = ") << moveOp.charid();

                        TRACE(sqlMoveToTemp.str());
                        m_db->Exec(sqlMoveToTemp.str());
                    }
                    {
                        std::tstringstream sqlMoveNewItem;
                        sqlMoveNewItem << _T("UPDATE tItems SET parent = ") << newParent
                            << _T(", slot = ") << newSlot
                            << _T(" WHERE parent = ") << fromContainerId
                            << _T(" AND slot = ") << moveOp.fromItemSlotId()
                            << _T(" AND owner = ") << moveOp.charid();

                        TRACE(sqlMoveNewItem.str());
                        m_db->Exec(sqlMoveNewItem.str());
                    }
                    {
                        std::tstringstream sqlMoveFromTemp;
                        sqlMoveFromTemp << _T("UPDATE tItems SET parent = ") << fromContainerId
                            << _T(" WHERE parent = ") << aoia::INV_HOTSWAPTEMP
                            << _T(" AND slot = ") << moveOp.fromItemSlotId()
                            << _T(" AND owner = ") << moveOp.charid();

                        TRACE(sqlMoveFromTemp.str());
                        m_db->Exec(sqlMoveFromTemp.str());
                    }

                    m_db->Commit();
                    g_DBManager.UnLock();
                    return;
                }
                else if (newSlot >= 0x6f)
                {
                    //Check if item is stackable, splittable, we are moving to inventory 
                    //and there is an item with the same itemkey there.
                    //If so join with the one with the lowest slotId (I think).

                    unsigned int itemProp = g_DBManager.GetItemProperties(moveOp.charid(), fromContainerId,
                                                                          moveOp.fromItemSlotId());

                    //DID during downtime, check this with extruder bars and ammo at least
                    if ((itemProp & PROP_STACKABLE) && (!(itemProp & PROP_NO_SPLIT)))
                    {
                        //OutputDebugString(_T("Its stackable"));
                        unsigned int newSlotId = g_DBManager.FindFirstItemOfSameType(moveOp.charid(), fromContainerId,
                                                                                     moveOp.fromItemSlotId(),
                                                                                     newParent);

                        if (newSlotId > 0)
                        {
                            TRACE(_T("Stack join from move operation"));

                            g_DBManager.Lock();
                            m_db->Begin();
                            //join them and delete the old one.
                            {
                                std::tstringstream sqlUpd;
                                sqlUpd << _T("UPDATE tItems SET stack = ")
                                    << _T(" (stack + (SELECT tItems2.stack FROM tItems tItems2 WHERE")
                                    << _T(" tItems2.owner = ") << msg.characterId()
                                    << _T(" AND tItems2.parent = ") << fromContainerId
                                    << _T(" AND tItems2.slot = ") << moveOp.fromItemSlotId() << _T("))")
                                    << _T(" WHERE owner = ") << msg.characterId()
                                    << _T(" AND parent = ") << newParent
                                    << _T(" AND slot = ") << newSlotId;

                                //OutputDebugString(sqlUpd.str().c_str());
                                m_db->Exec(sqlUpd.str());
                            }

                            {
                                std::tstringstream sql;
                                sql << _T("DELETE FROM tItems WHERE owner = ") << msg.characterId()
                                    << _T(" AND parent = ") << fromContainerId
                                    << _T(" AND slot = ") << moveOp.fromItemSlotId();//itemId().High();

                                //OutputDebugString(sql.str().c_str());
                                m_db->Exec(sql.str());
                            }

                            m_db->Commit();
                            g_DBManager.UnLock();
                            return;
                        }
                    }
                }
            }

            g_DBManager.Lock();
            m_db->Begin();

            if (newSlot >= 0x6f || newSlot == 0x00)
            {
                newSlot = g_DBManager.FindNextAvailableContainerSlot(moveOp.charid(), newParent);
            }

            std::tstringstream sql;
            sql << _T("UPDATE tItems SET parent = ") << newParent
                << _T(", slot = ") << newSlot
                << _T(" WHERE parent = ") << fromContainerId
                << _T(" AND slot = ") << moveOp.fromItemSlotId()
                << _T(" AND owner = ") << moveOp.charid();

            TRACE(sql.str());
            m_db->Exec(sql.str());

            m_db->Commit();
            g_DBManager.UnLock();
        }
        break;

    case AO::MSG_BANK:
        {
            LOG(_T("MSG_BANK"));

            Native::AOBank bank((AO::Bank*)msg.start(), true);
            g_DBManager.Lock();
            m_db->Begin();
            {  // Remove old stuff from the bank. Every update is a complete update.
                std::tstringstream sql;
                sql << _T("DELETE FROM tItems WHERE owner = ") << msg.characterId() << _T(" AND parent = 1");
                m_db->Exec(sql.str());
            }
            for (unsigned int i = 0; i < bank.numitems(); i++)
            {
                Native::AOItem item = bank.item(i);
                g_DBManager.InsertItem(item.itemid().Low(),
                                       item.itemid().High(),
                                       item.ql(),
                                       item.flags(),
                                       item.stack(),
                                       aoia::INV_BANK,
                                       item.index(),
                                       item.containerid().High(),
                                       msg.characterId());
            }
            m_db->Commit();
            g_DBManager.UnLock();
        }
        break;

    case AO::MSG_BACKPACK:
        {
            LOG(_T("MSG_BACKPACK"));
            Native::AOBackpack bp((AO::Backpack*)msg.start(), true);

            if (bp.owner().High() == msg.characterId())
            {
                TRACE(bp.print());
                if (bp.operationId() == 0x65) //I bought a backpack (I think)
                {
                    unsigned int containerId = bp.targetId();
                    unsigned int newParent = aoia::INV_TOONINV;

                    g_DBManager.Lock();
                    m_db->Begin();

                    //find a free spot:
                    unsigned int slotId = bp.invSlot();
                    if (slotId >= 0x6f)
                        slotId = g_DBManager.FindNextAvailableContainerSlot(msg.characterId(), newParent);
                    {
                        // Remove old ref to backpack:
                        std::tstringstream sql;
                        sql << _T("DELETE FROM tItems WHERE owner = ") << msg.characterId()
                            << _T(" AND children = ") << containerId;
                        m_db->Exec(sql.str());
                    }

                    // Add backpack:
                    g_DBManager.InsertItem(bp.keyLow(),
                                           bp.keyHigh(),
                                           bp.ql(),
                                           bp.flags(),
                                           1,//stack
                                           newParent,
                                           slotId,//bp.invSlot(),
                                           containerId,
                                           msg.characterId());

                    TRACE(_T("BP: ") << bp.keyLow() << _T(" FL: ") << bp.flags());

                    m_db->Commit();
                    g_DBManager.UnLock();
                }
                /*else if (bp.operationId() == 0x01) //backpack refresh
                {
                unsigned int containerId = bp.target().High();
                
                unsigned int newParent = aoia::INV_TOONINV;
                
                unsigned int slotId = bp.invSlot();
                
                g_DBManager.lock();
                m_db->Begin();
                
                {
                // Remove old ref to backpack:
                std::tstringstream sql;
                sql << _T("DELETE FROM tItems WHERE children = ") << containerId;
                m_db->Exec(sql.str());
                }
                
                g_DBManager.insertItem(
                bp.keyLow(),
                bp.keyHigh(),
                bp.ql(),
                bp.flags(),
                1,//stack
                aoia::INV_TRADE,
                slotId ,//bp.invSlot(),
                containerId,
                msg.characterId());
                
                m_db->Commit();
                g_DBManager.unLock();
                }*/
            }
        }
        break;

    case AO::MSG_CONTAINER:
        {
            LOG(_T("MSG_CONTAINER"));

            //Happens when overflow opens, loot container opens or I get a new container from a trade.
            Native::AOContainer bp((AO::Header*)msg.start());
            TRACE(bp.print());

			unsigned int containerId = bp.containerid().High();
			unsigned int containerId2;
			auto loot = parseLootdata(msg, containerId2);

            if (bp.tempContainerId() == 0x006e && bp.containerid().Low() == 0xc350)
            {
                TRACE(_T("Overflow container msg"));
                containerId = aoia::INV_OVERFLOW;
            }
			else if (containerId2 == 0 && !loot.empty()) {
				time_t rawtime;
				struct tm * timeinfo;

				time (&rawtime);
				timeinfo = localtime (&rawtime);
				char buffer [80];
				sprintf_s(buffer, 80, "%s", asctime(timeinfo));

				
				std::tstringstream script;
				std::tstring ts = from_ascii_copy(buffer);
				boost::algorithm::trim(ts);
				script << _T("<a href=\"text://Item drops for last corpse:<br>") << ts.c_str() << _T("<br>");

				g_DBManager.Lock();
				for (auto it : loot) {
					std::tstring itemname;
					{
						std::tstringstream st;
						st << _T("SELECT name FROM tblAo WHERE aoid in (") << it.highid << _T(",") << it.lowid << _T(")");
						sqlite::ITablePtr itemnames = m_db->ExecTable(st.str());	
						if (itemnames != NULL) {

							std::string tmp = itemnames->Data(0, 0);
							if (tmp.empty())
								itemname = _T("Unknown item");
							else 
								itemname = from_ascii_copy(tmp);
						}
					}

					script << _T("QL") << (it.ql < 10 ? _T("<font color=#000000>00</font>") : (it.ql < 100)?_T("<font color=#000000>0</font>"):_T(""))
						<< it.ql 
						<< _T(" ") 
						<< _T("<a href='itemref://") 
						<< it.lowid << _T("/") << it.highid << _T("/") << it.ql 
						<< _T("'>") << itemname << _T("</a><br>");

				}
				g_DBManager.UnLock();
				script << _T("<br>Generated by AOIA++\">Click to see drops</a>");
				std::tstring path = AOManager::instance().getAOPrefsFolder() + _T("\\..\\scripts\\loot");
				std::tofstream outfile(path);
				outfile << script.str();
				outfile.close();
				int x = 9;
			}
			
			else if (containerId2 != 0 && !loot.empty()) {
				std::tstringstream script;
				script << _T("<a href=\"text://Contents of backpack:<br>") << _T("<br>");
				std::tstring containername = m_containerManager->GetContainerName(msg.characterId(), containerId);
				g_DBManager.Lock();
				for (auto it : loot) {
					std::tstring itemname;
					{
						std::tstringstream st;
						st << _T("SELECT name FROM tblAo WHERE aoid in (") << it.highid << _T(",") << it.lowid << _T(")");
						sqlite::ITablePtr itemnames = m_db->ExecTable(st.str());	
						if (itemnames != NULL) {

							std::string tmp = itemnames->Data(0, 0);
							if (tmp.empty())
								itemname = _T("Unknown item");
							else 
								itemname = from_ascii_copy(tmp);
						}
					}

					
					script << _T("QL") << (it.ql < 10 ? _T("<font color=#000000>00</font>") : (it.ql < 100)?_T("<font color=#000000>0</font>"):_T(""))
						<< it.ql 
						<< _T(" ") 
						<< _T("<a href='itemref://") 
						<< it.lowid << _T("/") << it.highid << _T("/") << it.ql 
						<< _T("'>") << itemname << _T("</a><br>");

				}
				g_DBManager.UnLock();
				script << _T("<br>Generated by AOIA++\">") << containername << _T("</a>");
				std::tstring path = AOManager::instance().getAOPrefsFolder() + _T("\\..\\scripts\\bp");
				std::tofstream outfile(path);
				outfile << script.str();
				outfile.close();
				int x = 9;
			}

            //bp.containerid().Low() = C76A  => This is some loot container.
            //bp.containerid().Low() = C749  => I think it means it is in my inv.

            /*
            if (bp.containerid().Low() == 0xc749)
            {
            //if one with same containerId does not exist, create it!
            "INSERT INTO tItems VALUES ( ) WHERE not exist tItems...")
            g_DBManager.insertItem(
            item.itemid().Low(),
            item.itemid().High(),
            item.ql(),
            item.flags(),
            item.stack(),
            containerId,
            item.index(),
            0,
            msg.characterId());
            }*/

            g_DBManager.Lock();
            m_db->Begin();
            {
                // Remove old contents from BP
                std::tstringstream sql;
                sql << _T("DELETE FROM tItems WHERE parent = ") << containerId;
                m_db->Exec(sql.str());
            }
            // Register BP contents
            for (unsigned int i = 0; i < bp.numitems(); i++)
            {
                Native::AOItem item = bp.item(i);
                g_DBManager.InsertItem(item.itemid().Low(),
                                       item.itemid().High(),
                                       item.ql(),
                                       item.flags(),
                                       item.stack(),
                                       containerId,
                                       item.index(),
                                       0,
                                       msg.characterId());
                /*#ifdef DEBUG
                {
                // Log
                std::tstringstream sql;
                sql << _T("Bp IT:") << item.itemid().Low() << _T("FL:\t") << item.flags();
                OutputDebugString(sql.str().c_str());
                }
                #endif*/
            }

            //don't store INV_OVERFLOW as contId 110, this id can be used by a backpack!
            if (containerId != aoia::INV_OVERFLOW)
            {
                m_tempContainers->UpdateTempContainerId(bp.charid(), bp.tempContainerId(), containerId);
            }

            m_db->Commit();
            g_DBManager.UnLock();
            //AddToTreeView(msg.charid(), bp.containerid().High());
        }
        break;

    case AO::MSG_FULLSYNC:
        {
            Parsers::AOFullCharacterMessage parser(msg.start(), msg.size());
            LOG(_T("MSG_FULLSYNC CharacterID=") << parser.characterId());

            g_DBManager.Lock();
            m_db->Begin();
            {
                // Remove old contents from DB (inventory, trade, remote trade and overflow win.
                std::tstringstream sql;
                sql << _T("DELETE FROM tItems WHERE (parent = 2 or parent >= 4 and parent <= 8) AND owner = ") <<
                    parser.characterId();
                m_db->Exec(sql.str());
            }

            m_tempContainers->ClearTempContainerIdCache(parser.characterId());

            // Register items
            std::vector<AOContainerItemPtr> items;
            parser.getAllItems(items);
            for (std::vector<AOContainerItemPtr>::const_iterator it = items.begin(); it != items.end(); ++it)
            {
                Parsers::AOContainerItemPtr item = *it;

                g_DBManager.InsertItem(item->itemId().low(),
                                       item->itemId().high(),
                                       item->ql(),
                                       item->flags(),
                                       item->stack(),
                                       2,             // parent 2 = equip
                                       item->index(),
                                       item->containerId().high(),
                                       msg.characterId());

                //TRACE("Inv: " << item.itemid().Low() << "FL:\t" << item.flags());
            }
            m_db->Commit();

            // Record stats in the DB
            StatMap stats;
            parser.getStats(stats);
            g_DBManager.SetToonStats(msg.characterId(), stats);

            g_DBManager.UnLock();

            CleanupDB(msg.characterId());
        }
        break;

    case AO::MSG_MOB_SYNC:
        {
            // Make sure this is the message for the currently playing toon (and not other mobs in vicinity).
            if (msg.characterId() == msg.entityId())
            {
                LOG(_T("MSG_MOB_SYNC"));

                AO::MobInfo* pMobInfo = (AO::MobInfo*)msg.start();
                std::string name(&(pMobInfo->characterName.str), pMobInfo->characterName.strLen - 1);

                g_DBManager.Lock();
                g_DBManager.SetToonName(msg.characterId(), from_ascii_copy(name));
                g_DBManager.UnLock();
            }
        }
        break;

    case AO::MSG_SHOP_INFO:
        {
            AOPlayerShopInfo shop(msg.start(), msg.size());
            LOG(_T("MSG_SHOP_INFO target=") << shop.characterId() << _T(" owner=") << shop.ownerId() << _T(" shop=") <<
                shop.shopId());

            if (shop.shopId() != 0 && shop.ownerId() != 0)
            {
                // As long as the shop and owner IDs are > 0 we record them.
                // setToonShopId will not create new toon entries in the DB, 
                // so its safe to just dump all the shop IDs to it.
                g_DBManager.SetToonShopId(shop.ownerId(), shop.shopId());
            }
        }
        break;

	case AO::SMSG_UNKNOWN_WITH_ZONEID2:
		{
			msg.skip(13);
			int zoneid = msg.popInteger();
			m_characterPositionMap[msg.characterId()].zoneid = zoneid;
			m_characterPositionMap[msg.characterId()].lastSeen = timeGetTime();
			WriteWaypointsScript();
		}
		break ;


	case AO::SMSG_MOVED:
		{
			SMSG_MOVED_DATA md(msg);
			m_characterPositionMap[msg.characterId()].x = md.x;
			m_characterPositionMap[msg.characterId()].z = md.y;
			m_characterPositionMap[msg.characterId()].lastSeen = timeGetTime();
			WriteWaypointsScript();
		}
		break;

    case AO::MSG_SHOP_ITEMS:
        {
            AOPlayerShopContent shop(msg.start(), msg.size());
            LOG(_T("MSG_SHOP_ITEMS target=") << shop.characterId() << _T(" shop=") << shop.shopid());

            // This message should only update the shops for already registered shop IDs, and then use the character 
            // ID of who-ever that shop is registered to. This will allow you to update all your toons shops without 
            // logging them in. (You only need to visit the shop with one of them.)

            unsigned int owner = g_DBManager.GetShopOwner(shop.shopid());

            unsigned int parent = aoia::INV_PLAYERSHOP;

            if (owner == 0)
            {
                LOG(_T("Shop owner not found."));
                //that means we will use this one in playershop trades
                owner = msg.characterId();
                parent = aoia::INV_OTHER_PLAYERSHOP;
            }

            g_DBManager.Lock();
            m_db->Begin();
            {
                // Remove old contents from container
                std::tstringstream sql;
                sql << _T("DELETE FROM tItems WHERE parent = ") << parent << _T(" AND owner = ") << owner;
                m_db->Exec(sql.str());
            }
            // Register container contents
            for (unsigned int i = 0; i < shop.numitems(); i++)
            {
                AOContainerItem item = shop.item(i);
                unsigned int price = shop.price(item.index());

                g_DBManager.InsertItem(item.itemId().low(),
                                       item.itemId().high(),
                                       item.ql(),
                                       item.flags(),
                                       item.stack(),
                                       parent,
                                       item.index(),
                                       0,
                                       owner);
            }
            m_db->Commit();
            g_DBManager.UnLock();

        }
        break;
        /*case AO::MSG_OPENBACKPACK:// 0x52526858://1196653092:
        {
        
        return;//handled on server msg. 
        //Since the server msg is only sent once, we might want to update tempId here, but it shouldnt be needeed.
        Native::AOOpenBackpackOperation moveOp((AO::OpenBackpackOperation*)msg.start());
        
        OutputDebugString(moveOp.print().c_str());
        
        }
        break;*/

    default:
        break;
    }
}


void InventoryView::CleanupDB(unsigned int charid)
{
    // Statement to delete all the orfaned containers for this toon.
    std::tstringstream sql;
    sql << _T("DELETE FROM tItems WHERE owner = ") << charid <<
        _T(" AND parent > 3 AND parent NOT IN (SELECT DISTINCT children FROM tItems)");

    g_DBManager.Lock();
    m_db->Exec(sql.str());
    g_DBManager.UnLock();
}


bool InventoryView::PreTranslateMsg(MSG* pMsg)
{
    if (m_accelerators.TranslateAccelerator(m_hWnd, pMsg))
    {
        return true;
    }
    if (m_findview.PreTranslateMsg(pMsg))
    {
        return true;
    }

    return false;
}


// Updates the list view with the results of the SQL query. 'where' is used as the expression after the WHERE keyword.
void InventoryView::UpdateListView(std::tstring const& where)
{
    if (m_datagrid->getModel())
    {
        m_datagrid->getModel()->disconnect(m_addSignalConnection);
        m_datagrid->getModel()->disconnect(m_removeSignalConnection);
        m_datagrid->getModel()->disconnect(m_clearSignalConnection);
        m_datagrid->getModel()->disconnect(m_updateSignalConnection);
    }

    m_datagridmodel.reset(new ItemListDataModel(m_db, m_containerManager, where, m_sortColumn, !m_sortDesc));

    m_addSignalConnection = m_datagridmodel->connectItemAdded(boost::bind(&InventoryView::onItemAdded, this, _1));
    m_removeSignalConnection = m_datagridmodel->connectItemRemoved(boost::bind(&InventoryView::onItemRemoved, this, _1));
    m_clearSignalConnection = m_datagridmodel->connectAllRemoved(boost::bind(&InventoryView::onAllItemsRemoved, this));
    m_updateSignalConnection = m_datagridmodel->connectCollectionUpdated(boost::bind(&InventoryView::onAllItemsUpdated, this));

    m_datagrid->setModel(m_datagridmodel);
    m_datagrid->autosizeColumnsUseData();

    OnSelectionChanged();
}


std::tstring InventoryView::ensureEncapsulation(std::tstring const& field)
{
    std::tstring retval;
    if (!starts_with(field, _T("\"")))
    {
        retval = _T("\"");
    }
    retval += field;
    if (!ends_with(field, _T("\"")))
    {
        retval += _T("\"");
    }
    return retval;
}


// Exports all items matching the where statement.
void InventoryView::exportToCSV(std::tstring const& where)
{
    std::tstring where_statement;
    if (!where.empty())
    {
        where_statement = _T("(") + where + _T(") AND ");
    }
    where_statement += _T("I.children=0");

    ItemListDataModelPtr data(new ItemListDataModel(m_db, m_containerManager, where_statement, m_sortColumn, !m_sortDesc));

    // Ask the user which server to use for item URLs
    ServerSelectorDialog dlg;
    if (dlg.DoModal(m_hWnd) != IDOK)
    {
        return;
    }

    std::tstring itemURLTemplate;
    switch (dlg.getChoice())
    {
    case 1:
        itemURLTemplate = GetServerItemURLTemplate(SERVER_AUNO);
        break;
    case 2:
        itemURLTemplate = GetServerItemURLTemplate(SERVER_JAYDEE);
        break;
    case 3:
        itemURLTemplate = GetServerItemURLTemplate(SERVER_XYPHOS);
        break;
    }

    // Ask the user for a name for the CSV file
    std::tstring filename = BrowseForOutputFile(GetTopLevelWindow(), _T("Select Name of Output File"),
                                                _T("CSV Files (Comma-Separated Values)\0*.csv\0\0"), _T("csv"));
    if (filename.empty())
    {
        return;
    }

    // Output to the selected CSV file.
    std::tofstream ofs(filename.c_str());
    if (!ofs.is_open())
    {
        return;
    }

    CSVDataModelPtr model(new CSVDataModel(m_db, m_containerManager, where_statement, itemURLTemplate));
    CSVExporter exporter;
    exporter.DoExport(ofs, model);
}


void InventoryView::OnSelectionChanged()
{
    std::set<unsigned int> items = m_datagrid->getSelectedItems();

    if (items.size() == 1)
    {
        ItemListDataModelPtr model = boost::static_pointer_cast<ItemListDataModel>(m_datagrid->getModel());
        m_infoview.SetCurrentItem(model->getItemIndex(*items.begin()));
    }
    else
    {
        m_infoview.SetCurrentItem(0);
    }

    UpdateStatusText();
}


std::tstring InventoryView::GetServerItemURLTemplate(ItemServer server)
{
    std::tstring retval;

    switch (server)
    {
    case SERVER_AUNO:
        {
            TCHAR buffer[1024];
            ATL::AtlLoadString(IDS_AUNO_ITEMREF_URL, buffer, 1024);
            retval = std::tstring(buffer);
        }
        break;

    case SERVER_JAYDEE:
        {
            TCHAR buffer[1024];
            ATL::AtlLoadString(IDS_JAYDEE_ITEMREF_URL, buffer, 1024);
            retval = std::tstring(buffer);
        }
        break;

    case SERVER_AO:
        {
            TCHAR buffer[1024];
            ATL::AtlLoadString(IDS_AO_ITEMREF_URL, buffer, 1024);
            retval = std::tstring(buffer);
        }
        break;

    case SERVER_XYPHOS:
        {
            TCHAR buffer[1024];
            ATL::AtlLoadString(IDS_XYPHOS_ITEMREF_URL, buffer, 1024);
            retval = std::tstring(buffer);
        }
    }

    return retval;
}


void InventoryView::UpdateStatusText()
{
    // Set status text
    boost::basic_format<TCHAR> status(_T("%1% of %2% items selected."));
    status % m_datagrid->getSelectedCount() % m_datagrid->getModel()->getItemCount();
    setStatusText(status.str());
}


void InventoryView::onAllItemsUpdated()
{
    UpdateStatusText();
}


void InventoryView::onItemAdded(unsigned int index)
{
    UpdateStatusText();
}


void InventoryView::onItemRemoved(unsigned int index)
{
    UpdateStatusText();
}


void InventoryView::onAllItemsRemoved()
{
    UpdateStatusText();
}


bool InventoryView::SetClipboardText(std::tstring const& text)
{
    if (text.empty())
    {
        return false;
    }

    if (OpenClipboard() && EmptyClipboard())
    {
        HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, (text.length() * 2 + 2));
        if (hglbCopy == NULL)
        {
            CloseClipboard();
            return false;
        }

        // Lock the handle and copy the text to the buffer.
        TCHAR* lptstrCopy = (TCHAR*)GlobalLock(hglbCopy);
        ZeroMemory(lptstrCopy, text.length() * 2);
        memcpy(lptstrCopy, text.c_str(), text.length() * 2);
        GlobalUnlock(hglbCopy);

        // Place the handle on the clipboard.
#ifdef UNICODE
        SetClipboardData(CF_UNICODETEXT, hglbCopy);
#else
        SetClipboardData(CF_TEXT, hglbCopy);
#endif
        CloseClipboard();
        return true;
    }
    return false;
}


std::tstring InventoryView::GetContainerNameForItem( OwnedItemInfoPtr pItemInfo ) const
{
    unsigned int containerid = boost::lexical_cast<unsigned int>(pItemInfo->containerid);
    unsigned int ownerid = boost::lexical_cast<unsigned int>(pItemInfo->ownerid);
    return m_containerManager->GetContainerName(ownerid, containerid);
}


void InventoryView::GetSelectedItemIds( std::set<unsigned int> &ids )
{
    std::set<unsigned int> selectedIndexes = m_datagrid->getSelectedItems();
    for (std::set<unsigned int>::const_iterator it = selectedIndexes.begin(); it != selectedIndexes.end(); ++it)
    {
        ids.insert(m_datagridmodel->getItemIndex(*it));
    }
}


LRESULT InventoryView::OnDelete( WORD FromAccelerator, WORD CommandId, HWND hWndCtrl, BOOL& bHandled )
{
    if (m_datagrid->getSelectedCount() < 1)
    {
        return 0;
    }

    std::tstring message = _T("Are you sure you want to delete these items?");
    if (m_datagrid->getSelectedCount() == 1)
    {
        message = _T("Are you sure you want to delete this item?");
    }

    if (IDYES != MessageBox(message.c_str(), _T("Verify Operation"), MB_YESNO | MB_ICONQUESTION))
    {
        m_datagrid->SetFocus();
        return 0;
    }

    std::set<unsigned int> ids;
    GetSelectedItemIds(ids);
    m_datagrid->ClearSelection();
    m_datagridmodel->DeleteItems(ids);

    return 0;
}
