#include "AOPlayerShopContent.h"

namespace Parsers {

    AOPlayerShopContent::AOPlayerShopContent(char *pRaw, unsigned int size)
        : AOMessageBase(pRaw, size)
    {
        // Starts the same as a container...
        skip(4);                    // 0x01000000 object type?
        m_slots = popChar();        // 0x15 for backpacks
        skip(4);                    // 0x00000003 ??? introduced after 16.1 or so..

        m_itemCount = (popInteger() - 1009)/1009;   // mass? seems related to number of items in backpack. 1009 + 1009*numitems
        for (unsigned int i = 0; i < m_itemCount; ++i) {
            m_items.push_back(AOContainerItem(*this));
        }

        // ...but has a different ending.
        skip(4);    // ?
        m_shopid = popInteger();
        skip(12);    // ?
        for (unsigned int i = 0; i < 21; ++i) {
            m_prices[i] = popInteger();
        }
        skip(1);    // ?
    }

}   // namespace
