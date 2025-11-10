#ifndef BTRAW_H
#define BTRAW_H

#include <stdint.h>

#include <glib.h>
#include <gio/gio.h>

#include <string>
#include <vector>
#include <map>

class BtRaw
{
	constexpr static const char *BLUEZ_DBUS = "org.bluez";

	public:
		typedef struct {
			std::string path;
			int handle = 0;
			std::string uuid;
		} GattDescriptor;

		typedef struct {
			std::string path;
			int handle = 0;
			std::string uuid;

			std::vector<GattDescriptor> descriptors;
		} GattCharacteristic;

		typedef struct {
			std::string path;
			int handle = 0;
			std::string uuid;

			std::vector<GattCharacteristic> characteristics;
		} GattService;

		typedef struct {
			std::string path;
			std::string name;
			std::string alias;
			std::string adapter;
			uint64_t address;
			std::string addressType;
			bool connected;
			bool paired;
			bool bonded;
			std::vector<GattService> services;
		} BtInfo;

	public:
		BtRaw();
		virtual ~BtRaw();

		bool open();
		void close();
		int list();
		int collect();
		BtInfo *find(int mode, const std::string& search);
		void tree();

		int write(const char *characteristic, const std::vector<guint8>& data);

		std::ostream *cout(int bit = 0) const;

		// usage ???
		std::vector<std::string> adapters();
		int stringProperty(const std::string& path, const std::string& par1, const std::string& par2, std::string& value);
		int boolProperty(const std::string& path, const std::string& par1, const std::string& par2, bool& value);

	private:
		uint64_t toNumber(const std::string& value);
		bool assignStringValue(GString& string, GVariant *variant, const std::string& name, std::string& value);
		bool assignBoolValue(GString& string, GVariant *variant, const std::string& name, bool& value);
		bool assignUInt16Value(GString& string, GVariant *variant, const std::string& name, guint16& value);
		bool assignObjectpathValue(GString& string, GVariant *variant, const std::string& name, std::string& value);

	private:
		GDBusConnection *m_connection = nullptr;
		std::map<uint64_t, BtInfo> m_btinfos;

	private:
		static int s_verbose;
};

#endif /* BTRAW_H */

