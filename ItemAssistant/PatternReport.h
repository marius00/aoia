#ifndef PATTERNREPORT_H
#define PATTERNREPORT_H

#include <shared/IDB.h>
#include <PluginSDK/IContainerManager.h>


/**
 * \brief
 * This class generates a HTML report for the specified search criterias.
 *
 * Based on the criterias specified in the constructor a HTML report-template 
 * is loaded from a resource, and filled with the results of the search.
 */
class PatternReport
{
public:
    PatternReport(sqlite::IDBPtr db, aoia::IContainerManagerPtr containerManager, unsigned int pbid, unsigned int toonid, bool excludeassembled);
    virtual ~PatternReport();

    std::tstring toString() const;

private:
    unsigned int m_pbid;
    unsigned int m_toonid;
    bool m_excludeassembled;

    std::tstring m_avail;
    std::tstring m_toonname;
    std::tstring m_time;
    std::tstring m_pbname;
    std::tstring m_table;

    sqlite::IDBPtr m_db;
    aoia::IContainerManagerPtr m_containerManager;
};


#endif // PATTERNREPORT_H
