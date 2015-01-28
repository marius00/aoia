#ifndef SHAREDSERVICES_H
#define SHAREDSERVICES_H

#include <vector>
#include <string>
#include <map>
#include <PluginSDK/IGuiServices.h>
#include <PluginSDK/IContainerManager.h>
#include <shared/IDB.h>


// Puts the hex value of the specified char into a string
#define CHAR2HEXSTR( character ) \
    (((std::ostringstream&)(std::ostringstream().flush() << std::setfill('0') \
    << std::setw(2) << std::hex << std::uppercase \
    << static_cast<unsigned int>(static_cast<unsigned char>(character)))).str())

// Encodes the character to an escape character followed by the hex code
#define ENCODECHAR( character, escapechar ) \
    (((std::ostringstream&)(std::ostringstream().flush() << escapechar \
    << std::uppercase << std::setfill('0') << std::setw(2) << std::hex \
    << static_cast<unsigned int>(static_cast<unsigned char>(character)))).str()) 

//class CTrayNotifyIcon;

class ContainerManager
    : public aoia::IContainerManager
{
public:
    ContainerManager(sqlite::IDBPtr db);
    virtual ~ContainerManager();

    // Implementation of \e IContainerManager
    virtual std::tstring GetContainerName(unsigned int character_id, unsigned int container_id) const;

    std::map<std::tstring, std::tstring> GetAOItemInfo(unsigned int lowkey) const;


	//void ClearTempContainerIdCache(unsigned int charId);
	//void UpdateTempContainerId(unsigned int charId, unsigned int tempId, unsigned int containerId);
	//unsigned int GetContainerId(unsigned int charId, unsigned int tempId) const;

	//unsigned int GetItemSlotId(unsigned int charId, unsigned int itemTempId) const;
	//void UpdateTempItemId(unsigned int charId, unsigned int itemTempId, unsigned int slotId);

protected:
    std::vector<std::tstring> GetAccountNames() const;
    std::tstring MakeContainerName(unsigned int charid, unsigned int containerid) const;

private:
    sqlite::IDBPtr m_db;
    std::vector<std::tstring> m_accounts;

    mutable std::map< __int64, std::pair<std::tstring, FILETIME> > m_containerFileCache;
    mutable std::map< __int64, std::tstring > m_containerDBCache;

	//std::map< __int32, std::map<__int32, unsigned int> > m_containerIdCache; //char id and container Id are keys.
	//std::map< __int32, std::map<__int32, unsigned int> > m_invSlotForTempItemCache; //char id and container Id are keys.
};


#endif // SHAREDSERVICES_H
