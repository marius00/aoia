#ifndef AOOBJECTID_H
#define AOOBJECTID_H

#include <shared/Parser.h>

namespace Parsers {

    class AOObjectId
    {
    public:
        AOObjectId();
        AOObjectId(Parser &p);

        unsigned int low() const { return m_low; }
        unsigned int high() const { return m_high; }

    private:
        unsigned int m_low;
        unsigned int m_high;
    };

}   // namespace

#endif // AOOBJECTID_H