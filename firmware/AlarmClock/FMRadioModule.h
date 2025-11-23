#ifndef FM_RADIO_MODULE_H
#define FM_RADIO_MODULE_H

#include <RDA5807.h>

class FMRadioModule {
private:
    RDA5807 radio;
    bool isInitialized;
    float currentFrequency;

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
};

#endif