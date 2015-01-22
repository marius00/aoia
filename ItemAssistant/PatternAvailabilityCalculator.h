#ifndef PATTERNAVAILABILITYCALCULATOR_H
#define PATTERNAVAILABILITYCALCULATOR_H

#include <Shared/Thread.h>
#include <Shared/IDB.h>


// Forward declarations
class PatternMatchView;


/**
 * \brief
 * This class spawns a new thread that will search for pocket-boss patterns 
 * given the current search criteria.
 * 
 * To trigger a search, set the appropriate filters and call Begin().
 * To cancel a search call StopPlease() followed by End().
 * Updates are posted to the owner as \e WM_UPDATE_PBLIST messages.
 */
class PatternAvailabilityCalculator : public Thread
{
public:
    PatternAvailabilityCalculator(sqlite::IDBPtr db) : m_db(db), m_index(0), m_term(false), m_toon(0), m_excludeAssembled(false) { }
    virtual ~PatternAvailabilityCalculator() { }

    void SetOwner(PatternMatchView* owner) { m_pOwner = owner; }
    void SetToon(unsigned int toon = 0) { m_toon = toon; }
    unsigned int Toon() const { return m_toon; }
    void SetExcludeAssembled(bool newVal) { m_excludeAssembled = newVal; }
    bool ExcludeAssembled() const { return m_excludeAssembled; }
    void StopPlease() { if (IsRunning()) { m_term = true; } }

    virtual DWORD ThreadProc();

    static float CalcPbAvailability(sqlite::IDBPtr db, unsigned int pbid, unsigned int toonid, bool excludeassembled);

private:
    PatternMatchView* m_pOwner;
    int m_index;
    unsigned int m_toon;
    bool m_term;
    bool m_excludeAssembled;
    sqlite::IDBPtr m_db;
};




#endif // PATTERNAVAILABILITYCALCULATOR_H
