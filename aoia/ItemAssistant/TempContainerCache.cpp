#include "StdAfx.h"
#include "TempContainerCache.h"

namespace aoia
{
    namespace inv
    {

        TempContainerCache::TempContainerCache() {}


        TempContainerCache::~TempContainerCache() {}


        void TempContainerCache::ClearTempContainerIdCache(unsigned int charId)
        {
            //TODO: Clear cache for only 1 char (if duallogged)
            if (m_containerIdCache.find(charId) != m_containerIdCache.end())
            {
                m_containerIdCache[charId].clear();
            }
        }


        void TempContainerCache::UpdateTempContainerId(unsigned int charId, unsigned int tempId, unsigned int containerId)
        {
            TRACE("UPDATE Temp Cont Id " << tempId << " / " << containerId);

            //__int64 key = ((__int64)charId) << 32;
            //key += fromId;
            //m_InvSlotIndexCache[key] = slotId;

            if (m_containerIdCache.find(charId) == m_containerIdCache.end())
            {
                std::map<__int32, unsigned int> newCharIdCache;
                m_containerIdCache[charId] = newCharIdCache;
            }

            m_containerIdCache[charId][tempId] = containerId;
        }


        unsigned int TempContainerCache::GetContainerId(unsigned int charId, unsigned int tempId) const
        {
            std::map<__int32, std::map<__int32, unsigned int> >::const_iterator it = m_containerIdCache.find(charId);
            if (it != m_containerIdCache.end())
            {
                std::map<__int32, unsigned int>::const_iterator it2 = it->second.find(tempId);
                if (it2 != it->second.end())
                {
                    return it2->second;
                }
            }

            return 0;
        }


        unsigned int TempContainerCache::GetItemSlotId(unsigned int charId, unsigned int itemTempId) const
        {
            std::map<__int32, std::map<__int32, unsigned int> >::const_iterator it = m_invSlotForTempItemCache
                .find(charId);
            if (it != m_invSlotForTempItemCache.end())
            {
                std::map<__int32, unsigned int>::const_iterator it2 = it->second.find(itemTempId);
                if (it2 != it->second.end())
                {
                    return it2->second;
                }
            }

            return 0;
        }


        void TempContainerCache::UpdateTempItemId(unsigned int charId, unsigned int itemTempId, unsigned int slotId)
        {
            TRACE("UPDATE Temp Item id " << itemTempId << " / " << slotId);

            //__int64 key = ((__int64)charId) << 32;
            //key += fromId;
            //m_InvSlotIndexCache[key] = slotId;

            if (m_invSlotForTempItemCache.find(charId) == m_invSlotForTempItemCache.end())
            {
                std::map<__int32, unsigned int> newCharIdCache;
                m_invSlotForTempItemCache[charId] = newCharIdCache;
            }

            m_invSlotForTempItemCache[charId][itemTempId] = slotId;
        }
    }
}
