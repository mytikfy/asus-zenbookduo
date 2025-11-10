
#include "azbduobt.h"

AzbDuoBt::AzbDuoBt()
{
}

BtRaw::BtInfo *AzbDuoBt::findKeyboard()
{
	collect();
	return find(0, m_kbdname);
}

int AzbDuoBt::setKbdBacklight(int value)
{
	// no memory loss
	auto&& keyboard = findKeyboard();

	if ((keyboard) && keyboard->connected) {
		std::vector<guint8> v = {0xba, 0xc5, 0xc4, (guint8)value};
		// no memory loss
		return super::write("/org/bluez/hci0/dev_DA_88_5F_79_06_F1/service001b/char003b", v);
	}
	else {
		printf("not detected\n");
	}

	return 1;
}
