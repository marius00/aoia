#ifndef IBACKPACKNAMES_H
#define IBACKPACKNAMES_H

namespace aoia {

    struct IContainerManager 
    {
        virtual std::tstring GetContainerName(unsigned int character_id, unsigned int container_id) const = 0;
    };

    typedef boost::shared_ptr<IContainerManager> IContainerManagerPtr;

}

#endif // IBACKPACKNAMES_H
