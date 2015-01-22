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

bool verify(AOMessageBase msg, int id) {
	int _target = 1183747; // backyard hunting grounds
	//
	bool s = false;

	int skips = 0;
	while (msg.remaining() >= 4) {
		int n = msg.popInteger();
		if (n == _target) {
			std::stringstream st;
			st << "Voila: " << id << " skips: " << skips <<  "\n";
			OutputDebugStringA(st.str().c_str());
			s = true;
		}	
		skips++ ;
	}
	return s;
}

bool verify4x(AOMessageBase msg) {
	bool b1 = verify(msg, 0);
	msg.popChar();

	bool b2 = verify(msg, 1);
	msg.popChar();

	bool b3 = verify(msg, 2);
	//msg.popChar();
	return b1 || b2 || b3;
}

std::set<unsigned int> ignore;
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
    /*
	case AO::SMSG_POSSIBLE_CORPSESPAWN:
		{
			std::stringstream st;
			st << "SMSG_POSSIBLE_CORPSESPAWN: ";
			char const hex[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A',   'B','C','D','E','F'};

			while (msg.remaining() > 0) {
				const char ch = msg.popChar();
				st << hex[(ch  & 0xF0) >> 4];
				st << hex[ch & 0xF];
			
			}
			st << "\n";
			OutputDebugStringA(st.str().c_str());
		}
		break;*/


		/*
	default: 
		{

			std::stringstream st;
			if (ignore.empty()) {
				unsigned int ids[] = {1,11,541676156,543902579,675889264,1245782078,490475292,623988077,638531185,656095851,724778350,824779579,894457412,911278200,923805036,959724648,1180327283,1314089334,1347702041,1381132376,1410404643,1581741936};
				for (int i = 0; i < sizeof(ids)/sizeof(unsigned int); i++)
					ignore.insert(ids[i]);
			}
			if (ignore.find(msg.messageId()) == ignore.cend()) {
				verify4x(msg);
			//ignore.insert(msg.messageId());
				st << "^-- IDs: " << msg.messageId();
				//for (auto it : ignore) { st << it << ","; }
				st << std::endl;
				OutputDebugStringA(st.str().c_str());
			}

		 }
		break;*/
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
