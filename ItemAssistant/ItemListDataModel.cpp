#include "StdAfx.h"
#include "ItemListDataModel.h"
#include "DBManager.h"


namespace aoia 
{

    ItemListDataModel::ItemListDataModel(sqlite::IDBPtr db, IContainerManagerPtr containerManager, 
        std::tstring const& predicate, unsigned int sortColumnIndex, bool sortAscending)
        : m_db(db)
        , m_containerManager(containerManager)
    {
        runQuery(predicate, sortColumnIndex, sortAscending);
    }


    ItemListDataModel::ItemListDataModel(sqlite::IDBPtr db, IContainerManagerPtr containerManager, 
        std::set<unsigned int> const& aoids, unsigned int sortColumnIndex, bool sortAscending)
        : m_db(db)
        , m_containerManager(containerManager)
    {
        std::tstringstream aoid_array;
        for (std::set<unsigned int>::const_iterator it = aoids.begin(); it != aoids.end(); ++it)
        {
            if (it != aoids.begin())
            {
                aoid_array << ",";
            }
            aoid_array << *it;
        }

        std::tstring predicate = _T("A.aoid IN (") + aoid_array.str() + _T(")");
        runQuery(predicate, sortColumnIndex, sortAscending);
    }


    ItemListDataModel::~ItemListDataModel()
    {
    }


    void ItemListDataModel::runQuery( std::tstring const& predicate, int sortColumn, bool sortAscending )
    {
        std::tstring sql = 
            _T("SELECT ")
            _T("    A.name, ")
            _T("    I.ql, ")
            _T("    T.charname, ")
            _T("    I.parent, ")
            _T("    CASE ")
            _T("        WHEN parent = 1 THEN 'Bank' ")
            _T("        WHEN parent = 2 AND slot > 63 THEN 'Inventory' ")
            _T("        WHEN parent = 2 AND slot < 16 THEN 'Equipped (Weapons)' ")
            _T("        WHEN parent = 2 AND slot >= 16 AND slot < 32 THEN 'Equipped (Cloth)' ")
            _T("        WHEN parent = 2 AND slot >= 32 AND slot < 47 THEN 'Equipped (Implants)' ")
            _T("        WHEN parent = 2 AND slot >= 47 AND slot < 64 THEN 'Equipped (Social)' ")
            _T("        WHEN parent = 3 THEN 'Shop' ")
            _T("        WHEN I.parent > 10 THEN (SELECT ")  // This resolvs container for a items in backpacks.
            _T("            CASE ")
            _T("                WHEN I2.parent = 1 THEN 'Bank' ")
            _T("                WHEN I2.parent = 2 AND I2.slot > 63 THEN 'Inventory' ")
            _T("                WHEN I2.parent = 2 AND I2.slot < 16 THEN 'Equipped (Weapons)' ")
            _T("                WHEN I2.parent = 2 AND I2.slot >= 16 AND I2.slot < 32 THEN 'Equipped (Cloth)' ")
            _T("                WHEN I2.parent = 2 AND I2.slot >= 32 AND I2.slot < 47 THEN 'Equipped (Implants)' ")
            _T("                WHEN I2.parent = 2 AND I2.slot >= 47 AND I2.slot < 64 THEN 'Equipped (Social)' ")
            _T("                WHEN I2.parent = 3 THEN 'Shop' ")
            _T("            END ")
            _T("            FROM tItems I2 WHERE I2.children = I.parent) ")
            _T("    END AS container, ")
            _T("    T.charid, ")
            _T("    I.itemidx, ")
            _T("    A.aoid ")
            _T("FROM ")
            _T("    tItems I JOIN aodb.tblAO A ON I.keyhigh = A.aoid JOIN tToons T ON I.owner = T.charid ");

        if (!predicate.empty())
        {
            sql += _T(" WHERE ");
            sql += predicate;
        }

        std::tstring sort_predicate;
        if (sortColumn > -1 && sortColumn < COL_COUNT)
        {
            sort_predicate = _T(" ORDER BY ");
            sort_predicate += boost::lexical_cast<std::tstring>(sortColumn + 1);
        }

        if (!sort_predicate.empty())
        {
            sort_predicate += sortAscending ? _T(" ASC ") : _T(" DESC ");
            sql += sort_predicate;
        }

        m_result = m_db->ExecTable(sql);
        m_lastQuery = sql;
        m_lastPredicate = predicate;
    }


    unsigned int ItemListDataModel::getColumnCount() const
    {
        if (!m_result)
        {
            return 0;
        }

        return COL_COUNT;
    }


    std::tstring ItemListDataModel::getColumnName(unsigned int index) const
    {
        if (!m_result)
        {
            return _T("");
        }

        switch (index)
        {
        case COL_ITEM_NAME:
            return _T("Item Name");
        case COL_ITEM_QL:
            return _T("QL");
        case COL_TOON_NAME:
            return _T("Character");
        case COL_BACKPACK_NAME:
            return _T("Backpack");
        case COL_BACKPACK_LOCATION:
            return _T("Location");
        default:
            return _T("");
        }
    }


    unsigned int ItemListDataModel::getItemCount() const
    {
        if (!m_result)
        {
            return 0;
        }

        return m_result->Rows();
    }


    std::tstring ItemListDataModel::getItemProperty(unsigned int index, unsigned int column) const
    {
        if (!m_result)
        {
            return _T("");
        }

        if (column == COL_BACKPACK_NAME)
        {
            try
            {
                unsigned int id = boost::lexical_cast<unsigned int>(m_result->Data(index, COL_BACKPACK_NAME));
                if (id <= 10)
                {
                    // If the item is in the bank or inventory we just display an empty name for the backpack.
                    return _T("");
                }
                unsigned int charid = boost::lexical_cast<unsigned int>(m_result->Data(index, COL_COUNT));
                return m_containerManager->GetContainerName(charid, id);
            }
            catch (boost::bad_lexical_cast &/*e*/)
            {
                return _T("");
            }
        }

        return from_ascii_copy(m_result->Data(index, column));
    }


    unsigned int ItemListDataModel::getItemId(unsigned int index) const
    {
        if (!m_result)
        {
            return 0;
        }

        // The AOID is selected as column index 7 in the query
        return boost::lexical_cast<unsigned int>(m_result->Data(index, 7));
    }


    unsigned int ItemListDataModel::getItemIndex(unsigned int rowIndex) const
    {
        if (!m_result)
        {
            return 0;
        }

        // The owned-item-index is selected as column index 6 in the query
        return boost::lexical_cast<unsigned int>(m_result->Data(rowIndex, 6));
    }


    void ItemListDataModel::sortData(unsigned int columnIndex, bool ascending)
    {
        // TODO: Add support for sorting the backpack column properly.
        runQuery(m_lastPredicate, columnIndex, ascending);
        signalCollectionUpdated();
    }


    void ItemListDataModel::DeleteItems( std::set<unsigned int> const& ids )
    {
        g_DBManager.Lock();
        g_DBManager.DeleteItems(ids);
        g_DBManager.UnLock();

        runQueryAgain();
        signalCollectionUpdated();
    }


    void ItemListDataModel::runQueryAgain()
    {
        if (m_lastQuery.empty())
        {
            m_result.reset();
            return;
        }

        m_result = m_db->ExecTable(m_lastQuery);
    }

}
