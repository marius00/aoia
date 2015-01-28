#include "AOObjectID.h"

namespace Parsers {

    AOObjectId::AOObjectId()
        : m_low(0)
        , m_high(0)
    {
    }

    AOObjectId::AOObjectId(Parser &p)
    {
        m_low = p.popInteger();
        m_high = p.popInteger();
    }

}   // namespace
