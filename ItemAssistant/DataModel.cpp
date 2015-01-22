#include "StdAfx.h"
#include "DataModel.h"
#include <boost/assign.hpp>
#include "DBManager.h"
#include "Shared/SQLite.h"

namespace aoia { namespace sv {

    DataModel::DataModel(sqlite::IDBPtr db)
        : m_db(db)
    {
        // Sets up the map between column-index and title.
        m_columnTitles = boost::assign::map_list_of
            (0, _T("Toon"))
            (1, _T("Levels"))
            (2, _T("AI Levels"))
            (3, _T("Credits"))
            ;

        // Set ut a list of the statids each column should be bound to. (Hardcoded to skip "toon name" column further down.)
        m_statids = boost::assign::list_of(54)(169)(61);

        std::tostringstream statids;
        for (unsigned int i = 0; i < m_statids.size(); ++i) {
            if (i > 0) {
                statids << ", ";
            }
            statids << m_statids.at(i);
        }

        sqlite::ITablePtr toons = m_db->ExecTable(STREAM2STR("SELECT charid, charname FROM tToons"));
        for (unsigned int i = 0; i < toons->Rows(); ++i)
        {
            DataModelItem item;
            item.charid = boost::lexical_cast<unsigned int>(toons->Data(i, 0));
            item.name = from_ascii_copy(toons->Data(i, 1));

            sqlite::ITablePtr stats = m_db->ExecTable(STREAM2STR("SELECT statid, statvalue FROM tToonStats WHERE charid = " << item.charid << " AND statid IN("<< statids.str() << ")"));
            for (unsigned int j = 0; j < stats->Rows(); ++j)
            {
                unsigned int id = boost::lexical_cast<unsigned int>(stats->Data(j, 0));
                item.stats[id] = from_ascii_copy(stats->Data(j, 1));
            }

            m_items.push_back(item);
        }
    }

    DataModel::~DataModel()
    {
    }

    unsigned int DataModel::getColumnCount() const
    {
        if (m_items.empty()) {
            return 0;
        }

        return m_columnTitles.size();
    }

    std::tstring DataModel::getColumnName( unsigned int index ) const
    {
        if (m_items.empty()) {
            return _T("");
        }

        std::map<unsigned int, std::tstring>::const_iterator it = m_columnTitles.find(index);
        if (it != m_columnTitles.end())
        {
            return it->second;
        }

        LOG("aoia::sv::DataModel::getColumnName() : Invalid index supplied.");
        return _T("");
    }

    unsigned int DataModel::getItemCount() const
    {
        if (m_items.empty()) {
            return 0;
        }

        return m_items.size();
    }

    std::tstring DataModel::getItemProperty( unsigned int index, unsigned int column ) const
    {
        if (m_items.empty()) {
            return _T("");
        }

        DataModelItem const& item = m_items.at(index);

        if (column == 0) {
            return item.name;
        }
        --column;

        std::map<unsigned int, std::tstring>::const_iterator it = item.stats.find(m_statids.at(column));
        if (it != item.stats.end()) {
            return it->second;
        }
        return _T("-");
    }

}}  // end of namespace
