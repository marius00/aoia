#include "StdAfx.h"
#include "Logger.h"


#define LB "\r\n"


SINGLETON_IMPL(Logger);


Logger::Logger()
  : m_enabled(false)
{
}


Logger::Logger(const Logger&)
{
}


Logger::~Logger()
{
}


void Logger::init(std::tstring const& filename, std::tstring const& versioninfo)
{
    m_enabled = !filename.empty();
    if (m_enabled)
    {
        m_out.open(filename.c_str());
        log(STREAM2STR(
                 "***********************"  << LB
              << "    Logging Started"      << LB
              << "***********************"  << LB
              << "Version: " << versioninfo
        ));
        {
            TCHAR buffer[MAX_PATH];
            DWORD size = GetCurrentDirectory(MAX_PATH, buffer);
            buffer[size] = '\0';

            log(STREAM2STR("Current Directory: " << buffer));
        }
    }
}


void Logger::destroy()
{
    log(STREAM2STR(
             "***********************" << LB << std::flush
          << "   Logging Terminated  " << LB << std::flush
          << "***********************" << LB << std::flush
    ));
    if (m_out.is_open()) {
        m_out.close();
    }
    m_enabled = false;
}


void Logger::log(std::tstring const& msg)
{
    if (m_enabled && m_out.good()) {
        m_out << to_ascii_copy(msg) << LB << std::flush;
    }
}

std::ostream& Logger::stream()
{
    return m_out;
}
