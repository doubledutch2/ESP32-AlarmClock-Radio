#include "Config.h"

// Define these if they aren't already in your other project files
#define AUDIO_SOURCE_INTERNET HIGH
#define AUDIO_SOURCE_RADIO    LOW
#define MODE_SWITCH_PIN  17  // 42 Switch between Internet/FM radio

void setup() {
  Serial.begin(115200);

  // Initialize the hardware switch pin
  pinMode(MODE_SWITCH_PIN, OUTPUT);

  // Default to Internet Radio on boot
  digitalWrite(MODE_SWITCH_PIN, AUDIO_SOURCE_INTERNET);
  
  Serial.println("Audio System Initialized: Defaulting to Internet Radio");
}

void loop() {
  // Example: Check if the 'Select' button from your Config.h is pressed
  // if (digitalRead(BTN_SELECT) == LOW) {
    toggleAudioSource();
    delay(2000); // Simple debounce
  // }
}

void toggleAudioSource() {
  static bool isRadio = false;
  isRadio = !isRadio;

  if (isRadio) {
    // Switch CD74HCT4053 to Radio inputs (NO pins)
    digitalWrite(MODE_SWITCH_PIN, AUDIO_SOURCE_RADIO);
    Serial.println("Source Switched: SI4735 FM Radio");
  } else {
    // Switch CD74HCT4053 to Internet Radio inputs (NC pins)
    digitalWrite(MODE_SWITCH_PIN, AUDIO_SOURCE_INTERNET);
    Serial.println("Source Switched: ESP32 Internet Radio");
  }
}