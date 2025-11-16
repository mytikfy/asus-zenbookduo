
#ifndef AZBDUOBT_H
#define AZBDUOBT_H

#include <string>

#include "btraw.h"

class AzbDuoBt : public BtRaw
{
	typedef BtRaw super;

	public:
		AzbDuoBt();

		BtInfo *findKeyboard();
		int setKbdBacklight(int value);

	private:
		std::string m_kbdname = "ASUS Zenbook Duo Keyboard";
};



#endif	// AZBDUOBT_H

