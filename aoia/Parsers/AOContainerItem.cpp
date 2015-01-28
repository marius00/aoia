#include "AOContainerItem.h"

namespace Parsers {

    AOContainerItem::AOContainerItem(Parser &p)
    {
        m_index = p.popInteger();
        m_flags = p.popShort();
        m_stack = p.popShort();
        m_containerid = AOObjectId(p);
        m_itemid = AOObjectId(p);
        m_ql = p.popInteger();
        p.skip(4);
    }

}   // namespace
