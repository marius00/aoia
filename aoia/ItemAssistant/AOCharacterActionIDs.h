#ifndef AOCHARACTERACTIONIDS_H
#define AOCHARACTERACTIONIDS_H

namespace AO {

    enum CharacterActionIds
    {
        CHAR_ACTION_RUN_NANO	= 0x13,

        CHAR_ACTION_DELETE_TEMP_ITEM  = 0x2f,

        CHAR_ACTION_SPLITSTACK	= 0x34,
        CHAR_ACTION_JOINSTACKS	= 0x35,
        CHAR_ACTION_DELETEITEM	= 0x70,
        CHAR_ACTION_PLAYERSHOP	= 0xb4,

        CHAR_ACTION_UKNOWN3		= 0xce,//item.high contains a number

        CHAR_ACTION_LE_PROC1	= 0xB1,
        CHAR_ACTION_LE_PROC2	= 0x50,
        CHAR_ACTION_RUN_PERK	= 0xB3,
        CHAR_ACTION_NANO_RAN	= 0x6b,//itemId contains 1/nanoitemid
        CHAR_ACTION_UNKNOWN1	= 0x69,
        CHAR_ACTION_UNKNOWN2    = 0x62,//unknown, something to do with other players I think
        CHAR_ACTION_TRADESKILL	= 0x51,
        CHAR_ACTION_LOGOFF1		= 0xd2,
        CHAR_ACTION_LOGOFF2		= 0x78,
        CHAR_ACTION_SNEAK_LEAVE1= 0xaa,
        CHAR_ACTION_SNEAK_LEAVE2= 0xa1,
        CHAR_ACTION_SNEAK_ENTER	= 0xa2,
        CHAR_ACTION_SNEAK		= 0xa3,
        CHAR_ACTION_SKILL_AVAIL	= 0xa4,
        CHAR_ACTION_ZONE		= 0xa7,//when starting to zone
        CHAR_ACTION_ZONE_DATA	= 0xb4,//MANY of these when entering new pf
        CHAR_ACTION_ZONED		= 0xb5,//after all xb4 messages.
        CHAR_ACTION_UNABLE_SKILL_LOCKED	= 0x84,
        CHAR_ACTION_JUMP		= 0x92,
        CHAR_ACTION_MOVED		= 0x89,//sidestep
        CHAR_ACTION_STAND		= 0x57,

        //0xdd = add/remove from Tradeskill window
        //opId == 0x80 => use an elevator
        //opId == 0x18 => leave team.
        //opId == 0x41 => cancel running nano.
        //0x57 ??? All zeroes, when you stand up
    };

}   // namespace

#endif // AOCHARACTERACTIONIDS_H