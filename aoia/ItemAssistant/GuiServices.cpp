#include "StdAfx.h"
#include "GuiServices.h"
#include "Version.h"
#include "ntray.h"


namespace aoia
{
    GuiServices::GuiServices(boost::shared_ptr<CTrayNotifyIcon> trayIcon)
        : m_trayIcon(trayIcon) {}


    GuiServices::~GuiServices() {}

    void GuiServices::ShowTrayIconBalloon(std::tstring const& message) const
    {
        m_trayIcon->SetBalloonDetails(message.c_str(), _T("AO Item Assistant++"), CTrayNotifyIcon::Info, 5000);
    }


    void GuiServices::ShowHelp(std::tstring const& topic) const
    {
        std::tstringstream url;
        url << _T("http://ia-help.frellu.net/?");
        if (!topic.empty()) {
            url << _T("topic=") << topic << _T("&");
        }
        url << _T("version=") << _T("latest");
        OpenURL(url.str());
    }


    void GuiServices::OpenURL(std::tstring const& url) const
    {
        ShellExecute(NULL, _T("open"), url.c_str(), NULL, NULL, SW_NORMAL);
    }

}
