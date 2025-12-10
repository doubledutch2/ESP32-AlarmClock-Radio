// ----------------------------------------------------
// TOUCH SCREEN NAVIGATION EXAMPLE
// Based on ILI9341 Display and XPT2046 Touch Controller
// ----------------------------------------------------

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>

// --- PIN DEFINITIONS (Use your existing pins) ---
#define TFT_CS    10
#define TFT_DC    45
#define TFT_RST   21
#define TFT_MOSI  11
#define TFT_MISO  12
#define TFT_SCLK  40
#define TFT_BL    47

#define TOUCH_CS  13   // change if different
#define TOUCH_IRQ 2    // change if different

// --- DISPLAY OBJECTS ---
// Initialize TFT and Touch with your setup
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);

// --- SCREEN AND CALIBRATION CONSTANTS ---
// Screen geometry (Assuming Rotation 1 for 320x240 landscape)
const int SCREEN_W = 320;
const int SCREEN_H = 240;

// *** YOUR FINAL, CORRECTED CALIBRATION VALUES ***
int rawX_min = 300;   // low raw X seen when touching bottom right
int rawX_max = 3800;  // high raw X seen when touching top left
int rawY_min = 270;   // low raw Y seen when touching bottom right
int rawY_max = 3700;  // high raw Y seen when touching top left

// --- NAVIGATION STATE ---
enum ScreenState {
  HOME_SCREEN,
  BACK_SCREEN
};
ScreenState currentScreen = HOME_SCREEN;

// --- BUTTON STRUCTURE (for simple handling) ---
struct Button {
  int x;
  int y;
  int w;
  int h;
  const char* label;
  uint16_t color;
};

// Define a common button size/position
#define BUTTON_W 100
#define BUTTON_H 60
#define BUTTON_X 110
#define BUTTON_Y 150

// --- FUNCTION PROTOTYPES ---
void drawHomeScreen();
void drawBackScreen();
bool checkButtonPress(TS_Point p, Button btn);

// ----------------------------------------------------
// SETUP
// ----------------------------------------------------
void setup() {
  Serial.begin(115200);

  // Start SPI (explicit pins)
  SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI);

  // Initialize display
  tft.begin();
  tft.setRotation(1); // Use the working rotation (1 is often 320x240 landscape)

  // Turn on backlight (Pin 47 HIGH)
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  // Initialize touch screen
  ts.begin();

  // Draw the initial screen
  drawHomeScreen();
}

// ----------------------------------------------------
// MAIN LOOP
// ----------------------------------------------------
void loop() {
  // Check for touch
  if (ts.touched()) {
    TS_Point p = ts.getPoint(); // raw p.x, p.y, p.z

    // Only process if pressure is above a small threshold
    if (p.z > 5) {
      // Map and invert raw point to screen coordinates
      int sx = map(p.x, rawX_min, rawX_max, 0, SCREEN_W);
      int sy = map(p.y, rawY_min, rawY_max, 0, SCREEN_H);

      // Apply the inversion/flip (Crucial for your calibration)
      sx = SCREEN_W - sx;
      sy = SCREEN_H - sy;

      // Constrain points to ensure they are on screen
      sx = constrain(sx, 0, SCREEN_W - 1);
      sy = constrain(sy, 0, SCREEN_H - 1);

      // DEBUG: print mapped coordinates (optional)
      // Serial.print("MAPPED: sx="); Serial.print(sx);
      // Serial.print("  sy="); Serial.println(sy);

      // Handle screen state transitions
      if (currentScreen == HOME_SCREEN) {
        // Define the button for the Home Screen
        Button homeButton = {
          BUTTON_X, BUTTON_Y, BUTTON_W, BUTTON_H, "NEXT >>", ILI9341_GREEN
        };
        if (checkButtonPress( {sx, sy, p.z}, homeButton) ) {
          currentScreen = BACK_SCREEN;
          drawBackScreen();
        }
      } else if (currentScreen == BACK_SCREEN) {
        // Define the button for the Back Screen
        Button backButton = {
          BUTTON_X, BUTTON_Y, BUTTON_W, BUTTON_H, "<< HOME", ILI9341_RED
        };
        if (checkButtonPress( {sx, sy, p.z}, backButton) ) {
          currentScreen = HOME_SCREEN;
          drawHomeScreen();
        }
      }
    }
  }
  delay(100); // Simple debounce and polling delay
}

// ----------------------------------------------------
// DRAWING FUNCTIONS
// ----------------------------------------------------

void drawHomeScreen() {
  tft.fillScreen(ILI9341_BLUE); // Clear screen with a blue background

  // Draw Title
  tft.setTextSize(3);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setCursor(60, 50);
  tft.println("WELCOME HOME");

  // Draw Button
  Button homeButton = {
    BUTTON_X, BUTTON_Y, BUTTON_W, BUTTON_H, "NEXT >>", ILI9341_GREEN
  };
  tft.fillRect(homeButton.x, homeButton.y, homeButton.w, homeButton.h, homeButton.color);
  tft.drawRect(homeButton.x, homeButton.y, homeButton.w, homeButton.h, ILI9341_WHITE);
  
  // Draw Button Text
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_BLACK);
  // Calculate text centering
  int text_w = 6 * strlen(homeButton.label) * 2; // Approx width of string
  tft.setCursor(homeButton.x + (homeButton.w - text_w) / 2, homeButton.y + BUTTON_H/2 - 8);
  tft.print(homeButton.label);
}

void drawBackScreen() {
  tft.fillScreen(ILI9341_BLACK); // Clear screen with a black background

  // Draw Title
  tft.setTextSize(3);
  tft.setTextColor(ILI9341_MAGENTA);
  tft.setCursor(60, 50);
  tft.println("YOU ARE BACK!");

  // Draw Button
  Button backButton = {
    BUTTON_X, BUTTON_Y, BUTTON_W, BUTTON_H, "<< HOME", ILI9341_RED
  };
  tft.fillRect(backButton.x, backButton.y, backButton.w, backButton.h, backButton.color);
  tft.drawRect(backButton.x, backButton.y, backButton.w, backButton.h, ILI9341_WHITE);
  
  // Draw Button Text
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);
  // Calculate text centering
  int text_w = 6 * strlen(backButton.label) * 2; // Approx width of string
  tft.setCursor(backButton.x + (backButton.w - text_w) / 2, backButton.y + BUTTON_H/2 - 8);
  tft.print(backButton.label);
}

// ----------------------------------------------------
// HELPER FUNCTION
// ----------------------------------------------------

// Checks if the given screen point (p) is within the button's boundaries
bool checkButtonPress(TS_Point p, Button btn) {
  // Check if x-coordinate is within bounds
  if (p.x >= btn.x && p.x <= btn.x + btn.w) {
    // Check if y-coordinate is within bounds
    if (p.y >= btn.y && p.y <= btn.y + btn.h) {
      // Small visual feedback on the button press
      tft.drawRect(btn.x, btn.y, btn.w, btn.h, ILI9341_YELLOW);
      delay(75); // Debounce to prevent multiple presses
      return true;
    }
  }
  return false;
}