#include "stdafx.h"
#include "PluginManager.h"
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

namespace bfs = boost::filesystem;
namespace ba = boost::algorithm;


SINGLETON_IMPL(PluginManager);


PluginManager::PluginManager()
{
}


PluginManager::PluginManager(const PluginManager&)
{
}


PluginManager::~PluginManager()
{
}


void PluginManager::AddLibraries(std::tstring const& path)
{
    bfs::wpath dir(path);
    if (!bfs::is_directory(dir)) {
        return;
    }

    for (bfs::directory_iterator it(dir); it != bfs::directory_iterator(); ++it) {
        bfs::wpath dirEntry = *it;
        if (bfs::is_directory(dirEntry)) {
            continue;
        }
        if (ba::iends_with(dirEntry.leaf().native(), _T(".dll"))) {
            // Try loading this DLL
            HMODULE hDll = ::LoadLibrary(dirEntry.c_str());
            if (!hDll) {
                continue;
            }

            // Look for exported plug-in factory function
            FARPROC fp = ::GetProcAddress(hDll, "AOIA_CreatePlugin");
            if (fp) {
                m_factories.push_back(fp);
            }
            else {
                ::FreeLibrary(hDll);
            }
        }
    }
}


//std::vector<boost::shared_ptr<PluginViewInterface> > PluginManager::createPlugins()
//{
//    std::vector<boost::shared_ptr<PluginViewInterface> > retval;
//    for (std::vector<FARPROC>::iterator it = m_factories.begin(); it != m_factories.end(); ++it) {
//        AOIA_CreatePlugin f = (AOIA_CreatePlugin)*it;
//        boost::shared_ptr<PluginViewInterface> plugin = f("");
//        retval.push_back(plugin);
//    }
//    return retval;
//}
