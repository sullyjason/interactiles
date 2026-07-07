#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>
#include <Keypad.h>
#include <string.h>  // ADD THIS CRITICAL MISSING LIBRARY

// Wi-Fi credentials
const char* ssid = "SSID";
const char* password = "PASSWORD";

#define PIN 20
#define NUMPIXELS 4

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_RGB + NEO_KHZ800);

// MQTT setup
const char* mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;
const char* base_topic = "interactiles/space1";

WiFiClient espClient;
PubSubClient client(espClient);

// Keypad setup
const byte ROWS = 2;
const byte COLS = 2;
char keys[ROWS][COLS] = {{'4','2'}, {'3','1'}};
byte rowPins[ROWS] = {21, 5};
byte colPins[COLS] = {9, 10};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

const int numStates = 8;
enum LedState { OFF, RED, ORANGE, GREEN, TEAL, BLUE, PURPLE, WHITE }; // FIXED TYPO: WHITE
LedState currentStates[NUMPIXELS] = {OFF, OFF, OFF, OFF};

void setup_wifi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  char msg[length + 1];
  memcpy(msg, payload, length);
  msg[length] = '\0';
  
  Serial.print("Received [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(msg);

  char topicCopy[50];
  strncpy(topicCopy, topic, sizeof(topicCopy));
  topicCopy[sizeof(topicCopy) - 1] = '\0';

  int row, col, r, g, b;
  if (sscanf(topicCopy, "interactiles/space1/%d/%d", &row, &col) == 2) {
    char* comma = strchr(msg, ',');
    if (comma) {
      *comma = ' ';
      char* comma2 = strchr(comma + 1, ',');
      if (comma2) *comma2 = ' ';
      
      if (sscanf(msg, "%d %d %d", &r, &g, &b) == 3) {
        int ledIndex = mapToLedIndex(row, col);
        if (ledIndex >= 0 && ledIndex < NUMPIXELS) {
          strip.setPixelColor(ledIndex, strip.Color(
            constrain(r, 0, 255),
            constrain(g, 0, 255),
            constrain(b, 0, 255)
          ));
          strip.show();
        }
      }
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    String clientId = "ESP32Client-2" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("interactiles/space1/#");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5s");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  strip.begin();
  strip.show();
  strip.setBrightness(80);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  randomSeed(micros());
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  char key = keypad.getKey();
  if (key) {
    Serial.print("Key pressed: ");
    Serial.println(key);

    int ledIndex = -1;
    int row = -1, col = -1;

    switch (key) {
      case '1': ledIndex = 0; row = 1; col = 1; break;
      case '2': ledIndex = 2; row = 1; col = 2; break;
      case '3': ledIndex = 1; row = 2; col = 1; break;
      case '4': ledIndex = 3; row = 2; col = 2; break;
    }

    if (ledIndex != -1) {
      currentStates[ledIndex] = static_cast<LedState>((currentStates[ledIndex] + 1) % numStates);
      updateLed(ledIndex);

      uint32_t color = strip.getPixelColor(ledIndex);
      uint8_t r = (color >> 16) & 0xFF;
      uint8_t g = (color >> 8) & 0xFF;
      uint8_t b = color & 0xFF;

      char full_topic[50];
      snprintf(full_topic, sizeof(full_topic), "%s/%d/%d", base_topic, row, col);
      
      char payload[20];
      snprintf(payload, sizeof(payload), "%d,%d,%d", r, g, b);
      
      if (client.publish(full_topic, payload)) {
        Serial.print("Published to ");
        Serial.print(full_topic);
        Serial.print(": ");
        Serial.println(payload);
      } else {
        Serial.println("Publish failed!");
      }
    }
    delay(50);
  }
}

void updateLed(int ledIndex) {
  switch (currentStates[ledIndex]) {
    case OFF:    strip.setPixelColor(ledIndex, 0); break;
    case RED:    strip.setPixelColor(ledIndex, strip.Color(255, 0, 0)); break;
    case ORANGE: strip.setPixelColor(ledIndex, strip.Color(255, 100, 0)); break;
    case GREEN:  strip.setPixelColor(ledIndex, strip.Color(0, 255, 0)); break;
    case TEAL:   strip.setPixelColor(ledIndex, strip.Color(0, 128, 128)); break;
    case BLUE:   strip.setPixelColor(ledIndex, strip.Color(0, 0, 255)); break;
    case PURPLE: strip.setPixelColor(ledIndex, strip.Color(128, 0, 128)); break;
    case WHITE:  strip.setPixelColor(ledIndex, strip.Color(255, 255, 255)); break;
  }
  strip.show();
}

int mapToLedIndex(int row, int col) {
  if (row == 1 && col == 1) return 0;
  if (row == 1 && col == 2) return 2;
  if (row == 2 && col == 1) return 1;
  if (row == 2 && col == 2) return 3;
  return -1;
}