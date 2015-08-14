#include "StdAfx.h"
#include "dbmanager.h"
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <shared/AODatabaseParser.h>
#include <shared/AODatabaseWriter.h>
#include <Shared/AODatabaseIndex.h>
#include <shared/FileUtils.h>
#include <sstream>
#include "ProgressDialog.h"
#include <ItemAssistantCore/AOManager.h>
#include <boost/assign.hpp>

namespace bfs = boost::filesystem;

#define CURRENT_DB_VERSION 8


DBManager::DBManager()
{
    m_db.reset(new sqlite::Db(Logger::instance().stream()));
}


DBManager::~DBManager()
{
}


bool DBManager::init(std::tstring dbfile)
{
    std::tstring aofolder = AOManager::instance().getAOFolder();
    if (aofolder.empty())
    {
        LOG("DBManager::init: Not a valid AO folder.");
        return false;
    }

    if (dbfile.empty())
    {
        dbfile = _T("ItemAssistant.db");
    }

    bool dbfileExists = bfs::exists(bfs::tpath(dbfile));

    if (!m_db->Init(dbfile))
    {
        LOG("DBManager::init: Unable to " << (dbfileExists ? "open" : "create") << " database. [" << dbfile << "]");
        return false;
    }

    if (!dbfileExists)
    {
        createDBScheme();
    }

    unsigned int dbVersion = getDBVersion();
    if (dbVersion < CURRENT_DB_VERSION)
    {
        if (IDOK != MessageBox(NULL, _T("AO Item Assistant needs to update its database file to a newer version."),
                               _T("Question - AO Item Assistant++"), MB_OKCANCEL | MB_ICONQUESTION))
        {
            return false;
        }
        updateDBVersion(dbVersion);
    }
    else if (dbVersion > CURRENT_DB_VERSION)
    {
        MessageBox(NULL,
                   _T("AO Item Assistant has detected a too new version of its database file. You should upgrade the software to continue."),
                   _T("Error - AO Item Assistant++"), MB_OK | MB_ICONERROR);
        return false;
    }

    if (!syncLocalItemsDB(_T("aoitems.db"), aofolder))
    {
        MessageBox(NULL, _T("AO Item Assistant cannot start without a valid item database."),
                   _T("Error - AO Item Assistant++"), MB_OK | MB_ICONERROR);
        return false;
    }

    m_db->Exec(_T("ATTACH DATABASE \"aoitems.db\" AS aodb"));

    return true;
}


void DBManager::destroy()
{
    m_db->Term();
}


/** 
* Check to see if we have a local database already.
* If it is, then check if it is up to date.
* If local database is missing or obsolete then recreate it.
* Return true if application has a local items database to run with, false otherwise.
*/
bool DBManager::syncLocalItemsDB(std::tstring const& localfile, std::tstring const& aofolder)
{
    bool hasLocalDB = false;
    std::time_t lastUpdateTime;

    bfs::path local(to_ascii_copy (localfile));
    bfs::path original(to_ascii_copy (aofolder));
    original = original / "cd_image/data/db/ResourceDatabase.dat";

    if (bfs::exists(local) && bfs::is_regular(local))
    {
        hasLocalDB = true;
        lastUpdateTime = bfs::last_write_time(local);
    }

    if (!exists(original))
    {
        Logger::instance().log(_T("Could not locate the original AO database."));
        return hasLocalDB;
    }

    if (hasLocalDB && getAODBSchemeVersion(localfile) == CURRENT_AODB_VERSION)
    {
        std::time_t lastOriginalUpdateTime = bfs::last_write_time(original);
        if (lastOriginalUpdateTime <= lastUpdateTime)
        {
            return true;
        }

        // Ask user if he wants to continue using the old (but compatible) DB or update it now.
        int answer = ::MessageBox(NULL,
                                  _T("You items database is out of date. Do you wish to update it now?\r\nAnswering 'NO' will continue using the old one."),
                                  _T("Question - AO Item Assistant++"), MB_ICONQUESTION | MB_YESNOCANCEL);
        if (answer == IDCANCEL)
        {
            exit(0);
        }
        else if (answer == IDNO)
        {
            return true;
        }
    }
 
    // If we come this far we need to update the DB.

    bfs::path tmpfile("tmp_" + local.string());
    bfs::remove(tmpfile);

    try
    {
        std::set<ResourceType> resource_types = boost::assign::list_of(AODB_TYP_ITEM)(AODB_TYP_NANO);
        AODatabaseIndex indexer(to_ascii_copy(aofolder) + "/cd_image/data/db/ResourceDatabase.idx", resource_types);
        std::vector<unsigned int> item_offsets = indexer.GetOffsets(AODB_TYP_ITEM);
        std::vector<unsigned int> nano_offsets = indexer.GetOffsets(AODB_TYP_NANO);
        unsigned int itemCount = item_offsets.size();
        unsigned int nanoCount = nano_offsets.size();

        std::vector<std::string> original_files = boost::assign::list_of(original.string())(original.string()+".001");
		for (int i = 2; i < 1000; i++) {
			char buf[512] = {0};
			sprintf(buf, "%03d", i);

			if (std::ifstream(original.string()+"." + buf)) {
				original_files.push_back( original.string()+"." + buf );
			} else {
				break;
			}
		}
        AODatabaseParser aodb(original_files);
        AODatabaseWriter writer(tmpfile.string(), Logger::instance().stream());

        CProgressDialog dlg(itemCount + nanoCount, itemCount);
        dlg.SetWindowText(_T("Progress Dialog - Item Assistant"));
        dlg.setText(0, _T("Extracting data from the AO DB..."));
        dlg.setText(1, STREAM2STR("Finished " << 0 << " out of " << itemCount << " items."));
        dlg.setText(2, _T("Overall progress: 0%"));

        // Extract items
        boost::shared_ptr<ao_item> item;
        unsigned int count = 0;
        writer.BeginWrite();
        for (std::vector<unsigned int>::iterator item_it = item_offsets.begin(); item_it != item_offsets.end(); ++item_it)
        {
            item = aodb.GetItem(*item_it);
            count++;
            if (!item)
            {
                LOG(_T("Parsing item ") << count << _T(" at offset ") << *item_it << _T(" failed!"));
                continue;
            }
            writer.WriteItem(item);
            if (count % 1000 == 0) {
                if (dlg.userCanceled()) {
                    return false;
                }
                dlg.setTaskProgress(count, itemCount);
                dlg.setText(1, STREAM2STR("Finished " << count << " out of " << itemCount << " items."));
                dlg.setOverallProgress(count, itemCount + nanoCount);
                dlg.setText(2, STREAM2STR("Overall progress: " << (count * 100) / max(1, itemCount + nanoCount) << "%"));
            }
            if (count % 10000 == 0) {
                writer.CommitItems();
                writer.BeginWrite();
            }
        }
        item.reset();
        dlg.setTaskProgress(count, itemCount);
        dlg.setText(1, STREAM2STR("Finished " << count << " out of " << itemCount << " items."));
        dlg.setOverallProgress(count, itemCount + nanoCount);
        dlg.setText(2, STREAM2STR("Overall progress: " << (count * 100) / max(1, itemCount + nanoCount) << "%"));
        writer.CommitItems();

        if (dlg.userCanceled())
        {
            return false;
        }

        // Extract nano programs
        boost::shared_ptr<ao_item> nano;
        count = 0;
        writer.BeginWrite();
        for (std::vector<unsigned int>::iterator nano_it = nano_offsets.begin(); nano_it != nano_offsets.end(); ++nano_it)
        {
            nano = aodb.GetItem(*nano_it);
            count++;
            if (!nano)
            {
                LOG(_T("Parsing nano ") << count << _T(" at offset ") << *nano_it << _T(" failed!"));
                continue;
            }
            writer.WriteItem(nano);
            if (count % 1000 == 0)
            {
                if (dlg.userCanceled())
                {
                    return false;
                }
                dlg.setTaskProgress(count, nanoCount);
                dlg.setText(1, STREAM2STR("Finished " << count << " out of " << nanoCount << " nanos."));
                dlg.setOverallProgress(itemCount + count, itemCount + nanoCount);
                dlg.setText(2, STREAM2STR("Overall progress: " << ((itemCount + count) * 100) / max(1, itemCount +
                                          nanoCount) << "%"));
            }
            if (count % 10000 == 0)
            {
                writer.CommitItems();
                writer.BeginWrite();
            }
        }
        nano.reset();
        dlg.setTaskProgress(count, nanoCount);
        dlg.setText(1, STREAM2STR("Finished " << count << " out of " << nanoCount << " nanos."));
        dlg.setOverallProgress(itemCount + count, itemCount + nanoCount);
        dlg.setText(2, STREAM2STR("Overall progress: " << ((itemCount + count) * 100) / max(1, itemCount + nanoCount) << "%"));
        writer.CommitItems();

        if (dlg.userCanceled())
        {
            return false;
        }
 
        writer.PostProcessData();
    }
    catch (std::bad_alloc& e)
    {
        assert(false);
        LOG(_T("Error creating item database. ") << e.what());
        MessageBox(NULL,
                   _T("Unable to parse the AO database.\n\rMore details might be found in the log-file (if enabled)."),
                   _T("Error - AO Item Assistant++"), MB_OK | MB_ICONERROR);
        return false;
    }
    catch (AODatabaseParser::Exception& e)
    {
        assert(false);
        LOG(_T("Error creating item database. ") << e.what());
        MessageBox(NULL,
                   _T("Unable to parse the AO database.\n\rMore details might be found in the log-file (if enabled)."),
                   _T("Error - AO Item Assistant++"), MB_OK | MB_ICONERROR);
        return false;
    }
    catch (std::exception& e)
    {
        assert(false);
        LOG(_T("Error creating item database. ") << e.what());
        MessageBox(NULL,
                   _T("Unable to parse the AO database.\n\rMore details might be found in the log-file (if enabled)."),
                   _T("Error - AO Item Assistant++"), MB_OK | MB_ICONERROR);
        return false;
    }
 
    remove(local);
    rename(tmpfile, local);

    return true;
}


void DBManager::InsertItem(unsigned int keylow,
    unsigned int keyhigh,
    unsigned short ql,
    unsigned short flags,
    unsigned short stack,
    unsigned int parent,
    unsigned short slot,
    unsigned int children,
    unsigned int owner)
{
    std::tstringstream sql;
    sql << _T("INSERT INTO tItems (keylow, keyhigh, ql, flags, stack, parent, slot, children, owner) VALUES (")
        << (unsigned int) keylow      << _T(", ")
        << (unsigned int) keyhigh     << _T(", ")
        << (unsigned int) ql          << _T(", ")
        << (unsigned int) flags       << _T(", ")
        << (unsigned int) stack       << _T(", ")
        << (unsigned int) parent      << _T(", ")
        << (unsigned int) slot        << _T(", ")
        << (unsigned int) children    << _T(", ")
        << (unsigned int) owner       << _T(")");
    m_db->Exec(sql.str());
}


std::tstring DBManager::GetToonName(unsigned int charid) const
{
    std::tstring result;

    sqlite::ITablePtr pT = m_db->ExecTable(STREAM2STR("SELECT charid, charname FROM tToons WHERE charid = " << charid));

    if (pT != NULL && pT->Rows())
    {
        if (!pT->Data(0, 1).empty())
        {
            result = from_ascii_copy(pT->Data(0, 1));
        }
    }

    return result;
}


void DBManager::SetToonName(unsigned int charid, std::tstring const& newName)
{
    m_db->Begin();

    boost::format sql("INSERT OR REPLACE INTO tToons (charid, charname) VALUES (%1%, '%2%')");
    sql % charid % to_ascii_copy(newName);

    if (!m_db->Exec(sql.str()))
    {
        // Insert failed, so update existing record instead.
        sql = boost::format("UPDATE OR IGNORE tToons SET charname='%1%' WHERE charid=%2%");
        sql % to_ascii_copy(newName) % charid;
        m_db->Exec(sql.str());
    }

    m_db->Commit();
}


void DBManager::SetToonShopId(unsigned int charid, unsigned int shopid)
{
    assert(charid != 0);
    assert(shopid != 0);

    boost::format sql("UPDATE OR IGNORE tToons SET shopid=%1% WHERE charid=%2%");
    sql % shopid % charid;

    m_db->Begin();
    m_db->Exec(sql.str());
    m_db->Commit();
}


unsigned int DBManager::GetToonShopId(unsigned int charid) const
{
    assert(charid != 0);

    unsigned int result = 0;

    sqlite::ITablePtr pT = m_db->ExecTable(STREAM2STR("SELECT shopid FROM tToons WHERE charid = " << charid));
    if (pT != NULL && pT->Rows())
    {
        try
        {
            result = boost::lexical_cast<unsigned int>(pT->Data(0,0));
        }
        catch (boost::bad_lexical_cast &/*e*/)
        {
            LOG("Error in getToonShopId(). Bad lexical cast.");
            // Wierd.. lets debug!
            assert(false);
        }
    }

    return result;
}


void DBManager::SetToonDimension(unsigned int charid, unsigned int dimensionid)
{
    assert(charid != 0);
    assert(dimensionid != 0);
    m_db->Begin();
    m_db->Exec(STREAM2STR("UPDATE OR IGNORE tToons SET dimensionid = " << dimensionid << " WHERE charid = " << charid));
    m_db->Commit();
}


unsigned int DBManager::GetToonDimension(unsigned int charid) const
{
    assert(charid != 0);

    unsigned int result = 0;

    sqlite::ITablePtr pT = m_db->ExecTable(STREAM2STR("SELECT dimensionid FROM tToons WHERE charid = " << charid));
    if (pT != NULL && pT->Rows())
    {
        try
        {
            result = boost::lexical_cast<unsigned int>(pT->Data(0,0));
        }
        catch (boost::bad_lexical_cast &/*e*/)
        {
            LOG("Error in getToonDimension(). Bad lexical cast.");
            // Wierd.. lets debug!
            assert(false);
        }
    }

    return result;
}


void DBManager::SetToonStats(unsigned int charid, StatMap const& stats)
{
    assert(charid != 0);
    TRACE("Updating stats on character id " << charid);

    m_db->Begin();
    m_db->Exec(STREAM2STR("DELETE FROM tToonStats WHERE charid = " << charid));
    for (StatMap::const_iterator it = stats.begin(); it != stats.end(); ++it)
    {
        unsigned int statid = it->first;
        unsigned int statvalue = it->second;
        m_db->Exec(STREAM2STR("INSERT OR IGNORE INTO tToonStats VALUES (" << charid << ", " << statid << ", " << statvalue << ")"));
    }
    m_db->Commit();
}


bool DBManager::GetDimensions(std::map<unsigned int, std::tstring> &dimensions) const
{
    sqlite::ITablePtr pT = m_db->ExecTable(STREAM2STR("SELECT dimensionid, dimensionname FROM tDimensions"));

    if (pT != NULL && pT->Rows())
    {
        for (unsigned int i = 0; i < pT->Rows(); ++i)
        {
            try
            {
                std::tstring name = from_ascii_copy(pT->Data(i, 1));
                dimensions[boost::lexical_cast<unsigned int>(pT->Data(i, 0))] = name;
            }
            catch (boost::bad_lexical_cast &/*e*/)
            {
                LOG("Error in getDimensions(). Bad lexical cast.");
                // Wierd.. lets debug!
                assert(false);
            }
        }
        return true;
    }
    return false;
}


unsigned int DBManager::FindNextAvailableContainerSlot(unsigned int charId, unsigned int containerId)
{
    assert(charId != 0);
    assert(containerId != 0);

    unsigned short posSlot = 0;

    if (containerId == 2)
        posSlot = 64; //start at slot 64 for inventory!

    sqlite::ITablePtr pT = m_db->ExecTable(STREAM2STR("SELECT slot FROM tItems WHERE parent = " << containerId << " AND slot >= " << posSlot
        << " AND owner = " << charId << " ORDER by slot"));

    if (pT == NULL || !pT->Rows())
        return 0; //empty backpack

    unsigned int count = pT->Rows();

    for (unsigned i=0;i<count;i++)
    {
        try
        {
            if (posSlot < boost::lexical_cast<unsigned short>(pT->Data(i,0)))
            {
                return posSlot; //we found a free slot in-between
            }
        }
        catch (boost::bad_lexical_cast &/*e*/)
        {
            //return 0xff; //Can't really imagine this to be possible
        }

        posSlot++;
    }

    return posSlot; //we return the next available slot
}


//returns the properties value in the AO db for an item in a particular slot in a container.
unsigned int DBManager::GetItemProperties(unsigned int charId, unsigned int containerId, unsigned int slot)
{
    assert(charId != 0);
    assert(containerId != 0);
    assert(slot >= 0);

    unsigned int result = 0;

    std::tstringstream sql;
    sql << _T("SELECT properties FROM tItems JOIN tblAO ON keylow = aoid WHERE owner = ") << charId
        << _T(" AND parent = ") << containerId << _T(" AND slot = ") << slot;

    //OutputDebugString(sql.str().c_str());

    sqlite::ITablePtr pT = m_db->ExecTable(sql.str());

    try
    {
        if (pT != NULL && pT->Rows())
        {
            result = boost::lexical_cast<unsigned int>(pT->Data(0, 0));
        }
    }
    catch (boost::bad_lexical_cast &/*e*/)
    {
    }

    return result;
}


//searches for items in containerIdToSearchIn with the same keylow and ql as the item specified
unsigned int DBManager::FindFirstItemOfSameType(unsigned int charId, unsigned int containerId, unsigned int slot, unsigned int containerIdToSearchIn)
{
    assert(charId != 0);
    assert(containerId != 0);
    assert(slot >= 0);
    assert(containerIdToSearchIn != 0);

    unsigned int result = 0;

    std::tstringstream sql;
    sql << _T("SELECT tTarget.slot FROM tItems tTarget, tItems tSource WHERE tSource.owner = ") << charId
        << _T(" AND tSource.parent = ") << containerId 
        << _T(" AND tSource.slot = ") << slot
        << _T(" AND tTarget.keylow = tSource.keylow AND tTarget.ql = tSource.ql")
        << _T(" AND tTarget.owner = ") << charId
        << _T(" AND tTarget.parent = ") << containerIdToSearchIn
        << _T(" ORDER BY tTarget.slot LIMIT 1");

    //OutputDebugString(sql.str().c_str());

    sqlite::ITablePtr pT = m_db->ExecTable(sql.str());

    try
    {
        if (pT != NULL && pT->Rows())
        {
            result = boost::lexical_cast<unsigned int>(pT->Data(0, 0));
        }
    }
    catch (boost::bad_lexical_cast &/*e*/)
    {
    }

    return result;
}


void DBManager::DeleteItems( std::set<unsigned int> const& ids ) const
{
    if (ids.empty())
    {
        return;
    }

    std::list<std::tstring> idStrings;
    for (std::set<unsigned int>::const_iterator it = ids.begin(); it != ids.end(); ++it)
    {
        idStrings.push_back(STREAM2STR(*it));
    }
    std::tstring idList = boost::join(idStrings, _T(","));

    std::tstringstream sql;
    sql << _T("DELETE FROM tItems WHERE itemidx IN(") << idList << _T(")");

    m_db->Exec(sql.str());
}


unsigned int DBManager::GetShopOwner(unsigned int shopid)
{
    assert(shopid != 0);
    unsigned int result = 0;

    sqlite::ITablePtr pT = m_db->ExecTable(STREAM2STR("SELECT charid FROM tToons WHERE shopid = " << shopid));
    try
    {
        if (pT != NULL && pT->Rows())
        {
            result = boost::lexical_cast<unsigned int>(pT->Data(0, 0));
        }
    }
    catch (boost::bad_lexical_cast &/*e*/)
    {
    }

    return result;
}


OwnedItemInfoPtr DBManager::GetOwnedItemInfo(unsigned int itemID)
{
    OwnedItemInfoPtr pRetVal(new OwnedItemInfo());

    std::tstringstream sql;
    sql << _T("SELECT tItems.keylow AS itemloid, tItems.keyhigh AS itemhiid, tItems.ql AS itemql, name AS itemname, ")
        << _T("(SELECT tToons.charname FROM tToons WHERE tToons.charid = owner) AS ownername, owner AS ownerid, ")
        << _T("parent AS containerid, tItems.flags ")
        << _T("FROM tItems JOIN tblAO ON keylow = aoid WHERE itemidx = ") << (int)itemID;

    sqlite::ITablePtr pT = m_db->ExecTable(sql.str());

    pRetVal->itemloid = from_ascii_copy(pT->Data(0, 0));
    pRetVal->itemhiid = from_ascii_copy(pT->Data(0, 1));
    pRetVal->itemql = from_ascii_copy(pT->Data(0, 2));
    pRetVal->itemname = from_ascii_copy(pT->Data(0, 3));
    pRetVal->ownername = from_ascii_copy(pT->Data(0, 4));
    pRetVal->ownerid = from_ascii_copy(pT->Data(0, 5));
    pRetVal->containerid = from_ascii_copy(pT->Data(0, 6));
    pRetVal->flags = boost::lexical_cast<unsigned short>(pT->Data(0, 7));

    return pRetVal;
}


unsigned int DBManager::getAODBSchemeVersion(std::tstring const& filename) const
{
    unsigned int retval = 0;
    sqlite::Db db(Logger::instance().stream());

    if (db.Init(filename))
    {
        try
        {
            sqlite::ITablePtr pT = db.ExecTable(_T("SELECT Version FROM vSchemeVersion"));
            retval = boost::lexical_cast<unsigned int>(pT->Data(0,0));
        }
        catch (sqlite::Db::QueryFailedException &/*e*/)
        {
            retval = 0;
        }
        catch (boost::bad_lexical_cast &/*e*/)
        {
            retval = 0;
        }
    }

    return retval;
}


unsigned int DBManager::getDBVersion() const
{
    unsigned int retval = 0;

    try 
    {
        sqlite::ITablePtr pT = m_db->ExecTable(_T("SELECT Version FROM vSchemeVersion"));
        retval = boost::lexical_cast<unsigned int>(pT->Data(0,0));
    }
    catch (sqlite::Db::QueryFailedException &/*e*/)
    {
        retval = 0;
    }
    catch (boost::bad_lexical_cast &/*e*/)
    {
        retval = 0;
    }

    return retval;
}


void DBManager::updateDBVersion(unsigned int fromVersion) const
{
    switch (fromVersion)
    {
    case 0:
        {
            m_db->Begin();
            m_db->Exec(_T("CREATE TABLE tToons2 (charid, charname)"));
            m_db->Exec(_T("INSERT INTO tToons2 (charid, charname) SELECT charid, charname FROM tToons"));
            m_db->Exec(_T("DROP TABLE tToons"));
            m_db->Exec(_T("CREATE TABLE tToons (charid, charname)"));
            m_db->Exec(_T("CREATE UNIQUE INDEX iCharId ON tToons (charid)"));
            m_db->Exec(_T("INSERT INTO tToons (charid, charname) SELECT charid, charname FROM tToons2"));
            m_db->Exec(_T("DROP TABLE tToons2"));
            m_db->Commit();
        }
        // Dropthrough

    case 1:
        {
            m_db->Begin();
            m_db->Exec(_T("CREATE TABLE tToons2 (charid INTEGER NOT NULL PRIMARY KEY UNIQUE, charname VARCHAR)"));
            m_db->Exec(_T("INSERT INTO tToons2 (charid, charname) SELECT charid, charname FROM tToons"));
            m_db->Exec(_T("DROP TABLE tToons"));
            m_db->Exec(_T("CREATE TABLE tToons (charid INTEGER NOT NULL PRIMARY KEY UNIQUE, charname VARCHAR)"));
            m_db->Exec(_T("INSERT INTO tToons (charid, charname) SELECT charid, charname FROM tToons2"));
            m_db->Exec(_T("DROP TABLE tToons2"));
            m_db->Exec(_T("CREATE UNIQUE INDEX iCharId ON tToons (charid)"));
            m_db->Commit();
        }
        // Dropthrough

    case 2: // Update from v2 is the added shopid column in the toons table
        {
            m_db->Begin();
            m_db->Exec(_T("CREATE TABLE tToons2 (charid INTEGER NOT NULL PRIMARY KEY UNIQUE, charname VARCHAR, shopid INTEGER DEFAULT '0')"));
            m_db->Exec(_T("INSERT INTO tToons2 (charid, charname) SELECT charid, charname FROM tToons"));
            m_db->Exec(_T("DROP TABLE tToons"));
            m_db->Exec(_T("CREATE TABLE tToons (charid INTEGER NOT NULL PRIMARY KEY UNIQUE, charname VARCHAR, shopid INTEGER DEFAULT '0')"));
            m_db->Exec(_T("INSERT INTO tToons (charid, charname) SELECT charid, charname FROM tToons2"));
            m_db->Exec(_T("DROP TABLE tToons2"));
            m_db->Exec(_T("CREATE UNIQUE INDEX iCharId ON tToons (charid)"));
            m_db->Commit();
        }
        // Dropthrough

    case 3: // Update from v3 is the added flags column in tItems as well as the new dimension table and a new dimensionid column in tToons.
        {
            m_db->Begin();
            m_db->Exec(_T("CREATE TABLE tItems2 (itemidx INTEGER NOT NULL PRIMARY KEY ON CONFLICT REPLACE AUTOINCREMENT UNIQUE DEFAULT '1', keylow INTEGER, keyhigh INTEGER, ql INTEGER, flags INTEGER DEFAULT '0', stack INTEGER DEFAULT '1', parent INTEGER NOT NULL DEFAULT '2', slot INTEGER, children INTEGER, owner INTEGER NOT NULL)"));
            m_db->Exec(_T("INSERT INTO tItems2 (itemidx, keylow, keyhigh, ql, stack, parent, slot, children, owner) SELECT itemidx, keylow, keyhigh, ql, stack, parent, slot, children, owner FROM tItems"));
            m_db->Exec(_T("DROP TABLE tItems"));
            m_db->Exec(_T("CREATE TABLE tItems (itemidx INTEGER NOT NULL PRIMARY KEY ON CONFLICT REPLACE AUTOINCREMENT UNIQUE DEFAULT '1', keylow INTEGER, keyhigh INTEGER, ql INTEGER, flags INTEGER DEFAULT '0', stack INTEGER DEFAULT '1', parent INTEGER NOT NULL DEFAULT '2', slot INTEGER, children INTEGER, owner INTEGER NOT NULL)"));
            m_db->Exec(_T("INSERT INTO tItems (itemidx, keylow, keyhigh, ql, stack, parent, slot, children, owner) SELECT itemidx, keylow, keyhigh, ql, stack, parent, slot, children, owner FROM tItems2"));
            m_db->Exec(_T("DROP TABLE tItems2"));
            m_db->Exec(_T("CREATE TABLE tDimensions (dimensionid INTEGER NOT NULL PRIMARY KEY UNIQUE, dimensionname VARCHAR)"));
            m_db->Exec(_T("INSERT INTO tDimensions (dimensionid, dimensionname) VALUES (13, 'Rubi-Ka')"));
            m_db->Exec(_T("INSERT INTO tDimensions (dimensionid, dimensionname) VALUES (15, 'TestLive')"));
            m_db->Exec(_T("CREATE TABLE tToons2 (charid INTEGER NOT NULL PRIMARY KEY UNIQUE, charname VARCHAR, shopid INTEGER DEFAULT '0', dimensionid INTEGER DEFAULT '0')"));
            m_db->Exec(_T("INSERT INTO tToons2 (charid, charname, shopid) SELECT charid, charname, shopid FROM tToons"));
            m_db->Exec(_T("DROP TABLE tToons"));
            m_db->Exec(_T("CREATE TABLE tToons (charid INTEGER NOT NULL PRIMARY KEY UNIQUE, charname VARCHAR, shopid INTEGER DEFAULT '0', dimensionid INTEGER DEFAULT '0')"));
            m_db->Exec(_T("INSERT INTO tToons (charid, charname, shopid) SELECT charid, charname, shopid FROM tToons2"));
            m_db->Exec(_T("DROP TABLE tToons2"));
            m_db->Exec(_T("CREATE UNIQUE INDEX iCharId ON tToons (charid)"));
            m_db->Commit();
        }
        // Dropthrough

    case 4: // Updates from v4: Added toon-stats table.
        {
            m_db->Begin();
            m_db->Exec(_T("CREATE TABLE tToonStats (charid INTEGER NOT NULL, statid INTEGER NOT NULL, statvalue INTEGER NOT NULL, FOREIGN KEY (charid) REFERENCES tToons (charid))"));
            m_db->Exec(_T("CREATE UNIQUE INDEX charstatindex ON tToonStats (charid ASC, statid ASC)"));
            m_db->Commit();
        }
        // Dropthrough

    case 5: // Updates from v5: Added index to tItems
        {
            m_db->Begin();
            m_db->Exec("CREATE INDEX idx_titems_keylow ON tItems (keylow ASC)");
            m_db->Commit();
        }
        // Dropthrough

    case 6: // Updates from v6: Redid missing indexes from previous updates.
        {
            m_db->Begin();
            m_db->Exec("DROP INDEX IF EXISTS iOwner");
            m_db->Exec("DROP INDEX IF EXISTS iParent");
            m_db->Exec("CREATE INDEX idx_titems_owner ON tItems (owner)");
            m_db->Exec("CREATE INDEX idx_titems_parent ON tItems (parent)");
            m_db->Commit();
        }
        // Dropthrough

	case 7: // Updates from v7: Include 'Account' in tToons
		{
            m_db->Begin();
			m_db->Exec("DELETE * FROM tDimensions");
			m_db->Exec("INSERT INTO tDimensions (dimensionid, dimensionname) VALUES (13, 'Rubi-Ka')");
			m_db->Exec("INSERT INTO tDimensions (dimensionid, dimensionname) VALUES (15, 'TestLive')");
            m_db->Commit();
		}
		// Dropthrough

    default:
        m_db->Exec("DROP VIEW IF EXISTS vSchemeVersion");
        m_db->Exec(STREAM2STR(_T("CREATE VIEW vSchemeVersion AS SELECT '") << CURRENT_DB_VERSION << _T("' AS Version")));
        break;
    }
}


void DBManager::createDBScheme() const
{
    LOG("DBManager::createDBScheme: Creating a new database scheme from scratch.");

    m_db->Begin();
    m_db->Exec("CREATE TABLE tItems (itemidx INTEGER NOT NULL PRIMARY KEY ON CONFLICT REPLACE AUTOINCREMENT UNIQUE DEFAULT '1', keylow INTEGER, keyhigh INTEGER, ql INTEGER, flags INTEGER DEFAULT '0', stack INTEGER DEFAULT '1', parent INTEGER NOT NULL DEFAULT '2', slot INTEGER, children INTEGER, owner INTEGER NOT NULL)");
    m_db->Exec("CREATE INDEX idx_titems_keylow ON tItems (keylow ASC)");
    m_db->Exec("CREATE INDEX idx_titems_owner ON tItems (owner)");
    m_db->Exec("CREATE INDEX idx_titems_parent ON tItems (parent)");
    m_db->Exec("CREATE VIEW vBankItems AS SELECT * FROM tItems WHERE parent=1");
    m_db->Exec("CREATE VIEW vContainers AS SELECT * FROM tItems WHERE children > 0");
    m_db->Exec("CREATE VIEW vInvItems AS SELECT * FROM tItems WHERE parent=2");
    m_db->Exec("CREATE TABLE tToons (charid INTEGER NOT NULL PRIMARY KEY UNIQUE, charname VARCHAR, shopid INTEGER DEFAULT '0', dimensionid INTEGER DEFAULT '0')");
    m_db->Exec("CREATE UNIQUE INDEX iCharId ON tToons (charid)");
    m_db->Exec("CREATE TABLE tDimensions (dimensionid INTEGER NOT NULL PRIMARY KEY UNIQUE, dimensionname VARCHAR)");
    m_db->Exec("INSERT INTO tDimensions (dimensionid, dimensionname) VALUES (13, 'Rubi-Ka')");
    m_db->Exec("INSERT INTO tDimensions (dimensionid, dimensionname) VALUES (15, 'TestLive')");
    m_db->Exec("CREATE TABLE tToonStats (charid INTEGER NOT NULL, statid INTEGER NOT NULL, statvalue INTEGER NOT NULL, FOREIGN KEY (charid) REFERENCES tToons (charid))");
    m_db->Exec("CREATE UNIQUE INDEX charstatindex ON tToonStats (charid ASC, statid ASC)");
    m_db->Exec(STREAM2STR(_T("CREATE VIEW vSchemeVersion AS SELECT '") << CURRENT_DB_VERSION << _T("' AS Version")));
    m_db->Commit();
}
