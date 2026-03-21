# Domiot MQTT Client

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
- If the `ota` section is omitted, the device falls back to `mqtt.user` and `mqtt.password`.
- If neither pair is configured, the `/update` endpoint stays unauthenticated.

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
pio run -e nodemcuv2-ota -t upload --upload-port 192.168.1.50
```

The uploader reads credentials from `data/config.json` using the same fallback order as the device. If you prefer a fixed URL in your local configuration, add this to `platformio.ini` locally:

```ini
[env:nodemcuv2-ota]
custom_upload_url = http://192.168.1.50
```

If you need to update the filesystem image manually through the web UI, use `.pio/build/nodemcuv2/littlefs.bin`.
