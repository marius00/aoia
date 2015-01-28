#pragma warning(disable: 4251)

#ifndef LOGGER_H
#define LOGGER_H

#include <ItemAssistantCore/ItemAssistantCore.h>
#include <string>
#include <fstream>
#include <Shared/UnicodeSupport.h>
#include <Shared/Singleton.h>


class ITEMASSISTANTCORE_API Logger
{
    SINGLETON(Logger);
public:
    ~Logger();

    void init(std::tstring const& filename, std::tstring const& versioninfo);
    void destroy();
    void log(std::tstring const& msg);
    std::ostream& stream();

private:
    std::ofstream m_out;
    bool m_enabled;
};


#ifdef DEBUG

#define TRACE(streamdef) \
{ \
    std::tstring msg = (((std::tostringstream&)(std::tostringstream().flush() << streamdef)).str()); \
    msg += _T("\n"); \
    OutputDebugString(msg.c_str()); \
}

#define LOG(streamdef) \
{ \
    std::tstring msg = (((std::tostringstream&)(std::tostringstream().flush() << streamdef)).str()); \
    Logger::instance().log(msg); \
    msg += _T("\n"); \
    OutputDebugString(msg.c_str()); \
}

#else

#define TRACE(streamdef) \
    __noop;

#define LOG(streamdef) \
{ \
    std::tstring msg = (((std::tostringstream&)(std::tostringstream().flush() << streamdef)).str()); \
    Logger::instance().log(msg); \
}

#endif

#endif // LOGGER_H
