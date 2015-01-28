#ifndef AOFULLECHARACTERSYNC_H
#define AOFULLECHARACTERSYNC_H

#include <vector>
#include <map>
#include "AOMessageBase.h"
#include "AOContainerItem.h"

namespace Parsers {

    class AOFullCharacterMessage
        : public AOMessageBase
    {
    public:
        AOFullCharacterMessage(char *pRaw, unsigned int size);

        unsigned int numitems() const;
        AOContainerItemPtr getItem(unsigned int index) const;
        void getAllItems(std::vector<AOContainerItemPtr> &items) const;
        void getStats(std::map<unsigned int, unsigned int> &stats) const;

    private:
        std::vector<AOContainerItemPtr> m_inventory;
        std::vector<unsigned int> m_nanos;
        std::map<unsigned int, unsigned int> m_stats;
    };

};  // namespace Parsers

#endif // AOFULLECHARACTERSYNC_H
