#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <shared/IDB.h>
#include <shared/Mutex.h>
#include <vector>
#include <PluginSDK/IDBManager.h>


class DBManager
    : public IDBManager
{
public:
    DBManager();
    virtual ~DBManager();

    bool init(std::tstring dbfile);
    void destroy();

    /// Records an item with the specified properties as owned by a the character.
    void InsertItem(unsigned int keylow, unsigned int keyhigh, unsigned short ql, unsigned short flags, unsigned short stack, 
        unsigned int parent, unsigned short slot, unsigned int children, unsigned int owner);

    /// Retrieve all known info about the item with the specified index.
    OwnedItemInfoPtr GetOwnedItemInfo(unsigned int itemID);

    /// Store the specified name for the character ID
    void SetToonName(unsigned int charid, std::tstring const& newName);

    /// Retrieve the stored name of a specified character ID
    std::tstring GetToonName(unsigned int charid) const;

    /// Records the specified shop ID for the character.
    void SetToonShopId(unsigned int charid, unsigned int shopid);

    /// Retrieves the recorded shop ID for the specified character.
    unsigned int GetToonShopId(unsigned int charid) const;

    /// Retrieves the recorded (if any) character ID associated with the specified shop ID.
    unsigned int GetShopOwner(unsigned int shopid);

    /// Assign a dimension ID to a specified character.
    void SetToonDimension(unsigned int charid, unsigned int dimensionid);

    /// Retrieves the dimension a character belongs to. 0 means dimension is unknown.
    unsigned int GetToonDimension(unsigned int charid) const;

    /// Record the current stats for a specific toon.
    void SetToonStats(unsigned int charid, StatMap const& stats);

    /// Retrieves all the dimension IDs and their descriptions. Returns false if query failed.
    bool GetDimensions(std::map<unsigned int, std::tstring> &dimensions) const;

    /// Determines the first available container slot ID for a specified character and container.
    unsigned int FindNextAvailableContainerSlot(unsigned int charId, unsigned int containerId);

	//returns the properties value in the AO db for an item in a particular slot in a container.
	unsigned int GetItemProperties(unsigned int charId, unsigned int containerId, unsigned int slot);
	
	//searches for items in containerIdToSearchIn with the same keylow and ql as the item specified
	unsigned int FindFirstItemOfSameType(unsigned int charId, unsigned int containerId, unsigned int slot, unsigned int containerIdToSearchIn);

    virtual void DeleteItems( std::set<unsigned int> const& ids ) const;

    /// Lock database for access.
    void Lock() { m_mutex.MutexOn(); }

    /// Release database lock.
    void UnLock() { m_mutex.MutexOff(); }

    sqlite::IDBPtr GetDatabase() const { return m_db; }

protected:
    bool syncLocalItemsDB(std::tstring const& localfile, std::tstring const& aofolder);

    unsigned int getAODBSchemeVersion(std::tstring const& filename) const;
    unsigned int getDBVersion() const;
    void updateDBVersion(unsigned int fromVersion) const;
    void createDBScheme() const;

private:
    Mutex m_mutex;
    sqlite::IDBPtr m_db;
};

#endif // DBMANAGER_H
