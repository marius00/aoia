#include "AOFullCharacterSync.h"

namespace Parsers {

    AOFullCharacterMessage::AOFullCharacterMessage(char *pRaw, unsigned int size)
        : AOMessageBase(pRaw, size)
    {
        popChar();  // padding?
        popInteger(); // version field (0x19)

        // Read inventory and equip
        unsigned int count = pop3F1Count();
        for (unsigned int i = 0; i < count; ++i)
        {
            m_inventory.push_back(AOContainerItemPtr(new AOContainerItem(*this)));
        }

        // Read uploaded nano programs
        count = pop3F1Count();
        for (unsigned int i = 0; i < count; ++i)
        {
            m_nanos.push_back(popInteger());
        }

        // Unknown (but empty) collection
        count = pop3F1Count();
        assert(count == 0);

        // unknown ints (perk timers?)
        for (unsigned int i = 0; i < 3; ++i)
        {
            unsigned int key = popInteger();
            unsigned int val = popInteger();
            for (unsigned int j = 0; j < val; ++j)
            {
                //00 00 00 01 00 00 d6 f3 00 00 01 cd 00 00 00 5a
                //00 00 00 50 00 00 00 01 00 00 d6 f3 00 00 01 91
                //00 00 00 f0 00 00 00 ed
                skip(sizeof(unsigned int) * 5);
            }
        }

        // Stats map (32 bit id, 32 bit value)
        count = pop3F1Count();
        for (unsigned int i = 0; i < count; ++i)
        {
            unsigned int key = popInteger();
            unsigned int val = popInteger();
            m_stats[key] = val;
        }

        // Stats map (32 bit id, 32 bit value)
        count = pop3F1Count();
        for (unsigned int i = 0; i < count; ++i)
        {
            unsigned int key = popInteger();
            unsigned int val = popInteger();
            m_stats[key] = val;
        }

        // Stats map (8 bit id, 8 bit value)
        count = pop3F1Count();
        for (unsigned int i = 0; i < count; ++i)
        {
            unsigned char key = popChar();
            unsigned char val = popChar();
            m_stats[key] = val;
        }

        // Stats map (8 bit id, 16 bit value)
        count = pop3F1Count();
        for (unsigned int i = 0; i < count; ++i)
        {
            unsigned char key = popChar();
            unsigned short val = popShort();
            m_stats[key] = val;
        }

        // Unknown array (resetting absorb AC?)
        count = popInteger();
        for (unsigned int i = 0; i < count; ++i) {
            unsigned int key = popInteger();
            unsigned int val = popInteger();
        }

        // unknown int
        skip(sizeof(unsigned int));

        // Unknown (but empty) collection
        count = pop3F1Count();
        assert(count == 0);

        // Unknown (but empty) collection
        count = pop3F1Count();
        assert(count == 0);

        // Unknown collection of 16 byte structs.
        // Probably perk information of some sort?
        count = pop3F1Count();
        skip(count * 16);

        assert(pos() == end());
    }


    unsigned int AOFullCharacterMessage::numitems() const
    {
        return m_inventory.size();
    }


    AOContainerItemPtr AOFullCharacterMessage::getItem(unsigned int index) const
    {
        if (index < m_inventory.size())
        {
            return m_inventory.at(index);
        }
        return AOContainerItemPtr();
    }


    void AOFullCharacterMessage::getAllItems( std::vector<AOContainerItemPtr> &items ) const
    {
        items.insert(items.begin(), m_inventory.begin(), m_inventory.end());
    }


    void AOFullCharacterMessage::getStats( std::map<unsigned int, unsigned int> &stats ) const
    {
        stats.insert(m_stats.begin(), m_stats.end());
    }

}   // namespace
