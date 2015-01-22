#include "StdAfx.h"
#include "PatternReport.h"
#include "PatternMatchView.h"
#include "resource.h"
#include "DBManager.h"
#include <sstream>
#include <iomanip>
#include <boost/algorithm/string/replace.hpp>
#include <boost/assign/list_of.hpp>
#include "boost/tuple/tuple.hpp"


PatternReport::PatternReport(sqlite::IDBPtr db, aoia::IContainerManagerPtr containerManager, unsigned int pbid, unsigned int toonid, bool excludeassembled)
    : m_pbid(pbid)
    , m_toonid(toonid)
    , m_excludeassembled(excludeassembled)
    , m_db(db)
    , m_containerManager(containerManager)
{
    SYSTEMTIME time;
    ::GetLocalTime(&time);

    std::tstringstream time_str;
    time_str << (unsigned int)time.wYear << _T("-")
        << std::setfill(_T('0')) << std::setw(2) << (unsigned int)time.wMonth << _T("-")
        << std::setfill(_T('0')) << std::setw(2) << (unsigned int)time.wDay << _T(" ")
        << std::setfill(_T('0')) << std::setw(2) << (unsigned int)time.wHour << _T(":")
        << std::setfill(_T('0')) << std::setw(2) << (unsigned int)time.wMinute << std::endl;

    m_time = time_str.str();

    if (m_toonid > 0)
    {
        g_DBManager.Lock();
        m_toonname = g_DBManager.GetToonName(toonid);
        g_DBManager.UnLock();
        if (m_toonname.empty())
        {
            m_toonname = STREAM2STR(_T("John Doe (") << m_toonid << _T(")"));
        }
    }
    else
    {
        m_toonname = _T("all toons");
    }

    float avail = PatternAvailabilityCalculator::CalcPbAvailability(db, pbid, toonid, excludeassembled);
    m_avail = STREAM2STR((int)avail);

    {  // Determine PB name
        g_DBManager.Lock();
        sqlite::ITablePtr pT = m_db->ExecTable(STREAM2STR(_T("SELECT name FROM tblPocketBoss WHERE pbid = ") << pbid));
        g_DBManager.UnLock();

        if (pT != NULL && pT->Rows() > 0)
        {
            m_pbname = from_ascii_copy(pT->Data(0, 0));
        }
    }

    //          userid               containerid             pattern    count
    std::map<unsigned int, std::map<unsigned int, std::map<std::tstring, int> > > pieces;

    {  // Find all patternpieces
        g_DBManager.Lock();
        sqlite::ITablePtr pIDs = m_db
            ->ExecTable(STREAM2STR(_T(
            "SELECT aoid, pattern FROM tblPatterns WHERE name = (SELECT name FROM tblPocketBoss WHERE pbid = ") << pbid << ")"));
        g_DBManager.UnLock();

        // Copy patternpiece IDs to map

        //std::map<std::tstring, unsigned int> PatternItemIds = boost::assign::map_list_of("A", 0)("B", 0)("C", 0)("D", 0)("AB", 0)("ABC", 0)("ABCD", 0);
        std::map<std::tstring, std::tstring> PatternItemIds;
        for (unsigned int idIdx = 0; idIdx < pIDs->Rows(); ++idIdx)
        {
            PatternItemIds[from_ascii_copy(pIDs->Data(idIdx, 1))] = from_ascii_copy(pIDs->Data(idIdx, 0));
        }

        for (std::map<std::tstring, std::tstring>::iterator it = PatternItemIds.begin();
            it != PatternItemIds.end(); ++it)
        {
            std::tstring pattern = it->first;
            std::tstring itemid = it->second;

            // Find all locations of specified piece.
            std::tstring sql = STREAM2STR(_T("SELECT parent, owner FROM tItems WHERE keylow = ") << itemid);
            if (m_toonid > 0)
            {
                sql += STREAM2STR(_T(" AND owner = ") << toonid);
            }
            g_DBManager.Lock();
            sqlite::ITablePtr pItems = m_db->ExecTable(sql);
            g_DBManager.UnLock();

            for (unsigned int itemIdx = 0; itemIdx < pItems->Rows(); ++itemIdx)
            {
                unsigned int containerid = boost::lexical_cast<unsigned int>(pItems->Data(itemIdx, 0));
                unsigned int owner = m_toonid > 0 ? m_toonid : boost::lexical_cast<unsigned int>(pItems->Data(itemIdx,
                    1));

                if (pieces[owner][containerid].find(pattern) != pieces[owner][containerid].end())
                {
                    pieces[owner][containerid][pattern] += 1;
                }
                else
                {
                    pieces[owner][containerid][pattern] = 1;
                }
            }
        }
    }

    std::map<unsigned int, int> rowsPerToon;
    std::map<unsigned int, int> rowsPerContainer;

    std::map<unsigned int, std::map<unsigned int, std::map<std::tstring, int> > >::iterator it;
    std::map<unsigned int, std::map<std::tstring, int> >::iterator it2;
    std::map<std::tstring, int>::iterator it3;

    for (it = pieces.begin(); it != pieces.end(); ++it)
    {
        rowsPerToon[it->first] = it->second.size();
        for (it2 = it->second.begin(); it2 != it->second.end(); ++it2)
        {
            rowsPerContainer[it2->first] = it2->second.size();
            rowsPerToon[it->first] += (rowsPerContainer[it2->first] - 1);
        }
    }

    std::tstringstream out;
    out << _T("<tr><th>Toon</th><th>Location</th><th>Pieces</th><th>Count</th></tr>\n");

    for (it = pieces.begin(); it != pieces.end(); ++it)
    {
        for (it2 = it->second.begin(); it2 != it->second.end(); ++it2)
        {
            for (it3 = it2->second.begin(); it3 != it2->second.end(); ++it3)
            {
                out << "<tr>";
                if (it2 == it->second.begin() && it3 == it2->second.begin())
                {
                    g_DBManager.Lock();
                    std::tstring toon_name = g_DBManager.GetToonName(it->first);
                    g_DBManager.UnLock();
                    out << _T("<td rowspan=") << rowsPerToon[it->first] << _T(">") << toon_name << _T("</td>");
                }
                if (it3 == it2->second.begin())
                {
                    out << "<td rowspan=" << rowsPerContainer[it2->first] << ">"
                        << m_containerManager->GetContainerName(it->first, it2->first)
                        << "</td>";
                }
                out << "<td>" << it3->first << "</td>";
                out << "<td>" << it3->second << "</td>";
                out << "</tr>\n";
            }
        }
    }

    m_table = out.str();
}


PatternReport::~PatternReport(void) {}


std::tstring PatternReport::toString() const
{
    std::tstringstream out;

    HRSRC hrsrc = ::FindResource(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_HTML1), RT_HTML);
    DWORD size = ::SizeofResource(_Module.GetResourceInstance(), hrsrc);
    HGLOBAL hGlobal = ::LoadResource(_Module.GetResourceInstance(), hrsrc);
    void* pData = ::LockResource(hGlobal);

    if (pData != NULL)
    {
        std::string raw((char*)pData, size);
        std::tstring html = from_ascii_copy(raw);

        boost::algorithm::replace_all(html, _T("%TIME_AND_DATE%"), m_time);
        boost::algorithm::replace_all(html, _T("%TOON_NAME%"), m_toonname);
        boost::algorithm::replace_all(html, _T("%PB_NAME%"), m_pbname);
        boost::algorithm::replace_all(html, _T("%PB_AVAILABILITY%"), m_avail);
        boost::algorithm::replace_all(html, _T("%RESULT_TABLE%"), m_table);

        out << html;
    }

    return out.str();
}
