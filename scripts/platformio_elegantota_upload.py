# Generated file by platformio, don't edit it manually.
import hashlib
import json
import time
from pathlib import Path
from urllib.parse import urlparse

Import("env")

try:
    import requests
    from requests.auth import HTTPDigestAuth
    from requests_toolbelt import MultipartEncoder, MultipartEncoderMonitor
    from tqdm import tqdm
except ImportError:
    env.Execute("$PYTHONEXE -m pip install requests requests_toolbelt tqdm")
    import requests
    from requests.auth import HTTPDigestAuth
    from requests_toolbelt import MultipartEncoder, MultipartEncoderMonitor
    from tqdm import tqdm


def sanitize_optional_string(value):
    if value is None:
        return ""

    text = str(value).strip()
    if text.lower() in ("null", "undefined"):
        return ""

    return text


def read_local_credentials(project_dir):
    config_path = Path(project_dir) / "data" / "config.json"
    if not config_path.exists():
        return "", ""

    try:
        config = json.loads(config_path.read_text(encoding="utf-8"))
    except Exception as exc:
        print(f"Warning: unable to read {config_path}: {exc}")
        return "", ""

    ota_config = config.get("ota") or {}
    mqtt_config = config.get("mqtt") or {}

    ota_username = sanitize_optional_string(ota_config.get("username"))
    ota_password = sanitize_optional_string(ota_config.get("password"))
    if ota_username and ota_password:
        return ota_username, ota_password

    mqtt_username = sanitize_optional_string(mqtt_config.get("user"))
    mqtt_password = sanitize_optional_string(mqtt_config.get("password"))
    if mqtt_username and mqtt_password:
        return mqtt_username, mqtt_password

    return "", ""


def get_project_option(name):
    try:
        return sanitize_optional_string(env.GetProjectOption(name))
    except Exception:
        return ""


def resolve_upload_base_url():
    custom_upload_url = get_project_option("custom_upload_url")
    if custom_upload_url:
        upload_base_url = custom_upload_url
    else:
        upload_port = sanitize_optional_string(env.get("UPLOAD_PORT"))
        if not upload_port:
            raise RuntimeError(
                "No OTA upload address configured. Use --upload-port <device-ip> or set custom_upload_url = http://<device-ip>."
            )

        upload_base_url = upload_port
        if "://" not in upload_base_url:
            upload_base_url = f"http://{upload_base_url}"

    upload_base_url = upload_base_url.rstrip("/")
    if upload_base_url.endswith("/update"):
        upload_base_url = upload_base_url[:-7]

    parsed = urlparse(upload_base_url)
    if not parsed.scheme or not parsed.netloc:
        raise RuntimeError(f"Invalid OTA upload URL: {upload_base_url}")

    return upload_base_url


def resolve_auth():
    username = get_project_option("custom_username")
    password = get_project_option("custom_password")
    if username and password:
        return username, password

    return read_local_credentials(env.subst("$PROJECT_DIR"))


def on_upload(source, target, env):
    firmware_path = Path(str(source[0]))
    upload_base_url = resolve_upload_base_url()
    upload_page_url = f"{upload_base_url}/update"
    parsed_url = urlparse(upload_base_url)
    host = parsed_url.netloc

    username, password = resolve_auth()
    auth = None

    try:
        auth_probe = requests.get(upload_page_url, timeout=10, allow_redirects=False)
    except Exception as exc:
        raise RuntimeError(f"Error checking OTA endpoint {upload_page_url}: {exc}") from exc

    if auth_probe.status_code == 401:
        if not username or not password:
            raise RuntimeError(
                "The OTA endpoint requires authentication. Set ota.username/password in data/config.json, "
                "or provide custom_username/custom_password in platformio.ini."
            )
        auth = HTTPDigestAuth(username, password)
        print("ElegantOTA authentication detected.")
    else:
        print("ElegantOTA authentication not required.")

    with firmware_path.open("rb") as firmware:
        md5 = hashlib.md5(firmware.read()).hexdigest()
        firmware.seek(0)

        is_filesystem_image = firmware_path.name in ("spiffs.bin", "littlefs.bin")
        upload_mode = "fs" if is_filesystem_image else "firmware"
        start_url = f"{upload_base_url}/ota/start?mode={upload_mode}&hash={md5}"

        request_headers = {
            "Host": host,
            "Accept": "*/*",
            "Referer": upload_page_url,
            "Connection": "keep-alive",
        }

        # Free MQTT heap on device before Update.begin() runs inside /ota/start.
        prepare_url = f"{upload_base_url}/ota/prepare"
        try:
            requests.get(prepare_url, headers=request_headers, auth=auth, timeout=10)
            time.sleep(0.5)  # give the device a moment to free memory
        except Exception:
            pass  # best-effort; older firmware without this endpoint continues

        try:
            start_response = requests.get(start_url, headers=request_headers, auth=auth, timeout=30)
        except Exception as exc:
            raise RuntimeError(f"Error starting OTA upload: {exc}") from exc

        if start_response.status_code != 200:
            raise RuntimeError(
                f"OTA start failed with status {start_response.status_code}: {start_response.text}"
            )

        encoder = MultipartEncoder(
            fields={
                "MD5": md5,
                "firmware": (firmware_path.name, firmware, "application/octet-stream"),
            }
        )

        progress = tqdm(
            desc="Upload Progress",
            total=encoder.len,
            dynamic_ncols=True,
            unit="B",
            unit_scale=True,
            unit_divisor=1024,
        )

        monitor = MultipartEncoderMonitor(
            encoder,
            lambda monitor: progress.update(monitor.bytes_read - progress.n),
        )

        post_headers = {
            "Host": host,
            "Accept": "*/*",
            "Referer": upload_page_url,
            "Connection": "keep-alive",
            "Content-Type": monitor.content_type,
            "Content-Length": str(monitor.len),
            "Origin": upload_base_url,
        }

        try:
            upload_response = requests.post(
                f"{upload_base_url}/ota/upload",
                data=monitor,
                headers=post_headers,
                auth=auth,
                timeout=120,
            )
        except Exception as exc:
            raise RuntimeError(f"Error uploading OTA image: {exc}") from exc
        finally:
            progress.close()

        time.sleep(0.1)

        if upload_response.status_code != 200:
            raise RuntimeError(
                f"Upload failed with status {upload_response.status_code}: {upload_response.text}"
            )

        print(f"Upload successful. Server response: {upload_response.text}")


env.Replace(UPLOADCMD=on_upload)