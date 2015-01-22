#include "StdAfx.h"
#include "RecipePanel.h"
#include "DBManager.h"

using namespace aoia;


LRESULT RecipePanel::OnManageCmd(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/) {
	RecipeManageDlg dlg;
	dlg.DoModal();
/*	dlg.Create(NULL);
	dlg.ShowWindow(SW_SHOW);*/

	return 0;
}
RecipePanel::RecipePanel(sqlite::IDBPtr db, aoia::ISettingsPtr settings)
	: m_db(db) 
	, m_recipeLoader(db)
	, m_settings(settings)
{
	
}


RecipePanel::~RecipePanel() {}


LRESULT RecipePanel::onInitDialog(UINT/*uMsg*/, WPARAM/*wParam*/, LPARAM/*lParam*/, BOOL&/*bHandled*/)
{
	SetWindowText(_T("Filter View"));

	updateCharList();



	{
		CComboBox cb = GetDlgItem(IDC_RECIPECOMBO);
		auto recipes = m_recipeLoader.getRecipes();
		cb.ResetContent();

		int item = cb.AddString(_T("-"));
		for (unsigned int i = 0; i < recipes.size(); i++) {
			std::tstring name = recipes[i];
			if ((item = cb.AddString(name.c_str())) != CB_ERR) {
				cb.SetItemData(item, i);
			}
		}
		cb.SetCurSel(0);
	}
	signalSettingsChanged();
	return 0;
}


LRESULT RecipePanel::onForwardMsg(UINT/*uMsg*/, WPARAM/*wParam*/, LPARAM lParam, BOOL&/*bHandled*/)
{
	LPMSG pMsg = (LPMSG)lParam;
	return this->PreTranslateMsg(pMsg);
}


void RecipePanel::updateCharList() {



	CComboBox cb = GetDlgItem(IDC_CHARACTERCOMBO);
	if (cb.GetCount() < 1)
	{
		WTL::CComboBox cb = GetDlgItem(IDC_CHARACTERCOMBO);
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


void RecipePanel::setProgress(unsigned short percent)
{/*
	WTL::CProgressBarCtrl progressbar = GetDlgItem(IDC_PROGRESS);
	progressbar.SetPos(percent);*/
}


unsigned int RecipePanel::getCharId() const
{
	
	unsigned int toonid = 0;
	
	WTL::CComboBox tooncb = GetDlgItem(IDC_CHARACTERCOMBO);
	if (tooncb.GetCurSel() > 0)
	{
		toonid = (unsigned int)tooncb.GetItemData(tooncb.GetCurSel());
	}
	
	return toonid;
}




BOOL RecipePanel::PreTranslateMsg(MSG* pMsg)
{
	return IsDialogMessage(pMsg);
}


LRESULT RecipePanel::onCbnSelchangeCharcombo(WORD/*wNotifyCode*/, WORD/*wID*/, HWND/*hWndCtl*/, BOOL&/*bHandled*/)
{
	signalSettingsChanged();
	return 0;
}




LRESULT RecipePanel::OnCbnDropdown(WORD/*wNotifyCode*/, WORD/*wID*/, HWND/*hWndCtl*/, BOOL&/*bHandled*/)
{
	updateCharList();
	return 0;
}



void RecipePanel::disconnect(Connection subscriber)
{
	subscriber.disconnect();
}


RecipePanel::Connection RecipePanel::connectSettingsChanged(SettingsChangedSignal::slot_function_type subscriber)
{
	return m_settingsChangedSignal.connect(subscriber);
}


void RecipePanel::signalSettingsChanged()
{
	m_settingsChangedSignal();
}


std::list<unsigned int> RecipePanel::getDesiredItemRefs() const {
	WTL::CComboBox templatecb = GetDlgItem(IDC_RECIPECOMBO);
	if (templatecb.GetCurSel() > 0) {
		TCHAR buf[256] = {0};
		templatecb.GetLBText(templatecb.GetCurSel(), buf);
		std::tstring recipe(buf);

		const RecipeList* const r = m_recipeLoader.getRecipe(recipe);
		if (r == NULL)
			return std::list<unsigned int>();
		else
			return r->getDesiredItemRefs();
	}
	else
		return std::list<unsigned int>();
}
std::set<Item> RecipePanel::getItemList(const std::list<Item>& itemsInDatabase) const {
	WTL::CComboBox templatecb = GetDlgItem(IDC_RECIPECOMBO);
	if (templatecb.GetCurSel() > 0) {
		TCHAR buf[256] = {0};
		templatecb.GetLBText(templatecb.GetCurSel(), buf);
		std::tstring recipe(buf);

		const RecipeList* const r = m_recipeLoader.getRecipe(recipe);
		if (r == NULL)
			return std::set<Item>();
		else
			return r->getItemList(itemsInDatabase);
	}
	else
		return std::set<Item>();
}
std::tstring RecipePanel::getRecipeName() const {

	WTL::CComboBox templatecb = GetDlgItem(IDC_RECIPECOMBO);
	if (templatecb.GetCurSel() > 0) {
		TCHAR buf[256] = {0};
		templatecb.GetLBText(templatecb.GetCurSel(), buf);
		std::tstring recipe(buf);

		const RecipeList* const r = m_recipeLoader.getRecipe(recipe);
		if (r == NULL)
			return _T("Unknown recipe??");
		else
			return r->getName();
	}
	else
		return _T("Unknown recipe??");
}