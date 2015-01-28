#pragma once

#include "RecipePanel.h"
#include "resource.h"
#include "shared/Mutex.h"
#include "shared/Thread.h"
#include "shared/aopackets.h"
#include <Shared/IDB.h>
#include <PluginSDK/IContainerManager.h>
#include <PluginSDK/IGuiServices.h>
#include <PluginSDK/ItemAssistView.h>
#include "WebView.h"




class RecipesView
	: public ItemAssistView<RecipesView>
{
	typedef ItemAssistView<RecipesView> inherited;
public:
	DECLARE_WND_CLASS(NULL)

	RecipesView(sqlite::IDBPtr db, aoia::IContainerManagerPtr containerManager, aoia::IGuiServicesPtr gui, aoia::ISettingsPtr settings);
	virtual ~RecipesView();

	BEGIN_MSG_MAP_EX(RecipesView)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_SIZE(OnSize)
		MSG_WM_SETFOCUS(OnFocus)
		COMMAND_ID_HANDLER(ID_RECALCULATE, OnRecalculate)
		COMMAND_ID_HANDLER(ID_HELP, OnHelp)
		CHAIN_MSG_MAP(inherited)
		DEFAULT_REFLECTION_HANDLER()
		END_MSG_MAP()

		virtual void OnActive(bool doActivation);
		virtual bool PreTranslateMsg(MSG* pMsg);

		// Pocket boss list and access methods
		struct PbItem {
			unsigned int pbid;
			std::tstring pbname;
			float pbavailability;
		};
		typedef std::vector<boost::shared_ptr<PbItem> > PbList;
		void SetBossAvail(unsigned int pbid, float avail);


		Mutex& PbListMutex() { return m_pblistMutex; }

protected:
	enum {
		AVAIL_TIMER = 1
	};
	void OnFocus(CWindow wndOld);
	LRESULT OnCreate(LPCREATESTRUCT createStruct);
	void OnDestroy();
	LRESULT OnSize(UINT wParam, CSize newSize);
	LRESULT OnRecalculate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnHelp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);


	void UpdateFilterProgress(unsigned short percent);

	void UpdateLayout(CSize newSize);

	void SetColumns(std::vector<std::tstring> &headings);
	void AddRow(unsigned int rowid, std::vector<std::tstring> &data);
	void UpdateRow(unsigned int rowid, std::vector<std::tstring> &data);

	void onFilterSettingsChanged();

private:
	unsigned int m_toonid;

	bool  m_sortDesc;
	int   m_sortColumn;
	
	//CListViewCtrl     m_listview;
	CButton			m_buttonRefresh;
	aoia::WebView     m_webview;
	Mutex             m_pblistMutex;
	RecipePanel		m_recipePanel;
	RecipePanel::Connection m_filterConnection;


	sqlite::IDBPtr m_db;
	aoia::IContainerManagerPtr m_containerManager;
	aoia::IGuiServicesPtr m_gui;
	DWORD m_lastView;
};

