#include "FMRadioModule.h"

FMRadioModule::FMRadioModule() : isInitialized(false), currentFrequency(98.0) {}

bool FMRadioModule::begin() {
    // Setup for PU2CLR RDA5807 library
    radio.setup();
    radio.setVolume(8);
    radio.setFrequency(9800); // 98.0 MHz (frequency in 10kHz units)

    if (radio.checkI2C(rdaAddresses)) {
        isInitialized = true;
        currentFrequency = 98.0;
    }
    return isInitialized;
}

void FMRadioModule::setFrequency(float freq) {
    if (isInitialized && freq >= 87.0 && freq <= 108.0) {
        currentFrequency = freq;
        radio.setFrequency((uint16_t)(freq * 100));
    }
}

float FMRadioModule::getFrequency() {
    if (isInitialized) {
        currentFrequency = radio.getFrequency() / 100.0;
    }
    return currentFrequency;
}

void FMRadioModule::setVolume(uint8_t vol) {
    if (isInitialized && vol <= 15) {
        radio.setVolume(vol);
    }
}

void FMRadioModule::seekUp() {
    if (isInitialized) {
        radio.seek(RDA_SEEK_WRAP, RDA_SEEK_UP);
        delay(300); // Wait for seek to complete
        currentFrequency = radio.getFrequency() / 100.0;
    }
}

void FMRadioModule::seekDown() {
    if (isInitialized) {
        radio.seek(RDA_SEEK_WRAP, RDA_SEEK_DOWN);
        delay(300); // Wait for seek to complete
        currentFrequency = radio.getFrequency() / 100.0;
    }
}

void FMRadioModule::mute(bool state) {
    if (isInitialized) {
        radio.setMute(state);
    }
}

bool FMRadioModule::isReady() {
    return isInitialized;
}