#pragma once

#include "shared/aopackets.h"


class IAOMessageTarget
{
public:
	virtual void OnAOMessage(AO::Header *pMsg) = 0;
};
