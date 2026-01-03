#ifndef AUDIO_SWITCH_H
#define AUDIO_SWITCH_H

#include <Arduino.h>
#include "Config.h"

enum AudioSource {
    SOURCE_FM_RADIO = 0,
    SOURCE_INTERNET_RADIO = 1
};

class AudioSwitch {
private:
    AudioSource currentSource;
    
public:
    AudioSwitch();
    
    void begin();
    void setSource(AudioSource source);
    AudioSource getCurrentSource();
    void toggleSource();
    
    bool isFMRadioActive();
    bool isInternetRadioActive();
};

#endif
