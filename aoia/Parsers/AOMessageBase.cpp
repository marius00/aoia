#include "AOMessageBase.h"
#include <assert.h>

namespace Parsers {

    AOMessageBase::AOMessageBase(char *pRaw, unsigned int size)
        : Parser(pRaw, size) 
        , m_type(UNKNOWN_MESSAGE)
        , m_size(0)
        , m_characterid(0)
        , m_messageid(0)
        , m_entityid(0)
    {
        skip(2);    // Skip the sequence number (short). introduced in v18.1 and replaces the old 0xDFDF sequence.

        // Parse and validate header
        unsigned short t = popShort();
        if (t == 0x000A) {
            m_type = MSG_TYPE_A;
        }
        else if (t == 0x000D) {
            m_type = MSG_TYPE_D;
            char c = popChar();
            assert(c == 0x0A);
        }
        else if (t == 0x000B) {
            m_type = MSG_TYPE_B;
        }
        else if (t == 0x000E) {
            m_type = MSG_TYPE_E;
        }
        else if (t == MSG_TYPE_1) {
            m_type = MSG_TYPE_1;
        }
        else {
            //Logger::instance().log(STREAM2STR(_T("Error unknown message header: ") << t));
            return;
        }

        skip(2);    // 0x0001		protocol version?
        m_size = popShort();
        skip(4);    // Receiver: Instance ID? Dimension? Subsystem?
        m_characterid = popInteger();
        m_messageid = popInteger();
        skip(4);    // Target Entity: Instance ID? Dimension? Subsystem?
        m_entityid = popInteger();
    }

}   // namespace
