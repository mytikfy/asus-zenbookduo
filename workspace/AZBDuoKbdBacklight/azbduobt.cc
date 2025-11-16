
#include "azbduobt.h"

AzbDuoBt::AzbDuoBt()
{
}

BtRaw::BtInfo *AzbDuoBt::findKeyboard()
{
	collect(true);
	return find(0, m_kbdname);
}

void AzbDuoBt::getKeyboard(bool force)
{
	if ((!m_keyboard) || (force)) {
		m_keyboard = findKeyboard();
	}
}

int AzbDuoBt::setKbdBacklight(int value)
{
	getKeyboard(true);

	if ((m_keyboard) && m_keyboard->connected) {
		std::vector<guint8> v = {0xba, 0xc5, 0xc4, (guint8)value};
		return super::write("/org/bluez/hci0/dev_DA_88_5F_79_06_F1/service001b/char003b", v);
	}

	return 1;
}

bool AzbDuoBt::isConnected()
{
	getKeyboard(true);

	if (m_keyboard) {
		return m_keyboard->connected;
	}

	return false;
}
