#include <DHT.h>
#include <PubSubClient.h>
#include "constants.h"

class DHTTempHumiditySensor
{
public:
    DHTTempHumiditySensor(uint8_t pin) : dht11(pin, DHT11) {};
    void setup();
    float readTemperature();
    float readHumidity();
    void publishSensorValues(PubSubClient &mqttClient, long tempSensorId, long humiditySensorId, const char *tempTopic, const char *humidityTopic, const String &timestamp, unsigned long now, unsigned long lastTempPublish, unsigned long lastHumidityPublish);

private:
    static float _lastSentTemperature;
    static float _lastSentHumidity;
    DHT dht11;
};
