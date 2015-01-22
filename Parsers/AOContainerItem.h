#ifndef AOCONTAINERITEM_H
#define AOCONTAINERITEM_H

#include <shared/Parser.h>
#include <boost/shared_ptr.hpp>
#include "AOObjectID.h"

namespace Parsers {

    class AOContainerItem
    {
    public:
        AOContainerItem(Parser &p);

        unsigned int index() const { return m_index; }
        unsigned short stack() const { return m_stack; }
        AOObjectId containerId() const { return m_containerid; }
        AOObjectId itemId() const { return m_itemid; }
        unsigned int ql() const { return m_ql; }
        unsigned short flags() const { return m_flags; }

    private:
        unsigned int m_index;
        unsigned short m_stack;
        unsigned short m_flags;
        AOObjectId m_containerid;
        AOObjectId m_itemid;
        unsigned int m_ql;
    };

    typedef boost::shared_ptr<AOContainerItem> AOContainerItemPtr;

}   // namespace

#endif // AOCONTAINERITEM_H
