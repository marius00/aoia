#ifndef ITEMLISTDATAMODEL_H
#define ITEMLISTDATAMODEL_H

#include <PluginSDK/IContainerManager.h>
#include <datagrid/DataGridModel.h>
#include <Shared/IDB.h>
#include <set>


namespace aoia
{

    /**
     * Datamodel that searches for all recorded items that matches the specified WHERE predicate.
     *
     * The predicate must make sure to address the columns in an un-ambigous 
     * way since the query is a joining of 3 different tables. Each table has 
     * its own alias in the query:
     * - tItems has the alias \e I
     * - aodb.tblAO has the alias \e A
     * - tToons has the alias \e T 
     *
     * Example predicate:
     * - keyhigh = 123456
     */
    class ItemListDataModel
        : public DataGridModel
    {
    public:
        /// Constructor that takes a general SQL predicate as input.
        ItemListDataModel(sqlite::IDBPtr db, IContainerManagerPtr containerManager, 
            std::tstring const& predicate, unsigned int sortColumnIndex = -1, bool sortAscending = true);

        /// Convenience constructor for running a query where the predicate is an 
        /// IN-statement of the AOIDs in the supplied set.
        ItemListDataModel(sqlite::IDBPtr db, IContainerManagerPtr containerManager, 
            std::set<unsigned int> const& aoids, unsigned int sortColumnIndex = -1, bool sortAscending = true);
        virtual ~ItemListDataModel();

        /// Return number of columns.
        virtual unsigned int getColumnCount() const;

        /// Return the column header for a specific column.
        virtual std::tstring getColumnName(unsigned int index) const;

        /// Return the number of items.
        virtual unsigned int getItemCount() const;

        /// Return the name of a specified item.
        virtual std::tstring getItemProperty(unsigned int index, unsigned int column) const;

		// Returns the container id of specified item
		bool getItemContainerId(unsigned int index, unsigned int& charid, unsigned int& containerid) const;

        /// Return the AOID of the specified item.
        unsigned int getItemId(unsigned int index) const;

        /// Return the owned-item-index of the item at the specified row.
        unsigned int getItemIndex(unsigned int rowIndex) const;

        /// Tell the datamodel to resort based on a particular column and direction.
        void sortData(unsigned int columnIndex, bool ascending = true);

        /// Delete the specified items from the database, and signal changes to observers.
        void DeleteItems( std::set<unsigned int> const& ids );

		void selectItemContainerId(unsigned int containerid);

    protected:
        // Enumeration of publicly visible columns.
        enum ColumnID
        {
            COL_ITEM_NAME = 0,
            COL_ITEM_QL,
            COL_TOON_NAME,
            COL_BACKPACK_NAME,
            COL_BACKPACK_LOCATION,
            COL_COUNT       // This should always be last!
        };

        void runQuery(std::tstring const& predicate, int sortColumn = -1, bool sortAscending = true);
        void runQueryAgain();

    private:
        sqlite::IDBPtr m_db;
        IContainerManagerPtr m_containerManager;
        sqlite::ITablePtr m_result;
        std::tstring m_lastPredicate;
        std::tstring m_lastQuery;
    };

    typedef boost::shared_ptr<ItemListDataModel> ItemListDataModelPtr;

}

#endif // ITEMLISTDATAMODEL_H
