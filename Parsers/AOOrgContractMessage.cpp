#include "AOOrgContractMessage.h"

namespace Parsers {

    AOOrgContractMessage::AOOrgContractMessage(char *pRaw, unsigned int size)
        : AOMessageBase(pRaw, size)
    {
        m_check_value = popShort();
    }

}   // namespace
