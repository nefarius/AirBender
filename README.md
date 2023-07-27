<img src="AirBender/AirBender.png" height="128px" align="right" />

# AirBender
Windows Bluetooth Host Driver for Sony DualShock Controllers

---

This was a research project and testbed for getting familiar with the [UMDF](https://docs.microsoft.com/en-us/windows-hardware/drivers/wdf/getting-started-with-umdf-version-2) and an attempt of porting over the Bluetooth host stack from the C#-implementation used in [ScpToolkit](https://github.com/nefarius/ScpToolkit) to a native C Windows user-mode driver. It has been discontinued [in favour of better solutions](https://forums.vigem.org/topic/242/bluetooth-filter-driver-for-ds3-compatibility-research-notes). The code will stay up for anyone to use as either an inspiration or a negative example 😜

---

## Summary
`AirBender` consists of a custom Windows Bluetooth Stack containing a user-mode driver and a user-mode dispatch service handling wireless communication with Sony DualShock 3 and 4 controllers. It allows 3rd party developers to handle controller inputs and ouputs via a simple plug-in system.

## Architecture
Since the Sony DualShock 3 utilizes a butchered non-standard Bluetooth protocol incompatible with standard HID profiles a custom Bluetooth stack is required to establish a connection on the Windows platform. The [AirBender user-mode driver](../../tree/master/AirBender) implements a compatible Bluetooth stack and also acts as a bus emulator allowing for multiple devices to connect and transmit. It's designed to work with most USB Bluetooth host devices obeying at least [Core Version 2.1 + EDR](https://www.bluetooth.com/specifications/bluetooth-core-specification/legacy-specifications) standards.

The actual input and output reports flow between the driver and a [user-mode service](https://github.com/nefarius/Shibari) running in the background. This service handles detection of "AirBender Dongle Devices", reacts to child device arrival/removal and forwards input/output data to or from a plugin sub-system where arbitrary code can process it. In the default implementation, input data is exposed to the system via [ViGEm](https://github.com/nefarius/ViGEmBus).

## Supported systems
The driver is built for and tested with Windows 8.1 up to Windows 10 (x86 and amd64).

## Sources
 * https://cloud.nefarius.at/s/BKMWItpNLHKC5zA
 * https://nadavrub.wordpress.com/2015/07/17/simulate-hid-device-with-windows-desktop/
 * https://github.com/felis/USB_Host_Shield_2.0/wiki/PS3-Information#Bluetooth
 * https://msdn.microsoft.com/en-us/library/windows/hardware/ff536572%28v=vs.85%29.aspx?f=255
 * http://www.psdevwiki.com/ps3/DualShock_3
 * http://eleccelerator.com/wiki/index.php?title=DualShock_3
