#include <SI4735.h>
#include <WiFi.h>
#include "Audio.h"        
#include <driver/i2s.h>
#include "driver/ledc.h"
#include "config.h" 

SI4735 rx;
Audio audio; 

// Forward Declarations
void showHelp();
void showStatus();
void showRDS();
void scanChannels();
void runI2CScanner();
void setupRCLK();
void toggleSource();

// State Management
bool isRadioMode = true;
uint8_t currentVolume = 45; 
uint16_t fmStartFrequency = 10560;
unsigned long lastRdsCheck = 0;
char *rdsText;

// BBC World Service Stream
const char* streamUrl = "http://stream.live.vc.bbcmedia.co.uk/bbc_world_service";

void setup() {
  Serial.begin(115200);
  while(!Serial);
  Serial.println("\n--- Initializing System ---");

  // 1. WiFi Setup
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); 
    Serial.print("."); 
  }
  Serial.println("\nWiFi Connected");

  // 2. Multiplexer Setup (GPIO 21)
  pinMode(MODE_SWITCH_PIN, OUTPUT);
  digitalWrite(MODE_SWITCH_PIN, LOW); // Start in FM Radio Mode

  // 3. Hardware Reset & I2C Scan
  pinMode(FM_RESET_PIN, OUTPUT);
  digitalWrite(FM_RESET_PIN, LOW);
  delay(100);
  digitalWrite(FM_RESET_PIN, HIGH);
  delay(100);

  Wire.begin(I2C_SDA, I2C_SCL);
  runI2CScanner();

  // 4. Start 32.768kHz Clock for Si4735
  setupRCLK();

  // 5. Initialize Si4735 Radio
  rx.setup(FM_RESET_PIN, -1, FM_CURRENT_MODE, SI473X_ANALOG_DIGITAL_AUDIO, XOSCEN_RCLK);
  rx.setVolume(currentVolume);
  rx.setFM(8400, 10800, fmStartFrequency, 10);
  rx.digitalOutputSampleRate(44100); 
  rx.digitalOutputFormat(0, 0, 0, 0); 
  rx.setRdsConfig(1, 1, 1, 1, 1);

  // 6. Initialize Audio.h (ESP32-audioI2S)
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(12); // Volume range 0-21

  showHelp();
  showStatus();
}

void loop() {
  if (isRadioMode) {
    if (millis() - lastRdsCheck > 500) {
      showRDS();
      lastRdsCheck = millis();
    }
  } else {
    audio.loop(); 
  }

  if (Serial.available() > 0) {
    char key = Serial.read();
    if (key == 's' || key == 'S') {
      toggleSource();
    } else {
      handleSerial(key);
    }
  }
}

void toggleSource() {
  isRadioMode = !isRadioMode;
  if (isRadioMode) {
    audio.stopSong();
    digitalWrite(MODE_SWITCH_PIN, LOW); // 4053 Selects Radio
    Serial.println("\nMODE: FM RADIO");
    showStatus();
  } else {
    digitalWrite(MODE_SWITCH_PIN, HIGH); // 4053 Selects ESP32
    Serial.println("\nMODE: INTERNET STREAM (BBC World Service)");
    audio.connecttohost(streamUrl);
  }
}

void handleSerial(char key) {
  if (isRadioMode) {
    switch (key) {
      case '+': rx.setVolume(++currentVolume); break;
      case '-': rx.setVolume(--currentVolume); break;
      case 'u': case 'U': rx.frequencyUp(); showStatus(); break;
      case 'd': case 'D': rx.frequencyDown(); showStatus(); break;
      case 'x': case 'X': scanChannels(); break;
    }
  } else {
    if (key == '+') audio.setVolume(min(21, (int)audio.getVolume() + 1));
    if (key == '-') audio.setVolume(max(0, (int)audio.getVolume() - 1));
  }
}

void scanChannels() {
  Serial.println("\n--- Scanning FM Band ---");
  uint16_t oldFreq = rx.getFrequency();
  for (int f = 8750; f <= 10800; f += 10) {
    rx.setFrequency(f);
    delay(150);
    rx.getCurrentReceivedSignalQuality();
    if (rx.getCurrentRSSI() > 18) {
      Serial.print(f / 100.0);
      Serial.print(" MHz | RSSI: "); Serial.println(rx.getCurrentRSSI());
    }
  }
  rx.setFrequency(oldFreq);
  Serial.println("Scan complete.");
}

void runI2CScanner() {
  Serial.println("I2C Scanner starting...");
  for (byte i = 1; i < 127; i++) {
    Wire.beginTransmission(i);
    if (Wire.endTransmission() == 0) Serial.printf("Found 0x%02X\n", i);
  }
}

void setupRCLK() {
  ledc_timer_config_t ledc_timer = {
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .duty_resolution = LEDC_TIMER_8_BIT,
    .timer_num = LEDC_TIMER_0,
    .freq_hz = 32768
  };
  ledc_timer_config(&ledc_timer);
  ledc_channel_config_t ledc_channel = {
    .gpio_num = FM_RCLK_PIN,
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .channel = LEDC_CHANNEL_0,
    .timer_sel = LEDC_TIMER_0,
    .duty = 128
  };
  ledc_channel_config(&ledc_channel);
}

void showRDS() {
  if (rx.isCurrentTuneFM()) {
    rx.getRdsStatus();
    if (rx.getRdsReceived() && rx.getRdsSync()) {
      rdsText = rx.getRdsText();
      if (rdsText && strlen(rdsText) > 0) Serial.printf("\r[RDS]: %-64s", rdsText);
    }
  }
}

void showStatus() {
  Serial.printf("Tuned: %.2f MHz | Vol: %d\n", rx.getFrequency()/100.0, rx.getVolume());
}

void showHelp() {
  Serial.println("\n--- COMMANDS ---");
  Serial.println("S: Switch (Radio/Internet)");
  Serial.println("+/-: Volume");
  Serial.println("U/D: Tune Freq Up/Down");
  Serial.println("X: Scan FM Band");
}