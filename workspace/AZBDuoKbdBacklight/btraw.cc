
 /*
 * based on:
 * https://github.com/wcbonner/BlueZ-DBus/blob/master/bluez-dbus.cpp
 *
 * https://blog.linumiz.com/archives/16537
 */

#include <iostream>
#include <iomanip>
#include <ios>
#include <map>
#include <stdexcept>

#include "btraw.h"

#define free_gstring(X)      { free(X.str) ; X.str = nullptr; X.len = X.allocated_len = 0; }

int BtRaw::s_verbose = 2;

BtRaw::BtRaw()
{
}

BtRaw::~BtRaw()
{
	close();
}

bool BtRaw::open()
{
	if (!m_connection) {
	    GError *error = nullptr;
		m_connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);

		if (error != nullptr) {
			g_printerr("Error connecting to system bus: %s\n", error->message);
			g_error_free(error);
			return false;
		}
	}

	return true;
}

void BtRaw::close()
{
	if (m_connection) {
	    GError *error = nullptr;
		g_dbus_connection_close_sync(m_connection, NULL, &error);

		if (error != nullptr) {
			g_printerr("Error connecting to system bus: %s\n", error->message);
			g_error_free(error);
		}

        g_object_unref(m_connection);
		m_connection = nullptr;
	}
}

std::ostream *BtRaw::cout(int bit) const
{
    static std::basic_streambuf<char> *buf = nullptr;

	if (s_verbose & (1 << bit)) {
		if (buf != nullptr) {
			std::cout.rdbuf(buf);
		}
	}
	else {
		if (buf == nullptr) {
			buf = std::cout.rdbuf();
		}
		std::cout.rdbuf(0);
	}

	return &std::cout;
}

uint64_t BtRaw::toNumber(const std::string& value)
{
	uint64_t retv = 0;
	char *s = new char[value.size() + 1];
	memset(s, 0, value.size() + 1);
	strcpy(s, value.c_str());

	char *tok = ::strtok(s, ":");

	while (tok) {
		retv *= 256;
		retv += ::strtol(tok, nullptr, 16);
		tok = ::strtok(nullptr, ":");
	}

	delete [] s;

	return retv;
}

int rprint(GVariant *v, int indent)
{
	gsize size = g_variant_n_children(v);

	for (int i = 0; i < indent; i++) {
		printf("  ");
	}

	printf("%4d: %3d, %s\n", __LINE__, (int)g_variant_n_children(v), g_variant_print(v, false));

	for (gsize i = 0; i < size; i++) {
		GVariant *c = g_variant_get_child_value(v, i);

		if (g_variant_is_container(c)) {
			rprint(c, indent + 1);
		}
		else {
			for (int ii = 0; ii < indent + 1; ii++) {
				printf("  ");
			}

			if (g_variant_is_of_type(c, G_VARIANT_TYPE_BOOLEAN)) {
				gboolean gs;
				g_variant_get(c, "b", &gs);
				printf("%4d: %s\n", __LINE__,  gs ? "true" : "false");
			}
			else if (g_variant_is_of_type(c, G_VARIANT_TYPE_STRING)) {
				GString gs;
				g_variant_get(c, "s", &gs);
				printf("%4d: %s\n", __LINE__, gs.str);
			}
			else if (g_variant_is_of_type(c, G_VARIANT_TYPE_UINT32)) {
				guint32 gs;
				g_variant_get(c, "u", &gs);
				printf("%4d: %d\n", __LINE__,  gs);
			}
			else if (g_variant_is_of_type(c, G_VARIANT_TYPE_BYTE)) {
				guint8 gs;
				g_variant_get(c, "y", &gs);
				printf("%4d: %d\n", __LINE__,  gs);
			}
			else if (g_variant_is_of_type(c, G_VARIANT_TYPE_UINT16)) {
				guint16 gs;
				g_variant_get(c, "q", &gs);
				printf("%4d: %d\n", __LINE__,  gs);
			}
			else if (g_variant_is_of_type(c, G_VARIANT_TYPE_INT16)) {
				gint16 gs;
				g_variant_get(c, "n", &gs);
				printf("%4d: %d\n", __LINE__,  gs);
			}
			else if (g_variant_is_of_type(c, G_VARIANT_TYPE_OBJECT_PATH)) {
				GString gs;
				g_variant_get(c, "o", &gs);
				printf("%4d: %s\n", __LINE__, gs.str);
			}
			else {
				printf("%4d: ----- %s\n", __LINE__, g_variant_get_type_string(c));
			}
		}
	}

	return 0;
}

int BtRaw::list()
{
    GError *error = nullptr;
    GVariant *result = g_dbus_connection_call_sync(m_connection,
            "org.bluez",                   /* Bus name of the BlueZ service */
            "/",					//  "/org/bluez/hci0/dev_DA_88_5F_79_04_F1",             /* Object path of the adapter */
            "org.freedesktop.DBus.ObjectManager", /* Interface name */
            "GetManagedObjects",                         /* Method name */
            nullptr, // g_variant_new("(ss)", par1.c_str(),  par2.c_str()), //  "org.bluez.Device1", "Name"), /* Parameters */
            G_VARIANT_TYPE("(a{oa{sa{sv}}})"),         /* Expected return type */
            G_DBUS_CALL_FLAGS_NONE,
            -1,                            /* Default timeout */
            NULL,                          /* GCancellable */
            &error);

    if (error) {
        g_printerr("Error calling Get method: %s\n", error->message);
        g_error_free(error);
        return 1;
    }

	g_free(result);

	rprint(result, 0);

	return 0;
}

void BtRaw::tree()
{
	for (auto&& info : m_btinfos) {
		*cout(1) << "d     " << std::setw(10) << std::hex << info.first << " | " << std::setw(35) << std::left << info.second.name << " | "
				<< info.second.path << " | " << info.second.services.size() << std::endl;

		for (size_t i = 0; i < info.second.services.size(); i++) {
			*cout(1) << " S      " << std::setw(10) << std::hex << "" << " | " << std::setw(35) << std::left << "" << " | "
					<< info.second.services[i].path << " | " << std::dec << info.second.services[i].characteristics.size() << std::endl;

			for (size_t ii = 0; ii < info.second.services[i].characteristics.size();  ii++) {
				*cout(1) << "  C     " << std::setw(10) << std::hex << "" << " | " << std::setw(35) << std::left << ""
						<< " | " << info.second.services[i].characteristics[ii].path
						<< " | " << info.second.services[i].characteristics[ii].uuid
						<< " | " << std::dec << info.second.services[i].characteristics[ii].descriptors.size() << std::endl;

				for (size_t iii = 0; iii < info.second.services[i].characteristics[ii].descriptors.size();  iii++) {
					*cout(1) << "   D    " << std::setw(10) << std::hex << "" << " | " << std::setw(35) << std::left << "" << " | "
							<< info.second.services[i].characteristics[ii].descriptors[iii].path << " | " << std::endl;
				}
			}
		}
	}
}

BtRaw::BtInfo *BtRaw::find(int mode, const std::string& search)
{
	for (auto&& info : m_btinfos) {
		if (mode == 0) {
			if (info.second.name == search) {
				return &info.second;
			}
		}
	}

	return nullptr;
}

int BtRaw::collect()
{
	int cflags = 0;
    GError *error = nullptr;
    GVariant *result;

    result = g_dbus_connection_call_sync(m_connection,
            "org.bluez",									/* Bus name of the BlueZ service */
            "/",											/* Object path of the adapter */
            "org.freedesktop.DBus.ObjectManager",			/* Interface name */
            "GetManagedObjects",							/* Method name */
            NULL,											/* Parameters */
            G_VARIANT_TYPE("(a{oa{sa{sv}}})"),				/* Expected return type */
            G_DBUS_CALL_FLAGS_NONE,
            -1,												/* Default timeout */
            NULL,											/* GCancellable */
            &error);

    if (error) {
        g_printerr("Error calling Get method: %s\n", error->message);
        g_error_free(error);
        return 1;
    }

	gsize size = g_variant_n_children(result);
	int indent = 0;

	gchar *ptr;
	*cout(cflags) << std::setw(4) << __LINE__ << std::setw(4 + indent) << g_variant_n_children(result) << " " << (ptr = g_variant_print(result, false)) << std::endl;
	g_free(ptr);

	indent += 2;

	for (gsize i = 0; i < size; i++) {
		GVariant *c = g_variant_get_child_value(result, i);

		if (g_variant_is_container(c)) {
			gsize s = g_variant_n_children(c);

			*cout(cflags) << std::setw(4) << __LINE__ << std::setw(4 + indent) << g_variant_n_children(c) << " " << (ptr = g_variant_print(c, false)) << std::endl;
			g_free(ptr);

			indent += 2;

			for (gsize ii = 0; ii < s; ii++) {
				GVariant *c1 = g_variant_get_child_value(c, ii);

				*cout(cflags) << std::setw(4) << __LINE__ << std::setw(4 + indent) << g_variant_n_children(c1) << " " << (ptr = g_variant_print(c1, false)) << std::endl;
				g_free(ptr);

				if (!strcmp(g_variant_get_type_string(c1), "{oa{sa{sv}}}")) {
					BtInfo info;

					GVariant *c20 = g_variant_get_child_value(c1, 0);
					*cout(cflags) << std::setw(4) << __LINE__ << std::setw(4 + indent) << 0 << " " << (ptr = g_variant_print(c20, false)) << std::endl;
					g_free(ptr);

					GString c20s;
					g_variant_get(c20, "o", &c20s);
					std::string path = c20s.str;
					g_variant_unref(c20);
					free_gstring(c20s);

					GVariant *c2 = g_variant_get_child_value(c1, 1);
					*cout(cflags) << std::setw(4) << __LINE__ << std::setw(4 + indent) << g_variant_n_children(c2) << " " << (ptr = g_variant_print(c2, false)) << std::endl;
					g_free(ptr);

					GVariant *c3 = g_variant_get_child_value(c2, 1);
					*cout(cflags) << std::setw(4) << __LINE__ << std::setw(4 + indent) << g_variant_n_children(c3) << " " << (ptr = g_variant_print(c3, false)) << std::endl;
					g_free(ptr);

					GVariant *c4 = g_variant_get_child_value(c3, 0);
					*cout(cflags) << std::setw(4) << __LINE__ << std::setw(4 + indent) << 0 << " " << (ptr = g_variant_print(c4, false)) << std::endl;
					g_free(ptr);

					GString c4s;
					g_variant_get(c4, "s", &c4s);
					g_variant_unref(c4);

					indent += 2;

					if (!strcmp(c4s.str, "org.bluez.Device1")) {
						info.path = path;
						c4 = g_variant_get_child_value(c3, 1);

						*cout(cflags) << std::setw(4) << __LINE__ << std::setw(4 + indent) << g_variant_n_children(c4) << " " << (ptr = g_variant_print(c4, false)) << std::endl;
						g_free(ptr);

						indent += 2;
						for (gsize iii = 0; iii < g_variant_n_children(c4); iii++) {
							GVariant *c5 = g_variant_get_child_value(c4, iii);

							*cout(cflags) << std::setw(4) << __LINE__ << std::setw(4 + indent) << g_variant_n_children(c5) << " " << (ptr = g_variant_print(c5, false)) << std::endl;
							g_free(ptr);

							indent += 2;
							GVariant *c6 = g_variant_get_child_value(c5, 0);

							*cout(cflags) << std::setw(4) << __LINE__ << std::setw(4 + indent) << 0 << " " << (ptr = g_variant_print(c6, true)) << std::endl;
							g_free(ptr);

							GString c6s;
							g_variant_get(c6, "s", &c6s);

							GVariant *c7 = g_variant_get_child_value(c5, 1);
							*cout(cflags) << std::setw(4) << __LINE__ << std::setw(4 + indent) << 0 << " " << (ptr = g_variant_print(c7, true)) << std::endl;
							g_free(ptr);

							do {
								if (assignStringValue(c6s, c7, "Name", info.name)) {
									break;
								}

								std::string address;
								if (assignStringValue(c6s, c7, "Address", address)) {
									info.address = toNumber(address);
									break;
								}

								if (assignStringValue(c6s, c7, "Alias", info.alias)) {
									break;
								}

								if (assignStringValue(c6s, c7, "AddressType", info.addressType)) {
									break;
								}

								if (assignObjectpathValue(c6s, c7, "Adapter", info.adapter)) {
									break;
								}

								if (assignBoolValue(c6s, c7, "Connected", info.connected)) {
									break;
								}

								if (assignBoolValue(c6s, c7, "Paired", info.paired)) {
									break;
								}

								if (assignBoolValue(c6s, c7, "Bonded", info.bonded)) {
									break;
								}
							} while (false);

							free_gstring(c6s);
							g_variant_unref(c7);
							c7 = nullptr;
							g_variant_unref(c6);
							c6 = nullptr;
							g_variant_unref(c5);
							c5 = nullptr;

							indent -= 2;
						}

						g_variant_unref(c4);
						c4 = nullptr;

						m_btinfos.insert(std::pair<uint64_t, BtInfo>(info.address, info));
						indent -= 2;
					}
					else if (!strcmp(c4s.str, "org.bluez.GattService1")) {
						GVariant *c4a = g_variant_get_child_value(c3, 1);
						*cout(cflags) << std::setw(4) << __LINE__ << std::setw(4 + indent) << g_variant_n_children(c4a) << " " << (ptr = g_variant_print(c4a, false)) << std::endl;
						g_free(ptr);

						guint16 handle = 0;
						std::string uuid;
						std::string device;

						indent += 2;
						for (gsize iii = 0; iii <  g_variant_n_children(c4a); iii++) {
							GVariant *c4a1 = g_variant_get_child_value(c4a, iii);
							*cout(cflags) << std::setw(4) << __LINE__ << std::setw(4 + indent) << g_variant_n_children(c4a1) << " " << (ptr = g_variant_print(c4a1, false)) << std::endl;
							g_free(ptr);

							indent += 2;
							GVariant *c4a2 = g_variant_get_child_value(c4a1, 0);
							*cout(cflags) << std::setw(4) << __LINE__ << std::setw(4 + indent) << 0 << " " << (ptr = g_variant_print(c4a2, false)) << std::endl;
							g_free(ptr);

							GString s1;
						 	g_variant_get(c4a2, "s", &s1);

							GVariant *c4a3 = g_variant_get_child_value(c4a1, 1);
							*cout(cflags) << std::setw(4) << __LINE__ << std::setw(4 + indent) << 0 << " " << (ptr = g_variant_print(c4a3, false)) << std::endl;
							g_free(ptr);

							do {
								if (assignUInt16Value(s1, c4a3, "Handle", handle)) {
									break;
								}

								if (assignStringValue(s1, c4a3, "UUID", uuid)) {
									break;
								}

								if (assignObjectpathValue(s1, c4a3, "Device", device)) {
									break;
								}
							} while (false);

							free_gstring(s1);

							indent -= 2;
						}


						indent -= 2;

						BtInfo *infop = nullptr;

						for (auto&& inf : m_btinfos) {
							if (device.rfind(inf.second.path, 0) == 0) {
								infop = &inf.second;
								break;
							}
						}

						if (infop) {
							GattService gs;

							gs.handle = handle;
							gs.uuid = uuid;
							gs.path = path;

							infop->services.push_back(gs);
						}
						else {
							printf("unrelated service\n");
						}
					}
					else if (!strcmp(c4s.str, "org.bluez.GattCharacteristic1")) {
						GVariant *c4a = g_variant_get_child_value(c3, 1);
						*cout(cflags) << std::setw(4) << __LINE__ << std::setw(4 + indent) << g_variant_n_children(c4a) << " " << (ptr = g_variant_print(c4a, false)) << std::endl;
						g_free(ptr);

						indent += 2;

						guint16 handle = 0;
						std::string uuid;
						std::string service;

						for (gsize iii = 0; iii <  g_variant_n_children(c4a); iii++) {
							GVariant *c4a1 = g_variant_get_child_value(c4a, iii);
							*cout(cflags) << std::setw(4) << __LINE__ << std::setw(4 + indent) << g_variant_n_children(c4a1) << " " << (ptr = g_variant_print(c4a1, false)) << std::endl;
							g_free(ptr);

							indent += 2;
							GVariant *c4a2 = g_variant_get_child_value(c4a1, 0);
							*cout(cflags) << std::setw(4) << __LINE__ << std::setw(4 + indent) << 0 << " " << (ptr = g_variant_print(c4a2, false)) << std::endl;
							g_free(ptr);

							GString s1;
						 	g_variant_get(c4a2, "s", &s1);

							GVariant *c4a3 = g_variant_get_child_value(c4a1, 1);
							*cout(cflags) << std::setw(4) << __LINE__ << std::setw(4 + indent) << 0 << " " << (ptr = g_variant_print(c4a3, false)) << std::endl;
							g_free(ptr);

							indent -= 2;

							do {
								if (assignUInt16Value(s1, c4a3, "Handle", handle)) {
									break;
								}

								if (assignStringValue(s1, c4a3, "UUID", uuid)) {
									break;
								}

								if (assignObjectpathValue(s1, c4a3, "Service", service)) {
									break;
								}
							} while(false);

							free_gstring(s1);
						}

						indent -= 2;

						auto&& find = [this] (const std::string& search) {
							for (auto&& inf : m_btinfos) {
								for (auto&& serv : inf.second.services) {
									if (serv.path == search) {
										return &serv;
									}

								}
							}

							return (GattService *)nullptr;
						};

						GattService *gs = find(service);

						if (gs != nullptr) {
							GattCharacteristic gc;

							gc.handle = handle;
							gc.uuid = uuid;
							gc.path = path;

							gs->characteristics.push_back(gc);
						}
					}
					else if (!strcmp(c4s.str, "org.bluez.GattDescriptor1")) {
						GVariant *c4a = g_variant_get_child_value(c3, 1);
						*cout(cflags) << std::setw(4) << __LINE__ << std::setw(4 + indent) << g_variant_n_children(c4a) << " " << (ptr = g_variant_print(c4a, false)) << std::endl;
						g_free(ptr);

						guint16 handle = 0;
						std::string uuid;
						std::string characteristic;

						indent += 2;

						for (gsize iii = 0; iii <  g_variant_n_children(c4a); iii++) {
							GVariant *c4a1 = g_variant_get_child_value(c4a, iii);
							*cout(cflags) << std::setw(4) << __LINE__ << std::setw(4 + indent) << g_variant_n_children(c4a1) << " " << (ptr = g_variant_print(c4a1, false)) << std::endl;
							g_free(ptr);

							indent += 2;
							GVariant *c4a2 = g_variant_get_child_value(c4a1, 0);
							*cout(cflags) << std::setw(4) << __LINE__ << std::setw(4 + indent) << 0 << " " << (ptr = g_variant_print(c4a2, false)) << std::endl;
							g_free(ptr);

							GString s1;
						 	g_variant_get(c4a2, "s", &s1);

							GVariant *c4a3 = g_variant_get_child_value(c4a1, 1);
							*cout(cflags) << std::setw(4) << __LINE__ << std::setw(4 + indent) << 0 << " " << (ptr = g_variant_print(c4a3, false)) << std::endl;
							g_free(ptr);

							do {
								if (assignUInt16Value(s1, c4a3, "Handle", handle)) {
									break;
								}

								if (assignStringValue(s1, c4a3, "UUID", uuid)) {
									break;
								}

								if (assignObjectpathValue(s1, c4a3, "Characteristic", characteristic)) {
									break;
								}
							} while(false);

							free_gstring(s1);

							indent -= 2;
						}

						auto&& find = [this] (const std::string& search) {
							for (auto&& inf : m_btinfos) {
								for (auto&& serv : inf.second.services) {
									for (auto&& chars : serv.characteristics) {
										if (chars.path == search) {
											return &chars;
										}
									}
								}
							}

							return (GattCharacteristic *)nullptr;
						};

						GattCharacteristic *gc = find(characteristic);

						if (gc != nullptr) {
							GattDescriptor gd;

							gd.handle = handle;
							gd.uuid = uuid;
							gd.path = path;

							gc->descriptors.push_back(gd);
						}
						else {
							printf("char not found\n");
						}

						indent -= 2;
					}

					free_gstring(c4s);
					g_variant_unref(c3);
					c3 = nullptr;

					indent -= 2;
				}
				else {
					printf("unknown\n");
				}
			}

			indent -= 2;
		}
		else {
			printf("failed\n");
		}

		g_variant_unref(c);
	}

	g_variant_unref(result);
	result = nullptr;

	return 0;
}

int BtRaw::stringProperty(const std::string& path, const std::string& par1, const std::string& par2, std::string& value)
{
    GError *error = nullptr;
    GVariant *result;
    GVariant *rv;
	GString rvs;

    result = g_dbus_connection_call_sync(m_connection,
            "org.bluez",                   /* Bus name of the BlueZ service */
            path.c_str(),					//  "/org/bluez/hci0/dev_DA_88_5F_79_04_F1",             /* Object path of the adapter */
            "org.freedesktop.DBus.Properties", /* Interface name */
            "Get",                         /* Method name */
            g_variant_new("(ss)", par1.c_str(),  par2.c_str()), //  "org.bluez.Device1", "Name"), /* Parameters */
            G_VARIANT_TYPE("(v)"),         /* Expected return type */
            G_DBUS_CALL_FLAGS_NONE,
            -1,                            /* Default timeout */
            NULL,                          /* GCancellable */
            &error);

    if (error) {
        g_printerr("Error calling Get method: %s\n", error->message);
        g_error_free(error);
        return 1;
    }

    g_variant_get(result, "(v)", &rv);
    g_variant_get(rv, "s", &rvs);

    g_print("Name: %s\n", rvs.str);
	value = rvs.str;

    g_variant_unref(rv);
    g_variant_unref(result);

	return 0;
}

int BtRaw::boolProperty(const std::string& path, const std::string& par1, const std::string& par2, bool& value)
{
    GError *error = nullptr;
    GVariant *result;
    GVariant *rv;
	gboolean rvb;

    result = g_dbus_connection_call_sync(m_connection,
            "org.bluez",                   /* Bus name of the BlueZ service */
            path.c_str(),					//  "/org/bluez/hci0/dev_DA_88_5F_79_04_F1",             /* Object path of the adapter */
            "org.freedesktop.DBus.Properties", /* Interface name */
            "Get",                         /* Method name */
            g_variant_new("(ss)", par1.c_str(),  par2.c_str()), //  "org.bluez.Device1", "Name"), /* Parameters */
            G_VARIANT_TYPE("(v)"),         /* Expected return type */
            G_DBUS_CALL_FLAGS_NONE,
            -1,                            /* Default timeout */
            NULL,                          /* GCancellable */
            &error);

    if (error) {
        g_printerr("Error calling Get method: %s\n", error->message);
        g_error_free(error);
        return 1;
    }

    g_variant_get(result, "(v)", &rv);
    g_variant_get(rv, "b", &rvb);

	value = rvb;

    g_variant_unref(rv);
    g_variant_unref(result);

	return 0;
}

bool BtRaw::assignStringValue(GString& string, GVariant *variant, const std::string& name, std::string& value)
{
	if (name == string.str) {
		GVariant *c7v;
		GString c7s;
		g_variant_get(variant, "v", &c7v);
		g_variant_get(c7v, "s", &c7s);
		value = c7s.str;
		g_variant_unref(c7v);
		free_gstring(c7s);
		return true;
	}

	return false;
};

bool BtRaw::assignBoolValue(GString& string, GVariant *variant, const std::string& name, bool& value)
{
	if (name == string.str) {
		GVariant *c7v;
		gboolean c7b;
		g_variant_get(variant, "v", &c7v);
		g_variant_get(c7v, "b", &c7b);
		value = c7b;
		g_variant_unref(c7v);
		return true;
	}

	return false;
};

bool BtRaw::assignUInt16Value(GString& string, GVariant *variant, const std::string& name, guint16& value)
{
	if (name == string.str) {
		GVariant *c7v;
		guint16 c7b;
		g_variant_get(variant, "v", &c7v);
		g_variant_get(c7v, "q", &c7b);
		value = c7b;
		g_variant_unref(c7v);
		return true;
	}

	return false;
};

bool BtRaw::assignObjectpathValue(GString& string, GVariant *variant, const std::string& name, std::string& value)
{
	if (name == string.str) {
		GVariant *c7v = nullptr;
		GString c7s;
		g_variant_get(variant, "v", &c7v);
		g_variant_get(c7v, "o", &c7s);
		value = c7s.str;
		g_variant_unref(c7v);
		c7v = nullptr;
		free_gstring(c7s);
		return true;
	}

	return false;
};

int BtRaw::write(const char *characteristic, const std::vector<guint8>& data)
{
	GVariant *value = g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE, &data[0], data.size(), sizeof(guint8));

#define WITH_RESPONSE 37

	int writeType = WITH_RESPONSE;
	GError *error = nullptr;

	guint16 offset = 0;
    const char *writeTypeString = (writeType == WITH_RESPONSE) ? "request" : "command";
	GVariantBuilder *optionsBuilder = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));
	g_variant_builder_add(optionsBuilder, "{sv}", "offset", g_variant_new_uint16(offset));
	g_variant_builder_add(optionsBuilder, "{sv}", "type", g_variant_new_string(writeTypeString));
	GVariant *options = g_variant_builder_end(optionsBuilder);

	auto&& status = g_dbus_connection_call_sync(m_connection,
			BLUEZ_DBUS,
			characteristic, // characteristic->path,
			"org.bluez.GattCharacteristic1", // INTERFACE_CHARACTERISTIC,
			"WriteValue", // CHARACTERISTIC_METHOD_WRITE_VALUE,
			g_variant_new("(@ay@a{sv})", value, options),
			NULL,
			G_DBUS_CALL_FLAGS_NONE,
			10000, // 10 second timeout
			NULL, // Cancellable
			&error  // error
			);

	//- g_free(value);
	g_free(optionsBuilder);

	if (error != nullptr) {
		/// g_printerr("Error connecting to system bus: %s\n", error->message);
		printf("Error connecting to system bus: %s\n", error->message);
		g_error_free(error);
		return 1;
	}

	g_free(status);

	return 0;
}

