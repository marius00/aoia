#ifndef AOPLAYERSHOPCONTENT_H
#define AOPLAYERSHOPCONTENT_H

#include <vector>
#include "AOMessageBase.h"
#include "AOContainerItem.h"

namespace Parsers {

    class AOPlayerShopContent
        : public AOMessageBase
    {
    public:
        AOPlayerShopContent(char *pRaw, unsigned int size);

        unsigned char numslots() const { return m_slots; }
        unsigned int numitems() const { return m_itemCount; }
        AOContainerItem item(int index) const { return m_items.at(index); }
        unsigned int shopid() const { return m_shopid; }
        unsigned int price(int index) const { return m_prices[index]; }

    private:
        unsigned char m_slots;
        unsigned int m_itemCount;
        std::vector<AOContainerItem> m_items;
        unsigned int m_shopid;
        unsigned int m_prices[21];     // Array of prices per shop slot
    };

}   // namespace

#endif // AOPLAYERSHOPCONTENT_H
