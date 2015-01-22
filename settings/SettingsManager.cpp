#include "SettingsManager.h"
#include <fstream>


namespace aoia
{

    SettingsManager::SettingsManager() {}


    SettingsManager::~SettingsManager() {}


    std::tstring SettingsManager::getValue(std::tstring const& key) const
    {
        std::map<std::tstring, std::tstring>::const_iterator it = m_values.find(key);
        if (it != m_values.end())
        {
            return it->second;
        }
        return _T("");
    }


    void SettingsManager::setValue(std::tstring const& key, std::tstring const& value)
    {
        m_values[key] = value;
    }


    void SettingsManager::readSettings(std::tstring const& filename)
    {
        m_values.clear();

        std::basic_ifstream<TCHAR> ifs(filename.c_str());

        if (!ifs.is_open())
        {
            return;
        }

        TCHAR buffer[256];
        while (!ifs.eof())
        {
            ifs.getline(buffer, 256, _T('\n'));
            std::tstring line(buffer);
            size_t splitpos = line.find(_T("="));
            if (splitpos == std::tstring::npos)
            {
                continue;
            }
            m_values[line.substr(0, splitpos)] = line.substr(splitpos + 1);
        }
    }


    void SettingsManager::writeSettings(std::tstring const& filename)
    {
        std::basic_ofstream<TCHAR> ofs(filename.c_str());
        if (!ofs.is_open())
        {
            return;
        }

        std::map<std::tstring, std::tstring>::const_iterator it = m_values.begin();
        while (it != m_values.end())
        {
            ofs << it->first << "=" << it->second << std::endl;
            ++it;
        }
    }
}
