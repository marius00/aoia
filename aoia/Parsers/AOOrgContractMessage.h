#ifndef AOORGCONTRACTMSG_H
#define AOORGCONTRACTMSG_H

#include "AOMessageBase.h"

namespace Parsers {

    class AOOrgContractMessage
        : public AOMessageBase
    {
    public:
        AOOrgContractMessage(char *pRaw, unsigned int size);
		unsigned int check_value() { return m_check_value; };

    private:
        unsigned int m_check_value;
    };

};  // namespace Parsers

#endif // AOORGCONTRACTMSG_H
