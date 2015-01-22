#ifndef AOCONTAINER_H
#define AOCONTAINER_H

#include <vector>
#include "AOMessageBase.h"
#include "AOObjectID.h"
#include "AOContainerItem.h"

namespace Parsers {

    class AOContainer
        : public AOMessageBase
    {
    public:
        AOContainer(char *pRaw, unsigned int size);

        unsigned char numslots() const { return m_slots; }
        unsigned int numitems() const { return m_itemCount; }
        unsigned int counter() const { return m_counter; }
        AOObjectId containerid() const { return m_containerId; }
        AOContainerItem item(int index) const { return m_items.at(index); }

    private:
        unsigned char m_slots;
        unsigned int m_itemCount;
        std::vector<AOContainerItem> m_items;
        AOObjectId m_containerId;
        unsigned int m_counter;
    };

}   // namespace

#endif // AOCONTAINER_H
