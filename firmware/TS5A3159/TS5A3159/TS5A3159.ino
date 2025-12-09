// Pin used to drive TS5A3159 COM control
const int SWITCH_PIN = 42;   // choose a valid GPIO on ESP32-S3

void setup() {
  pinMode(SWITCH_PIN, OUTPUT);
  digitalWrite(SWITCH_PIN, LOW);  // start LOW
}

void loop() {
  // Toggle pin every second
  digitalWrite(SWITCH_PIN, HIGH);
  delay(1000);

  digitalWrite(SWITCH_PIN, LOW);
  delay(1000);
}