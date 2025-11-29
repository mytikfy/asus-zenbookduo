
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#include <libusb-1.0/libusb.h>

#include <string>

#include "azbduobt.h"
#include "version.h"

using namespace std;

// taken from :
//# BSD 2-Clause License
//#
//# Copyright (c) 2024, Alesya Huzik
//#
//import sys
//import usb.core
//import usb.util
//
//# USB Parameters
//VENDOR_ID = 0x${VENDOR_ID}
//PRODUCT_ID = 0x${PRODUCT_ID}
//REPORT_ID = 0x5A
//WVALUE = 0x035A
//WINDEX = 4
//WLENGTH = 16
//
//if len(sys.argv) != 2:
//    print(f\"Usage: {sys.argv[0]} <level>\")
//    sys.exit(1)
//
//try:
//    level = int(sys.argv[1])
//    if level < 0 or level > 3:
//        raise ValueError
//except ValueError:
//    print(\"Invalid level. Must be an integer between 0 and 3.\")
//    sys.exit(1)
//
//# Prepare the data packet
//data = [0] * WLENGTH
//data[0] = REPORT_ID
//data[1] = 0xBA
//data[2] = 0xC5
//data[3] = 0xC4
//data[4] = level
//
//# Find the device
//dev = usb.core.find(idVendor=VENDOR_ID, idProduct=PRODUCT_ID)
//
//if dev is None:
//    print(f\"Device not found (Vendor ID: 0x{VENDOR_ID:04X}, Product ID: 0x{PRODUCT_ID:04X})\")
//    sys.exit(1)
//
//# Detach kernel driver if necessary
//if dev.is_kernel_driver_active(WINDEX):
//    try:
//        dev.detach_kernel_driver(WINDEX)
//    except usb.core.USBError as e:
//        print(f\"Could not detach kernel driver: {str(e)}\")
//        sys.exit(1)
//
//# try:
//#     dev.set_configuration()
//#     usb.util.claim_interface(dev, WINDEX)
//# except usb.core.USBError as e:
//#     print(f\"Could not set configuration or claim interface: {str(e)}\")
//#     sys.exit(1)
//
//# Send the control transfer
//try:
//    bmRequestType = 0x21  # Host to Device | Class | Interface
//    bRequest = 0x09       # SET_REPORT
//    wValue = WVALUE       # 0x035A
//    wIndex = WINDEX       # Interface number
//    ret = dev.ctrl_transfer(bmRequestType, bRequest, wValue, wIndex, data, timeout=1000)
//    if ret != WLENGTH:
//        print(f\"Warning: Only {ret} bytes sent out of {WLENGTH}.\")
//    else:
//        print(\"Data packet sent successfully.\")
//except usb.core.USBError as e:
//    print(f\"Control transfer failed: {str(e)}\")
//    usb.util.release_interface(dev, WINDEX)
//    sys.exit(1)
//
//# Release the interface
//usb.util.release_interface(dev, WINDEX)
//# Reattach the kernel driver if necessary
//try:
//    dev.attach_kernel_driver(WINDEX)
//except usb.core.USBError:
//    pass  # Ignore if we can't reattach the driver
//
//sys.exit(0)


#define VID  0x0b05
#define PID  0x1bf2

typedef struct {
	int level = 3;
	bool hotplugActive = false;
	bool pluggedInState = false;
	bool dpmsSentinel = true;
	bool btkbdLights = false;
	bool btkbdLost = false;
	bool sigusr2 = false;
} CallbackData;

static CallbackData cbData = {};

static int LIBUSB_CALL cb(libusb_context *ctx, libusb_device *device, libusb_hotplug_event event, void *user_data)
{
	(void) ctx;
	(void) device;
	(void) event;

	CallbackData *_cbData = (CallbackData *)user_data;
	_cbData->hotplugActive = true;
	_cbData->pluggedInState = event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED ? true : false;

	return 0;
}

static void updateUsbKbdBacklight(libusb_context *ctx, int level)
{
	libusb_device_handle *hdl = libusb_open_device_with_vid_pid(ctx, VID, PID);

	if (hdl != nullptr) {
		if (libusb_kernel_driver_active(hdl, 4)) {
			printf("kernel driver active\n");
			libusb_detach_kernel_driver(hdl, 4);
		}

		// general :: https://docs.kernel.org/hid/hidintro.html

		int retv = libusb_claim_interface(hdl, 4);

		if (retv < 0) {
			printf("libusb_claim_interface failed: %d\n", retv);
		}

		//  https://www.engineersgarage.com/usb-requests-and-stages-of-control-transfer-part-4-6/
		// Host to Device, class, interface
		uint8_t       bmReqType = 0x21;			// the request type (direction of transfer)
		// https://www.beyondlogic.org/usbnutshell/usb6.shtml
		// SET_CONFIGURATION
		uint8_t            bReq = 0x09;			// the request field for this packet
		uint16_t           wVal = 0x035a;		// the value field for this packet
		uint16_t         wIndex = 4;			// the index field for this packet
		uint8_t        data[16] = { 0x5a, 0xBA, 0xC5, 0xC4, (uint8_t) level};
		uint16_t           wLen = sizeof(data);	// length of this setup packet
		unsigned int         to = 1000;			// timeout duration (if transfer fails)

		retv = libusb_control_transfer(hdl, bmReqType, bReq, wVal, wIndex, data, wLen, to);

		if (retv < 0) {
			printf("libusb_control_transfer failed: %d\n", retv);
		}

		libusb_close(hdl);
	}
}

static int checkDPMS(const char *card)
{
	int retv = 0;
	std::string path = "/sys/devices/pci0000:00/0000:00:02.0/drm/";
	path += card;
	path += "/dpms";

	FILE *fhd = ::fopen(path.c_str(), "r");

	if (fhd != NULL) {
		char buf[32] = {};

		int readed = ::fread(buf, sizeof(char), (sizeof(buf) / sizeof(char)) -1, fhd);

		if (readed > 0) {
			{
				static int lastReaded = -1;

				if (lastReaded != readed) {
					printf("dpms = %d %s", readed, buf);
					lastReaded = readed;
				}
			}

			if (!strncmp(buf, "On", 2)) {
				retv = 1;
			}
			else if (!strncmp(buf, "Off", 3)) {
				retv = 3;
			}
			else {
				printf("unknown\n");
			}
		}

		::fclose(fhd);
	}

	return retv;
}

static int checkPogo(const char *card)
{
	(void) card;

	static bool first = true;

	if (first) {
		const char *path = "/sys/kernel/debug/asus-nb-wmi/dev_id";
		FILE *fhd = ::fopen(path, "w");

		if (!fhd) {
			perror("cannot open pogo");
			return -1;
		}


		::fwrite("0x00050051", sizeof(char), 10, fhd);

		::fclose(fhd);

		first = false;
	}

	if (!first) {
		const char *path = "/sys/kernel/debug/asus-nb-wmi/dsts";

		FILE *fhd = ::fopen(path, "r");

		if (!fhd) {
			perror("cannot open");
			return -1;
		}

		char buffer[100] = {};

		int readed = ::fread(buffer, sizeof(char), sizeof(buffer) - 1, fhd);

		::fclose(fhd);

		if (readed <= 0) {
			return -1;
		}

		printf("pogo %d, %s", readed, buffer);

		static const char *sep = "= ";
		char *tok = strtok(buffer, sep);

		while (tok) {
//			printf("-- %s\n", tok);

			if (strncmp(tok, "DSTS", 4)) {
				int value = ::stol(tok, nullptr, 16);
				return value & 1;
			}

			tok = strtok(nullptr, sep);
		}
	}

	return -1;
}

static int worker(long timeout, long timetics)
{
	printf("running worker\n");

	if (!libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG)) {
		printf("no hotplug support\n");
	}

	libusb_context *ctx = nullptr;

	struct libusb_init_option initOptions[1] = { /* { LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_INFO } */ };

	int retv = libusb_init_context(&ctx, initOptions, 0);

	if (retv) {
		printf("libusb_init_context failed : %d\n", retv);
		exit(1);
	}

	libusb_hotplug_callback_handle hdl;

	retv = libusb_hotplug_register_callback(ctx, LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED  | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT, LIBUSB_HOTPLUG_ENUMERATE, VID, PID,
			LIBUSB_HOTPLUG_MATCH_ANY, cb, (void *)&cbData, &hdl);

	if (retv) {
		printf("libusb_hotplug_register_call failed : %d\n", retv);
		exit(1);
	}

	struct timeval tv = {};
	tv.tv_sec = 0;
	tv.tv_usec = timeout * 1000;

	if (tv.tv_usec >= 1000 * 1000) {
		tv.tv_sec = tv.tv_usec / 1000 / 1000;
		tv.tv_usec -= tv.tv_sec * 1000 * 1000;
	}

	int completed = 0;

	AzbDuoBt bt;

	if (!bt.open()) {
	}

	time_t startTime = time(0);

	for (;;)  {
		retv = libusb_handle_events_timeout_completed(ctx, &tv, &completed);

		// https://github.com/libusb/libusb/issues/408
		if (cbData.hotplugActive) {
			if (cbData.pluggedInState) {
				updateUsbKbdBacklight(ctx, cbData.level);

				{
					static const char *path = "/sys/devices/pci0000:00/0000:00:02.0/drm/card1/card1-eDP-2/status";
					// check if usb cable or pogo pin and switch off second screen
					int pogo = checkPogo("");

					FILE *fhd = ::fopen(path, "w");

					if (fhd) {
						if (pogo == 0) {
							::fwrite("off", 1, 3, fhd);
						}
						else if (pogo == 1) {
							::fwrite("on", 1, 2, fhd);
						}

						::fclose(fhd);
					}
				}
			}

			cbData.hotplugActive = false;
		}

		if (!cbData.pluggedInState) {
			// set by bluetooth
			if (!cbData.btkbdLights) {
				if (!bt.setKbdBacklight(cbData.level)) {
					printf("bt on\n");
					cbData.btkbdLights = true;
				}
			}
			else {
				if (!bt.isConnected()) {
					printf("bt off\n");
					cbData.btkbdLights = false;
				}
			}
		}
		else {
			cbData.btkbdLights = false;
		}

		// check if display is connected
		if (cbData.dpmsSentinel) {
			static int lastDpms = -1;
			int dpms = checkDPMS("card1/card1-eDP-1");

			if (lastDpms != dpms) {
				updateUsbKbdBacklight(ctx, (dpms == 3) ? 0 : cbData.level);
				lastDpms = dpms;
			}
		}

		if (timetics != 0) {
			printf("timetics : %d\n", (int)(time(0) - startTime));
			if ((time(0) - startTime) > timetics) {
				break;
			}
		}
	}

	bt.close();

	libusb_exit(ctx);
	ctx = nullptr;

	return 0;
}

static void sighdl(int sig)
{
	if (sig == SIGUSR1) {
		cbData.level++;

		if (cbData.level > 3) {
			cbData.level = 0;
		}

		cbData.hotplugActive = true;
	}
	else if (sig == SIGUSR2) {
		cbData.sigusr2 = true;
	}
}

int main(int argc, char** argv)
{
	bool daemon = false;
	long timeout = 1000;
	long timetics = 0;

	for (int i = 1; i < argc; i++) {
		if (!strncmp("--level", argv[i], 7)) {
			i++;

			cbData.level = atoi(argv[i]);
		}
		else if (!strncmp("--nodpms", argv[i], 8)) {
			cbData.dpmsSentinel = false;
		}
		else if (!strncmp("--daemon", argv[i], 8)) {
			daemon = true;
		}
		else if (!strncmp("--timetics", argv[i], 10)) {
			i++;

			if ((i >= argc) || (argv[i][0] == '-')) {
				fprintf(stderr, "missing parameter for timetics\n");
				exit(1);
			}

			timetics = ::atol(argv[i]);
		}
		else if (!strncmp("--timeout", argv[i], 9)) {
			i++;

			if ((i >= argc) || (argv[i][0] == '-')) {
				fprintf(stderr, "missing parameter for timeout\n");
				exit(1);
			}

			timeout = ::atol(argv[i]);
		}
		else if ((!strncmp("--version", argv[i], 9)) || (!strncmp("-v", argv[i], 2))) {
			printf("version : %d.%d.%d-%d\n", version, revision, subrevision, buildno);
			return 0;
		}
	}

	signal(SIGUSR1, &sighdl);
	signal(SIGUSR2, &sighdl);
	signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

	if (daemon) {
		pid_t pid = fork();

		if (pid < 0) {
			exit(EXIT_FAILURE);
		}
		else if (pid > 0) {
			exit(EXIT_SUCCESS);
		}

		umask(0);

	    for (int x = sysconf(_SC_OPEN_MAX); x >= 0; x--) {
			close (x);
		}
	}

	return worker(timeout, timetics);
}
