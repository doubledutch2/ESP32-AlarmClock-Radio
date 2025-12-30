#include <Wire.h>
#include <SI4735.h>
#include "Config.h"

// const int FM_RCLK_PIN = 16; 
SI4735 radio;


void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("\n--- Si4735 FM Band Scanner ---");

  // 1. Start Clock (Heartbeat) for Pin 19
  ledcAttach(FM_RCLK_PIN, 32768, 8); 
  ledcWrite(FM_RCLK_PIN, 128); 
  delay(200); 

  // 2. Setup Radio
  Wire.begin(I2C_SDA, I2C_SCL);
  pinMode(FM_RESET_PIN, OUTPUT);
  digitalWrite(FM_RESET_PIN, LOW);
  delay(100);
  digitalWrite(FM_RESET_PIN, HIGH);
  delay(100);

  // 0=Ext Clock, 1=FM, DIGITAL_AUDIO1=Pins 1,2,3
  radio.setup(FM_RESET_PIN, -1, FM_CURRENT_MODE, SI473X_ANALOG_DIGITAL_AUDIO, XOSCEN_RCLK);  
  //radio.setup(FM_RESET_PIN, 0, 1, SI473X_DIGITAL_AUDIO1); 
  radio.digitalOutputFormat(0, 0, 0, 0); 
  radio.digitalOutputSampleRate(44100);
  radio.setVolume(63); // Set digital gain to max


}

void runFullScan() {
  Serial.println("Scanning FM Band (87.5 - 108.0 MHz)...");
  Serial.println("FREQ | RSSI | SNR");
  Serial.println("------------------");

  for (int freq = 8750; freq <= 10800; freq += 10) {
    radio.setFrequency(freq);
    delay(50); // Give the tuner a moment to settle
    radio.getCurrentReceivedSignalQuality();
    
    int rssi = radio.getCurrentRSSI();
    int snr = radio.getCurrentSNR();

    // Only print if there is even a hint of a station
    if (rssi > 15 || snr > 0) {
      Serial.print(freq / 100.0);
      Serial.print(" | "); Serial.print(rssi);
      Serial.print(" | "); Serial.println(snr);
    }
  }
  Serial.println("Scan Complete.");
}

void loop() {

  testChipCommunication();
  runFullScan();
  delay(5000);

}

void testChipCommunication() {
  // Attempt to read the Status Byte (Command 0x01 is GET_REV)
  Wire.beginTransmission(0x11); // Standard SI4735 address
  Serial.print("Check I2C");
  Wire.write(0x10);             // GET_REV command
  if (Wire.endTransmission() != 0) {
    Serial.println("Failed to send command to 0x11");
    return;
  }

  delay(10);
  Wire.requestFrom(0x11, 1);
  Serial.print("Check I2C-1");
  if (Wire.available()) {
    byte status = Wire.read();
    Serial.print("Chip Status Byte: 0b");
    Serial.println(status, BIN);
    
    // Bit 7 of the status byte is the CTS (Clear to Send) bit
    if (status & 0x80) {
      Serial.println("SUCCESS: Chip is responding and Ready to Send (CTS=1)");
    } else {
      Serial.println("BUSY: Chip is responding but not ready (CTS=0). Check RCLK!");
    }
  }
  Serial.print("Check I2C-2");

}