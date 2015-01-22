#ifndef INVENTORYIDS_H
#define INVENTORYIDS_H

namespace aoia {

    enum InventoryIds
    {
        INV_BANK                = 1,    // Bank container
        INV_TOONINV             = 2,    // Toon inventory, includes equipment etc.
        INV_PLAYERSHOP          = 3,    // Stuff in your shop
        INV_TRADE               = 4,    // The stuff I put on the table in a trade
        INV_TRADEPARTNER        = 5,    // The stuff a tradepartner adds to a trade.
        INV_OVERFLOW            = 6,    // Overflow window. Clear on zone!
        INV_HOTSWAPTEMP         = 7,    //  used by IA when hotswapping as temp storage.
        INV_OTHER_PLAYERSHOP    = 8,    //  Current playershop in trade.
    };

}

#endif // INVENTORYIDS_H