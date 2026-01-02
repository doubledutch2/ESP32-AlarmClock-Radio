#include <SI4735.h>
#include <driver/i2s.h>
#include "driver/ledc.h"
#include "config.h"

SI4735 rx;

// --- Forward Declarations (Fixes the 'not declared in this scope' error) ---
void showHelp();
void showStatus();
void showRDS();
void scanChannels();
void clearRDS();

// --- Global Variables from your source ---
char *stationName;
char *rdsText;
char *rdsTime;
char lastStationName[10];
uint8_t currentVolume = 45; 
uint16_t fmStartFrequency = 10560;
uint16_t amLastFrequency = 810;
uint16_t fmLastFrequency = 10390;
unsigned long lastRdsCheck = 0;
#define bufferLen 64

// I2S Configuration: ESP32 provides BCLK and LRC [cite: 69, 70]
const i2s_config_t i2s_config = {
  .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
  .sample_rate = 48000,
  .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
  .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
  .communication_format = I2S_COMM_FORMAT_STAND_I2S,
  .intr_alloc_flags = 0,
  .dma_buf_count = 8,
  .dma_buf_len = bufferLen,
  .use_apll = false
};

const i2s_pin_config_t pin_config = {
  .bck_io_num = I2S_BCLK, // Pin 8
  .ws_io_num = I2S_LRC,   // Pin 7
  .data_out_num = -1,     // Direct Radio->Amp path: ESP32 doesn't send data
  .data_in_num = -1
};

// 32.768kHz RCLK for Si4735 [cite: 73, 74]
ledc_timer_config_t ledc_timer = {
    .speed_mode      = LEDC_LOW_SPEED_MODE,
    .duty_resolution = LEDC_TIMER_8_BIT,
    .timer_num       = LEDC_TIMER_0,
    .freq_hz         = 32768
};

ledc_channel_config_t ledc_channel = {
    .gpio_num   = FM_RCLK_PIN, // Pin 40
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .channel    = LEDC_CHANNEL_0,
    .timer_sel  = LEDC_TIMER_0,
    .duty       = 128 // 50% duty
};

void setup() {
  Serial.begin(115200);
  while(!Serial);
  
  // 1. Hardware Reset Sequence
  pinMode(FM_RESET_PIN, OUTPUT);
  digitalWrite(FM_RESET_PIN, LOW);
  delay(100);
  digitalWrite(FM_RESET_PIN, HIGH);
  delay(100);

  // 2. Start Clocks [cite: 88, 99]
  ledc_timer_config(&ledc_timer);
  ledc_channel_config(&ledc_channel);
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  i2s_start(I2S_NUM_0);

  // 3. Initialize Radio [cite: 87, 90, 91]
  Wire.begin(I2C_SDA, I2C_SCL);
  rx.setRefClock(32768);
  rx.setRefClockPrescaler(1); 
  rx.setup(FM_RESET_PIN, -1, FM_CURRENT_MODE, SI473X_ANALOG_DIGITAL_AUDIO, XOSCEN_RCLK);
  
  // 4. Audio Config [cite: 94, 98, 108]
  rx.setVolume(currentVolume);
  rx.setFM(8400, 10800, fmStartFrequency, 10);
  rx.digitalOutputSampleRate(48000);
  rx.digitalOutputFormat(0, 0, 0, 0); 
  rx.setRdsConfig(1, 1, 1, 1, 1);

  showHelp();
  showStatus();
}

void loop() {
  if (millis() - lastRdsCheck > 500) {
    showRDS();
    lastRdsCheck = millis();
  }

  if (Serial.available() > 0) {
    char key = Serial.read();
    switch (key) {
      case '+': rx.setVolume(++currentVolume); Serial.println(currentVolume); break;
      case '-': rx.setVolume(--currentVolume); Serial.println(currentVolume); break;
      case 'u': case 'U': rx.frequencyUp(); showStatus(); break;
      case 'd': case 'D': rx.frequencyDown(); showStatus(); break;
      case 'x': case 'X': scanChannels(); break;
      case '0': showStatus(); break;
      case '?': showHelp(); break;
    }
  }
}

// --- Original Functions Restored ---

void showStatus() { // [cite: 80, 81, 82, 83]
  uint16_t freq = rx.getFrequency();
  Serial.print("Tuned to: ");
  if (rx.isCurrentTuneFM()) Serial.print(String(freq / 100.0, 2) + "MHz ");
  else Serial.print(String(freq) + "kHz ");
  Serial.println("Vol: " + String(rx.getVolume()));
}

void showHelp() { // [cite: 75, 76, 77, 78, 79]
  Serial.println("F/A: Mode, U/D: Step, S: Seek, +/-: Vol, X: Scan, ?: Help");
}

void showRDS() { // [cite: 139, 140, 141, 142]
  if (rx.isCurrentTuneFM()) {
    rx.getRdsStatus();
    if (rx.getRdsReceived() && rx.getRdsSync()) {
      rdsText = rx.getRdsText();
      if (rdsText != NULL && strlen(rdsText) > 0) {
        Serial.print("[RDS]: "); Serial.println(rdsText);
      }
    }
  }
}

void scanChannels() { // [cite: 132, 133, 134, 135]
  int oldFreq = rx.getFrequency();
  for (int f = 8750; f <= 10800; f += 10) {
    rx.setFrequency(f);
    delay(200);
    rx.getCurrentReceivedSignalQuality();
    if (rx.getCurrentRSSI() > 15) {
      Serial.println(String(f/100.0) + " MHz | RSSI: " + String(rx.getCurrentRSSI()));
    }
  }
  rx.setFrequency(oldFreq);
}

void clearRDS() { // [cite: 137, 138, 139]
  stationName = NULL; rdsText = NULL;
  lastStationName[0] = '\0';
  rx.clearRdsBuffer();
}