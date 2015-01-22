#include "StdAfx.h"
#include "Desktop.h"

namespace aoia
{
    Desktop::Desktop()
    {
    }


    Desktop::~Desktop()
    {
    }


    bool Desktop::ContainsPoint(int x, int y) const
    {
        POINT p = { x, y };

        return MonitorFromPoint(p, MONITOR_DEFAULTTONULL) != NULL;
    }


    bool Desktop::IntersectsRect( RECT rect ) const
    {
        return MonitorFromRect(&rect, MONITOR_DEFAULTTONULL) != NULL;
    }

}
