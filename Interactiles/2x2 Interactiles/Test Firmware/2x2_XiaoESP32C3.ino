#Working code for 2x2 PCB Design and Xiao Seeeduino ESP32C3


#include <Adafruit_NeoPixel.h>
#include <Keypad.h>

#define PIN 20       // Pin connected to the NeoPixel data line
#define NUMPIXELS 4  // Total number of pixels in the strip

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_RGB + NEO_KHZ800);

// Keypad setup
const byte ROWS = 2;  // 2 rows
const byte COLS = 2;  // 2 columns
char keys[ROWS][COLS] = {
  { '4', '2' },
  { '3', '1' },
};
byte rowPins[ROWS] = { 21, 5 };  // Connect to the row pinouts of the keypad
byte colPins[COLS] = { 9, 10 };  // Connect to the column pinouts of the keypad

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

const int numStates = 8;
enum LedState {
  OFF,
  RED,
  ORANGE,
  GREEN,
  TEAL,
  BLUE,
  PURPLE,
  WHITE
};

LedState currentStates[NUMPIXELS] = { OFF, OFF, OFF, OFF };  // Start with all LEDs off

void setup() {
  Serial.begin(115200);
  Serial.println("Setting up NeoPixel...");

  strip.begin();
  strip.show();             // Initialize all pixels to 'off'
  strip.setBrightness(80);  // Set brightness to a moderate level

  Serial.println("Setup complete.");
}

void loop() {
  char key = keypad.getKey();
  if (key) {
    Serial.print("Key pressed: ");
    Serial.println(key);

    // Mapping keys to the correct LED index
    int ledIndex = -1;
    switch (key) {
      case '1': ledIndex = 0; break;  // Key 1 -> LED 4 -> NeoPixel index 3
      case '2': ledIndex = 2; break;  // Key 2 -> LED 3 -> NeoPixel index 2
      case '3': ledIndex = 1; break;  // Key 3 -> LED 2 -> NeoPixel index 1
      case '4': ledIndex = 3; break;  // Key 4 -> LED 1 -> NeoPixel index 0
    }

    if (ledIndex != -1) {
      Serial.print("Controlling LED at index: ");
      Serial.println(ledIndex);

      currentStates[ledIndex] = static_cast<LedState>((currentStates[ledIndex] + 1) % numStates);
      updateLed(ledIndex);
    } else {
      Serial.println("Invalid key detected.");
    }

    // Simple debounce delay
    delay(50);
  }
}

void updateLed(int ledIndex) {
  Serial.print("Updating LED ");
  Serial.print(ledIndex);
  Serial.print(" to state ");
  Serial.println(currentStates[ledIndex]);

  switch (currentStates[ledIndex]) {
    case OFF:
      strip.setPixelColor(ledIndex, strip.Color(0, 0, 0));  // Turn off
      break;
    case RED:
      strip.setPixelColor(ledIndex, strip.Color(255, 0, 0));  // Red
      break;
    case ORANGE:
      strip.setPixelColor(ledIndex, strip.Color(255, 165, 0));  // Orange
      break;
    case GREEN:
      strip.setPixelColor(ledIndex, strip.Color(0, 255, 0));  // Green
      break;
    case TEAL:
      strip.setPixelColor(ledIndex, strip.Color(0, 128, 128));  // Teal
      break;
    case BLUE:
      strip.setPixelColor(ledIndex, strip.Color(0, 0, 255));  // Blue
      break;
    case PURPLE:
      strip.setPixelColor(ledIndex, strip.Color(128, 0, 128));  // Purple
      break;
    case WHITE:
      strip.setPixelColor(ledIndex, strip.Color(255, 255, 255));  // White
      break;
  }

  strip.show();  // Update the strip after setting the color
}
