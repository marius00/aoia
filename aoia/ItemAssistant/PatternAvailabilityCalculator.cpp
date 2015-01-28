#include "stdafx.h"
#include "PatternAvailabilityCalculator.h"
#include "PatternMatchView.h"
#include "DBManager.h"


DWORD PatternAvailabilityCalculator::ThreadProc()
{
    m_index = 0;
    m_term = false;

    do
    {
        int lastUpdate = -1;

        m_pOwner->PbListMutex().MutexOn();

        PatternMatchView::PbList& list = m_pOwner->PbListRef();

        if (m_index < (int)list.size())
        {
            if (list[m_index]->pbavailability < 0)
            {
                unsigned int pbid = list[m_index]->pbid;
                m_pOwner->PbListMutex().MutexOff();
                float avail = CalcPbAvailability(m_db, pbid, m_toon, m_excludeAssembled);
                m_pOwner->PbListMutex().MutexOn();
                m_pOwner->SetBossAvail(pbid, avail);
                unsigned short percent = (unsigned int)(((m_index + 1) * 100) / list.size());
                if (percent > lastUpdate)
                {
                    m_pOwner->PostMessage(WM_UPDATE_PBLIST, percent, pbid);
                    lastUpdate = percent;
                }
            }
            ++m_index;
        }
        else
        {
            //m_index = 0;
            m_term = true;
        }

        m_pOwner->PbListMutex().MutexOff();

        ::Sleep(0);
    }
    while (!m_term);

    m_index = 0;
    m_term = false;

    return 0;
}


float PatternAvailabilityCalculator::CalcPbAvailability(sqlite::IDBPtr db, unsigned int pbid, unsigned int toonid, bool excludeassembled)
{
    std::map<std::tstring, unsigned int> vals;

    // Get a list of all pattern pieces for the specified pocket boss (optionally exclude ABCD assemblies)
    sqlite::ITablePtr pIDs;
    g_DBManager.Lock();
    {
        std::tstringstream sql;
        sql << _T("SELECT aoid, pattern FROM tblPatterns WHERE name = (SELECT name FROM tblPocketBoss WHERE pbid = ")
            << pbid << _T(")");
        if (excludeassembled)
        {
            sql << _T("AND pattern != 'ABCD'");
        }
        pIDs = db->ExecTable(sql.str());
    }
    g_DBManager.UnLock();

    if (pIDs == NULL)
    {
        return 0.0f;
    }

    // Loop all the pattern pieces and searches for recorded items.
    for (unsigned int idIdx = 0; idIdx < pIDs->Rows(); ++idIdx)
    {
        std::tstring pattern = from_ascii_copy(pIDs->Data(idIdx, 1));
        std::tstring id = from_ascii_copy(pIDs->Data(idIdx, 0));

        std::tstring sql = STREAM2STR("SELECT COUNT(itemidx) FROM tItems WHERE keylow = " << id);
        if (toonid > 0)
        {
            sql += STREAM2STR(" AND owner = " << toonid);
        }

        g_DBManager.Lock();
        sqlite::ITablePtr pItemCount = db->ExecTable(sql);
        g_DBManager.UnLock();

        if (pItemCount && pItemCount->Rows() > 0)
        {
            vals[pattern] += boost::lexical_cast<unsigned int>(pItemCount->Data(0, 0));
        }
    }

    float result = 0;

    if (vals[_T("AB")] > 0)
    {
        vals[_T("A")] += vals[_T("AB")];
        vals[_T("B")] += vals[_T("AB")];
    }
    if (vals[_T("ABC")] > 0)
    {
        vals[_T("A")] += vals[_T("ABC")];
        vals[_T("B")] += vals[_T("ABC")];
        vals[_T("C")] += vals[_T("ABC")];
    }

    result += vals[_T("ABCD")];

    unsigned int v = min(vals[_T("A")], min(vals[_T("B")], min(vals[_T("C")], vals[_T("D")])));

    if (v > 0)
    {
        result += v;
        vals[_T("A")] -= v;
        vals[_T("B")] -= v;
        vals[_T("C")] -= v;
        vals[_T("D")] -= v;
    }

    if (vals[_T("A")] > 0)
    {
        result += 0.25;
    }
    if (vals[_T("B")] > 0)
    {
        result += 0.25;
    }
    if (vals[_T("C")] > 0)
    {
        result += 0.25;
    }
    if (vals[_T("D")] > 0)
    {
        result += 0.25;
    }

    return result;
}
