#ifndef IDBMANAGER_H
#define IDBMANAGER_H

#include <map>
#include <set>
#include <Shared/IDB.h>


struct OwnedItemInfo
{
    std::tstring itemloid;
    std::tstring itemhiid;
    std::tstring itemql;
    std::tstring itemname;
    std::tstring ownername;
    std::tstring ownerid;
    std::tstring containerid;
    unsigned short flags;
};

typedef boost::shared_ptr<OwnedItemInfo> OwnedItemInfoPtr;
typedef std::map<unsigned int, unsigned int> StatMap;


struct IDBManager
{
    virtual ~IDBManager() {}

    /// Records an item with the specified properties as owned by a the character.
    virtual void InsertItem(unsigned int keylow, unsigned int keyhigh, unsigned short ql, unsigned short flags, unsigned short stack, unsigned int parent, unsigned short slot, unsigned int children, unsigned int owner) = 0;

    /// Retrieve all known info about the item with the specified index.
    virtual OwnedItemInfoPtr GetOwnedItemInfo(unsigned int itemID) = 0;

    /// Store the specified name for the character ID
    virtual void SetToonName(unsigned int charid, std::tstring const& newName) = 0;

    /// Retrieve the stored name of a specified character ID
    virtual std::tstring GetToonName(unsigned int charid) const = 0;

    /// Records the specified shop ID for the character.
    virtual void SetToonShopId(unsigned int charid, unsigned int shopid) = 0;

    /// Retrieves the recorded shop ID for the specified character.
    virtual unsigned int GetToonShopId(unsigned int charid) const = 0;

    /// Retrieves the recorded (if any) character ID associated with the specified shop ID.
    virtual unsigned int GetShopOwner(unsigned int shopid) = 0;

    /// Assign a dimension ID to a specified character.
    virtual void SetToonDimension(unsigned int charid, unsigned int dimensionid) = 0;

    /// Retrieves the dimension a character belongs to. 0 means dimension is unknown.
    virtual unsigned int GetToonDimension(unsigned int charid) const = 0;

    /// Record the current stats for a specific toon.
    virtual void SetToonStats(unsigned int charid, StatMap const& stats) = 0;

    /// Retrieves all the dimension IDs and their descriptions. Returns false if query failed.
    virtual bool GetDimensions(std::map<unsigned int, std::tstring>& dimensions) const = 0;

    /// Determines the first available container slot ID for a specified character and container.
    virtual unsigned int FindNextAvailableContainerSlot(unsigned int charId, unsigned int containerId) = 0;

    /// returns the properties value in the AO db for an item in a particular slot in a container.
    virtual unsigned int GetItemProperties(unsigned int charId, unsigned int containerId, unsigned int slot) = 0;

    /// searches for items in containerIdToSearchIn with the same keylow and ql as the item specified
    virtual unsigned int FindFirstItemOfSameType(unsigned int charId, unsigned int containerId, unsigned int slot, unsigned int containerIdToSearchIn) = 0;

    /// Deletes the specific set of item IDs from the database.
    virtual void DeleteItems(std::set<unsigned int> const& ids) const = 0;
};

typedef boost::shared_ptr<IDBManager> IDBManagerPtr;


#endif // IDBMANAGER_H
