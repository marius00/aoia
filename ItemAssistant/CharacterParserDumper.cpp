#include "StdAfx.h"
#include "CharacterParserDumper.h"
#include "DBManager.h"
#include "AOMessageParsers.h"
#include "AOMessageIDs.h"
#include "Parsers/AOOrgContractMessage.h"

namespace bfs = boost::filesystem;
using namespace Parsers;


CharacterParserDumper::CharacterParserDumper(std::tstring const& folder_name)
  : m_folder(folder_name)
{
    try
    {
        if (!bfs::exists(m_folder))
        {
            bfs::create_directory(m_folder);
        }
    }
    catch (bfs::filesystem_error &e)
    {
        LOG(_T("Unable to create directory ") << m_folder << _T("for dumping CharacterParser files.") << e.what());
    }
}

CharacterParserDumper::~CharacterParserDumper()
{
}

void CharacterParserDumper::OnAOServerMessage(AOMessageBase &msg)
{
    switch(msg.messageId())
    {
    case AO::MSG_FULLSYNC:
        {
            std::tstring toon_name = g_DBManager.GetToonName(msg.characterId());
            DumpMessageToFile(STREAM2STR(_T("fullsync_") << toon_name << _T(".bin")), msg);
        }
        break;

    case AO::MSG_CHAR_INFO:
        {
            std::tstring toon_name = g_DBManager.GetToonName(msg.characterId());
            DumpMessageToFile(STREAM2STR(_T("char_info_") << toon_name << _T(".dat")), msg);
        }
        break;

    case AO::MSG_ORG_CONTRACT:
        {
			// 59 byte versions of this packet are sent on zoning, ignore these
			if (msg.size()>59) {
				// check the check short value? need to create an AOOrgContractMessage from msg so we can see this
				AOOrgContractMessage oc_msg(msg.start(), msg.end() - msg.start());
				if (oc_msg.check_value() == 1) {
				    std::tstring toon_name = g_DBManager.GetToonName(msg.characterId());
				    DumpMessageToFile(STREAM2STR(_T("org_contracts_") << toon_name << _T(".dat")), msg);
				}
			}
        }
        break;

    case AO::MSG_ORG_CITY_P1:
        {
            std::tstring toon_name = g_DBManager.GetToonName(msg.characterId());
            DumpMessageToFile(STREAM2STR(_T("org_city_") << toon_name << _T(".dat")), msg);
        }
        break;
    }
}

void CharacterParserDumper::DumpMessageToFile(std::tstring const& file_name, Parser &msg)
{
    std::ofstream ofs((m_folder / file_name).string().c_str(), std::ios_base::out | std::ios_base::binary);
    DumpMessageToStream(ofs, msg);
}

void CharacterParserDumper::DumpMessageToStream(std::ostream &out, Parser &msg)
{
    char* p = msg.start();
    while (p < msg.end())
    {
        out << *p;
        ++p;
    }
}
