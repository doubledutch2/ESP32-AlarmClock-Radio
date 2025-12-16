#ifndef FEATURE_FLAGS_H
#define FEATURE_FLAGS_H

// Feature flags structure
struct FeatureFlags {
    bool enableTouchScreen;
    bool enableButtons;
    bool enableDraw;
    bool enableAudio;
    bool enableStereo;
    bool enableLED;
    bool enableAlarms;
    bool enableWeb;
    bool enableFMRadio;
    bool enablePRAM;
    bool enableI2CScan;
    
    // Constructor with defaults
    FeatureFlags() {
        enableTouchScreen = true;
        enableButtons = true;
        enableDraw = true;
        enableAudio = true;
        enableStereo = true;
        enableLED = true;
        enableAlarms = true;
        enableWeb = true;
        enableFMRadio = false;
        enablePRAM = true;
        enableI2CScan = true;
    }
};

#endif
