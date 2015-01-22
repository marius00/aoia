#ifndef AOCLIENTMESSAGEBASE_H
#define AOCLIENTMESSAGEBASE_H

#include <shared/Parser.h>

namespace Parsers {

    class AOClientMessageBase
        : public Parser
    {
    public:
        enum HeaderType {
            UNKNOWN_MESSAGE,
            MSG_TYPE_1 = 0x00000001,
            MSG_TYPE_A = 0x0000000A,
            MSG_TYPE_B = 0x0000000B,
            MSG_TYPE_D = 0x0000000D,
            MSG_TYPE_E = 0x0000000E,
        };

        AOClientMessageBase(char *pRaw, unsigned int size);

        HeaderType headerType() const { return m_type; }
        unsigned int characterId() const { return m_characterid; }
        unsigned int messageId() const { return m_messageid; }

    private:
        HeaderType m_type;
        unsigned int m_characterid;
        unsigned int m_messageid;
    };

}   // namespace

#endif // AOCLIENTMESSAGEBASE_H
