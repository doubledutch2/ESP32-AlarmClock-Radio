
// ==================== FMRadioModule.cpp ====================
#include "FMRadioModule.h"

FMRadioModule::FMRadioModule(uint8_t resetPin) {
  this->resetPin = resetPin;
  radio = new SI4735();
  currentBand = BAND_FM;
  currentFrequency = 9580;  // 95.80 MHz
  currentVolume = 40;
  rdsEnabled = false;
  rdsUpdated = false;
  fmPresetCount = 0;
  amPresetCount = 0;
  memset(rdsStationName, 0, sizeof(rdsStationName));
  memset(rdsText, 0, sizeof(rdsText));
}

FMRadioModule::~FMRadioModule() {
  delete radio;
}

bool FMRadioModule::begin() {
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, HIGH);
  
  // Initialize Si4735
  Wire.begin();  // Uses default I2C pins
  
  // Reset the radio
  digitalWrite(resetPin, LOW);
  delay(10);
  digitalWrite(resetPin, HIGH);
  delay(10);
  
  // Setup for FM mode
  radio->setup(resetPin, FM_BAND_TYPE);
  delay(100);
  
  setFM(8800, 10800);  // FM 88.0-108.0 MHz
  setFrequency(currentFrequency);
  setVolume(currentVolume);
  
  Serial.println("FM Radio module initialized");
  Serial.printf("Initial frequency: %s\n", getFrequencyString().c_str());
  
  return true;
}

void FMRadioModule::end() {
  radio->powerDown();
}

void FMRadioModule::setBand(RadioBand band) {
  currentBand = band;
  
  switch(band) {
    case BAND_FM:
      setFM();
      setFrequency(9580);  // Default to 95.80 MHz
      break;
    case BAND_AM:
      setAM();
      setFrequency(693);   // Default to 693 kHz (BBC Radio 5 Live MW)
      break;
    case BAND_SW:
      // Shortwave implementation if needed
      break;
  }
}

void FMRadioModule::setFM(uint16_t minFreq, uint16_t maxFreq) {
  radio->setFM(minFreq, maxFreq, 10800, 10);  // 10kHz step
  currentBand = BAND_FM;
  if (rdsEnabled) {
    radio->setRdsConfig(1, 2, 2, 2, 2);
  }
}

void FMRadioModule::setAM(uint16_t minFreq, uint16_t maxFreq) {
  radio->setAM(minFreq, maxFreq, 1000, 9);  // 9kHz step (UK standard)
  currentBand = BAND_AM;
  rdsEnabled = false;
}

void FMRadioModule::setFrequency(uint16_t freq) {
  currentFrequency = freq;
  
  if (currentBand == BAND_FM) {
    radio->setFrequency(freq);
  } else {
    radio->setFrequency(freq);
  }
  
  Serial.printf("Tuned to: %s\n", getFrequencyString().c_str());
}

void FMRadioModule::seekUp() {
  radio->seekStationUp();
  delay(30);
  currentFrequency = radio->getFrequency();
  Serial.printf("Seek up: %s (RSSI: %d)\n", getFrequencyString().c_str(), getRSSI());
}

void FMRadioModule::seekDown() {
  radio->seekStationDown();
  delay(30);
  currentFrequency = radio->getFrequency();
  Serial.printf("Seek down: %s (RSSI: %d)\n", getFrequencyString().c_str(), getRSSI());
}

void FMRadioModule::frequencyUp() {
  if (currentBand == BAND_FM) {
    radio->frequencyUp();
  } else {
    radio->frequencyUp();
  }
  currentFrequency = radio->getFrequency();
}

void FMRadioModule::frequencyDown() {
  if (currentBand == BAND_FM) {
    radio->frequencyDown();
  } else {
    radio->frequencyDown();
  }
  currentFrequency = radio->getFrequency();
}

void FMRadioModule::setVolume(uint8_t vol) {
  if (vol > 63) vol = 63;  // Si4735 max volume is 63
  currentVolume = vol;
  radio->setVolume(vol);
}

void FMRadioModule::volumeUp() {
  if (currentVolume < 63) {
    currentVolume++;
    radio->setVolume(currentVolume);
  }
}

void FMRadioModule::volumeDown() {
  if (currentVolume > 0) {
    currentVolume--;
    radio->setVolume(currentVolume);
  }
}

void FMRadioModule::mute() {
  radio->setVolume(0);
}

void FMRadioModule::unmute() {
  radio->setVolume(currentVolume);
}

uint8_t FMRadioModule::getRSSI() {
  return radio->getCurrentRSSI();
}

uint8_t FMRadioModule::getSNR() {
  return radio->getCurrentSNR();
}

bool FMRadioModule::isStereo() {
  if (currentBand == BAND_FM) {
    return radio->getCurrentPilot();
  }
  return false;
}

void FMRadioModule::enableRDS() {
  if (currentBand == BAND_FM) {
    radio->setRdsConfig(1, 2, 2, 2, 2);
    rdsEnabled = true;
  }
}

void FMRadioModule::disableRDS() {
  rdsEnabled = false;
}

bool FMRadioModule::checkRDS() {
  if (!rdsEnabled || currentBand != BAND_FM) {
    return false;
  }
  
  if (radio->getRdsReady()) {
    char* station = radio->getRdsText0A();
    char* text = radio->getRdsText2A();
    
    if (station && strlen(station) > 0) {
      if (strcmp(rdsStationName, station) != 0) {
        strncpy(rdsStationName, station, 8);
        rdsStationName[8] = '\0';
        rdsUpdated = true;
      }
    }
    
    if (text && strlen(text) > 0) {
      if (strcmp(rdsText, text) != 0) {
        strncpy(rdsText, text, 64);
        rdsText[64] = '\0';
        rdsUpdated = true;
      }
    }
    
    return rdsUpdated;
  }
  
  return false;
}

const char* FMRadioModule::getRDSStationName() {
  return rdsStationName;
}

const char* FMRadioModule::getRDSText() {
  return rdsText;
}

bool FMRadioModule::saveFMPreset(int index, uint16_t freq, const char* name) {
  if (index < 0 || index >= MAX_FM_PRESETS) {
    return false;
  }
  
  fmPresets[index].frequency = freq;
  fmPresets[index].rssi = getRSSI();
  
  if (name) {
    strncpy(fmPresets[index].name, name, 31);
    fmPresets[index].name[31] = '\0';
  } else {
    sprintf(fmPresets[index].name, "FM %.1f", freq / 100.0);
  }
  
  if (index >= fmPresetCount) {
    fmPresetCount = index + 1;
  }
  
  return true;
}

bool FMRadioModule::saveAMPreset(int index, uint16_t freq, const char* name) {
  if (index < 0 || index >= MAX_AM_PRESETS) {
    return false;
  }
  
  amPresets[index].frequency = freq;
  amPresets[index].rssi = getRSSI();
  
  if (name) {
    strncpy(amPresets[index].name, name, 31);
    amPresets[index].name[31] = '\0';
  } else {
    sprintf(amPresets[index].name, "AM %d", freq);
  }
  
  if (index >= amPresetCount) {
    amPresetCount = index + 1;
  }
  
  return true;
}

bool FMRadioModule::loadFMPreset(int index) {
  if (index < 0 || index >= fmPresetCount) {
    return false;
  }
  
  setBand(BAND_FM);
  setFrequency(fmPresets[index].frequency);
  
  Serial.printf("Loaded FM preset %d: %s\n", index, fmPresets[index].name);
  return true;
}

bool FMRadioModule::loadAMPreset(int index) {
  if (index < 0 || index >= amPresetCount) {
    return false;
  }
  
  setBand(BAND_AM);
  setFrequency(amPresets[index].frequency);
  
  Serial.printf("Loaded AM preset %d: %s\n", index, amPresets[index].name);
  return true;
}

int FMRadioModule::autoScanFM() {
  Serial.println("Auto-scanning FM stations...");
  
  setBand(BAND_FM);
  setFrequency(8800);  // Start at 88.0 MHz
  
  fmPresetCount = 0;
  
  for (int i = 0; i < MAX_FM_PRESETS && fmPresetCount < MAX_FM_PRESETS; i++) {
    seekUp();
    delay(100);
    
    uint8_t rssi = getRSSI();
    if (rssi > 20) {  // Minimum signal threshold
      uint16_t freq = getFrequency();
      
      // Check if already in list
      bool duplicate = false;
      for (int j = 0; j < fmPresetCount; j++) {
        if (fmPresets[j].frequency == freq) {
          duplicate = true;
          break;
        }
      }
      
      if (!duplicate) {
        saveFMPreset(fmPresetCount, freq);
        Serial.printf("Found: %s (RSSI: %d)\n", getFrequencyString().c_str(), rssi);
      }
    }
    
    if (getFrequency() < 8900) {  // Wrapped around
      break;
    }
  }
  
  Serial.printf("Auto-scan complete: %d FM stations found\n", fmPresetCount);
  return fmPresetCount;
}

int FMRadioModule::autoScanAM() {
  Serial.println("Auto-scanning AM stations...");
  
  setBand(BAND_AM);
  setFrequency(520);  // Start at 520 kHz
  
  amPresetCount = 0;
  
  for (int i = 0; i < MAX_AM_PRESETS && amPresetCount < MAX_AM_PRESETS; i++) {
    seekUp();
    delay(200);
    
    uint8_t rssi = getRSSI();
    if (rssi > 25) {  // Minimum signal threshold (AM needs higher)
      uint16_t freq = getFrequency();
      
      bool duplicate = false;
      for (int j = 0; j < amPresetCount; j++) {
        if (amPresets[j].frequency == freq) {
          duplicate = true;
          break;
        }
      }
      
      if (!duplicate) {
        saveAMPreset(amPresetCount, freq);
        Serial.printf("Found: %s (RSSI: %d)\n", getFrequencyString().c_str(), rssi);
      }
    }
    
    if (getFrequency() < 600) {  // Wrapped around
      break;
    }
  }
  
  Serial.printf("Auto-scan complete: %d AM stations found\n", amPresetCount);
  return amPresetCount;
}

String FMRadioModule::getFrequencyString() {
  char buffer[16];
  
  if (currentBand == BAND_FM) {
    sprintf(buffer, "%.2f MHz", currentFrequency / 100.0);
  } else {
    sprintf(buffer, "%d kHz", currentFrequency);
  }
  
  return String(buffer);
}

String FMRadioModule::getBandString() {
  switch(currentBand) {
    case BAND_FM: return "FM";
    case BAND_AM: return "AM";
    case BAND_SW: return "SW";
    default: return "Unknown";
  }
}

/*
 * USAGE EXAMPLE:
 * 
 * #define FM_RESET_PIN 5
 * FMRadioModule* fmRadio = new FMRadioModule(FM_RESET_PIN);
 * 
 * void setup() {
 *   fmRadio->begin();
 *   fmRadio->setFM();
 *   fmRadio->setFrequency(9580);  // 95.80 MHz
 *   fmRadio->enableRDS();
 *   fmRadio->autoScanFM();  // Find all stations
 * }
 * 
 * void loop() {
 *   if (fmRadio->checkRDS()) {
 *     Serial.println(fmRadio->getRDSStationName());
 *   }
 * }
 */
