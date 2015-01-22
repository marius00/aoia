#ifndef IDATAGRIDMODEL_H
#define IDATAGRIDMODEL_H

#include <boost/signal.hpp>
#include <shared/UnicodeSupport.h>

namespace aoia {

    /**
     * This is the interface a DataGridControl model will have to implement to be 
     * able to populate the DataGridControl.
     */
    struct IDataGridModel
    {
        typedef boost::signal<void (unsigned int)>  item_signal_t;
        typedef boost::signal<void ()>  collection_signal_t;
        typedef boost::signals::connection connection_t;

        virtual ~IDataGridModel() {}

        virtual connection_t connectCollectionUpdated(collection_signal_t::slot_function_type subscriber) = 0;
        virtual connection_t connectItemAdded(item_signal_t::slot_function_type subscriber) = 0;
        virtual connection_t connectItemRemoved(item_signal_t::slot_function_type subscriber) = 0;
        virtual connection_t connectAllRemoved(collection_signal_t::slot_function_type subscriber) = 0;

        virtual void disconnect(connection_t subscriber) = 0;

        /// Return number of columns.
        virtual unsigned int getColumnCount() const = 0;

        /// Return the column header for a specific column.
        virtual std::tstring getColumnName(unsigned int index) const = 0;

        /// Return the number of items.
        virtual unsigned int getItemCount() const = 0;

        /// Return the name of a specified item.
        virtual std::tstring getItemProperty(unsigned int index, unsigned int column) const = 0;
    };

    typedef boost::shared_ptr<IDataGridModel> IDataGridModelPtr;
}

#endif // IDATAGRIDMODEL_H
