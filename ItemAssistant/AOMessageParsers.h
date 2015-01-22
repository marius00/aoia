#ifndef AOMESSAGEPARSERS_H
#define AOMESSAGEPARSERS_H

#include "shared/aopackets.h"
#include <shared/Parser.h>

namespace Native {

    class ObjectId
    {
    public:
        ObjectId(AO::AoObjectId const& id)
            : m_low(_byteswap_ulong(id.low))
            , m_high(_byteswap_ulong(id.high))
        { }

        unsigned int Low() const { return m_low; }
        unsigned int High() const { return m_high; }

        std::tstring print() const {
            std::tstringstream out;
            out << "[" << Low() << "|" << High() << "]";
            return out.str();
        }
    private:
        unsigned int	m_low;
        unsigned int	m_high;
    };


    class AOMessageHeader
    {
    public:
        AOMessageHeader(AO::Header *pHeader, bool bIsFromServer) : m_pHeader(pHeader) { m_bIsFromServer = bIsFromServer; }

        unsigned int msgid()    const { return _byteswap_ulong(m_pHeader->msgid); }
        unsigned int serverid() const { return _byteswap_ulong(m_pHeader->serverid); }
        unsigned int charid()   const { if (m_bIsFromServer) return _byteswap_ulong(m_pHeader->charid); else return _byteswap_ulong(m_pHeader->target.high);}
        unsigned short size()   const { return _byteswap_ushort(m_pHeader->msgsize); }
        unsigned int targetId()	const { return _byteswap_ulong(m_pHeader->target.high); }//for client ops, always the current char, else the real target!

        std::tstring printHeader() const {
            std::tstringstream out;
            out << "MsgId\t" << std::hex << msgid() << "\tCharId\t" << std::dec << charid() << "\tTarget\t" << targetId() << "\r\n";
            return out.str();
        }

        std::tstring print() const {
            std::tstringstream out;
            out << "MessageHeader:" << "\r\n"
                << "MsgId\t" << std::hex << msgid() << "\r\n"
                //<< "ServerId\t" << serverid() << "\r\n"
                << "Size\t" << std::dec << size() << "\r\n"
                << "Target\t" << targetId() << "\r\n"
                << "CharId\t" << charid() << "\r\n";
            return out.str();
        }

    protected:
        AO::Header *m_pHeader;
        bool m_bIsFromServer;
    };


    class AOItem
    {
    public:
        AOItem(AO::ContItem* pItem) : m_pItem(pItem) { }

        unsigned short flags() const { return _byteswap_ushort(m_pItem->flags); }
        unsigned int nullval() const { return _byteswap_ulong(m_pItem->nullval); }
        unsigned int index() const { return _byteswap_ulong(m_pItem->index); }
        unsigned int ql() const { return _byteswap_ulong(m_pItem->ql); }
        unsigned short stack() const { return _byteswap_ushort(m_pItem->stack); }
        ObjectId containerid() const { return ObjectId(m_pItem->containerid); }
        ObjectId itemid() const { return ObjectId(m_pItem->itemid); }

        std::tstring print() const {
            std::tstringstream out;
            out << "AOItem:\r\n"
                << "flags\t" << flags() << "\r\n"
                << "nullval\t" << nullval() << "\r\n"
                << "index\t" << index() << "\r\n"
                << "containerid\t" << containerid().print().c_str() << "\r\n"
                << "itemid\t" << itemid().print().c_str() << "\r\n";

            return out.str();
        }

    protected:
        AO::ContItem* m_pItem;
    };


    class AOContainer
        : public AOMessageHeader
    {
    public:
        AOContainer(AO::Header *pHeader) : AOMessageHeader(pHeader, true) { }

        unsigned char unknown1() const { return ((AO::Container*)m_pHeader)->unknown1; }
        unsigned char unknown2() const { return ((AO::Container*)m_pHeader)->unknown2; }
        unsigned char numslots() const { return ((AO::Container*)m_pHeader)->numslots; }

        unsigned int numitems() const { /*return (_byteswap_ulong(((AO::Container*)m_pHeader)->unknown2)-1009)/1009;*/ 
            AO::Container* p = ((AO::Container*)m_pHeader);
            unsigned int mass = _byteswap_ulong(p->mass);
            unsigned int result = (mass - 1009) / 1009;
            return result;
        }

        unsigned int tempContainerId() const { return _byteswap_ulong(((AO::ContEnd*)(((char*)m_pHeader)+sizeof(AO::Container)+sizeof(AO::ContItem)*numitems()))->tempContainerId); }
        ObjectId containerid() const { return ((AO::ContEnd*)(((char*)m_pHeader)+sizeof(AO::Container)+sizeof(AO::ContItem)*numitems()))->containerId; }
        AOItem item(int index) const { return AOItem( (AO::ContItem*)(((char*)m_pHeader)+sizeof(AO::Container)+sizeof(AO::ContItem)*index) ); }

        std::tstring print() const {
            std::tstringstream out;
            out << "AOContainer:" << printHeader()
                << "numslots\t" << numslots() << "\r\n"
                << "numitems\t" << numitems() << "\r\n"
                << "tempContainerId\t" << tempContainerId() << "\r\n"
                << "unknown1\t" << unknown1() << "\r\n"
                << "unknown2\t" << unknown2() << "\r\n"
                << "containerid\t" << containerid().print().c_str() << "\r\n";

            return out.str();
        }
    };


    class AOCharacterAction
        : public AOMessageHeader
    {
    public:
        AOCharacterAction(AO::CharacterAction* pDelOp, bool isFromServer) : AOMessageHeader(&(pDelOp->header), isFromServer), m_pDelOp(pDelOp) { }

        unsigned int operationId() const { return _byteswap_ulong(m_pDelOp->operationId); }
        unsigned int unknown3() const { return _byteswap_ulong(m_pDelOp->unknown3); }
        unsigned short fromType() const { return _byteswap_ushort(m_pDelOp->itemToDelete.type); }
        unsigned short fromContainerTempId() const { return _byteswap_ushort(m_pDelOp->itemToDelete.containerTempId); }
        unsigned short fromItemSlotId() const { return _byteswap_ushort(m_pDelOp->itemToDelete.itemSlotId); }
        ObjectId itemId() const { return m_pDelOp->itemId;}

        std::tstring print() const {
            std::tstringstream out;
            out << "AOCharacterAction:" << printHeader()
                << "operationId\t 0x" << std::hex << operationId() << "\r\n"
                << "unknown3\t 0x"<< std::hex << unknown3() << "\r\n"
                << "fromType\t 0x"<< std::hex << fromType() << "\r\n"
                << "fromContainerTempId\t" << fromContainerTempId() << "\r\n"
                << "fromItemSlotId\t" << fromItemSlotId() << "\r\n"
                << "itemId\t" << itemId().print().c_str() << "\r\n";

            return out.str();
        }

    protected:
        AO::CharacterAction* m_pDelOp;
    };


    class AOGiveToNPC
        : public AOMessageHeader
    {
    public:
        AOGiveToNPC(AO::GiveToNPC* pNPCOp, bool isFromServer) : AOMessageHeader(&(pNPCOp->header), isFromServer), m_pNPCOp(pNPCOp) { }

        unsigned short operationId() const { return _byteswap_ushort(m_pNPCOp->operationId); }
        unsigned char unknown1() const { return m_pNPCOp->unknown1; }
        ObjectId npcid() const { return ObjectId(m_pNPCOp->npcID); }
        unsigned int direction() const { return _byteswap_ulong(m_pNPCOp->direction); }
        unsigned int unknown3() const { return _byteswap_ulong(m_pNPCOp->unknown3); }
        unsigned int unknown4() const { return _byteswap_ulong(m_pNPCOp->unknown4); }
        unsigned short fromType() const { return _byteswap_ushort(m_pNPCOp->invItemGiven.type); }
        unsigned short fromContainerTempId() const { return _byteswap_ushort(m_pNPCOp->invItemGiven.containerTempId); }
        unsigned short fromItemSlotId() const { return _byteswap_ushort(m_pNPCOp->invItemGiven.itemSlotId); }

        std::tstring print() const {
            std::tstringstream out;
            out << "AOGiveToNPC:" << printHeader()
                << "operationId\t 0x" << std::hex << operationId() << "\r\n"
                << "npcid\t " << npcid().print() << "\r\n"
                << "unknown1\t 0x"<< std::hex << unknown1() << "\r\n"
                << "direction\t 0x"<< std::hex << direction() << "\r\n"
                << "unknown3\t 0x"<< std::hex << unknown3() << "\r\n"
                << "unknown4\t 0x"<< std::hex << unknown4() << "\r\n"
                << "fromType\t 0x"<< std::hex << fromType() << "\r\n"
                << "fromContainerTempId\t" << fromContainerTempId() << "\r\n"
                << "fromItemSlotId\t" << fromItemSlotId() << "\r\n";

            return out.str();
        }

    protected:
        AO::GiveToNPC* m_pNPCOp;
    };


    class AOEndNPCTrade
        : public AOMessageHeader
    {
    public:
        AOEndNPCTrade(AO::EndNPCTrade* pNPCOp, bool isFromServer) : AOMessageHeader(&(pNPCOp->header), isFromServer), m_pNPCOp(pNPCOp) { }

        unsigned short operationId() const { return _byteswap_ushort(m_pNPCOp->operationId); }
        unsigned char unknown1() const { return m_pNPCOp->unknown1; }
        ObjectId npcid() const { return ObjectId(m_pNPCOp->npcID); }
        unsigned int operation() const { return _byteswap_ulong(m_pNPCOp->operation); }
        unsigned int unknown2() const { return _byteswap_ulong(m_pNPCOp->unknown2); }

        std::tstring print() const {
            std::tstringstream out;
            out << "AOEndNPCTrade:" << printHeader()
                << "operationId\t 0x" << std::hex << operationId() << "\r\n"
                << "npcid\t " << npcid().print() << "\r\n"
                << "unknown1\t 0x"<< std::hex << unknown1() << " unknown2\t 0x"<< std::hex << unknown2() << "\r\n"
                << "operation\t 0x"<< std::hex << operation() << "\r\n";

            return out.str();
        }

    protected:
        AO::EndNPCTrade* m_pNPCOp;
    };


    class AONPCTradeRejectedItem
    {
    public:
        AONPCTradeRejectedItem(AO::NPCTradeRejectedItem* pRejItem): m_pNPCRejItem(pRejItem) { }

        unsigned int ql() const { return _byteswap_ulong(m_pNPCRejItem->ql); }
        unsigned int unknown1() const { return _byteswap_ulong(m_pNPCRejItem->unknown1); }
        ObjectId itemId() const { return ObjectId(m_pNPCRejItem->itemid); }

        std::tstring print() const {
            std::tstringstream out;
            out << "AONPCTradeRejectedItem:" << itemId().print()
                << "unknown1\t 0x" << std::hex << unknown1() << "\r\n"
                << "ql\t " << ql() << "\r\n";

            return out.str();
        }

    protected:
        AO::NPCTradeRejectedItem* m_pNPCRejItem;
    };


    class AONPCTradeAccept
        : public AOMessageHeader
    {
    public:
        AONPCTradeAccept(AO::NPCTradeAcceptBase* pNPCOp, bool isFromServer) : AOMessageHeader(&(pNPCOp->header), isFromServer), m_pNPCOp(pNPCOp) { }

        unsigned short operationId() const { return _byteswap_ushort(m_pNPCOp->operationId); }
        unsigned char unknown1() const { return m_pNPCOp->unknown1; }
        ObjectId npcid() const { return ObjectId(m_pNPCOp->npcID); }
        unsigned int itemCount() const { return _byteswap_ulong(m_pNPCOp->itemCount); }
        AONPCTradeRejectedItem rejectedItem(int index) const { return AONPCTradeRejectedItem((AO::NPCTradeRejectedItem*)(((char*)m_pNPCOp)+sizeof(AO::NPCTradeAcceptBase)+sizeof(AO::NPCTradeRejectedItem)*index)); }

        std::tstring print() const {
            std::tstringstream out;
            out << "AONPCTradeAccept:" << printHeader()
                << "operationId\t 0x" << std::hex << operationId() << "\r\n"
                << "npcid\t " << npcid().print() << "\r\n"
                << "unknown1\t 0x"<< std::hex << unknown1() << "\r\n"
                << "itemCount\t "<< std::dec << itemCount() << "\r\n";

            return out.str();
        }

    protected:
        AO::NPCTradeAcceptBase* m_pNPCOp;
    };


    class AOPartnerTradeItem
        : public AOMessageHeader
    {
    public:
        AOPartnerTradeItem(AO::PartnerTradeItem* pTrans) : AOMessageHeader(&(pTrans->header), true), m_pTrans(pTrans) { }

        unsigned int operationId() const { return _byteswap_ulong(m_pTrans->operationId); }
        ObjectId itemid() const { return ObjectId(m_pTrans->itemid); }
        unsigned int ql() const { return _byteswap_ulong(m_pTrans->ql); }
        unsigned short flags() const { return _byteswap_ushort(m_pTrans->flags); }
        unsigned short stack() const { return _byteswap_ushort(m_pTrans->stack); }
        unsigned short	partnerFromType() const { return _byteswap_ushort(m_pTrans->partnerInvItem.type); }
        unsigned short	partnerFromContainerTempId() const { return _byteswap_ushort(m_pTrans->partnerInvItem.containerTempId); }
        unsigned short	partnerFromItemSlotId() const { return _byteswap_ushort(m_pTrans->partnerInvItem.itemSlotId); }

        std::tstring print() const {
            std::tstringstream out;
            out << "AOPartnerTradeItem: "  << printHeader()
                //	<< "target\t" << target().print() << "\r\n"
                << "operationId\t0x" << std::hex << operationId() << "\r\n"
                << "itemid\t" << itemid().print().c_str() << "\r\n"
                << "ql\t" << std::dec << ql() << " stack\t" << stack() <<"\r\n"
                << "partnerFromType\t0x" << std::hex << partnerFromType() << "\r\n"
                << "partnerFromContainerTempId\t" << partnerFromContainerTempId() << "\r\n"
                << "partnerFromItemSlotId\t" << partnerFromItemSlotId();


            return out.str();
        }

    protected:
        AO::PartnerTradeItem* m_pTrans;
    };


    class AOTradeTransaction
        : public AOMessageHeader
    {
    public:
        AOTradeTransaction(AO::TradeTransaction* pTrans, bool isFromServer) : AOMessageHeader(&(pTrans->header), isFromServer), m_pTrans(pTrans) { }

        unsigned int	unknown2() const { return _byteswap_ulong(m_pTrans->unknown2); }
        unsigned short	fromType() const { return _byteswap_ushort(m_pTrans->itemToTrade.type); }
        unsigned short	fromContainerTempId() const { return _byteswap_ushort(m_pTrans->itemToTrade.containerTempId); }
        unsigned short	fromItemSlotId() const { return _byteswap_ushort(m_pTrans->itemToTrade.itemSlotId); }
        unsigned char	operationId() const { return m_pTrans->operationId; } //01=accept, 02=decline,03=?start?, 04=commit,05=add item,06=remove item
        ObjectId		fromId() const { return ObjectId(m_pTrans->fromId);}

        std::tstring print() const {
            std::tstringstream out;
            out << "AOTradeTransaction:"   << printHeader()
                << "unknown2\t" << std::hex << unknown2() << "\r\n"
                << "operationId\t" << std::hex << operationId() << "\r\n"
                << "fromType\t" << std::hex << fromType() << "\r\n"
                << "fromContainerTempId\t" << fromContainerTempId() << "\r\n"
                << "fromItemSlotId\t" << fromItemSlotId() << "\r\n"
                << "fromId\t" << fromId().print().c_str() << "\r\n";

            return out.str();
        }

    protected:
        AO::TradeTransaction* m_pTrans;
    };


    class AOBoughtItemFromShop
        : public AOMessageHeader
    {
    public:
        AOBoughtItemFromShop(AO::BoughtItemFromShop* pBoughItem, bool isFromServer) : AOMessageHeader(&(pBoughItem->header), isFromServer), m_pBoughItem(pBoughItem) { }

        unsigned int ql() const { return _byteswap_ulong(m_pBoughItem->ql); }
        unsigned short stack() const { return _byteswap_ushort(m_pBoughItem->stack); }
        ObjectId itemid() const { return ObjectId(m_pBoughItem->itemid); }
        unsigned short flags() const { return _byteswap_ushort(m_pBoughItem->flags); }

        std::tstring print() const {
            std::tstringstream out;
            out << "AOBoughtItemFromShop:" << printHeader()
                << "ql\t" << ql() << "\r\n"
                << "flags\t" << flags() << "\r\n"
                << "stack\t" << stack() << "\r\n"
                << "itemid\t" << itemid().print().c_str() << "\r\n";

            return out.str();
        }

    protected:
        AO::BoughtItemFromShop* m_pBoughItem;
    };


    class AOBackpack
        : public AOMessageHeader
    {
    public:
        AOBackpack(AO::Backpack* pItem, bool isFromServer) : AOMessageHeader(&(pItem->header), isFromServer), m_pItem(pItem) { }

        unsigned char	operationId() const { return m_pItem->operationId; }
        unsigned char	invSlot() const { return m_pItem->invSlot; }
        ObjectId		owner()  const { return ObjectId(m_pItem->ownerId); }
        unsigned int ql() const { return _byteswap_ulong(m_pItem->ql); }
        unsigned int keyLow() const { return _byteswap_ulong(m_pItem->itemKeyLow); }
        unsigned int keyHigh() const { return _byteswap_ulong(m_pItem->itemKeyHigh); }
        unsigned int flags() const { return _byteswap_ulong(m_pItem->flags); }

        std::tstring print() const {
            std::tstringstream out;
            out << "AOBackpack:" << printHeader()
                << "operationId\t 0x" << std::hex << operationId() << "\r\n"
                << "invSlot\t" << std::dec << invSlot() << "\r\n"
                << "ql\t" << ql() << "\r\n"
                << "keyLow\t" << keyLow() << "keyHigh\t" << keyHigh() << "\r\n"
                //	<< "target\t" << target().print() << "keyHigh\t" << keyHigh() << "\r\n"
                << "flags\t" << flags() << "\r\n";

            return out.str();
        }

    protected:
        AO::Backpack* m_pItem;
    };


    class AOItemMoved
        : public AOMessageHeader
    {
    public:
        AOItemMoved(AO::ItemMoved* pMoveOp, bool isFromServer) : AOMessageHeader(&(pMoveOp->header), isFromServer), m_pMoveOp(pMoveOp) { }

        unsigned short fromType() const { return _byteswap_ushort(m_pMoveOp->moveData.fromItem.type); }
        unsigned short fromContainerTempId() const { return _byteswap_ushort(m_pMoveOp->moveData.fromItem.containerTempId); }
        unsigned short fromItemSlotId() const { return _byteswap_ushort(m_pMoveOp->moveData.fromItem.itemSlotId); }
        ObjectId toContainer() const { return m_pMoveOp->moveData.toContainer;}
        unsigned short targetSlotId() const { return _byteswap_ushort(m_pMoveOp->moveData.targetSlot); }

        std::tstring print() const {
            std::tstringstream out;
            out << "AOItemMoved:" << printHeader()
                << "fromType\t 0x" << std::hex << fromType() << "\r\n"
                << "fromContainerTempId\t" << fromContainerTempId() << "\r\n"
                << "fromItemSlotId\t" << fromItemSlotId() << "\r\n"
                << "toContainer\t" << toContainer().print().c_str() << "\r\n"
                << "targetSlotId\t" << targetSlotId() << "\r\n";

            return out.str();
        }

    protected:
        AO::ItemMoved* m_pMoveOp;
    };

 
    class AOBank
        : public AOMessageHeader
    {
    public:
        AOBank(AO::Bank* pBank, bool isFromServer) : AOMessageHeader(&(pBank->header), isFromServer), m_pBank(pBank) { }

        unsigned int numitems() const { return (_byteswap_ulong(m_pBank->mass)-1009)/1009; }
        AOItem item(int index) const { return AOItem((AO::ContItem*)(((char*)m_pBank)+sizeof(AO::Bank)+sizeof(AO::ContItem)*index)); }

    protected:
        AO::Bank* m_pBank;
    };

};	// namespace


#endif // AOMESSAGEPARSERS_H
