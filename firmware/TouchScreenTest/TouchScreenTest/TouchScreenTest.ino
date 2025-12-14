#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h> 

// **********************************************
// PIN DEFINITIONS (From your Config.h)
// **********************************************
// TFT_eSPI uses User_Setup.h for display pins.
// These are used by the Touch library initialization below.
 #define TFT_BL           4   // Backlight pin (using the working pin 4)
 #define TOUCH_CS         13
 // #define TOUCH_IRQ        2   // Required for proper touch interrupt handling

// **********************************************
// TFT_eSPI Setup
// **********************************************
TFT_eSPI tft = TFT_eSPI(); 

// **********************************************
// Touchscreen Setup (FIXED CONSTRUCTOR)
// **********************************************
// Removed the third argument (tft.getSPIinstance()) to match your library version.
// XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ); 

// **********************************************
// Backlight (PWM) Fix Variables
// **********************************************
uint8_t ledc_channel = 0; // Will hold the actual assigned channel

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println("\n--- Starting Debug Sketch ---");

    // 1. TFT Initialization (Ensure User_Setup.h has TFT_RST set to -1)
    tft.init();
    Serial.println("\n--- Done Init ---");
    
    tft.setRotation(1); // Landscape
    tft.fillScreen(TFT_BLACK);
    tft.setTextFont(1); // FIX: Prevents LoadProhibited crash on first text draw
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Display Initialized (TFT_eSPI)", 10, 10, 2);

    // 2. Initialize Backlight (PWM FIX)
    pinMode(TFT_BL, OUTPUT);
    ledc_channel = ledcAttach(TFT_BL, 5000, 8); 
    
    // Set brightness to a visible level
    ledcWrite(ledc_channel, 200); 
    Serial.printf("Backlight ON (Pin %d, Channel %d)\n", TFT_BL, ledc_channel);

    // 3. Initialize Touchscreen
    /*
    ts.begin();
    ts.setRotation(tft.getRotation()); // Match rotation to display
    Serial.println("Touchscreen Initialized.");
    */

}

void loop() {
    Serial.println("Hello");
    delay (1000);
/*
    tft.getTouchRaw(&x, &y);
    Serial.printf("x: %i     ", x);
    Serial.printf("y: %i     ", y);
    Serial.printf("z: %i \n", tft.getTouchRawZ());
*/
/*
  delay(250);
    // 4. Read Touch Input and Check Stability
    if (ts.tirqTouched() && ts.touched()) {
        TS_Point p = ts.getPoint();
        Serial.printf("Touch at X:%d, Y:%d, Z:%d\n", p.x, p.y, p.z);
        tft.fillCircle(p.x, p.y, 5, TFT_YELLOW);

        // Re-assert backlight on touch
        ledcWrite(ledc_channel, 200);
    }
*/

    // 5. Simple counter to test continuous operation
    static unsigned long last_sec = 0;
    static int counter = 0;
    if (millis() - last_sec > 1000) {
        last_sec = millis();
        tft.fillRect(10, 30, 200, 20, TFT_BLACK); 
        tft.drawNumber(counter++, 10, 30, 2); 
        
        // This log will prove the loop is running after 30 seconds
        Serial.printf("Loop running: %d\n", counter); 
        
        // CRITICAL CHECK: Re-assert backlight every second to fight external dimming/reset
        ledcWrite(ledc_channel, 200); 
    }
    
    delay(1);
}