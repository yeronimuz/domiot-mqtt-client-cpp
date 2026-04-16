#include "DHTTempHumidity.h"
#include <SensorValue.h>

float DHTTempHumiditySensor::_lastSentTemperature = 0.0f;
float DHTTempHumiditySensor::_lastSentHumidity = 0.0f;

void DHTTempHumiditySensor::setup() {
  dht11.begin(); // initialize the DHT sensor
}

float DHTTempHumiditySensor::readTemperature() {
    return dht11.readTemperature();
}

float DHTTempHumiditySensor::readHumidity() {
    return dht11.readHumidity();
}

void DHTTempHumiditySensor::publishSensorValues(
    PubSubClient &mqttClient,
    long tempSensorId,
    long humiditySensorId,
    const char *tempTopic,
    const char *humidityTopic,
    const String &timestamp,
    unsigned long now,
    unsigned long lastTempPublish,
    unsigned long lastHumidityPublish)
{
    if (tempSensorId > 0 && now - lastTempPublish >= 1000)
    {
        float temperature = readTemperature();
        if (temperature != _lastSentTemperature)
        {
            _lastSentTemperature = temperature;

            SensorValue temperatureValue;
            temperatureValue.setSensorId(tempSensorId);
            temperatureValue.setTimestamp(timestamp);
            temperatureValue.setValue(temperature);
            String tempPayload = temperatureValue.toJson();

            mqttClient.publish(tempTopic, reinterpret_cast<const uint8_t *>(tempPayload.c_str()), tempPayload.length());
        }
    }

    if (humiditySensorId > 0 && now - lastHumidityPublish >= 1000)
    {
        float humidity = readHumidity();
        if (humidity != _lastSentHumidity)
        {
            _lastSentHumidity = humidity;

            SensorValue humidityValue;
            humidityValue.setSensorId(humiditySensorId);
            humidityValue.setTimestamp(timestamp);
            humidityValue.setValue(humidity);
            String humidityPayload = humidityValue.toJson();

            mqttClient.publish(humidityTopic, reinterpret_cast<const uint8_t *>(humidityPayload.c_str()), humidityPayload.length());
        }
    }
}
