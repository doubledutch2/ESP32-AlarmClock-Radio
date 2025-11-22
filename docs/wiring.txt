╔═══════════════════════════════════════════════════════════════════════════════╗
║                    ESP32-S3 ALARM CLOCK RADIO                                 ║
║                    COMPLETE WIRING DIAGRAM                                    ║
║                    Rev 1.0 - All Components                                   ║
╚═══════════════════════════════════════════════════════════════════════════════╝

PRINT INSTRUCTIONS:
-------------------
- Print in LANDSCAPE orientation
- Use A3 paper for best readability (or A4 at 85% scale)
- Keep this document at your workbench during assembly


═══════════════════════════════════════════════════════════════════════════════
SECTION 1: POWER DISTRIBUTION
═══════════════════════════════════════════════════════════════════════════════

                           USB-C Power Input (J1)
                                  │
                                  │ 5V
                                  │
                    ┌─────────────┴─────────────┐
                    │                           │
                    │  C1                       │
                   ═╪═ 10µF                     │
                    │  (1206)                   │
                   GND                          │
                                                │
                    ┌───────────────────────────┼────────────────┐
                    │                           │                │
             ┌──────┴──────┐              ┌────┴────┐      ┌────┴────┐
             │  AMS1117-3.3│              │ MAX98357│      │ MAX98357│
             │     (U6)    │              │  Left   │      │  Right  │
             │  VIN    VOUT│              │  (U3)   │      │  (U4)   │
             │   │      │  │              │  VIN    │      │  VIN    │
             └───┼──────┼──┘              └─────────┘      └─────────┘
                 │      │
                 │      │ 3.3V
                 │      │
                 │      └─────┬────────────────────────────────────────┐
                 │            │                                        │
                 │       ┌────┴────┐    ┌────────┐    ┌──────────┐   │
                 │       │  ESP32  │    │ Si4735 │    │ ILI9341  │   │
                 │       │  (U1)   │    │  (U2)  │    │ Display  │   │
                 │       │  3V3    │    │ VD/VA  │    │   VCC    │   │
                 │       │         │    │  /VIO  │    │          │   │
                 │       └─────────┘    └────────┘    └──────────┘   │
                 │            │              │              │         │
                GND          GND            GND            GND       GND
                 │            │              │              │         │
                 └────────────┴──────────────┴──────────────┴─────────┘
                                        COMMON GROUND


Power Supply Summary:
---------------------
5V Rail:   USB-C → MAX98357A Left (U3)
                 → MAX98357A Right (U4)
                 → AMS1117-3.3 input

3.3V Rail: AMS1117-3.3 output → ESP32-S3 (U1)
                               → Si4735 (U2)
                               → ILI9341 Display
                               → CD4066 (U5)
                               → All pull-up resistors
                               → All logic circuits

Ground:    COMMON GROUND - Connect all GND pins together


═══════════════════════════════════════════════════════════════════════════════
SECTION 2: ESP32-S3-DevKitC-1 PIN CONNECTIONS (LOCKED PINOUT)
═══════════════════════════════════════════════════════════════════════════════

                        ESP32-S3-DevKitC-1 (U1)
                    ┌─────────────────────────────┐
                    │                             │
     5V ────────────┤ 5V                      GND ├──────────── GND
     3.3V ──────────┤ 3V3                  GPIO48 ├──────────── LED (Built-in RGB)
                    │                      GPIO47 ├──────────── TFT_BL (Backlight PWM)
                    │                      GPIO21 ├──────────── TFT_RST
                    │                      GPIO20 ├─── USB D+ (DO NOT USE)
                    │                      GPIO19 ├─── USB D- (DO NOT USE)
                    │                             │
     I2C_SDA ───────┤ GPIO8               GPIO18  ├─── (Reserved)
     I2C_SCL ───────┤ GPIO9               GPIO17  ├──────────── I2S_DOUT_R (Right audio)
     TFT_CS ────────┤ GPIO10              GPIO16  ├──────────── I2S_DOUT_L (Left audio)
     TFT_MOSI ──────┤ GPIO11              GPIO15  ├──────────── I2S_BCLK
     TFT_MISO ──────┤ GPIO12              GPIO14  ├──────────── TFT_DC
     TFT_SCLK ──────┤ GPIO13               GPIO7  ├──────────── I2S_LRC
                    │                             │
     VOL_POT ───────┤ GPIO1               GPIO42  ├──────────── FM_RESET*
                    │                      GPIO41  ├──────────── AUDIO_SWITCH*
                    │                      GPIO40  ├──────────── MODE_SWITCH (optional)
     BTN_BRIGHT ────┤ GPIO38              GPIO39  ├──────────── BTN_NEXT_STATION
                    │                             │
                    └─────────────────────────────┘

* FM_RESET and AUDIO_SWITCH only used when ENABLE_FM_RADIO = true


═══════════════════════════════════════════════════════════════════════════════
SECTION 3: I2C BUS (Shared between OLED and Si4735)
═══════════════════════════════════════════════════════════════════════════════

                            +3.3V
                              │
                    ┌─────────┼─────────┐
                    │         │         │
                   R1        R2         │
                  2.2kΩ    2.2kΩ       │
                  (1206)   (1206)      │
                    │         │         │
                    │         │         │
     ESP32 ─────────┼─────────┼─────────┼───────────┐
     GPIO8          │         │         │           │
     (SDA)    ──────┴─────┬───┘         │           │
                          │             │           │
     ESP32               SDA           SDA         SDA
     GPIO9          ┌─────┴─────┐ ┌────┴────┐ ┌────┴────┐
     (SCL)    ──────┼───────────┼─┼─────────┼─┼─────────┤
                    │  OLED     │ │ Si4735  │ │ (Future │
                   SCL Display  │ │  (U2)   │ │ I2C     │
                    │  (128x64) │ │  Pin 15 │ │ Device) │
                    │  0.96"    │ │  Pin 16 │ │         │
                    └───────────┘ └─────────┘ └─────────┘

I2C Pull-up Resistors:
  R1: 2.2kΩ (1206) - SDA to +3.3V
  R2: 2.2kΩ (1206) - SCL to +3.3V

Decoupling Capacitors (close to each device):
  C4:  0.1µF (1206) - OLED VCC to GND
  C5:  0.1µF (1206) - Si4735 VD to GND
  C6:  0.1µF (1206) - Si4735 VA to GND
  C7:  10µF (1206)  - Si4735 VA to GND
  C8:  0.1µF (1206) - Si4735 VIO to GND


═══════════════════════════════════════════════════════════════════════════════
SECTION 4: I2S AUDIO (Internet Radio - Stereo)
═══════════════════════════════════════════════════════════════════════════════

     ESP32-S3                                            
     ┌────────┐                                          
     │ GPIO7  ├────────┬────────────────────────────────┬────────────┐
     │ (LRC)  │        │                                │            │
     │        │       R13                              R14           │
     │ GPIO15 ├───────┬┴7Ω───────────────────────────┬──47Ω─────────┤
     │ (BCLK) │       │(1206)                         │  (1206)      │
     │        │       │                               │              │
     │ GPIO16 ├───────┼────────────┬──────────────────┼──────────────┤
     │ (DOUT_L)       │            │                  │              │
     │        │       │           R15                 │              │
     │ GPIO17 ├───────┼────────────┼──47Ω─────────────┼──────────────┤
     │ (DOUT_R)       │            │  (1206)          │              │
     └────────┘       │            │                  │              │
                      │            │                  │              │
               ┌──────┴─────┐   ┌──┴──────────┐   ┌───┴──────────┐  │
               │ MAX98357A  │   │  MAX98357A  │   │              │  │
               │   LEFT     │   │   RIGHT     │   │              │  │
               │   (U3)     │   │   (U4)      │   │              │  │
               │            │   │             │   │              │  │
     5V ───────┤ VIN    OUT+├───┤→ Speaker L+ │   │              │  │
     GND ──────┤ GND    OUT-├───┤→ Speaker L- │   │              │  │
               │ LRC        │   │             │   │              │  │
               │ BCLK       │   │ LRC         │   │              │  │
               │ DIN        │   │ BCLK        │   │              │  │
               │ GAIN   SD  ├─3.3V DIN    GAIN├─GND              │  │
               └────────────┘   │         SD  ├─3.3V             │  │
                                │ VIN    OUT+ ├───→ Speaker R+   │  │
                         5V ────┤ GND    OUT- ├───→ Speaker R-   │  │
                         GND────┘             │                  │  │
                                └─────────────┘                  │  │
                                                                 │  │
Decoupling Capacitors:                                           │  │
  C20: 22µF (1206) - U3 VIN to GND                              │  │
  C21: 22µF (1206) - U4 VIN to GND                              │  │
                                                                 │  │
GAIN Settings:                                                   │  │
  U3 GAIN → GND (12dB gain - recommended)                        │  │
  U4 GAIN → GND (12dB gain - recommended)                        │  │
                                                                 │  │
SD (Shutdown) Pins:                                              │  │
  U3 SD → 3.3V (always enabled)                                  │  │
  U4 SD → 3.3V (always enabled)                                  │  │
                                                                 │  │
Speakers:                                                        │  │
  SP1: 4Ω 3W (Left)  - Connect to U3 OUT+/OUT-                  │  │
  SP2: 4Ω 3W (Right) - Connect to U4 OUT+/OUT-                  │  │


NOTE: For simplified MONO operation, wire both U3 and U4 DIN to GPIO16
      This gives dual-speaker mono (louder, but not stereo)


═══════════════════════════════════════════════════════════════════════════════
SECTION 5: SPI BUS (ILI9341 TFT Display - 2.8" 240x320)
═══════════════════════════════════════════════════════════════════════════════

     ESP32-S3                    ILI9341 Display Module
     ┌────────┐                  ┌──────────────────┐
     │ GPIO13 ├──────────────────┤ SCK              │
     │ (SCLK) │                  │                  │
     │        │                  │                  │
     │ GPIO11 ├──────────────────┤ MOSI/SDI         │
     │ (MOSI) │                  │                  │
     │        │                  │                  │
     │ GPIO12 ├──────────────────┤ MISO/SDO         │ (Optional)
     │ (MISO) │                  │                  │
     │        │                  │                  │
     │ GPIO10 ├──────────────────┤ CS               │
     │ (CS)   │                  │                  │
     │        │                  │                  │
     │ GPIO14 ├──────────────────┤ DC/RS            │
     │ (DC)   │                  │                  │
     │        │                  │                  │
     │ GPIO21 ├──────────────────┤ RESET            │
     │ (RST)  │                  │                  │
     │        │                  │                  │
     │ GPIO47 ├────R11───────────┤ LED (Backlight)  │
     │ (BL)   │   100Ω           │                  │
     │        │   (1206)         │                  │
     └────────┘                  │                  │
                                 │                  │
     3.3V ───────────────────────┤ VCC              │
     GND ────────────────────────┤ GND              │
                                 └──────────────────┘

Decoupling Capacitors:
  C9:  0.1µF (1206) - Display VCC to GND (place close to display)
  C10: 10µF (1206)  - Display VCC to GND

Backlight Current Limiting:
  R11: 100Ω (1206) - Limits LED current to ~30mA

Notes:
  - MISO is optional (not needed for display-only operation)
  - If display has SD card slot, MISO is needed for SD card access
  - Backlight is PWM controlled via GPIO47 for brightness adjustment


═══════════════════════════════════════════════════════════════════════════════
SECTION 6: Si4735 FM/AM RADIO CHIP (SSOP-24) - Only if ENABLE_FM_RADIO = true
═══════════════════════════════════════════════════════════════════════════════

                            Si4735-D60-GU (U2) SSOP-24
                         ┌─────────────────────────┐
                         │●1  AMI           DOUT 24│── NC
   J2 (AM Ant) ──────────┤ 2  FMI            DFS 23│── NC
   J3 (FM Ant)           │                         │
                 GND ────┤ 3  RFGND    GPO3/DCLK 22│── NC
                         │ 4  RIN       GPO2/DFS 21│── NC
                         │ 5  LIN           GPO1 20│── NC
   C16 (100µF) ──────────┤ 6  ROUT          RCLK 19├───┐ 32.768kHz
   (Audio Out R)         │ 7  LOUT         RCLKI 13├─┐ │ Crystal (Y1)
   C17 (100µF) ──────────┤ 8  NC             RST 18├─┼─┼─┐
   (Audio Out L)         │                         │ │ │ │
                 GND ────┤ 9  GND           SCLK 16├─┼─┼─┼─→ I2C_SCL (GPIO9)
                 3.3V ───┤10  VD            SDIO 15├─┼─┼─┼─→ I2C_SDA (GPIO8)
                 3.3V ───┤11  VA            GPO2 14│ │ │ │
                 3.3V ───┤12  VIO                  │ │ │ │
                         └─────────────────────────┘ │ │ │
                                  │  │  │            │ │ │
                                 C14 C15 C18         │ │ │
                                 18pF 18pF          │ │ │
                                (1206)(1206)  10µF  │ │ │
                                  │  │  │  (1206)   │ │ │
                                 GND GND GND        │ │ │
                                                    │ │ │
Power Decoupling:                                   │ │ │
  C5:  0.1µF (1206) - Pin 10 (VD) to GND           │ │ │
  C6:  0.1µF (1206) - Pin 11 (VA) to GND           │ │ │
  C7:  10µF (1206)  - Pin 11 (VA) to GND           │ │ │
  C8:  0.1µF (1206) - Pin 12 (VIO) to GND          │ │ │
  C18: 10µF (1206)  - Pin 11 (VA) to GND (audio)   │ │ │
                                                    │ │ │
Crystal Circuit (32.768 kHz):                       │ │ │
  Y1:  32.768kHz crystal - Between pins 13 & 19    │ │ │
  C14: 18pF (1206) - Pin 13 to GND                 │ │ │
  C15: 18pF (1206) - Pin 19 to GND                 │ │ │
                                                    │ │ │
I2C Connection:                                     │ │ │
  Pin 15 (SDIO/SDA) ────────────────────────────────┘ │ │
  Pin 16 (SCLK/SCL) ──────────────────────────────────┘ │
  Pin 17 (SEN) ── GND (I2C mode select)                 │
  Pin 18 (RST) ───────R5────3.3V (10kΩ pull-up)────────┼─→ GPIO42
                     (1206)                             │
                                                        │
Audio Output (to CD4066 or mixing circuit):            │
  Pin 6 (ROUT) ──C16──→ FM_AUDIO_R                     │
  Pin 7 (LOUT) ──C17──→ FM_AUDIO_L                     │
                                                        │
Antenna Connections:                                   │
  Pin 1 (AMI) ── J2 (AM antenna - ferrite rod)         │
  Pin 2 (FMI) ── J3 (FM antenna - 75cm wire)           │


═══════════════════════════════════════════════════════════════════════════════
SECTION 7: AUDIO SWITCHING (CD4066 Quad Switch) - Only if ENABLE_FM_RADIO = true
═══════════════════════════════════════════════════════════════════════════════

Audio Source Switching using CD4066 (U5):

                 CD4066BM96 (U5) - SOP-14
                 ┌───────────────────┐
                 │                   │
     3.3V ───────┤14 VDD         VSS├7──── GND
                 │                   │
                 │   Switch A        │
  I2S_AUDIO ─────┤1  (Internet)    2├───┬───→ MIXED_AUDIO → Final Output
                 │                   │   │
          ┌──────┤13 Control A       │   │
          │      │                   │   │
          │      │   Switch B        │   │
  FM_AUDIO ──────┤4  (FM Radio)    3├───┘
                 │                   │
  GPIO41 ────────┤5  Control B       │
  (AUDIO_SWITCH) │                   │
                 │   Switch C & D    │
                 │   (Not Used)      │
                 ├6──────────────────┤8
                 ├9──────────────────┤10
                 ├11─────────────────┤12
                 └───────────────────┘

Inverter Circuit for Switch A Control:
---------------------------------------

                     +3.3V
                       │
                      R5
                     10kΩ
                     (1206)
                       │
                       ├──→ Pin 13 (Switch A Control)
                       │
                    C──┘
  GPIO41 ──R9─── B     Q1 (NPN: S8050, SOT-23)
  (1kΩ)         E
  (1206)        │
               GND

Logic Operation:
  GPIO41 = LOW  → Q1 OFF → Pin 13 = HIGH → Switch A ON  (Internet Radio)
                          → Pin 5  = LOW  → Switch B OFF
  
  GPIO41 = HIGH → Q1 ON  → Pin 13 = LOW  → Switch A OFF
                          → Pin 5  = HIGH → Switch B ON  (FM Radio)

Decoupling:
  C11: 0.1µF (1206) - U5 VDD to GND


ALTERNATIVE: Use 5V SPDT Relay (Simpler)
-----------------------------------------
                 5V Relay Module
                 ┌────────────┐
  I2S_AUDIO ─────┤ NC      COM├───→ Final Output
                 │             │
  FM_AUDIO ──────┤ NO          │
                 │             │
          ┌──────┤ Coil+       │
          │      │ Coil-  ─────┤─── GND
          │      └─────────────┘
          │          │
          │       D3 ▼│ 1N5819 (SOD-323)
          │          │
         C──         │
  GPIO41 ──R10─ B    Q2 (NPN: S8050)
  (1kΩ)        E
  (1206)       │
              GND

Note: Relay will produce audible click when switching


═══════════════════════════════════════════════════════════════════════════════
SECTION 8: USER INTERFACE - BUTTONS AND CONTROLS
═══════════════════════════════════════════════════════════════════════════════

Volume Potentiometer (RV1):
---------------------------
                  10kΩ Linear Pot
                  ┌───────────┐
     3.3V ────────┤1         3├──── GND
                  │           │
     GPIO1 ───────┤2  (Wiper) │
     (VOL_POT)    └───────────┘

Note: ESP32 GPIO1 has built-in ADC


Brightness Button (S2):
-----------------------
                  Tactile Switch 6x6mm
                  ┌─────────┐
     GPIO38 ──────┤         ├──── GND
     (BTN_BRIGHT) └─────────┘

Internal pull-up enabled in software (pinMode INPUT_PULLUP)


Next Station Button (S3):
--------------------------
                  Tactile Switch 6x6mm
                  ┌─────────┐
     GPIO39 ──────┤         ├──── GND
     (BTN_NEXT)   └─────────┘

Internal pull-up enabled in software


Mode Switch Button (S4) - Optional:
------------------------------------
                  Tactile Switch 6x6mm
                  ┌─────────┐
     GPIO40 ──────┤         ├──── GND
     (MODE_SW)    └─────────┘

Internal pull-up enabled in software

Function: Switch between Internet Radio and FM Radio modes


Reset Button (S1):
------------------
                  Tactile Switch 6x6mm
                  ┌─────────┐
     EN pin ──────┤         ├──── GND
     (ESP32)      └─────────┘

Note: Connect to EN pin on ESP32-S3-DevKitC-1


Built-in RGB LED:
-----------------
     GPIO48 (ESP32) ──→ Built-in WS2812B RGB LED on DevKitC board
     
No external wiring needed - LED is on the board!


Optional External Status LED (D2):
-----------------------------------
If you want an external RGB LED:

                  WS2812B-Mini (SMD)
                  ┌────────────┐
     3.3V ────────┤ VDD    DOUT├──── (Not used)
     GND ─────────┤ GND    DIN ├──── GPIO48
                  └────────────┘

Add 0.1µF (1206) capacitor across VDD-GND


═══════════════════════════════════════════════════════════════════════════════
SECTION 9: ANTENNA CONNECTIONS
═══════════════════════════════════════════════════════════════════════════════

FM Antenna (J3):
----------------
                  3.5mm Audio Jack
                  ┌─────────┐
     Si4735 ──────┤ Tip     │
     Pin 2 (FMI)  │         │
                  │ Sleeve  ├──── GND
                  └─────────┘

Connect: 75cm wire antenna (1/4 wavelength at 100 MHz)
Wire: Single core, 0.5mm diameter
Orientation: Vertical for best reception


AM Antenna (J2):
----------------
                  3.5mm Audio Jack
                  ┌─────────┐
     Si4735 ──────┤ Tip     │
     Pin 1 (AMI)  │         │
                  │ Sleeve  ├──── GND
                  └─────────┘

Connect: Ferrite rod antenna with coil
Typical: 200mm ferrite rod, 60-100 turns of 0.2mm wire


Alternative: PCB Trace Antennas
--------------------------------
FM: Meandering trace, 75cm total length
AM: Loop antenna, 10cm x 10cm square


═══════════════════════════════════════════════════════════════════════════════
SECTION 10: SPEAKER CONNECTIONS
═══════════════════════════════════════════════════════════════════════════════

Left Speaker (SP1):
-------------------
     MAX98357A (U3)          Speaker Terminal (J4)
     ┌────────────┐          ┌──────────┐
     │ OUT+       ├──────────┤ Red (+)  │
     │            │          │          │──→ 4Ω 3W Speaker
     │ OUT-       ├──────────┤ Black(-)│
     └────────────┘          └──────────┘

Wire: 18-22 AWG, keep length < 30cm


Right Speaker (SP2):
--------------------
     MAX98357A (U4)          Speaker Terminal (J4)
     ┌────────────┐          ┌──────────┐
     │ OUT+       ├──────────┤ Red (+)  │
     │            │          │          │──→ 4Ω 3W Speaker
     │ OUT-       ├──────────┤ Black(-)│
     └────────────┘          └──────────┘

Wire: 18-22 AWG, keep length < 30cm


IMPORTANT: Speaker Polarity
----------------------------
✓ Always connect OUT+ to speaker + (red)
✓ Always connect OUT- to speaker - (black)
✗ Never short OUT+ to OUT- (will damage amplifier)
✓ Minimum speaker impedance: 4Ω
✓ Maximum speaker power: 3W continuous


═══════════════════════════════════════════════════════════════════════════════
SECTION 11: GROUNDING AND POWER INTEGRITY
═══════════════════════════════════════════════════════════════════════════════

Critical Grounding Points:
--------------------------

1. POWER SUPPLY GROUND (Primary Ground Point):
   - USB-C connector GND
   - Connect to large ground plane
   - Star grounding topology

2. DIGITAL GROUND:
   - ESP32 GND pins
   - Logic ICs (CD4066)
   - Digital decoupling capacitors

3. ANALOG GROUND:
   - Si4735 RFGND (Pin 3)
   - Audio path grounds
   - Should connect to digital ground at ONE point only

4. POWER GROUND:
   - MAX98357A GND
   - Power supply filter caps
   - Heavy copper trace or ground plane

5. SHIELD GROUND:
   - Enclosure (if metal)
   - Antenna jack shields


Ground Plane Recommendations:
------------------------------
- Use copper pour on bottom layer (if 2-layer PCB)
- Connect all GND pads to ground plane with vias
- Minimum 4 vias per IC ground pad
- Keep digital and analog grounds separate until one connection point
- Use thick traces (1mm+) for high current paths


Decoupling Capacitor Placement:
--------------------------------
✓ Place as close as possible to IC power pins (< 5mm)
✓ Connect to ground plane with short via
✓ Use 0.1µF for high frequency, 10µF for low frequency
✓ Both values in parallel for best performance


═══════════════════════════════════════════════════════════════════════════════
SECTION 12: ASSEMBLY SEQUENCE
═══════════════════════════════════════════════════════════════════════════════

Recommended Assembly Order:
---------------------------

Phase 1: Power Supply
□ 1. Solder USB-C connector (J1)
□ 2. Solder AMS1117-3.3 (U6)
□ 3. Solder power capacitors (C1, C:wq

