#pragma warning(disable: 4251)

#ifndef AOMANAGER_H
#define AOMANAGER_H

#include <ItemAssistantCore/ItemAssistantCore.h>
#include <Shared/UnicodeSupport.h>
#include <Shared/Singleton.h>
#include <settings/ISettings.h>
#include <vector>
#include <exception>


class ITEMASSISTANTCORE_API AOManager
{
    SINGLETON(AOManager);
public:
    ~AOManager();

    struct ITEMASSISTANTCORE_API AOManagerException : public std::exception {
        AOManagerException(std::tstring const& message) : std::exception(to_ascii_copy(message).c_str()) {}
    };

    void SettingsManager(aoia::ISettingsPtr settings);
    std::tstring getAOFolder() const;
	std::tstring getAOPrefsFolder() const;
	std::tstring getAOPrefsFolderManual() const;
	bool getAutoPrefs() const;
	void AOManager::setAutoPrefs(bool);
	bool getShowCreds() const;
	void AOManager::setShowCreds(bool);
	bool getMinToTaskbar() const;
	void AOManager::setMinToTaskbar(bool);
    std::vector<std::tstring> getAccountNames() const;

private:
    mutable std::tstring m_aofolder;
    mutable std::tstring m_prefsfolder;
	mutable std::tstring m_autoprefs;
	mutable std::tstring m_showcreds;
	mutable std::tstring m_minToTaskbar;
    aoia::ISettingsPtr m_settings;
};

#endif // AOMANAGER_H
