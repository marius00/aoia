#include "StdAfx.h"
#include "Tabframe.h"
#include <boost/bind.hpp>

using namespace aoia;
using namespace Parsers;


TabFrame::TabFrame(sqlite::IDBPtr db, aoia::IContainerManagerPtr containerManager, aoia::IGuiServicesPtr gui, aoia::ISettingsPtr settings)
  : m_toobarVisibility(true)
  , m_inventoryView(db, containerManager, gui, settings)
  , m_summaryView(db, gui)
//  , m_playershopView(gui)
  , m_patternView(db, containerManager, gui, settings)
  , m_recipesView(db, containerManager, gui, settings)
  , m_identifyView(db, containerManager, gui)
{
    //SetForwardNotifications(true);
}


TabFrame::~TabFrame()
{
}


LRESULT TabFrame::OnSelChange(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled)
{
    IPluginView* oldplugin = GetActivePluginView();
    baseClass::OnSelChange(0, pnmh, bHandled);
    IPluginView* newplugin = GetActivePluginView();

    if (oldplugin) {
        oldplugin->disconnect(m_statusTextSignalConnection);
        oldplugin->OnActive(false);
    }

    if (newplugin)
    {
        newplugin->OnActive(true);

        // Assign new toolbar.
        m_activeViewToolbar.Detach();
        m_activeViewToolbar.Attach(newplugin->GetToolbar());

        int nBandIndex = m_rebarControl.IdToIndex(ATL_IDW_BAND_FIRST + 1);	// toolbar is 2nd added band
        if (nBandIndex < 0)
        {
            // Insert new band
            WTL::CFrameWindowImplBase<>::AddSimpleReBarBandCtrl(m_rebarControl, m_activeViewToolbar, ATL_IDW_BAND_FIRST + 1, NULL, TRUE);
            m_rebarControl.ShowBand(m_rebarControl.IdToIndex(ATL_IDW_BAND_FIRST + 1), m_toobarVisibility);
            m_rebarControl.LockBands(true);
        }
        else
        {
            // Replace band
            REBARBANDINFO rbbi;
            ZeroMemory(&rbbi, sizeof(REBARBANDINFO));
            rbbi.cbSize = sizeof(REBARBANDINFO);
            rbbi.fMask = RBBIM_CHILD;
            rbbi.hwndChild = m_activeViewToolbar.m_hWnd;

            m_rebarControl.SetBandInfo(nBandIndex, &rbbi);
        }

        // Update statusbar
        m_statusBar.SetText(0, newplugin->GetStatusText().c_str());
        m_statusTextSignalConnection = newplugin->connectStatusChanged(boost::bind(&TabFrame::onStatusChanged, this));
    }

    bHandled = TRUE;
    return 0;
}


void TabFrame::onStatusChanged()
{
    m_statusBar.SetText(0, GetActivePluginView()->GetStatusText().c_str());
}


LRESULT TabFrame::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    RECT defRect = { 0, 0, 640, 480 };

    baseClass::OnCreate(uMsg, wParam, lParam, bHandled);

    DWORD style = WS_CHILD | /*WS_VISIBLE |*/ WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

    m_summaryView.Create(*this, defRect, 0, style);
    AddTab(m_summaryView, _T("Summary"));
    m_viewPlugins.push_back(&m_summaryView);

    m_inventoryView.Create(*this, defRect, 0, style);
    AddTab(m_inventoryView, _T("Inventory"));
    m_viewPlugins.push_back(&m_inventoryView);

    m_patternView.Create(*this, defRect, 0, style);
    AddTab(m_patternView, _T("Pattern Matcher"));
	m_viewPlugins.push_back(&m_patternView);

	m_recipesView.Create(*this, defRect, 0, style);
	AddTab(m_recipesView, _T("Recipes"));
	m_viewPlugins.push_back(&m_recipesView);
/*
    m_playershopView.Create(*this, defRect, 0, style);
    AddTab(m_playershopView, _T("Playershop Monitor"));
    m_viewPlugins.push_back(&m_playershopView);

    m_playershopView.StartMonitoring();
	*/
	m_identifyView.Create(*this, defRect, 0, style);
	AddTab(m_identifyView, _T("Identify"));
	m_viewPlugins.push_back(&m_identifyView);

#ifdef _DEBUG
    m_msgView.Create(*this, defRect, 0, style);
    AddTab(m_msgView, _T("Messages (Debug)"));
    m_viewPlugins.push_back(&m_msgView);
#endif

    DisplayTab(m_summaryView);

    return 0;
}


IPluginView* TabFrame::GetActivePluginView()
{
    IPluginView* result = NULL;

    HWND hWnd = GetActiveView();
    if (hWnd != NULL)
    {
        for (unsigned int i = 0; i < m_viewPlugins.size(); i++)
        {
            if (m_viewPlugins[i]->GetWindow() == hWnd)
            {
                result = m_viewPlugins[i];
                break;
            }
        }
    }

    return result;
}


void TabFrame::OnAOServerMessage(AOMessageBase &msg)
{
    GetActivePluginView()->OnAOServerMessage(msg);
    for (unsigned int i = 0; i < m_viewPlugins.size(); i++)
    {
        if (m_viewPlugins[i] != GetActivePluginView()) {
            m_viewPlugins[i]->OnAOServerMessage(msg);
        }
    }
}


void TabFrame::OnAOClientMessage(AOClientMessageBase &msg)
{
    for (unsigned int i = 0; i < m_viewPlugins.size(); i++)
    {
        m_viewPlugins[i]->OnAOClientMessage(msg);
    }
}


void TabFrame::SetToolbarVisibility(bool visible)
{
    m_toobarVisibility = visible;
    m_rebarControl.ShowBand(m_rebarControl.IdToIndex(ATL_IDW_BAND_FIRST + 1), m_toobarVisibility);
}
