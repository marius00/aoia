#ifndef AOMESSAGEIDS_H
#define AOMESSAGEIDS_H

namespace AO {

    enum MsgIds
    {
        MSG_SKILLSYNC       = 0x3E205660,
        MSG_CONTAINER       = 0x4E536976,
        MSG_BACKPACK        = 0x465A5D73,
        MSG_CHAT            = 0x5F4B442A,
        MSG_MISSIONS        = 0x5C436609,
        MSG_BANK            = 0x343c287f,   // size 2285.. dynamic most likely
        // MSG_POS_SYNC     = 0x5E477770,   // size = 55 bytes Thats wrong?
        MSG_UNKNOWN_1       = 0x25314D6D,   // size = 53 bytes
        MSG_UNKNOWN_2       = 0x00000001,   // size = 40 bytes
        MSG_UNKNOWN_3       = 0x0000000b,   // size = 548 bytes
        MSG_FULLSYNC        = 0x29304349,   // size 4252.. dynamic?  Contains equip and inv + atribs (?) +++
        MSG_UNKNOWN_4       = 0x54111123,
        MSG_MOB_SYNC        = 0x271B3A6B,
        MSG_SHOP_ITEMS      = 0x353F4F52,   // Content of a player shop
        MSG_SHOP_INFO       = 0x2e072a78,   // Information about a playershop. Received on zone into a playershop.
        MSG_ITEM_MOVE       = 0x47537a24,   //Client or server msg move item between inv/bank/wear/backpack
        MSG_ITEM_BOUGHT     = 0x052e2f0c,   //Sent from server when buying stuff, 1 message pr item.
        MSG_OPENBACKPACK    = 0x52526858,   //1196653092: from client when opening backpack
        MSG_CHAR_OPERATION  = 0x5e477770,   //was MSG_ITEM_OPERATION //from client when deleting an item, splitting or joining, nano, perk, tradeskill+++
        MSG_SHOP_TRANSACTION= 0x36284f6e,   //from server and client when adding/removing/accepting shop stuff
        MSG_PARTNER_TRADE   = 0x35505644,   //from server when trade partner adds/removes items.
        MSG_SHOW_TEXT       = 0x206b4b73,   //from server when showing yellow text (like "You succesfully combined.../XXX is already fully loaded")
        MSG_SPAWN_REWARD    = 0x3b11256f,
        //MSG_DELETE_MISH??= 0x212c487a 
        //MSG_NPC_CHATLINE = 0x5d70532a
        //MSG_NPC_CHATCHOICES = 0x55704d31
        //MSG_NPC_TRACE_CANCEL = 0x2d212407
        MSG_GIVE_TO_NPC     = 0x3a1b2c0c,   //give or remove from npc trade win
        MSG_ACCEPT_NPC_TRADE= 0x2d212407,   //From server when accepting trade, contains items npc didnt want..
        MSG_END_NPC_TRADE   = 0x55682b24,   //From client when ending trade, contains accept or cancel
        /*
        followed by a move slotId0 from overflow each!
        spiked food sack: (id 0187C4)
        DFDF000A 00010077 00000BF9 991FC4E6 3B11256F
        0000C73D 498B058E 00000000 0B0000C3 50991FC4
        E6000002 1C000F42 4F000000 00716F00 001F8800
        00000080 00000300 00001700 0187C400 0002BD00
        00000100 0002BE00 0187C400 0002BF00 0187C400
        00019C00 00000100 0001EB00 00000100 000000FD

        mission key:
        DFDF000A 00010089 00000BF9 991FC4E6 3B11256F
        0000C76D 0907C318 00000000 0B0000C3 50991FC4
        E6000002 1C000F42 4F000000 00716F00 001B9700
        00000080 00020500 00001700 006FA100 0002BD00
        00000100 0002BE00 006FA100 0002BF00 006FA100
        00019C00 00000100 00001A4D 69737369 6F6E206B
        65792074 6F206120 6275696C 64696E67 00FDFDFD

        [5500] operationId  0xe 
        [5500] operationId  0x71 

        //Also TODO: mish items deleted when mish deleted not auto
        */

        MSG_CHAR_INFO       = 0x4d38242e,
        MSG_ORG_CONTRACT    = 0x64582a07,
        MSG_ORG_CITY_P1     = 0x365e555b,
        MSG_ORG_CITY_P2     = 0x5f4b442a,

        /*
        Sent to client when a nano program terminated.
        Ex: Body Boost (29091 / 0x71A3) terminated:
        2AB4000A 00010027 00000BDE AB106B4D 39343C68
        0000C350 AB106B4D 00000000 00CF1B00 0071A3FD <-- ID almost at the end.
        */
        MSG_PROGRAM_TERMINATED = 0x39343c68,    


		// UNUSED / DUPLICATES
		SMSG_MOB_SYNC = 0x271B3A6B, // 656095851
		CMSG_ATTACK_MOB = 675889264,
		CMSG_TARGET_MOB = 575816799,
		CMSG_LOOT = 1381132376,
		CMSG_FLINGSHOT = 1363747104,
		CMSG_NCURELATED = 1581741936, // could be multi-usage (get = 2 of these per jump),
		SMSG_SUPERSPAM = 724778350,
		SMSG_PARTIALSPAM = 1174417174,
		SMSG_MOVED = 1410404643,
		SMSG_UNKNOWN2 = 638531185, // Containes mob names, whatever it is.
		SMSG_UNKNOWN1 = 2136230149,
		SMSG_TBLOB = 1295524910,
		MSG_TYPE_SERVER = 1,
		MSG_TYPE_CLIENT = 2,
		SMSG_NANO_ME = 623988077,
		SMSG_UNKNOWN3 = 1599226158,
		SMSG_UNKNOWN4 = 691028809,
		SMSG_UNKNOWN5 = 1428293414,
		SMSG_UNKNOWN6 = 1495335206,
		SMSG_UNKNOWN7 = 490475292,			// One every start-attack,
		SMSG_UNKNOWN8 = 1096961805,
		SMSG_UNKNOWN9 = 624755264,
		SMSG_UNKNOWN10 = 1460412473,
		SMSG_ITEMLOOT = 1196653092,
		SMSG_CONTAINER = 1314089334,
		SMSG_LOOT2 = 1381132376,

		SMSG_STARTATTACK = 675889264, // Receive one on start-attack,
		SMSG_STOPATTACK = 1245782078, // Receive = 2 on stop attack

		SMSG_UNKNOWN_WITH_ZONEID0 = 1125743906,	// [2+8*4], [2+12*4]
		SMSG_UNKNOWN_WITH_ZONEID1 = 1598757433,	// [2+5*4], [2+9*4]
		SMSG_UNKNOWN_WITH_ZONEID2 = 991765096,	// [1 + 3*4]
		SMSG_POSSIBLE_CORPSESPAWN = 1330073093, // it triggers on a mob death, nothing to indicate its useful as of yet..
    };

}   // namespace

#endif // AOMESSAGEIDS_H