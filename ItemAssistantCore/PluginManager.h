#pragma warning(disable: 4251)

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <ItemAssistantCore/ItemAssistantCore.h>
#include <vector>
#include <boost/smart_ptr.hpp>
#include <shared/UnicodeSupport.h>
#include <Shared/Singleton.h>


class ITEMASSISTANTCORE_API PluginManager
{
    SINGLETON(PluginManager);
public:
    ~PluginManager();

    void AddLibraries(std::tstring const& path);
    //std::vector<boost::shared_ptr<PluginViewInterface> > createPlugins();

private:
	std::vector<FARPROC> m_factories;
};

#endif // PLUGINMANAGER_H
