#include "AudioSwitch.h"

AudioSwitch::AudioSwitch() : currentSource(SOURCE_INTERNET_RADIO) {
}

void AudioSwitch::begin() {
    pinMode(MODE_SWITCH_PIN, OUTPUT);
    
    // Default to Internet Radio (LOW = Internet, HIGH = FM)
    digitalWrite(MODE_SWITCH_PIN, LOW);
    currentSource = SOURCE_INTERNET_RADIO;
    
    Serial.println("AudioSwitch: Initialized (default: Internet Radio)");
    Serial.println("  CD74HCT4053 Multiplexer Configuration:");
    Serial.printf("  MODE_SWITCH_PIN (GPIO %d) = LOW (Internet Radio)\n", MODE_SWITCH_PIN);
    Serial.println("  LOW  = Yn input (ESP32 I2S) -> Common output");
    Serial.println("  HIGH = Zn input (Si4735 I2S) -> Common output");
}

void AudioSwitch::setSource(AudioSource source) {
    currentSource = source;
    
    if (source == SOURCE_FM_RADIO) {
        // HIGH = CD74HCT4053 connects Si4735 (Zn) to MAX98357A (Common)
        digitalWrite(MODE_SWITCH_PIN, HIGH);
        Serial.println("AudioSwitch: Switched to FM RADIO");
        Serial.printf("  MODE_SWITCH_PIN (GPIO %d) = HIGH\n", MODE_SWITCH_PIN);
        Serial.println("  CD74HCT4053: Zn (Si4735) -> Common (MAX98357A)");
    } else {
        // LOW = CD74HCT4053 connects ESP32 I2S (Yn) to MAX98357A (Common)
        digitalWrite(MODE_SWITCH_PIN, LOW);
        Serial.println("AudioSwitch: Switched to INTERNET RADIO");
        Serial.printf("  MODE_SWITCH_PIN (GPIO %d) = LOW\n", MODE_SWITCH_PIN);
        Serial.println("  CD74HCT4053: Yn (ESP32) -> Common (MAX98357A)");
    }
}

AudioSource AudioSwitch::getCurrentSource() {
    return currentSource;
}

void AudioSwitch::toggleSource() {
    if (currentSource == SOURCE_FM_RADIO) {
        setSource(SOURCE_INTERNET_RADIO);
    } else {
        setSource(SOURCE_FM_RADIO);
    }
}

bool AudioSwitch::isFMRadioActive() {
    return currentSource == SOURCE_FM_RADIO;
}

bool AudioSwitch::isInternetRadioActive() {
    return currentSource == SOURCE_INTERNET_RADIO;
}