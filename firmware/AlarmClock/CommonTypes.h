#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include <Arduino.h>

// Internet Radio Station (for streaming)
struct InternetRadioStation {
    String name;
    String url;
};

// FM Radio Preset (for RDA5807 presets)
struct FMRadioPreset {
    float frequency;
    char name[32];
};

#endif
