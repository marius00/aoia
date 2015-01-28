#include "StdAfx.h"
#include "ContainerManager.h"

#define TIXML_USE_STL
#include <tinyxml.h>

#include <sstream>
#include <boost/algorithm/string.hpp>
#include "DBManager.h"
#include "MainFrm.h"
#include <ItemAssistantCore/AOManager.h>


namespace ba = boost::algorithm;


ContainerManager::ContainerManager(sqlite::IDBPtr db)
    : m_db(db)
{
    m_accounts = GetAccountNames();
}


ContainerManager::~ContainerManager()
{
}


std::tstring ContainerManager::GetContainerName(unsigned int charid, unsigned int containerid) const
{
    if (containerid == 1) {
        return _T("Bank");
    }
    else if (containerid == 2) {
        return _T("Inventory/Equip");
    }
    else if (containerid == 3) {
        return _T("Shop");
    }

    std::tstring result;

    __int64 key = ((__int64)charid) << 32;
    key += containerid;

    FILETIME lastWrite;
    lastWrite.dwHighDateTime = lastWrite.dwLowDateTime = 0;

    std::tstring filename;
    for (unsigned int i = 0; i < m_accounts.size(); i++)
    {
        filename = STREAM2STR( AOManager::instance().getAOPrefsFolder() << _T("\\") << m_accounts[i] << _T("\\Char") << charid << _T("\\Containers\\Container_51017x") << containerid << _T(".xml") );
        if (PathFileExists(filename.c_str()))
        {
            WIN32_FILE_ATTRIBUTE_DATA atribs; 
            if (GetFileAttributesEx(filename.c_str(), GetFileExInfoStandard, &atribs))
            {
                lastWrite = atribs.ftLastWriteTime;
                break;
            }
        }
    }

    bool wasInCache = m_containerFileCache.find(key) != m_containerFileCache.end();

    // Clear invalid cache
    if ((filename.empty() || (lastWrite.dwHighDateTime == 0 && lastWrite.dwHighDateTime == 0)) && wasInCache)
    {
        m_containerFileCache.erase(m_containerFileCache.find(key));
    }

    // Create cache from file
    if (!filename.empty())
    {
        bool update = true;

        // If already in cache, check timestamps
        if (m_containerFileCache.find(key) != m_containerFileCache.end())
        {
            FILETIME stamp = m_containerFileCache[key].second;
            if (stamp.dwHighDateTime == lastWrite.dwHighDateTime &&
                stamp.dwLowDateTime == lastWrite.dwLowDateTime)
            {
                update = false;
                result = m_containerFileCache[key].first;
            }
        }

        if (update)
        {
            TiXmlDocument document;
            if (document.LoadFile(to_ascii_copy(filename), TIXML_ENCODING_UTF8))
            {
                TiXmlHandle docHandle( &document );
                TiXmlElement* element = docHandle.FirstChild( "Archive" ).FirstChild( "String" ).Element();

                while (element)
                {
                    if (StrCmpA(element->Attribute("name"), "container_name") == 0)
                    {
                        result = from_utf8_copy(element->Attribute("value"));
                        boost::algorithm::replace_all(result, _T("&amp;"), _T("&"));    // Fixes wierd encoding in the AO xml.
                        m_containerFileCache[key] = std::pair<std::tstring, FILETIME>(result, lastWrite);
                        break;
                    }
                    element = element->NextSiblingElement();
                }
            }
        }
    }

    if (result.empty())
    {
        result = MakeContainerName(charid, containerid);
    }

    return result;
}


std::tstring ContainerManager::MakeContainerName(unsigned int charid, unsigned int containerid) const
{
    __int64 key = ((__int64)charid) << 32;
    key += containerid;

    // Return cached value if exist
    if (m_containerDBCache.find(key) != m_containerDBCache.end())
    {
        return m_containerDBCache[key];
    }

    std::tstring result;

    // Lets find the item description for the specified container.
    g_DBManager.Lock();
    sqlite::ITablePtr pT2 = m_db->ExecTable( STREAM2STR( "SELECT keylow from tItems WHERE children = " << containerid ));
    g_DBManager.UnLock();

    if (pT2 != NULL && pT2->Rows())
    {
        try {
            unsigned int keylow = boost::lexical_cast<unsigned int>(pT2->Data(0,0));
            std::map<std::tstring, std::tstring> item = GetAOItemInfo(keylow);
            result = item[_T("name")];
        }
        catch(boost::bad_lexical_cast &/*e*/) {
            assert(false); // This should not happen!
            result = STREAM2STR( "Backpack: " << containerid );
        }      
    }
    else
    {
        result = STREAM2STR( "Backpack: " << containerid );
    }

    m_containerDBCache[key] = result;

    return result;
}


/**
* Returns all columns from the AO Item database for the specified key value. 
* The items are returned in a map where the key is the name of the property,
* and the value is the string representation of the property value.
* The following properties are currently available:
*    aoid, ql, type, fromaoid, name, description, flags, properties and icon ID.
*/
std::map<std::tstring, std::tstring> ContainerManager::GetAOItemInfo(unsigned int lowkey) const
{
    std::map<std::tstring, std::tstring> result;

    std::tstringstream sql;
    sql << _T("SELECT aoid, ql, type, name, description, flags, properties, icon FROM tblAO WHERE aoid = ") << lowkey;

    g_DBManager.Lock();
    sqlite::ITablePtr pT = m_db->ExecTable(sql.str());
    g_DBManager.UnLock();

    if (pT != NULL)
    {
        if (pT->Rows() > 0)
        {
            for (unsigned int col = 0; col < pT->Columns(); col++)
            {
                std::tstring column = from_ascii_copy(pT->Headers(col));
                std::tstring data = ba::trim_copy(from_ascii_copy(pT->Data(0, col)));

                result[column] = data;
            }
        }
    }

    return result;
}

//
//void SharedServices::ClearTempContainerIdCache(unsigned int charId)
//{
//	//TODO: Clear cache for only 1 char (if duallogged)
//	if (m_containerIdCache.find(charId) != m_containerIdCache.end())
//	{
//		m_containerIdCache[charId].clear();
//	}
//}
//
//
//void SharedServices::UpdateTempContainerId(unsigned int charId, unsigned int tempId, unsigned int containerId)
//{
//    TRACE("UPDATE Temp Cont Id " << tempId << " / " << containerId);
//
//	//__int64 key = ((__int64)charId) << 32;
//    //key += fromId;
//	//m_InvSlotIndexCache[key] = slotId;
//
//	if (m_containerIdCache.find(charId) == m_containerIdCache.end())
//	{
//		std::map< __int32, unsigned int > newCharIdCache;
//		m_containerIdCache[charId] = newCharIdCache;
//	}
//
//	m_containerIdCache[charId][tempId] = containerId;
//}
//
//
//unsigned int SharedServices::GetContainerId(unsigned int charId, unsigned int tempId) const
//{
//    std::map< __int32, std::map<__int32, unsigned int> >::const_iterator it = m_containerIdCache.find(charId);
//	if (it != m_containerIdCache.end())
//	{
//        std::map<__int32, unsigned int>::const_iterator it2 = it->second.find(tempId);
//        if (it2 != it->second.end())
//        {
//            return it2->second;
//        }
//	}
//
//	return 0;
//}
//
//
//unsigned int SharedServices::GetItemSlotId(unsigned int charId, unsigned int itemTempId) const
//{
//    std::map< __int32, std::map<__int32, unsigned int> >::const_iterator it = m_invSlotForTempItemCache.find(charId);
//    if (it != m_invSlotForTempItemCache.end())
//    {
//        std::map<__int32, unsigned int>::const_iterator it2 = it->second.find(itemTempId);
//        if (it2 != it->second.end())
//        {
//            return it2->second;
//        }
//    }
//
//	return 0;
//}
//
//
//void SharedServices::UpdateTempItemId(unsigned int charId, unsigned int itemTempId, unsigned int slotId)
//{
//    TRACE("UPDATE Temp Item id " << itemTempId << " / " << slotId);
//
//	//__int64 key = ((__int64)charId) << 32;
//    //key += fromId;
//	//m_InvSlotIndexCache[key] = slotId;
//
//	if (m_invSlotForTempItemCache.find(charId) == m_invSlotForTempItemCache.end())
//	{
//		std::map< __int32, unsigned int > newCharIdCache;
//		m_invSlotForTempItemCache[charId] = newCharIdCache;
//	}
//
//	m_invSlotForTempItemCache[charId][itemTempId] = slotId;
//}


std::vector<std::tstring> ContainerManager::GetAccountNames() const
{
    std::vector<std::tstring> result;

    std::tstring path = AOManager::instance().getAOPrefsFolder();
    path += _T("\\*");

    WIN32_FIND_DATA findData;

    HANDLE hFind = FindFirstFileEx(path.c_str(), FindExInfoStandard, &findData, FindExSearchLimitToDirectories, NULL, 0);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
                && (findData.cFileName[0] != NULL) 
                && (findData.cFileName[0] != '.')
				&& (std::tstring(findData.cFileName) != _T("Browser")))
            {
                result.push_back(std::tstring(findData.cFileName));
            }
        }
        while (FindNextFile(hFind, &findData));
        FindClose(hFind);
    }

    return result;
}
