#include "DataGridModel.h"

namespace aoia {

    DataGridModel::DataGridModel()
    {
    }


    DataGridModel::connection_t DataGridModel::connectCollectionUpdated(collection_signal_t::slot_function_type subscriber)
    {
        return m_allUpdatedSignal.connect(subscriber);
    }


    DataGridModel::connection_t DataGridModel::connectItemAdded(item_signal_t::slot_function_type subscriber)
    {
        return m_itemAddedSignal.connect(subscriber);
    }


    DataGridModel::connection_t DataGridModel::connectItemRemoved(item_signal_t::slot_function_type subscriber)
    {
        return m_itemRemovedSignal.connect(subscriber);
    }


    DataGridModel::connection_t DataGridModel::connectAllRemoved(collection_signal_t::slot_function_type subscriber)
    {
        return m_allClearedSignal.connect(subscriber);
    }


    void DataGridModel::disconnect(connection_t subscriber)
    {
        subscriber.disconnect();
    }


    void DataGridModel::signalCollectionUpdated()
    {
        m_allUpdatedSignal();
    }


    void DataGridModel::signalItemAdded(unsigned int index)
    {
        m_itemAddedSignal(index);
    }


    void DataGridModel::signalItemRemoved(unsigned int index)
    {
        m_itemRemovedSignal(index);
    }


    void DataGridModel::signalAllCleared()
    {
        m_allClearedSignal();
    }

}   // namespace
