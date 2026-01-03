#ifndef FM_RADIO_MODULE_H
#define FM_RADIO_MODULE_H

#include <SI4735.h>
#include "Config.h"

class FMRadioModule {
private:
    SI4735 radio;
    bool isInitialized;
    float currentFrequency;
    uint8_t currentVolume;

public:
    FMRadioModule();
    bool begin();
    void setFrequency(float freq);
    float getFrequency();
    void setVolume(uint8_t vol);
    void seekUp();
    void seekDown();
    void mute(bool state);
    bool isReady();
    
    // Si4735 specific methods
    void setupDigitalAudio();
    int getRSSI();
    bool getRdsReceived();
    char* getRdsText();
    void getRdsStatus();
};

#endif