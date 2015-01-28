#ifndef ICORE_H
#define ICORE_H

#include <boost/shared_ptr.hpp>
#include <PluginSDK/IContainerManager.h>
#include <PluginSDK/IDBManager.h>
#include <PluginSDK/IGuiServices.h>


namespace aoia 
{
    struct ICore
    {
        virtual IGuiServicesPtr GuiServices() const = 0;
        virtual IDBManagerPtr DataServices() const = 0;
        virtual IContainerManagerPtr AccountServices() const = 0;
    };

    typedef boost::shared_ptr<ICore> ICorePtr;
}

#endif // ICORE_H
