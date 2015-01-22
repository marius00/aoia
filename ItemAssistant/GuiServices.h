#ifndef GUISERVICES_H
#define GUISERVICES_H

#include <PluginSDK/IGuiServices.h>


class CTrayNotifyIcon;

namespace aoia
{
    class GuiServices
        : public IGuiServices
    {
    public:
        GuiServices(boost::shared_ptr<CTrayNotifyIcon> trayIcon);
        virtual ~GuiServices();

        virtual void ShowTrayIconBalloon(std::tstring const& message) const;

        /// Opens the online help for the specified topic. 
        /// (ex: topic = "patternmatcher" will expand to http://ia.frellu.net/help.php?topic=patternmatcher&version=0.9.0")
        virtual void ShowHelp(std::tstring const& topic) const;

        /// Opens the specified URL in a new browser window.
        virtual void OpenURL(std::tstring const& url) const;

    private:
        boost::shared_ptr<CTrayNotifyIcon> m_trayIcon;
    };

}



#endif // GUISERVICES_H
