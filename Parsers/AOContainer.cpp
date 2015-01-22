#include "AOContainer.h"

namespace Parsers {

    AOContainer::AOContainer(char *pRaw, unsigned int size)
        : AOMessageBase(pRaw, size)
    {
        skip(4);                    // 0x01000000 object type?
        m_slots = popChar();        // 0x15 for backpacks
        skip(4);                    // 0x00000003 ??? introduced after 16.1 or so..

        m_itemCount = (popInteger() - 1009)/1009;   // mass? seems related to number of items in backpack. 1009 + 1009*numitems
        for (unsigned int i = 0; i < m_itemCount; ++i) {
            m_items.push_back(AOContainerItem(*this));
        }

        m_containerId = AOObjectId(*this);
        m_counter = popInteger();   // Number of times this message has been received?
        skip(4);
    }

}   // namespace
