#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <map>
#include <settings/ISettings.h>


namespace aoia {

    class SettingsManager
        : public ISettings
    {
    public:
        SettingsManager();
        ~SettingsManager();

        // ISettings implementation
        std::tstring getValue(std::tstring const& key) const;
        void setValue(std::tstring const& key, std::tstring const& value);

        /// Reads in settings from a specified file.
        void readSettings(std::tstring const& filename);

        /// Writes current settings to the specified file.
        void writeSettings(std::tstring const& filename);

    private:
        std::map<std::tstring, std::tstring> m_values;
    };
}

#endif // SETTINGSMANAGER_H
