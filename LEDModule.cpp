// ==================== LEDModule.cpp ====================
#include "LEDModule.h"

// Define static color constants
const RGB LEDModule::COLOR_OFF = {0, 0, 0};
const RGB LEDModule::COLOR_RED = {255, 0, 0};
const RGB LEDModule::COLOR_GREEN = {0, 255, 0};
const RGB LEDModule::COLOR_BLUE = {0, 0, 255};
const RGB LEDModule::COLOR_YELLOW = {255, 255, 0};
const RGB LEDModule::COLOR_MAGENTA = {255, 0, 255};

LEDModule::LEDModule(uint8_t pin, uint8_t numLeds) {
  led = new Adafruit_NeoPixel(numLeds, pin, NEO_GRB + NEO_KHZ800);
  brightness = 250;
}

LEDModule::~LEDModule() {
  delete led;
}

void LEDModule::begin() {
  led->begin();
  led->show();
  Serial.println("LED module initialized");
}

void LEDModule::setColor(const RGB& color, uint8_t bright) {
  brightness = bright;
  led->setPixelColor(0, led->Color(color.r, color.g, color.b));
  led->setBrightness(brightness);
  led->show();
}

void LEDModule::off() {
  setColor(COLOR_OFF, 0);
}

