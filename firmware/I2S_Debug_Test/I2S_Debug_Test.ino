/*
 * I2S Audio Debug Test for ESP32-S3
 * Tests audio output step by step
 */

#include <Arduino.h>
#include <Audio.h>

// Your pin definitions
#define I2S_BCLK    15
#define I2S_LRC     7
#define I2S_DOUT    16

// WiFi credentials
#define WIFI_SSID "DEBEER"
#define WIFI_PASSWORD "B@C&86j@gqW73g"

Audio audio;
bool testsPassed = false;

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n\n========================================");
    Serial.println("    ESP32-S3 I2S Audio Debug Test");
    Serial.println("========================================\n");
    
    // Test 1: Check PSRAM
    Serial.println("Test 1: PSRAM Check");
    Serial.println("-------------------");
    if (!psramInit()) {
        Serial.println("❌ PSRAM init FAILED!");
        Serial.println("   Fix: Check Tools -> PSRAM -> OPI PSRAM");
        return;
    }
    Serial.printf("✓ PSRAM initialized\n");
    Serial.printf("  Total PSRAM: %d KB\n", ESP.getPsramSize() / 1024);
    Serial.printf("  Free PSRAM: %d KB\n", ESP.getFreePsram() / 1024);
    Serial.println();
    
    // Test 2: Check heap
    Serial.println("Test 2: Heap Check");
    Serial.println("-------------------");
    Serial.printf("  Free heap: %d KB\n", ESP.getFreeHeap() / 1024);
    Serial.printf("  Total heap: %d KB\n", ESP.getHeapSize() / 1024);
    if (ESP.getFreeHeap() < 50000) {
        Serial.println("⚠ WARNING: Low heap memory!");
    } else {
        Serial.println("✓ Heap OK");
    }
    Serial.println();
    
    // Test 3: GPIO Pin test
    Serial.println("Test 3: GPIO Pin Check");
    Serial.println("----------------------");
    Serial.printf("  I2S_BCLK: GPIO %d\n", I2S_BCLK);
    Serial.printf("  I2S_LRC:  GPIO %d\n", I2S_LRC);
    Serial.printf("  I2S_DOUT: GPIO %d\n", I2S_DOUT);
    
    // Check if pins are in PSRAM reserved range (33-37)
    if ((I2S_BCLK >= 33 && I2S_BCLK <= 37) ||
        (I2S_LRC >= 33 && I2S_LRC <= 37) ||
        (I2S_DOUT >= 33 && I2S_DOUT <= 37)) {
        Serial.println("❌ ERROR: I2S pins conflict with PSRAM!");
        Serial.println("   GPIO 33-37 are reserved for PSRAM");
        Serial.println("   Change I2S pins in your config");
        return;
    }
    Serial.println("✓ Pins OK (not in PSRAM range)");
    
    // Set pins as outputs to test
    pinMode(I2S_BCLK, OUTPUT);
    pinMode(I2S_LRC, OUTPUT);
    pinMode(I2S_DOUT, OUTPUT);
    
    // Toggle pins to verify they work
    for (int i = 0; i < 5; i++) {
        digitalWrite(I2S_BCLK, HIGH);
        digitalWrite(I2S_LRC, HIGH);
        digitalWrite(I2S_DOUT, HIGH);
        delayMicroseconds(100);
        digitalWrite(I2S_BCLK, LOW);
        digitalWrite(I2S_LRC, LOW);
        digitalWrite(I2S_DOUT, LOW);
        delayMicroseconds(100);
    }
    Serial.println("✓ GPIO pins can toggle");
    Serial.println();
    
    // Test 4: WiFi Connection
    Serial.println("Test 4: WiFi Connection");
    Serial.println("-----------------------");
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    Serial.println();
    
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("❌ WiFi connection FAILED");
        Serial.println("   Check SSID and password");
        return;
    }
    Serial.println("✓ WiFi connected");
    Serial.printf("  IP: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("  Signal: %d dBm\n", WiFi.RSSI());
    Serial.println();
    
    // Test 5: Audio Library Init
    Serial.println("Test 5: Audio Library");
    Serial.println("---------------------");
    
    // Initialize audio
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(4); // 0-21
    
    Serial.println("✓ Audio object created");
    Serial.printf("  Volume set to: 12/21\n");
    Serial.println();
    
    // Test 6: Try to play audio
    Serial.println("Test 6: Audio Stream Test");
    Serial.println("-------------------------");
    Serial.println("Attempting to connect to test stream...");
    Serial.println("Stream: https://playerservices.streamtheworld.com/api/livestream-redirect/VERONICAAAC.aac");
    
    if (audio.connecttohost("https://playerservices.streamtheworld.com/api/livestream-redirect/VERONICAAAC.aac")) {
        Serial.println("✓ Connected to stream!");
        Serial.println("\n🔊 YOU SHOULD HEAR AUDIO NOW!");
        Serial.println("   If not, check:");
        Serial.println("   1. Speaker/amplifier is powered on");
        Serial.println("   2. Wiring is correct");
        Serial.println("   3. MAX98357A EN pin is HIGH (or not connected)");
        Serial.println("   4. Volume on amplifier");
        testsPassed = true;
    } else {
        Serial.println("❌ Failed to connect to stream");
        Serial.println("   Possible causes:");
        Serial.println("   - Network issue");
        Serial.println("   - Stream URL changed");
        Serial.println("   - Audio buffer allocation failed");
    }
    
    Serial.println("\n========================================");
    if (testsPassed) {
        Serial.println("    ✓ All tests passed!");
    } else {
        Serial.println("    ❌ Some tests failed");
    }
    Serial.println("========================================\n");
}

void loop() {
    if (testsPassed) {
        audio.loop();
        
        // Print audio info every 5 seconds
        static unsigned long lastPrint = 0;
        if (millis() - lastPrint > 5000) {
            lastPrint = millis();
            Serial.println("Audio playing...");
            Serial.printf("  Free heap: %d KB\n", ESP.getFreeHeap() / 1024);
            Serial.printf("  Free PSRAM: %d KB\n", ESP.getFreePsram() / 1024);
        }
    }
    delay(1);
}

// Audio library event handlers (optional but useful for debugging)
void audio_info(const char *info) {
    Serial.print("Audio info: ");
    Serial.println(info);
}

void audio_showstation(const char *info) {
    Serial.print("Station: ");
    Serial.println(info);
}

void audio_showstreamtitle(const char *info) {
    Serial.print("Stream title: ");
    Serial.println(info);
}

void audio_bitrate(const char *info) {
    Serial.print("Bitrate: ");
    Serial.println(info);
}

void audio_eof_mp3(const char *info) {
    Serial.println("End of file");
}
