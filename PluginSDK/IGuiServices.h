#ifndef IGUISERVICES_H
#define IGUISERVICES_H

namespace aoia
{
    struct IGuiServices
    {
        virtual void ShowHelp(std::tstring const& topic) const = 0;
        virtual void OpenURL(std::tstring const& url) const = 0;
        virtual void ShowTrayIconBalloon(std::tstring const& message) const = 0;
    };

    typedef boost::shared_ptr<IGuiServices> IGuiServicesPtr;
}

#endif // IGUISERVICES_H
