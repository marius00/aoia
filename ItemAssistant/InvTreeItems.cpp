#include "stdafx.h"
#include "InvTreeItems.h"
#include "InventoryView.h"
#include "Shared\TextFormat.h"
#include <ItemAssistantCore/AOManager.h>


SqlTreeViewItemBase::SqlTreeViewItemBase(InventoryView* pOwner)
    : m_pOwner(pOwner) {}


SqlTreeViewItemBase::~SqlTreeViewItemBase() {}


void SqlTreeViewItemBase::SetOwner(InventoryView* pOwner)
{
    m_pOwner = pOwner;
}


unsigned int SqlTreeViewItemBase::AppendMenuCmd(HMENU hMenu, unsigned int firstID, WTL::CTreeItem item) const
{
    return firstID;
}


bool SqlTreeViewItemBase::HandleMenuCmd(unsigned int commandID, WTL::CTreeItem item)
{
    return false;
}


void SqlTreeViewItemBase::SetLabel(std::tstring const& newLabel) {}


bool SqlTreeViewItemBase::SortChildren() const
{
    return false;
}


/***************************************************************************/
/** Container Tree View Item                                              **/
/***************************************************************************/

ContainerTreeViewItem::ContainerTreeViewItem(sqlite::IDBPtr db, aoia::IContainerManagerPtr containerManager, InventoryView* pOwner, unsigned int charid, unsigned int containerid, std::tstring const& constraints, std::tstring const& label)
    : m_db(db)
    , m_containerManager(containerManager)
    , m_charid(charid)
    , m_containerid(containerid)
    , m_constraints(constraints)
    , SqlTreeViewItemBase(pOwner)
{
    if (label.empty())
    {
        std::tstringstream str;
        std::tstring containerName = m_containerManager->GetContainerName(m_charid, m_containerid);

        if (containerName.empty())
        {
            str << "Backpack: " << m_containerid;
        }
        else
        {
            str << containerName;
        }

        m_label = str.str();
    }
    else
    {
        m_label = label;
    }
}


ContainerTreeViewItem::~ContainerTreeViewItem() {}


void ContainerTreeViewItem::OnSelected()
{
    std::tstringstream sql;
    sql << _T("owner = ") << m_charid << _T(" AND parent ");

    if (m_containerid == 0)
    {
        sql << _T("> 3 AND parent NOT IN (SELECT DISTINCT children FROM tItems)");
    }
    else
    {
        sql << _T(" = ") << m_containerid;
    }

    if (!m_constraints.empty())
    {
        sql << _T(" AND ") << m_constraints;
    }

    m_pOwner->UpdateListView(sql.str());
}


bool ContainerTreeViewItem::CanEdit() const
{
    return false;
}


std::tstring ContainerTreeViewItem::GetLabel() const
{
    return m_label;
}


bool ContainerTreeViewItem::HasChildren() const
{
    bool result = false;

    if (m_containerid == 1 || m_containerid == 2)
    {
        // Init contents from DB
        std::tstringstream sql;
        sql << _T("SELECT DISTINCT children FROM tItems WHERE children > 0 AND parent = ") << m_containerid <<
            _T(" AND owner = ") << m_charid;
        if (!m_constraints.empty())
        {
            sql << _T(" AND ") << m_constraints;
        }

        g_DBManager.Lock();
        sqlite::ITablePtr pT = m_db->ExecTable(sql.str());
        g_DBManager.UnLock();

        if (pT != NULL)
        {
            result = pT->Rows() > 0;
        }
    }

    return result;
}


std::vector<MFTreeViewItem*> ContainerTreeViewItem::GetChildren() const
{
    std::vector<MFTreeViewItem*> result;

    if (m_containerid == 1 || m_containerid == 2)
    {
        // Init contents from DB
        std::tstringstream sql;
        sql << _T("SELECT children FROM tItems WHERE children > 0 AND parent = ") << m_containerid <<
            _T(" AND owner = ") << m_charid;
        if (!m_constraints.empty())
        {
            sql << _T(" AND ") << m_constraints;
        }

        g_DBManager.Lock();
        sqlite::ITablePtr pT = m_db->ExecTable(sql.str());
        g_DBManager.UnLock();

        if (pT != NULL)
        {
            for (size_t i = 0; i < pT->Rows(); ++i)
            {
                unsigned int contid = boost::lexical_cast<unsigned int>(pT->Data(i, 0));
                result.push_back(new ContainerTreeViewItem(m_db, m_containerManager, m_pOwner, m_charid, contid));
            }
        }
    }

    return result;
}


unsigned int ContainerTreeViewItem::AppendMenuCmd(HMENU hMenu, unsigned int firstID, WTL::CTreeItem item) const
{
    if (m_containerid != 0)
    {
        m_commands[firstID] = SqlTreeViewItemBase::CMD_DELETE;
        AppendMenu(hMenu, MF_STRING, firstID++, _T("Delete Items From DB"));
    }
    return firstID;
}


bool ContainerTreeViewItem::HandleMenuCmd(unsigned int commandID, WTL::CTreeItem item)
{
    if (m_commands.find(commandID) != m_commands.end())
    {
        switch (m_commands[commandID])
        {
        case SqlTreeViewItemBase::CMD_DELETE:
            {
                g_DBManager.Lock();
                m_db->Begin();
                std::tstringstream sql;
                sql << _T("DELETE FROM tItems WHERE parent = ") << m_containerid <<
                    _T("; DELETE FROM tItems WHERE children = ") << m_containerid;
                if (m_db->Exec(sql.str()))
                {
                    m_db->Commit();
                }
                else
                {
                    m_db->Rollback();
                }
                g_DBManager.UnLock();
            }
            break;
        default:
            break;
        }
    }
    else
    {
        return false;
    }
    return true;
}


bool ContainerTreeViewItem::CanDelete() const
{
    return true;
}


bool ContainerTreeViewItem::SortChildren() const
{
    return true;
}


/***************************************************************************/
/** Character Tree View Item                                              **/
/***************************************************************************/

CharacterTreeViewItem::CharacterTreeViewItem(sqlite::IDBPtr db, aoia::IContainerManagerPtr containerManager, InventoryView* pOwner, unsigned int charid)
    : m_db(db)
    , m_containerManager(containerManager)
    , m_charid(charid)
    , SqlTreeViewItemBase(pOwner)
{
    g_DBManager.Lock();
    m_label = g_DBManager.GetToonName(charid);
    g_DBManager.UnLock();

    if (m_label.empty())
    {
        std::tstringstream str;
        str << charid;
        m_label = str.str();
    }
}


CharacterTreeViewItem::~CharacterTreeViewItem() {}


void CharacterTreeViewItem::OnSelected()
{
    std::tstringstream str;
    str << _T("owner = ") << m_charid;
    m_pOwner->UpdateListView(str.str());
}


bool CharacterTreeViewItem::CanEdit() const
{
    return false;
}


std::tstring CharacterTreeViewItem::GetLabel() const
{
    g_DBManager.Lock();
    std::tstring result = g_DBManager.GetToonName(m_charid);
    g_DBManager.UnLock();

    if (result.empty())
    {
        std::tstringstream str;
        str << m_charid;
        result = str.str();
    }
	std::tstring tCredsFmt(_T(""));
	if (AOManager::instance().getShowCreds())
	{
	    g_DBManager.Lock();
		sqlite::ITablePtr pCreds = m_db->ExecTable(STREAM2STR("SELECT statvalue FROM tToonStats WHERE charid = " << (unsigned int)m_charid << " AND statid = 61"));
	    g_DBManager.UnLock();
		long lCreds = boost::lexical_cast<long>(pCreds->Data(0,0));
		if (lCreds < 1) lCreds = 0;
		TextFormat fT(3, ",");
		tCredsFmt = _T(" :: ") + fT.FormatLongToString(lCreds) + _T("cr");
	}
	
	return result + tCredsFmt;
}


void CharacterTreeViewItem::SetLabel(std::tstring const& newLabel)
{
    g_DBManager.Lock();
    g_DBManager.SetToonName(m_charid, newLabel);
    g_DBManager.UnLock();
}


bool CharacterTreeViewItem::HasChildren() const
{
    return true;
}


std::vector<MFTreeViewItem*> CharacterTreeViewItem::GetChildren() const
{
    std::vector<MFTreeViewItem*> result;

    result.push_back(new ContainerTreeViewItem(m_db, m_containerManager, m_pOwner, m_charid, 1, _T(""), _T("Bank"))); // bank
    result.push_back(new ContainerTreeViewItem(m_db, m_containerManager, m_pOwner, m_charid, 2, _T("slot > 63"), _T("Inventory"))); // inventory
    result.push_back(new ContainerTreeViewItem(m_db, m_containerManager, m_pOwner, m_charid, 2, _T("slot < 16"), _T("Weapons"))); // Weapons tab
    result.push_back(new ContainerTreeViewItem(m_db, m_containerManager, m_pOwner, m_charid, 2, _T("slot >= 16 AND slot < 32"), _T("Cloth"))); // Armor tab
    result.push_back(new ContainerTreeViewItem(m_db, m_containerManager, m_pOwner, m_charid, 2, _T("slot >= 32 AND slot < 47"), _T("Implants"))); // Implants tab
    result.push_back(new ContainerTreeViewItem(m_db, m_containerManager, m_pOwner, m_charid, 2, _T("slot >= 47 AND slot < 64"), _T("Social"))); // Social tab
    result.push_back(new ContainerTreeViewItem(m_db, m_containerManager, m_pOwner, m_charid, 3, _T(""), _T("Shop"))); // Player shop
#ifdef DEBUG
    result.push_back(new ContainerTreeViewItem(m_db, m_containerManager, m_pOwner, m_charid, 0, _T(""), _T("Unknown"))); // Unknown
#endif

    return result;
}


unsigned int CharacterTreeViewItem::AppendMenuCmd(HMENU hMenu, unsigned int firstID, WTL::CTreeItem item) const
{
    m_commands[firstID] = SqlTreeViewItemBase::CMD_DELETE;
    AppendMenu(hMenu, MF_STRING, firstID++, _T("Delete Toon"));
    m_commands[firstID] = SqlTreeViewItemBase::CMD_EXPORT;
    AppendMenu(hMenu, MF_STRING, firstID++, _T("Export Items..."));
    return firstID;
}


bool CharacterTreeViewItem::HandleMenuCmd(unsigned int commandID, WTL::CTreeItem item)
{
    if (m_commands.find(commandID) != m_commands.end())
    {
        switch (m_commands[commandID])
        {
        case SqlTreeViewItemBase::CMD_DELETE:
            {
                bool ok = true;
                g_DBManager.Lock();
                m_db->Begin();
                {
                    std::tstringstream sql;
                    sql << _T("DELETE FROM tItems WHERE owner = ") << m_charid;
                    ok = m_db->Exec(sql.str());
                }
                if (ok)
                {
                    std::tstringstream sql;
                    sql << _T("DELETE FROM tToons WHERE charid = ") << m_charid;
                    ok = m_db->Exec(sql.str());
                }
                if (ok)
                {
                    m_db->Commit();
                }
                else
                {
                    m_db->Rollback();
                }
                g_DBManager.UnLock();
            }
            break;

        case SqlTreeViewItemBase::CMD_EXPORT:
            {
                std::tstringstream str;
                str << _T("owner = ") << m_charid;
                m_pOwner->exportToCSV(str.str());
            }
            break;

        default:
            break;
        }
    }
    else
    {
        return false;
    }
    return true;
}


bool CharacterTreeViewItem::CanDelete() const
{
    return true;
}


/***************************************************************************/
/** Account Tree View Item                                                **/
/***************************************************************************/

AccountTreeViewItem::AccountTreeViewItem(sqlite::IDBPtr db, aoia::IContainerManagerPtr containerManager, InventoryView* pOwner, std::tstring accountName)
    : m_db(db)
    , m_containerManager(containerManager)
    , SqlTreeViewItemBase(pOwner)
    , m_label(accountName)
{
}


AccountTreeViewItem::~AccountTreeViewItem()
{
}


void AccountTreeViewItem::OnSelected() 
{
}


bool AccountTreeViewItem::CanEdit() const
{
    return false;
}


std::tstring AccountTreeViewItem::GetLabel() const
{
	boost::filesystem::path acc(AOManager::instance().getAOPrefsFolder(), boost::filesystem::native);
	acc /= m_label;
	long lCreds = -1;
	boost::filesystem::directory_iterator character(acc), filesEnd;
	for (; character != filesEnd; ++character)
	{
	    if (!is_directory(*character))
	        continue;
	
		try
		{
		    std::tstring subfolderName = character->path().leaf().native();
		    if (subfolderName.length() < 5)
			{
		        continue;   // No point parsing this folder since it cant have a valid 'Char123123' type of name anyway.
		    }
		    if (subfolderName.compare(_T("Browser")) == 0)
			{
		        continue;   // Skip the "Browser" subfolder
		    }
		    unsigned int charID = boost::lexical_cast<unsigned int>(subfolderName.substr(4));
		    if (charID != 0)
			{
				g_DBManager.Lock();
				sqlite::ITablePtr pToon = m_db->ExecTable(STREAM2STR("SELECT charname FROM tToons WHERE charid = " << (unsigned int)charID));
				if (!pToon->Data(0,0).empty())
				{
					sqlite::ITablePtr pCreds = m_db->ExecTable(STREAM2STR("SELECT statvalue FROM tToonStats WHERE charid = " << (unsigned int)charID << " AND statid = 61"));
					if (lCreds > -1)
						lCreds += boost::lexical_cast<long>(pCreds->Data(0,0));
					else
						lCreds = boost::lexical_cast<long>(pCreds->Data(0,0));
				}
				g_DBManager.UnLock();
		    }  
		}
		catch (boost::bad_lexical_cast &/*e*/)
		{
		    continue;   // do nothing with this folder since it isn't a valid toon folder.
		}
	}
	std::tstring tCredsFmt(_T(""));
	if (AOManager::instance().getShowCreds())
	{
		if (lCreds < 1) lCreds = 0;
		TextFormat fT(3, ",");
		tCredsFmt = _T(" :: ") + fT.FormatLongToString(lCreds) + _T("cr");
	}
	return _T("Account :: ") + m_label + tCredsFmt;
}


bool AccountTreeViewItem::HasChildren() const
{
    return true;
}


std::vector<MFTreeViewItem*> AccountTreeViewItem::GetChildren() const
{
    std::vector<MFTreeViewItem*> result;

	boost::filesystem::path accountPath(AOManager::instance().getAOPrefsFolder(), boost::filesystem::native);
    accountPath = accountPath / m_label;

	boost::filesystem::directory_iterator character(accountPath), filesEnd;
    for (; character != filesEnd; ++character)
    {
        if (!is_directory(*character)) {
            continue;
        }

        try
        {
            std::tstring subfolderName = character->path().leaf().native();
            if (subfolderName.length() < 5)
			{
                continue;   // No point parsing this folder since it cant have a valid 'Char123123' type of name anyway.
            }
            if (subfolderName.compare(_T("Browser")) == 0)
			{
                continue;   // Skip the "Browser" subfolder
            }
            unsigned int charID = boost::lexical_cast<unsigned int>(subfolderName.substr(4));
            if (charID != 0)
			{
				g_DBManager.Lock();
				sqlite::ITablePtr pToon = m_db->ExecTable(STREAM2STR("SELECT charname FROM tToons WHERE charid = " << (unsigned int)charID));
				g_DBManager.UnLock();
				if (!pToon->Data(0,0).empty())
	                result.push_back(new CharacterTreeViewItem(m_db, m_containerManager, m_pOwner, charID));
            }  
        }
        catch (boost::bad_lexical_cast &/*e*/) {
            continue;   // do nothing with this folder since it isn't a valid toon folder.
        }
    }

    return result;
}


unsigned int AccountTreeViewItem::AppendMenuCmd(HMENU hMenu, unsigned int firstID, WTL::CTreeItem item) const
{
    return firstID;
}


bool AccountTreeViewItem::HandleMenuCmd(unsigned int commandID, WTL::CTreeItem item)
{
    return false;
}


/***************************************************************************/
/** Account Root Tree View Item                                                **/
/***************************************************************************/

AccountRootTreeViewItem::AccountRootTreeViewItem(sqlite::IDBPtr db, aoia::IContainerManagerPtr containerManager, InventoryView* pOwner)
    : m_db(db)
    , m_containerManager(containerManager)
    , SqlTreeViewItemBase(pOwner)
{
}


AccountRootTreeViewItem::~AccountRootTreeViewItem()
{
}


void AccountRootTreeViewItem::OnSelected() 
{
}


bool AccountRootTreeViewItem::CanEdit() const
{
    return false;
}


std::tstring AccountRootTreeViewItem::GetLabel() const
{
	unsigned int iAccounts(0);
	std::tstring tRet(_T(":: "));
	long lCreds(0);
    std::tstring filename;
    filename = STREAM2STR( AOManager::instance().getAOPrefsFolder());
    if(!filename.empty())
	{
		boost::filesystem::path p(filename, boost::filesystem::native);
		boost::filesystem::directory_iterator account(p), dir_end;

		if(is_directory(p))
		{
			for (;account != dir_end; ++account)
			{
			    boost::filesystem::path acc = *account;
			    if (is_directory(acc))
			    {
					if (acc.leaf().native() != _T("Browser"))
					{
						// we found an account ? .. but does it have any logged toons from our DB?
						long lHasCreds = AccountRootTreeViewItem::DoesAccountHaveToons(acc);
						if (lHasCreds != -1)
						{
							lCreds += lHasCreds;
							iAccounts++;
						}
					}
			    }
			}
		}
		std::tstring tAcctWord = _T(" Account :: ");
		if (iAccounts == 0 || iAccounts > 1)
			tAcctWord = _T(" Accounts :: ");

		tRet = tRet + (boost::lexical_cast<std::tstring>(iAccounts)) + tAcctWord;
    }

    // Get number of logged toons from the DB
    g_DBManager.Lock();
    sqlite::ITablePtr pTC = m_db->ExecTable(_T("SELECT COUNT(DISTINCT charid) FROM tToons"));
    g_DBManager.UnLock();
	unsigned int iCount = boost::lexical_cast<unsigned int>(pTC->Data(0,0));
	std::tstring tWord = _T(" Character ::");
	if (iCount == 0 || iCount > 1)
		tWord = _T(" Characters ::");

	std::tstring tCredsFmt(_T(""));
	if (AOManager::instance().getShowCreds())
	{
		if (lCreds < 1) lCreds = 0;
		TextFormat fT(3, ",");
		tCredsFmt = _T(" ") + fT.FormatLongToString(lCreds) + _T("cr ::");
	}
	std::tstring tCount = boost::lexical_cast<std::tstring>(pTC->Data(0,0).c_str());
	return tRet + tCount + tWord + tCredsFmt;
}


bool AccountRootTreeViewItem::HasChildren() const
{
    return true;
}


std::vector<MFTreeViewItem*> AccountRootTreeViewItem::GetChildren() const
{
    std::vector<MFTreeViewItem*> result;

    std::tstring filename;
    filename = STREAM2STR( AOManager::instance().getAOPrefsFolder());
    if(filename.empty()) {
        return result;
    }

    boost::filesystem::path p(filename, boost::filesystem::native);
    boost::filesystem::directory_iterator account(p), dir_end;

    if(!is_directory(p))
    {
        return result;
    }

    for (;account != dir_end; ++account)
    {
        boost::filesystem::path acc = *account;
        if (is_directory(acc))
        {
			if (acc.leaf().native() != _T("Browser"))
			{
				// we found an account ? .. but does it have any logged toons from our DB?
				long lCreds = AccountRootTreeViewItem::DoesAccountHaveToons(acc);
				if (lCreds != -1)
				{
					result.push_back(new AccountTreeViewItem(m_db, m_containerManager, m_pOwner, acc.leaf().native()));
				}
			}
        }
    }

    return result;
}


unsigned int AccountRootTreeViewItem::AppendMenuCmd(HMENU hMenu, unsigned int firstID, WTL::CTreeItem item) const
{
    return firstID;
}


bool AccountRootTreeViewItem::HandleMenuCmd(unsigned int commandID, WTL::CTreeItem item)
{
    return false;
}


long AccountRootTreeViewItem::DoesAccountHaveToons(boost::filesystem::path acc) const
{
	long lCreds = -1;
	boost::filesystem::directory_iterator character(acc), filesEnd;
	for (; character != filesEnd; ++character)
	{
	    if (!is_directory(*character))
	        continue;
	
		try
		{
		    std::tstring subfolderName = character->path().leaf().native();
		    if (subfolderName.length() < 5)
			{
		        continue;   // No point parsing this folder since it cant have a valid 'Char123123' type of name anyway.
		    }
		    if (subfolderName.compare(_T("Browser")) == 0)
			{
		        continue;   // Skip the "Browser" subfolder
		    }
		    unsigned int charID = boost::lexical_cast<unsigned int>(subfolderName.substr(4));
		    if (charID != 0)
			{
				g_DBManager.Lock();
				sqlite::ITablePtr pToon = m_db->ExecTable(STREAM2STR("SELECT charname FROM tToons WHERE charid = " << (unsigned int)charID));
				g_DBManager.UnLock();
				if (!pToon->Data(0,0).empty())
				{
					sqlite::ITablePtr pCreds = m_db->ExecTable(STREAM2STR("SELECT statvalue FROM tToonStats WHERE charid = " << (unsigned int)charID << " AND statid = 61"));
					if (lCreds > -1)
						lCreds += boost::lexical_cast<long>(pCreds->Data(0,0));
					else
						lCreds = boost::lexical_cast<long>(pCreds->Data(0,0));
				}
		    }  
		}
		catch (boost::bad_lexical_cast &/*e*/)
		{
		    continue;   // do nothing with this folder since it isn't a valid toon folder.
		}
	}
	return lCreds;
}