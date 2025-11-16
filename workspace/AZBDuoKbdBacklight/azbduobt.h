
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

		bool isConnected() override;

	private:
		void getKeyboard(bool force = false);

	private:
		std::string m_kbdname = "ASUS Zenbook Duo Keyboard";
		BtRaw::BtInfo *m_keyboard = nullptr;
};



#endif	// AZBDUOBT_H

