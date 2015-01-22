#ifndef CSVDATAMODEL_H
#define CSVDATAMODEL_H

#include <datagrid/DataGridModel.h>
#include <shared/IDB.h>
#include <PluginSDK/IContainerManager.h>
#include <set>
#include <PluginSDK/IDBManager.h>


namespace aoia {

    /**
    * Datamodel for retrieving data that is to be exported to a CSV file.
    */
    class CSVDataModel
        : public DataGridModel
    {
    public:
        /// Constructor that takes a general SQL predicate as input.
        CSVDataModel(sqlite::IDBPtr db, IContainerManagerPtr bp, std::tstring const& predicate, std::tstring const& link_template);

        /// Constructor that takes a set of AOIA item IDs as input.
        CSVDataModel(sqlite::IDBPtr db, IContainerManagerPtr bp, std::set<unsigned int> const& ids, std::tstring const& link_template);

        virtual ~CSVDataModel();

        /// Return number of columns.
        virtual unsigned int getColumnCount() const;

        /// Return the column header for a specific column.
        virtual std::tstring getColumnName(unsigned int index) const;

        /// Return the number of items.
        virtual unsigned int getItemCount() const;

        /// Return the name of a specified item.
        virtual std::tstring getItemProperty(unsigned int index, unsigned int column) const;

        /// Return the AOID of the specified item.
        unsigned int getItemId(unsigned int index) const;

        /// Return the owned-item-index of the item at the specified row.
        unsigned int getItemIndex(unsigned int rowIndex) const;

        /// Tell the datamodel to resort based on a particular column and direction.
        void sortData(unsigned int columnIndex, bool ascending = true);

    protected:
        // Enumeration of publicly visible columns.
        enum ColumnID
        {
            COL_ITEM_NAME = 0,
            COL_ITEM_QL,
            COL_TOON_NAME,
            COL_BACKPACK_NAME,
            COL_BACKPACK_LOCATION,
            COL_LOW_ID,
            COL_HIGH_ID,
            COL_CONTAINER_ID,
            COL_LINK,
            COL_COUNT       // This should always be last!
        };

        void runQuery(std::tstring const& predicate);

    private:
        sqlite::IDBPtr m_db;
        IContainerManagerPtr m_bp;
        sqlite::ITablePtr m_result;
        std::tstring m_lastPredicate;
        std::tstring m_linkTemplate;
    };

    typedef boost::shared_ptr<CSVDataModel> CSVDataModelPtr;

}

#endif // CSVDATAMODEL_H
