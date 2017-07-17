#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define wifi_ssid "SSID"
#define wifi_password "SSID_KEY"

#define mqtt_server "mqtt.local.domain"
#define mqtt_user "mqtt"
#define mqtt_password "mqtt_passwd"

#define humidity_topic "sensor/humidity"
#define temperature_topic "sensor/temperature"

#define DHTTYPE DHT11
#define DHTPIN  2

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE, 11); // 11 works fine for ESP8266

void setup() {
  Serial.begin(115200);
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // if (client.connect("ESP8266Client")) {
    if (client.connect("ESP-01", mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

bool checkBound(float newValue, float prevValue, float maxDiff) {
  return !isnan(newValue) &&
         (newValue < prevValue - maxDiff || newValue > prevValue + maxDiff);
}

long lastMsg = 0;
float temp = 0.0;
float hum = 0.0;
float diff = 1.0;

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;

    float newTemp = dht.readTemperature();
    float newHum = dht.readHumidity();

    if (checkBound(newTemp, temp, diff)) {
      temp = newTemp * 9/5 + 32;
      Serial.print("Temp:");
      Serial.println(String(temp).c_str());
      client.publish(temperature_topic, String(temp).c_str(), true);
    }

    if (checkBound(newHum, hum, diff)) {
      hum = newHum;
      Serial.print("Humidity:");
      Serial.println(String(hum).c_str());
      client.publish(humidity_topic, String(hum).c_str(), true);
    }
  }
}
