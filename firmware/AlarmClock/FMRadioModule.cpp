#include "FMRadioModule.h"

FMRadioModule::FMRadioModule() 
    : isInitialized(false), currentFrequency(98.0), currentVolume(45) {}

bool FMRadioModule::begin() {
    Serial.println("FMRadioModule: Initializing Si4735...");
    
    // Hardware reset
    pinMode(FM_RESET_PIN, OUTPUT);
    digitalWrite(FM_RESET_PIN, LOW);
    delay(100);
    digitalWrite(FM_RESET_PIN, HIGH);
    delay(100);
    
    // Initialize Si4735 with digital audio output
    radio.setup(
        FM_RESET_PIN,              // Reset pin
        -1,                        // SDA pin (using default Wire)
        FM_CURRENT_MODE,           // Mode (defined in SI4735.h)
        SI473X_ANALOG_DIGITAL_AUDIO, // Digital audio output
        XOSCEN_RCLK                // Use external RCLK
    );
    
    // Configure for FM band (87.5 - 108.0 MHz)
    radio.setFM(8750, 10800, 9800, 10);  // min, max, default freq (98.0 MHz), step (100kHz)
    
    // Setup digital audio output
    setupDigitalAudio();
    
    // Set volume
    radio.setVolume(currentVolume);
    
    // Enable RDS
    radio.setRdsConfig(1, 1, 1, 1, 1);
    
    isInitialized = true;
    currentFrequency = 98.0;
    
    Serial.println("FMRadioModule: Si4735 initialized with digital audio");
    return true;
}

void FMRadioModule::setupDigitalAudio() {
    // Configure Si4735 digital audio output for I2S compatibility
    // Sample rate: 44.1kHz, I2S format
    radio.digitalOutputSampleRate(SI4735_DIGITAL_AUDIO_SAMPLE_RATE);
    
    // Digital output format:
    // - OSIZE[1:0] = 00 (8-bit)
    // - OMONO = 0 (stereo)
    // - OMODE[3:0] = 0000 (I2S compatible)
    // - OFALL = 0 (rising edge)
    radio.digitalOutputFormat(0, 0, 0, 0);
    
    Serial.println("FMRadioModule: Digital audio configured (44.1kHz, I2S)");
}

void FMRadioModule::setFrequency(float freq) {
    if (isInitialized && freq >= 87.5 && freq <= 108.0) {
        currentFrequency = freq;
        uint16_t freqInt = (uint16_t)(freq * 100);  // Convert to 10kHz units
        radio.setFrequency(freqInt);
        Serial.printf("FMRadioModule: Tuned to %.1f MHz\n", freq);
    }
}

float FMRadioModule::getFrequency() {
    if (isInitialized) {
        currentFrequency = radio.getFrequency() / 100.0;
    }
    return currentFrequency;
}

void FMRadioModule::setVolume(uint8_t vol) {
    if (isInitialized && vol <= 63) {  // Si4735 volume range 0-63
        currentVolume = vol;
        radio.setVolume(vol);
    }
}

void FMRadioModule::seekUp() {
    if (isInitialized) {
        radio.frequencyUp();
        currentFrequency = radio.getFrequency() / 100.0;
        Serial.printf("FMRadioModule: Seek up to %.1f MHz\n", currentFrequency);
    }
}

void FMRadioModule::seekDown() {
    if (isInitialized) {
        radio.frequencyDown();
        currentFrequency = radio.getFrequency() / 100.0;
        Serial.printf("FMRadioModule: Seek down to %.1f MHz\n", currentFrequency);
    }
}

void FMRadioModule::mute(bool state) {
    if (isInitialized) {
        radio.setAudioMute(state);
    }
}

bool FMRadioModule::isReady() {
    return isInitialized;
}

int FMRadioModule::getRSSI() {
    if (isInitialized) {
        radio.getCurrentReceivedSignalQuality();
        return radio.getCurrentRSSI();
    }
    return 0;
}

bool FMRadioModule::getRdsReceived() {
    if (isInitialized && radio.isCurrentTuneFM()) {
        return radio.getRdsReceived() && radio.getRdsSync();
    }
    return false;
}

char* FMRadioModule::getRdsText() {
    if (isInitialized) {
        return radio.getRdsText();
    }
    return nullptr;
}

void FMRadioModule::getRdsStatus() {
    if (isInitialized && radio.isCurrentTuneFM()) {
        radio.getRdsStatus();
    }
}