# Domiot MQTT Client

This project is the P1 smart meter reader implemented in C++. It conforms to the following requirements.
* The software runs on an ESP8266 device.
* The software shall use MQTT to publish to 'register' and 'sensor' topics.
* It shall support one device with multiple sensors.
* Upon startup it shall connect to wifi using the configured parameters
* Upon startup it shall connect to the configured MQTT broker.
* It shall read a configuration (device.json and general configuration config.json) from Little FileSystem.
* When one or more sensorIds are configured to be 0, a registration process shall start.
	* The registration of the device to the Domiot backend shall be done by publishing the device configuration on the 'register' mqtt topic
	* The device shall wait for a Device configuration including sensorIds by subscribing to the 'config' topic
	* It shall update it's device configuration with information received from the received config message.
	* The registration shall be done once during startup.
* It shall read P1 datagrams from a smart meter on the Rx port.
* It shall publish values for all sensors configured in the device.json.
* It shall support either reading a battery level or a temperature on A0 analog input
* It shall publish sensor values of all configured sensors at a one second rate

## Future requirements
* Update configuration at runtime
* Publish diagnostics
* CRC validation P1 datagram

# Hardware requirements
* The data line (5) of the P1 meter must be attached to the input (1) of a 74HC04 inverter.
* The Rx port of the ESP8266 is attached to the output (2) of the same 74HC04 inverter port.
* The data line needs a pull up resistor of 10k connected to the 5V at the data line (5).
* The data ready line (3) of the P1 meter must be connected to 5V
* The data ground (3) of the P1 meter must be connected to GND of the ESP8266.

## Build

Build the firmware image with:

```bash
pio run -e nodemcuv2
```

This produces the firmware image at `.pio/build/nodemcuv2/firmware.bin`.

## OTA Updates

The firmware exposes the ElegantOTA web updater on `/update` after the device has joined WiFi. The OTA-capable firmware must be flashed over USB once before OTA uploads can work.

OTA authentication is enabled automatically when credentials are available:

- `ota.username` and `ota.password` from `data/config.json` are used when both are set.
- If this pair is not configured, the `/update` endpoint stays unauthenticated.

Example configuration:

```json
{
	"wifi": {
		"ssid": "MYSSID",
		"password": "thekey",
		"hostname": "domiot-p1-wifi"
	},
	"mqtt": {
		"host": "192.168.1.10",
		"port": 1883,
		"user": "johndoe",
		"password": "secret",
		"client_id": "domiot-p1"
	},
	"ota": {
		"username": "johndoe",
		"password": "secret"
	}
}
```

### Browser Upload

1. Build the firmware.
2. Find the device IP address from the serial log.
3. Open `http://<device-ip>/update` in a browser.
4. Upload `.pio/build/nodemcuv2/firmware.bin`.

### PlatformIO OTA Upload

The project includes a dedicated OTA environment that uploads to ElegantOTA over HTTP without replacing the normal serial upload flow.

Use:

```bash
pio run -e nodemcuv2-ota -t upload --upload-port <device-ip>
```

Example:

```bash
pio run -e nodemcuv2-ota -t upload --upload-port 192.168.1.50 \
  --project-option "custom_username=myuser" \
  --project-option "custom_password=mypass"
```

The uploader reads credentials from `data/config.json` using the same fallback order as the device. If you prefer a fixed URL in your local configuration, add this to `platformio.ini` locally:

```ini
[env:nodemcuv2-ota]
custom_upload_url = http://192.168.1.50
```

If you need to update the filesystem image manually through the web UI, use `.pio/build/nodemcuv2/littlefs.bin`.
