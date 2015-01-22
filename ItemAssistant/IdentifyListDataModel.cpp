#include "StdAfx.h"
#include "IdentifyListDataModel.h"
#include "DBManager.h"


namespace aoia
{

    IdentifyListDataModel::IdentifyListDataModel(sqlite::IDBPtr db)
    {
		std::tstring sql = _T("SELECT AO.name, I.purpose, I.aoid, I.lowid, I.highid, I.ordering FROM tblIdentify I JOIN tblAO AO ON I.highid = AO.aoid ORDER BY I.ordering");
		m_result = db->ExecTable(sql);
    }


    IdentifyListDataModel::~IdentifyListDataModel() {}


    unsigned int IdentifyListDataModel::getColumnCount() const
    {
        if (!m_result)
        {
            return 0;
        }

        return 2;
    }


    std::tstring IdentifyListDataModel::getColumnName(unsigned int index) const
    {
        if (!m_result)
        {
            return _T("");
        }

		if (index == 0)
        {
            return _T("Item");
        }

        if (index == 1)
        {
            return _T("Purpose");
        }

        return _T("");
    }


    unsigned int IdentifyListDataModel::getItemCount() const
    {
        if (!m_result)
        {
            return 0;
        }

        return m_result->Rows();
    }


    std::tstring IdentifyListDataModel::getItemProperty(unsigned int index, unsigned int column) const
    {
        if (!m_result)
        {
            return _T("");
        }

        return from_ascii_copy(m_result->Data(index, column));
    }


    unsigned int IdentifyListDataModel::getItemId(unsigned int index) const
    {
        if (!m_result)
        {
            return 0;
        }

        // The AOID is selected as column index 2 in the query
        return boost::lexical_cast<unsigned int>(m_result->Data(index, 2));
    }


    unsigned int IdentifyListDataModel::getItemLowId(unsigned int rowIndex) const
    {
        if (!m_result)
        {
            return 0;
        }

        // The LOWID is selected as column index 3 in the query
        return boost::lexical_cast<unsigned int>(m_result->Data(rowIndex, 3));
    }


    unsigned int IdentifyListDataModel::getItemHighId(unsigned int rowIndex) const
    {
        if (!m_result)
        {
            return 0;
        }

        // The HIGHID is selected as column index 4 in the query
        return boost::lexical_cast<unsigned int>(m_result->Data(rowIndex, 4));
    }
}
