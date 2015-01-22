#include "AOClientMessageBase.h"

namespace Parsers {

    AOClientMessageBase::AOClientMessageBase(char *pRaw, unsigned int size)
        : Parser(pRaw, size) 
        , m_type(UNKNOWN_MESSAGE)
        , m_characterid(0)
        , m_messageid(0)
    {
        //Client Send Header parsing:

        //H3 = I4 msgType  I4 Zero     I4  someId  I4   02     I4 msgId I4+I4 charId
        // Parse and validate header
        unsigned int t = popInteger();
        if (t == 0x0000000A) {
            m_type = MSG_TYPE_A;
        }
        //else if (t == 0x0000000D) {
        //    m_type = MSG_TYPE_D;
        //   // char c = popChar();
        //   // assert(c == 0x0A);
        //}
        else if (t == 0x0000000B) {
            m_type = MSG_TYPE_B;
        }
        //else if (t == 0x0000000E) {
        //    m_type = MSG_TYPE_E;
        //}
        else if (t == 0x00000001) {
            m_type = MSG_TYPE_1;
        }
        else {
            //Logger::instance().log(STREAM2STR(_T("Error unknown client message header: ") << t));
            return;
        }

        skip(4);    // 0x00 00 00 00
        skip(4);    // Receiver: Instance ID? Dimension? Subsystem?
        skip(4); //??ex 00 00 00 02 or 00 00 0b ed  on logoff

        m_messageid = popInteger();

        skip(4);    // Target Entity: Instance ID? Dimension? Subsystem? 50000
        //m_entityid = popInteger();

        m_characterid = popInteger();
    }

}   // namespace