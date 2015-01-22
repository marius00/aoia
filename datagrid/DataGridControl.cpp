#include "DataGridControl.h"
#include <boost/bind.hpp>

namespace aoia {

    DataGridControl::DataGridControl()
    {
        AtlInitCommonControls(ICC_LISTVIEW_CLASSES);
    }


    DataGridControl::~DataGridControl()
    {
    }


    void DataGridControl::setModel(IDataGridModelPtr model)
    {
        // Clean out any previous data
        while (m_listView.GetHeader().GetItemCount())
        {
            m_listView.DeleteColumn(0);
        }
        m_listView.DeleteAllItems();

        if (m_model)
        {
            m_model->disconnect(m_addSignalConnection);
            m_model->disconnect(m_removeSignalConnection);
            m_model->disconnect(m_clearSignalConnection);
            m_model->disconnect(m_updateSignalConnection);
        }
        m_model = model;
        if (m_model)
        {
            m_addSignalConnection = m_model->connectItemAdded(boost::bind(&DataGridControl::onItemAdded, this, _1));
            m_removeSignalConnection = m_model->connectItemRemoved(boost::bind(&DataGridControl::onItemRemoved, this, _1));
            m_clearSignalConnection = m_model->connectAllRemoved(boost::bind(&DataGridControl::onAllItemsRemoved, this));
            m_updateSignalConnection = m_model->connectCollectionUpdated(boost::bind(&DataGridControl::onAllItemsUpdated, this));
        }

        updateColumns();
        m_listView.SetItemCountEx(m_model->getItemCount(), 0);
    }


    aoia::IDataGridModelPtr DataGridControl::getModel() const
    {
        return m_model;
    }


    LRESULT DataGridControl::onCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
    {
        DefWindowProc(uMsg, wParam, lParam);
        m_listView = m_hWnd;
        m_listView.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
        return 0;
    }


    LRESULT DataGridControl::onGetDispInfo(int wParam, LPNMHDR lParam, BOOL &bHandled)
    {
        NMLVDISPINFO *pdi = (NMLVDISPINFO*)lParam;
        if (pdi->item.mask & LVIF_TEXT)
        {
            std::tstring text = m_model->getItemProperty(pdi->item.iItem, pdi->item.iSubItem);
            StrCpyN(pdi->item.pszText, text.c_str(), min((int)(text.length()) + 1, pdi->item.cchTextMax));
        }
        return 0;
    }


    void DataGridControl::onAllItemsUpdated()
    {
        m_listView.SetItemCountEx(m_model->getItemCount(), 0);
        m_listView.Invalidate(FALSE);
    }


    void DataGridControl::onItemAdded(unsigned int index)
    {
        if (m_listView.GetItemCount() == 0) {
            updateColumns();
        }
        m_listView.InsertItem(index, LPSTR_TEXTCALLBACK);
        autosizeColumnsUseHeader();
    }


    void DataGridControl::onItemRemoved(unsigned int index)
    {
        m_listView.DeleteItem(index);
        if (m_listView.GetItemCount() == 0) {
            updateColumns();
        }
    }


    void DataGridControl::onAllItemsRemoved()
    {
        m_listView.DeleteAllItems();
        updateColumns();
    }


    void DataGridControl::updateColumns()
    {
        while (m_listView.GetHeader().GetItemCount()) {
            m_listView.DeleteColumn(0);
        }
        for (unsigned int i = 0; i < m_model->getColumnCount(); ++i) {
            m_listView.InsertColumn(i, m_model->getColumnName(i).c_str());
            m_listView.SetColumnWidth(i, 100);  // Default 100px width for all columns.
        }
    }


    void DataGridControl::autosizeColumnsUseHeader()
    {
        int numCol = m_listView.GetHeader().GetItemCount();
        for (int i = 0; i < numCol; ++i)
        {
            m_listView.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);            
        }
    }


    void DataGridControl::autosizeColumnsUseData(bool onlyVisible)
    {
        if (!onlyVisible)
        {
/*
			int numCol = m_listView.GetHeader().GetItemCount();
            for (int i = 0; i < numCol; ++i)
            {
                m_listView.SetColumnWidth(i, LVSCW_AUTOSIZE);
            }
*/
            // Loop all rows and calculate width on those.
            unsigned int firstRow = m_listView.GetTopIndex();
            unsigned int maxRow = m_listView.GetItemCount();
            TCHAR buffer[MAX_PATH];

            for (unsigned int i = 0; i < m_model->getColumnCount(); ++i) 
            {            
                unsigned int maxWidth = m_listView.GetStringWidth(m_model->getColumnName(i).c_str());
                for (unsigned int rowIndex = firstRow; rowIndex < maxRow; rowIndex++)
                {
                    m_listView.GetItemText(rowIndex, i, buffer, 200);
                    maxWidth = max(maxWidth, (unsigned int)m_listView.GetStringWidth(buffer));
                }
                m_listView.SetColumnWidth(i, maxWidth + 15);
            }

		}
        else 
        {
            // Loop the visible rows and calculate width on those only.
            unsigned int numRows = m_listView.GetCountPerPage();
            unsigned int firstRow = m_listView.GetTopIndex();
            unsigned int maxRow = m_listView.GetItemCount();
            TCHAR buffer[MAX_PATH];

            for (unsigned int i = 0; i < m_model->getColumnCount(); ++i) 
            {            
                unsigned int maxWidth = m_listView.GetStringWidth(m_model->getColumnName(i).c_str());
                for (unsigned int rowIndex = firstRow; (numRows-- > 0) && (rowIndex < maxRow); rowIndex++)
                {
                    m_listView.GetItemText(rowIndex, i, buffer, 200);
                    maxWidth = max(maxWidth, (unsigned int)m_listView.GetStringWidth(buffer));
                }
                m_listView.SetColumnWidth(i, max(maxWidth + 15, (unsigned int)m_listView.GetColumnWidth(i)));
            }
        }
    }


    unsigned int DataGridControl::getSelectedCount() const
    {
        return m_listView.GetSelectedCount();
    }


    std::set<unsigned int> DataGridControl::getSelectedItems() const
    {
        std::set<unsigned int> result;
        
        int lastId = -1;
        do
        {
            lastId = m_listView.GetNextItem(lastId, LVNI_SELECTED);
            if (lastId > -1)
            {
                result.insert(lastId);
            }
        }
        while(lastId > -1);

        return result;
    }


    void DataGridControl::ClearSelection()
    {
        m_listView.SetItemState(-1, 0, LVIS_SELECTED);
    }

}   // namespace
