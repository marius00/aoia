#include "AOPlayerShopInfo.h"

namespace Parsers {

    AOPlayerShopInfo::AOPlayerShopInfo(char *pRaw, unsigned int size)
        : AOMessageBase(pRaw, size)
    {
        // Parse payload
        skip(126);  // ?
        m_label = popString();
        skip(19);   // ?
        m_ownerid = popInteger();
        skip(1);
        m_ownerName = popString();
        skip(4);    // ?
        m_guildName = popString();
    }

}   // namespace
