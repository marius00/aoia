#ifndef AOPLAYERSHOPINFO_H
#define AOPLAYERSHOPINFO_H

#include "AOMessageBase.h"

namespace Parsers {

    class AOPlayerShopInfo
        : public AOMessageBase
    {
    public:
        AOPlayerShopInfo(char *pRaw, unsigned int size);

        unsigned int shopId() const { return entityId(); }
        unsigned int ownerId() const { return m_ownerid; }
        std::string label() const { return m_label; }
        std::string ownerName() const { return m_ownerName; }
        std::string guildName() const { return m_guildName; }

    private:
        unsigned int m_ownerid;
        std::string m_label;
        std::string m_ownerName;
        std::string m_guildName;
    };

}

#endif // AOPLAYERSHOPINFO_H