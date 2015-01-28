#ifndef AOMESSAGEBASE_H
#define AOMESSAGEBASE_H

#include <shared/Parser.h>

namespace Parsers {

    class AOMessageBase
        : public Parser
    {
    public:
        enum HeaderType {
            UNKNOWN_MESSAGE,
            MSG_TYPE_1 = 0x0001,
            MSG_TYPE_A = 0x000A,
            MSG_TYPE_B = 0x000B,
            MSG_TYPE_D = 0x000D,
            MSG_TYPE_E = 0x000E,
        };

        AOMessageBase(char *pRaw, unsigned int size);

        HeaderType headerType() const { return m_type; }
        unsigned int size() const { return m_size; }
        unsigned int characterId() const { return m_characterid; }
        unsigned int messageId() const { return m_messageid; }
        unsigned int entityId() const { return m_entityid; }

    private:
        HeaderType m_type;
        unsigned int m_size;
        unsigned int m_characterid;
        unsigned int m_messageid;
        unsigned int m_entityid;
    };


}   // namespace

#endif // AOMESSAGEBASE_H