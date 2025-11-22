// ==================== FMRadioModule.h ====================
#ifndef FM_RADIO_MODULE_H
#define FM_RADIO_MODULE_H

#include <Arduino.h>
#include <SI4735.h>  // PU2CLR Si4735 Arduino Library

#define MAX_FM_PRESETS 20
#define MAX_AM_PRESETS 20

enum RadioBand {
  BAND_FM = 0,
  BAND_AM = 1,
  BAND_SW = 2
};

struct FMPreset {
  uint16_t frequency;  // In 10kHz units (e.g., 9580 = 95.80 MHz)
  char name[32];
  uint8_t rssi;
};

struct AMPreset {
  uint16_t frequency;  // In kHz (e.g., 693 for 693 kHz)
  char name[32];
  uint8_t rssi;
};

class FMRadioModule {
private:
  SI4735* radio;
  uint8_t resetPin;
  RadioBand currentBand;
  uint16_t currentFrequency;
  uint8_t currentVolume;
  bool rdsEnabled;
  
  FMPreset fmPresets[MAX_FM_PRESETS];
  AMPreset amPresets[MAX_AM_PRESETS];
  int fmPresetCount;
  int amPresetCount;
  
  // RDS data
  char rdsStationName[9];
  char rdsText[65];
  bool rdsUpdated;
  
public:
  FMRadioModule(uint8_t resetPin);
  ~FMRadioModule();
  
  // Initialization
  bool begin();
  void end();
  
  // Band control
  void setBand(RadioBand band);
  RadioBand getBand() { return currentBand; }
  void setFM(uint16_t minFreq = 8800, uint16_t maxFreq = 10800); // FM 88.0-108.0 MHz
  void setAM(uint16_t minFreq = 520, uint16_t maxFreq = 1710);   // AM 520-1710 kHz
  
  // Tuning
  void setFrequency(uint16_t freq);
  uint16_t getFrequency() { return currentFrequency; }
  void seekUp();
  void seekDown();
  void frequencyUp();
  void frequencyDown();
  
  // Volume control
  void setVolume(uint8_t vol);
  uint8_t getVolume() { return currentVolume; }
  void volumeUp();
  void volumeDown();
  void mute();
  void unmute();
  
  // Signal quality
  uint8_t getRSSI();
  uint8_t getSNR();
  bool isStereo();
  
  // RDS (FM only)
  void enableRDS();
  void disableRDS();
  bool checkRDS();
  const char* getRDSStationName();
  const char* getRDSText();
  bool hasRDSUpdate() { return rdsUpdated; }
  void clearRDSUpdate() { rdsUpdated = false; }
  
  // Presets
  bool saveFMPreset(int index, uint16_t freq, const char* name = nullptr);
  bool saveAMPreset(int index, uint16_t freq, const char* name = nullptr);
  bool loadFMPreset(int index);
  bool loadAMPreset(int index);
  FMPreset* getFMPresets() { return fmPresets; }
  AMPreset* getAMPresets() { return amPresets; }
  int getFMPresetCount() { return fmPresetCount; }
  int getAMPresetCount() { return amPresetCount; }
  
  // Auto-scan and populate presets
  int autoScanFM();
  int autoScanAM();
  
  // Display info
  String getFrequencyString();
  String getBandString();
};

#endif

