#ifndef TEMPCONTAINERCACHE_H
#define TEMPCONTAINERCACHE_H

#include <map>
#include <boost/shared_ptr.hpp>


namespace aoia
{
    namespace inv
    {

        class TempContainerCache
        {
        public:
            TempContainerCache();
            virtual ~TempContainerCache();

            void ClearTempContainerIdCache(unsigned int charId);
            void UpdateTempContainerId(unsigned int charId, unsigned int tempId, unsigned int containerId);
            unsigned int GetContainerId(unsigned int charId, unsigned int tempId) const;

            unsigned int GetItemSlotId(unsigned int charId, unsigned int itemTempId) const;
            void UpdateTempItemId(unsigned int charId, unsigned int itemTempId, unsigned int slotId);

        private:
            std::map<__int32, std::map<__int32, unsigned int> > m_containerIdCache; //char id and container Id are keys.
            std::map<__int32, std::map<__int32, unsigned int> > m_invSlotForTempItemCache; //char id and container Id are keys.
        };

        typedef boost::shared_ptr<TempContainerCache> TempContainerCachePtr;

    }   // namespace inv
}  // namespace aoia



#endif // TEMPCONTAINERCACHE_H
