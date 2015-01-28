#ifndef DATAGRIDMODEL_H
#define DATAGRIDMODEL_H

#include <datagrid/IDataGridModel.h>


namespace aoia {

    /**
     * This class implements the common parts of the DataGridModel interface, 
     * specifically the signal connections.
     */
    class DataGridModel : public IDataGridModel
    {
    public:
        DataGridModel();

        virtual connection_t connectCollectionUpdated(collection_signal_t::slot_function_type subscriber);
        virtual connection_t connectItemAdded(item_signal_t::slot_function_type subscriber);
        virtual connection_t connectItemRemoved(item_signal_t::slot_function_type subscriber);
        virtual connection_t connectAllRemoved(collection_signal_t::slot_function_type subscriber);
        
        virtual void disconnect(connection_t subscriber);

    protected:
        void signalCollectionUpdated();
        void signalItemAdded(unsigned int index);
        void signalItemRemoved(unsigned int index);
        void signalAllCleared();

    private:
        /// Signal triggered when all items have been updated.
        collection_signal_t m_allUpdatedSignal;

        /// Signal triggered when an item is added.
        item_signal_t m_itemAddedSignal;

        /// Signal triggered when an item has been removed.
        item_signal_t m_itemRemovedSignal;

        /// Signal triggered when all items have been removed.
        collection_signal_t m_allClearedSignal;
    };

    typedef boost::shared_ptr<DataGridModel> DataGridModelPtr;

}   // namespace

#endif // DATAGRIDMODEL_H
