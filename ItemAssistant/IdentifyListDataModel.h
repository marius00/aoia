#ifndef IDENTIFYLISTDATAMODEL_H
#define IDENTIFYLISTDATAMODEL_H

#include "datagrid/DataGridModel.h"
#include <shared/IDB.h>


namespace aoia {

    /**
     * Datamodel that loads all identifyable items with a purpose description.
     */
    class IdentifyListDataModel
        : public DataGridModel
    {
    public:
        IdentifyListDataModel(sqlite::IDBPtr db);
        virtual ~IdentifyListDataModel();

        /// Return number of columns.
        virtual unsigned int getColumnCount() const;

        /// Return the column header for a specific column.
        virtual std::tstring getColumnName(unsigned int index) const;

        /// Return the number of items.
        virtual unsigned int getItemCount() const;

        /// Return the name of a specified item.
        virtual std::tstring getItemProperty(unsigned int index, unsigned int column) const;

        /// Return the AOID of the specified item that can be made from the item at row \e index.
        unsigned int getItemId(unsigned int index) const;

        /// Return the low ID of the item at row \e rowIndex.
        unsigned int getItemLowId(unsigned int rowIndex) const;

        /// Return the high ID of the item at row \e rowIndex.
        unsigned int getItemHighId(unsigned int rowIndex) const;

    private:
        sqlite::ITablePtr m_result;
    };

    typedef boost::shared_ptr<IdentifyListDataModel> IdentifyListDataModelPtr;
}

#endif // IDENTIFYLISTDATAMODEL_H
