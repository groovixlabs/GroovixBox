/***************************************************
  GlowDial MIDI Controller

  A feature-rich MIDI controller using Teensy MCU with:
  - 16 Rotary Encoders with RGB LED rings
  - 32 Backlit Keys with RGB LEDs
  - ILI9341 TFT Display (320x240)
  - USB MIDI Interface

  Hardware:
  - Teensy 4.1 (or compatible)
  - ILI9341 TFT Display
  - MAX7219 LED Matrix Drivers (x2)
  - 74HC165 Shift Registers
  - WS2812B RGB LEDs

  
 ****************************************************/

#ifdef _GSIMULATOR


#else

#include <FastLED.h>
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"


#include "USBHost_t36.h"

USBHost myusb;
USBHub hub1(myusb);
USBHub hub2(myusb);
USBHub hub3(myusb);
KeyboardController keyboard1(myusb);
KeyboardController keyboard2(myusb);
MIDIDevice_BigBuffer midi1(myusb);


#endif


#include "defines.h"

#include "eeprom_storage.h"

#include "display.h"
#include "settings.h"
#include "notesequence.h"
#include "sequenceplayback.h"
#include "keys.h"
#include "encoderled.h"
#include "serialhandler.h"
#include "usbmidihost.h"
#include "usbhidhost.h"



//==============================================================================
// GLOBAL OBJECTS
//==============================================================================

#ifndef _GSIMULATOR
// TFT Display
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
#endif

// RGB LEDs for Keys
CRGB key_rgb_leds[NUM_KEY_LEDS + 1];

// Encoder LED states (brightness values for MAX7219)
byte encoder_leds[LED_RING_LED_COUNT+100];
byte encoder_status_leds[NUM_ENCODERS+100];

// Encoder state tracking
byte EncodersState[NUM_ENCODERS + 1];
byte EncodersBtnState[NUM_ENCODERS + 1];         // Button state by physical index
byte EncodersBtnState_Logical[NUM_ENCODERS + 1]; // Button state by logical index (for display)
byte p_EncodersBtnState[NUM_ENCODERS + 1];
int  EncoderPosition[NUM_ENCODERS + 10];
int  p_EncoderPosition[NUM_ENCODERS + 10];       // Previous positions for display update detection

// Key state tracking
byte idata_keys[NUM_KEYS + 1];
byte p_idata_keys[NUM_KEYS + 1];

// Encoder shift register data
byte NumRegisters = NUM_ENCODERS / 2;
byte idata[NUM_ENCODERS + 1];
byte p_idata[NUM_ENCODERS + 1];

//==============================================================================
// ENCODER & KEY MAPPING TABLES
//==============================================================================

// Encoder physical to logical mapping
/*
Physical Order

1 3 5 7
0 2 4 6 

9 11 13 15
8 10 12 14

*/
/*
const int EncoderMap[NUM_ENCODERS + 10] = {
    7, 3, 6, 2,      // Physical 0-3 -> Logical
    5, 1, 4, 0,      // Physical 4-7 -> Logical
    15, 11, 14, 10,  // Physical 8-11 -> Logical
    13, 9, 12, 8     // Physical 12-15 -> Logical
};
*/

// Physical index -> Logical index
const int EncoderMap[NUM_ENCODERS] = {
    4, 0, 5, 1,   // Physical 0–3
    6, 2, 7, 3,   // Physical 4–7
    12, 8, 13, 9, // Physical 8–11
    14, 10, 15, 11 // Physical 12–15
};



const int KeyMap[16] = {

/*
Physical Key Order
2 3 4 5    10 11 12 13
1 0 7 6    9 8 15 14
*/

    9, 8, 0, 1, 2, 3, 11, 10,
    13, 12, 4, 5, 6, 7, 15, 14
};


// Key to LED mapping
const int KeyLedMap[16] = {
    //1, 3, 5, 7, 9, 11, 13, 15,
    //0, 2, 4, 6, 8, 10, 12, 14

    
    0,1,2,3,4,5,6,7,
    15,14,13,12,11,10,9,8
   
};

//==============================================================================
// INITIALIZATION FUNCTIONS
//==============================================================================

void InitSerial() {
    Serial.begin(115200);
    delay(100);
    Serial.println("===========================================");
    Serial.println("  GlowDial MIDI Controller Starting...    ");
    Serial.println("===========================================");
}

void InitTFT() {
    Serial.print("Initializing TFT Display... ");

#ifndef _GSIMULATOR    
    tft.begin();

    // Set to landscape mode (rotation 1 or 3)
    // 0 = portrait, 1 = landscape, 2 = portrait flipped, 3 = landscape flipped
    tft.setRotation(3);  // Landscape mode flipped 180 degrees (320x240)
#endif

    // Clear screen and set default colors
    tft.fillScreen(ILI9341_BLACK);
    tft.setTextColor(ILI9341_CYAN);
    tft.setTextSize(2);

    // Display startup message
    tft.setCursor(10, 10);
    tft.println("GlowDial Controller");
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(1);
    tft.setCursor(10, 35);
    tft.println("Initializing peripherals...");

    Serial.println("OK");
}

void InitFastLED() {
    Serial.print("Initializing RGB LEDs... ");

#ifndef _GSIMULATOR    
    FastLED.addLeds<NEOPIXEL, KEY_LED_DATA_PIN>(key_rgb_leds, NUM_KEY_LEDS);
#endif

    FastLED.setBrightness(5);  // 0-255

    // Initialize all LEDs to black (off)
    for (int i = 0; i < NUM_KEY_LEDS; i++) {
        key_rgb_leds[i] = CRGB::Black;
    }
    FastLED.show();

    // Brief startup animation
    for (int i = 0; i < 4; i++) {
        key_rgb_leds[i] = CRGB::Green;
    }
    FastLED.show();
    delay(200);

    Serial.println("OK");
}



void InitShiftRegisters() {
    Serial.print("Initializing Shift Registers... ");

    // Encoder shift register pins
    pinMode(PIN_ENC_LAT, OUTPUT);
    pinMode(PIN_ENC_CLK, OUTPUT);
    pinMode(PIN_ENC_DATAIN, INPUT);
    digitalWrite(PIN_ENC_LAT, HIGH);
    digitalWrite(PIN_ENC_CLK, LOW);

    // Key shift register pins
    pinMode(PIN_KEY_LAT, OUTPUT);
    pinMode(PIN_KEY_CLK, OUTPUT);
    pinMode(PIN_KEY_DATAIN, INPUT);
    digitalWrite(PIN_KEY_LAT, HIGH);
    digitalWrite(PIN_KEY_CLK, LOW);

    pinMode(KEYBOARD_SHIFT_0, INPUT);
    pinMode(KEYBOARD_SHIFT_1, INPUT);
    pinMode(KEYBOARD_SHIFT_2, INPUT);

    Serial.println("OK");
}

void InitEncoders() {
    Serial.print("Initializing Encoders... ");

    // Clear encoder states
    for (int i = 0; i < NUM_ENCODERS; i++) {
        EncodersState[i] = ENC_STATE_NONE;
        EncodersBtnState[i] = 0;
        EncodersBtnState_Logical[i] = 0;
        p_EncodersBtnState[i] = 0;
        EncoderPosition[i] = 0;
        p_EncoderPosition[i] = 0;
    }

    Serial.println("OK");
}

void InitUSBMIDI() {
    Serial.print("Initializing USB MIDI... ");

    // USB MIDI is automatically initialized on Teensy
    // No additional setup required

    Serial.println("OK");
}

void ShowReadyAnimation() {
    // Display ready screen on LCD
    DisplayReadyScreen();

    // Turn all key LEDs green briefly to indicate ready
    for (int i = 0; i < NUM_KEY_LEDS; i++) {
        key_rgb_leds[i] = CRGB::Green;
    }
    FastLED.show();
    delay(500);

    // Turn them off
    for (int i = 0; i < NUM_KEY_LEDS; i++) {
        key_rgb_leds[i] = CRGB::Black;
    }
    FastLED.show();
}

//==============================================================================
// MAX7219 HELPER FUNCTIONS
//==============================================================================
/*
void sendCommandAll(uint8_t reg, uint8_t data) {
    digitalWrite(PIN_MAX7219_CS, LOW);
    for (int i = 0; i < NUM_MAX7219_DEVICES; i++) {
        SPI1.transfer(reg);
        SPI1.transfer(data);
    }
    digitalWrite(PIN_MAX7219_CS, HIGH);
}

void clearAllEncoderLEDs() {
    for (int row = 0; row < 8; row++) {
        digitalWrite(PIN_MAX7219_CS, LOW);
        for (int dev = 0; dev < NUM_MAX7219_DEVICES; dev++) {
            SPI1.transfer(row + 1);
            SPI1.transfer(0x00);
        }
        digitalWrite(PIN_MAX7219_CS, HIGH);
    }
}


void InitMAX7219() {
    Serial.print("Initializing MAX7219 LED Drivers... ");

    pinMode(PIN_MAX7219_CS, OUTPUT);
    digitalWrite(PIN_MAX7219_CS, HIGH);

    // Configure SPI1 pins
    
    //SPI1.setMOSI(26);
    //SPI1.setSCK(27);
    //SPI1.setMISO(39);
    
    

    SPI1.begin();
    #ifndef _GSIMULATOR
    SPI1.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
    #endif

    // Initialize MAX7219 registers
    sendCommandAll(0x0F, 0x00);  // Display test off
    sendCommandAll(0x09, 0x00);  // Decode mode off (matrix mode)
    sendCommandAll(0x0B, 0x07);  // Scan limit (all 8 rows)
    sendCommandAll(0x0A, 0x04);  // Intensity (0x00-0x0F)
    sendCommandAll(0x0C, 0x01);  // Shutdown mode off (normal operation)

    // Clear all LEDs
    clearAllEncoderLEDs();

    Serial.println("OK");
}
*/


//==============================================================================
// ENCODER LED RING UPDATE FUNCTIONS
//==============================================================================

void UpdateEncoderLEDRing(int encoderID, int position) {
    // Clear all level LEDs (0-12) for this encoder
    for (int led = 0; led <= 12; led++) {
        ledRingSetLed(encoderID, led, false);
    }

    // Check encoder mode to determine LED display behavior
    EncoderMode mode = encoderMidiMaps[encoderID].mode;

    if (mode == ENC_MODE_CENTER) {
        // Center mode: LED 6 is center, position 7 is center value (half of 0-13 range)
        const int centerLED = 6;
        const int centerPosition = 7;

        // Always light up the center LED
        ledRingSetLed(encoderID, centerLED, true);

        if (position < centerPosition) {
            // CCW from center: Light up LEDs 5, 4, 3, 2, 1, 0
            int ledsToLight = centerPosition - position;
            for (int i = 1; i <= ledsToLight && (centerLED - i) >= 0; i++) {
                ledRingSetLed(encoderID, centerLED - i, true);
            }
        } else if (position > centerPosition) {
            // CW from center: Light up LEDs 7, 8, 9, 10, 11, 12
            int ledsToLight = position - centerPosition;
            for (int i = 1; i <= ledsToLight && (centerLED + i) <= 12; i++) {
                ledRingSetLed(encoderID, centerLED + i, true);
            }
        }
        // If position == centerPosition, only center LED is lit (already done above)
    } else {
        // Absolute/Relative modes: Original behavior
        // Light up LEDs based on position (position 0 = no LEDs, position 1 = LED 0, etc.)
        // Position 1-13 maps to LEDs 0-12
        if (position > 0) {
            for (int led = 0; led < position && led <= 12; led++) {
                ledRingSetLed(encoderID, led, true);
            }
        }
    }
}

//==============================================================================
// ENCODER READING FUNCTIONS
//==============================================================================

byte read165_encoder() {
    byte incoming = 0;

    // Read each of the 8 bits from shift register
    for (int i = 0; i < 8; i++) {
        incoming <<= 1;
        if (digitalRead(PIN_ENC_DATAIN)) {
            incoming |= 1;
        }
        // Clock pulse
        digitalWrite(PIN_ENC_CLK, HIGH);
        delayMicroseconds(CLOCK_DELAY);
        digitalWrite(PIN_ENC_CLK, LOW);
    }

    return incoming;
}

byte ReadAllEncoders() {
    
    #ifdef _GSIMULATOR
        return 0;
    #endif

    byte changed = 0;

    // Load the current input state into the shift register
    digitalWrite(PIN_ENC_LAT, LOW);
    delayMicroseconds(LATCH_DELAY);
    digitalWrite(PIN_ENC_LAT, HIGH);

    

    // Read all encoder shift registers (8 registers for 16 encoders)
    for (byte i = 0; i < NumRegisters; i++) {
        byte data = read165_encoder();
        p_idata[i] = idata[i];
        idata[i] = data;

        if (idata[i] != p_idata[i]) {
            changed = 1;
        }
    }

    return changed;
}

// Send MIDI message for encoder button press/release
void SendEncoderButtonMIDI(int logicalEncoderID, bool isPress) {
    if (logicalEncoderID < 0 || logicalEncoderID >= NUM_ENCODERS) return;

    EncoderButtonMidiMap &btnMap = encoderButtonMidiMaps[logicalEncoderID];
    ButtonMode mode = btnMap.buttonMode;

    // Handle different button modes
    if (mode == BTN_MODE_NORMAL) {
        // Normal mode: NoteOn on press, NoteOff on release
        switch (btnMap.msgType) {
            case MIDI_MSG_NOTE:
                if (isPress) {
                    usbMIDI.sendNoteOn(btnMap.noteNumber, 127, btnMap.channel);
                } else {
                    usbMIDI.sendNoteOff(btnMap.noteNumber, 0, btnMap.channel);
                }
                break;

            case MIDI_MSG_CC:
                // Send CC value on press, optionally 0 on release
                if (isPress) {
                    usbMIDI.sendControlChange(btnMap.ccNumber, btnMap.ccValue, btnMap.channel);
                } else {
                    usbMIDI.sendControlChange(btnMap.ccNumber, 0, btnMap.channel);
                }
                break;

            case MIDI_MSG_PROGRAM_CHANGE:
                // Program change only on press
                if (isPress) {
                    usbMIDI.sendProgramChange(btnMap.programNumber, btnMap.channel);
                }
                break;

            default:
                break;
        }
    }
    else if (mode == BTN_MODE_HOLD_SINGLE || mode == BTN_MODE_HOLD_MULTIPLE) {
        // Hold modes: Only act on button press (ignore release)
        if (!isPress) return;

        // Toggle the hold state
        bool wasHeld = encoderButtonHoldState[logicalEncoderID];
        encoderButtonHoldState[logicalEncoderID] = !wasHeld;

        // In HOLD_SINGLE mode, release the previously held button
        if (mode == BTN_MODE_HOLD_SINGLE && !wasHeld && lastHeldEncoderButton != -1 && lastHeldEncoderButton != logicalEncoderID) {
            // Release the previous button
            EncoderButtonMidiMap &prevBtnMap = encoderButtonMidiMaps[lastHeldEncoderButton];
            if (prevBtnMap.msgType == MIDI_MSG_NOTE) {
                usbMIDI.sendNoteOff(prevBtnMap.noteNumber, 0, prevBtnMap.channel);
            } else if (prevBtnMap.msgType == MIDI_MSG_CC) {
                usbMIDI.sendControlChange(prevBtnMap.ccNumber, 0, prevBtnMap.channel);
            }
            encoderButtonHoldState[lastHeldEncoderButton] = false;

            // Update LED for previous button (only first 8 encoders have LED rings)
            if (lastHeldEncoderButton < 8) {
                ShowEncoderStatus(lastHeldEncoderButton, 0, false);  // Turn off LED 13
            }
        }

        // Send MIDI message
        if (encoderButtonHoldState[logicalEncoderID]) {
            // Turning on
            if (btnMap.msgType == MIDI_MSG_NOTE) {
                usbMIDI.sendNoteOn(btnMap.noteNumber, 127, btnMap.channel);
            } else if (btnMap.msgType == MIDI_MSG_CC) {
                usbMIDI.sendControlChange(btnMap.ccNumber, btnMap.ccValue, btnMap.channel);
            } else if (btnMap.msgType == MIDI_MSG_PROGRAM_CHANGE) {
                usbMIDI.sendProgramChange(btnMap.programNumber, btnMap.channel);
            }

            // Update LED to indicate held state (use LED 13 - only for encoders 0-7)
            if (logicalEncoderID < 8) {
                ShowEncoderStatus(logicalEncoderID, 0, true);  // Turn on LED 13
            }

            // Update last held button for HOLD_SINGLE mode
            if (mode == BTN_MODE_HOLD_SINGLE) {
                lastHeldEncoderButton = logicalEncoderID;
            }
        } else {
            // Turning off
            if (btnMap.msgType == MIDI_MSG_NOTE) {
                usbMIDI.sendNoteOff(btnMap.noteNumber, 0, btnMap.channel);
            } else if (btnMap.msgType == MIDI_MSG_CC) {
                usbMIDI.sendControlChange(btnMap.ccNumber, 0, btnMap.channel);
            }

            // Update LED to turn off
            if (logicalEncoderID < 8) {
                ShowEncoderStatus(logicalEncoderID, 0, false);  // Turn off LED 13
            }

            // Clear last held button if this was it
            if (mode == BTN_MODE_HOLD_SINGLE && lastHeldEncoderButton == logicalEncoderID) {
                lastHeldEncoderButton = -1;
            }
        }
    }
}

// Send MIDI message based on encoder map configuration
void SendEncoderMIDI(int logicalEncoderID, bool clockwise) {
    EncoderMidiMap &midiMap = encoderMidiMaps[logicalEncoderID];

    // Handle different message types
    switch (midiMap.msgType) {
        case MIDI_MSG_CC: {
            // Calculate value based on encoder mode
            int value = 0;

            switch (midiMap.mode) {
                case ENC_MODE_ABSOLUTE:
                    // Use actual encoder position, scale to min-max range
                    value = map(EncoderPosition[logicalEncoderID], 0, MAX_ENC_POS, midiMap.minValue, midiMap.maxValue);
                    break;

                case ENC_MODE_RELATIVE:
                    // Send relative change (+1 for CW, -1 for CCW)
                    // Using values: 65 for +1, 63 for -1 (common relative CC encoding)
                    value = clockwise ? 65 : 63;
                    break;

                case ENC_MODE_CENTER:
                    // Center at 64, go up/down based on position
                    value = 64 + map(EncoderPosition[logicalEncoderID], 0, MAX_ENC_POS, -64, 63);
                    break;
            }

            // Ensure value is within valid MIDI range
            value = constrain(value, 0, 127);

            usbMIDI.sendControlChange(midiMap.ccNumber, value, midiMap.channel);
            break;
        }

        case MIDI_MSG_NOTE: {
            // Send note on for the appropriate direction
            uint8_t note = clockwise ? midiMap.noteCW : midiMap.noteCCW;
            usbMIDI.sendNoteOn(note, 127, midiMap.channel);

            // Immediately send note off (trigger mode)
            delay(10);
            usbMIDI.sendNoteOff(note, 0, midiMap.channel);
            break;
        }

        case MIDI_MSG_PROGRAM_CHANGE: {
            // Use encoder position to select program
            int program = map(EncoderPosition[logicalEncoderID], 0, MAX_ENC_POS, 0, 127);
            usbMIDI.sendProgramChange(program, midiMap.channel);
            break;
        }

        case MIDI_MSG_PITCH_BEND: {
            // Pitch bend uses 14-bit values (0-16383, center at 8192)
            int bendValue = map(EncoderPosition[logicalEncoderID], 0, MAX_ENC_POS, 0, 16383);
            usbMIDI.sendPitchBend(bendValue, midiMap.channel);
            break;
        }
    }
}

bool HandleEncoderRotation(int encoderIndex,int logicalEncoderID)
{
    bool encoderChanged=false;
    // Check if we're in settings mode and editing a parameter
    if (IsInSettingsMode() && isEditingParameter) {
        // Use encoder for editing parameter values in settings
        if (EncodersState[encoderIndex] == ENC_STATE_CW) {
            MenuNavigateRight();
            Serial.println("Encoder CW: Increasing parameter value");
        } else {
            MenuNavigateLeft();
            Serial.println("Encoder CCW: Decreasing parameter value");
        }
        DisplaySettingsMenu();
        EncodersState[encoderIndex] = ENC_STATE_NONE;
    }
    // Settings mode: Direct encoder-to-parameter mapping (when not in edit mode)
    else if (IsInSettingsMode() && !isEditingParameter &&
                (currentMenuLevel == MENU_ENCODER_MIDI_DETAIL ||
                currentMenuLevel == MENU_ENCODER_BTN_MIDI_DETAIL ||
                currentMenuLevel == MENU_KEYS_MIDI_DETAIL)) {
        // Map encoder rotation directly to parameter adjustment
        bool increment = (EncodersState[encoderIndex] == ENC_STATE_CW);
        AdjustParameterByEncoder(logicalEncoderID, increment);
        DisplaySettingsMenu();
        EncodersState[encoderIndex] = ENC_STATE_NONE;
    }
    // Settings mode: Update encoder/switch status display if viewing them
    else if (IsInSettingsMode() &&
             (currentMenuLevel == MENU_ENCODER_STATUS || currentMenuLevel == MENU_SWITCH_STATUS)) {
        // Update encoder position if in encoder status view
        if (currentMenuLevel == MENU_ENCODER_STATUS) {
            if (EncodersState[encoderIndex] == ENC_STATE_CW) {
                EncoderPosition[logicalEncoderID]++;
                if (EncoderPosition[logicalEncoderID] > MAX_ENC_POS) {
                    EncoderPosition[logicalEncoderID] = MAX_ENC_POS;
                }
                // Update LED ring to show encoder position
                UpdateEncoderLEDRing(logicalEncoderID, EncoderPosition[logicalEncoderID]);
                encoderChanged = true;
            } else if (EncodersState[encoderIndex] == ENC_STATE_CCW) {
                EncoderPosition[logicalEncoderID]--;
                if (EncoderPosition[logicalEncoderID] < 0) {
                    EncoderPosition[logicalEncoderID] = 0;
                }
                // Update LED ring to show encoder position
                UpdateEncoderLEDRing(logicalEncoderID, EncoderPosition[logicalEncoderID]);
                encoderChanged = true;
            }
        }
        // Update the status display
        DisplaySettingsMenu();
        EncodersState[encoderIndex] = ENC_STATE_NONE;
    }
    // Normal encoder operation
    else if (!IsInSettingsMode()) {
        if (EncodersState[encoderIndex] == ENC_STATE_CW) {
            EncoderPosition[logicalEncoderID]++;
            if (EncoderPosition[logicalEncoderID] > MAX_ENC_POS) {
                EncoderPosition[logicalEncoderID] = MAX_ENC_POS;
            }
            
            Serial.print("Encoder PH:");
            Serial.print(encoderIndex);
            Serial.print("  Mapped:");
            Serial.print(logicalEncoderID);
            Serial.print(": CW -> ");
            Serial.println(EncoderPosition[logicalEncoderID]);

            // Update LED ring to show encoder position
            UpdateEncoderLEDRing(logicalEncoderID, EncoderPosition[logicalEncoderID]);

            // Send MIDI based on configured map
            SendEncoderMIDI(logicalEncoderID, true);  // true = clockwise

            encoderChanged = true;
        } else {
            EncoderPosition[logicalEncoderID]--;
            if (EncoderPosition[logicalEncoderID] < 0) {
                EncoderPosition[logicalEncoderID] = 0;
            }
            Serial.print("Encoder PH:");
            Serial.print(encoderIndex);
            Serial.print("  Mapped:");
            Serial.print(logicalEncoderID);
            Serial.print(": CCW -> ");
            Serial.println(EncoderPosition[logicalEncoderID]);

            // Update LED ring to show encoder position
            UpdateEncoderLEDRing(logicalEncoderID, EncoderPosition[logicalEncoderID]);

            // Send MIDI based on configured map
            SendEncoderMIDI(logicalEncoderID, false);  // false = counter-clockwise

            encoderChanged = true;
        }

        EncodersState[encoderIndex] = ENC_STATE_NONE;
    } else {
        // In settings mode but not editing - ignore encoder rotation
        EncodersState[encoderIndex] = ENC_STATE_NONE;
    }

    return encoderChanged;
}

bool ProcessEncoders() {
    if (!ReadAllEncoders()) {
        return false;  // No changes detected
    }

    bool encoderChanged = false;

    for (byte i = 0; i < NumRegisters; i++) {
        byte currentData = idata[i];
        byte previousData = p_idata[i];

        int physicalEncoderIndex = i * 2;  // Two encoders per register

        // Process both encoders in this register
        for (byte enc = 0; enc < 2; enc++) {
            int encoderIndex = physicalEncoderIndex + enc;
            int logicalEncoderID = EncoderMap[encoderIndex];

            // Select the correct encoder pins based on which encoder (bottom or top)
            byte btnPin = (enc == 0) ? ENC_BOT_BTN : ENC_TOP_BTN;
            byte ioaPin = (enc == 0) ? ENC_BOT_IOA : ENC_TOP_IOA;
            byte iobPin = (enc == 0) ? ENC_BOT_IOB : ENC_TOP_IOB;

            // Check for button press (transition from 1 to 0)
            if (((previousData >> btnPin) & 1) == 1 && ((currentData >> btnPin) & 1) == 0) {
                EncodersBtnState[encoderIndex] = ENC_BTN_PRESS;
                EncodersBtnState_Logical[logicalEncoderID] = ENC_BTN_PRESS;
                Serial.print("DEBUG: Encoder ");
                Serial.print(logicalEncoderID);
                Serial.println(" button press detected");
            }

            // Check for button release (transition from 0 to 1)
            if (((previousData >> btnPin) & 1) == 0 && ((currentData >> btnPin) & 1) == 1) {
                EncodersBtnState[encoderIndex] = ENC_BTN_RELEASE;
                EncodersBtnState_Logical[logicalEncoderID] = ENC_BTN_RELEASE;
                Serial.print("DEBUG: Encoder ");
                Serial.print(logicalEncoderID);
                Serial.println(" button release detected");
            }

            // Check for clockwise rotation
            if (((previousData >> iobPin) & 1) == 0 && ((currentData >> iobPin) & 1) == 0 &&
                ((previousData >> ioaPin) & 1) == 1 && ((currentData >> ioaPin) & 1) == 0) {
                EncodersState[encoderIndex] = ENC_STATE_CW;
            }

            // Check for counter-clockwise rotation
            if (((previousData >> iobPin) & 1) == 1 && ((currentData >> iobPin) & 1) == 0 &&
                ((previousData >> ioaPin) & 1) == 0 && ((currentData >> ioaPin) & 1) == 0) {
                EncodersState[encoderIndex] = ENC_STATE_CCW;
            }

            // Handle encoder rotation events
            if (EncodersState[encoderIndex] != ENC_STATE_NONE) {
             encoderChanged=HandleEncoderRotation(encoderIndex,logicalEncoderID);   
            }

            // Handle encoder button events
            if (EncodersBtnState[encoderIndex] == ENC_BTN_PRESS &&
                EncodersBtnState[encoderIndex] != p_EncodersBtnState[encoderIndex]) {
                Serial.print("Encoder ");
                Serial.print(logicalEncoderID);
                Serial.println(": BUTTON PRESS");

                encoder_status_leds[logicalEncoderID]++ ;
                if (encoder_status_leds[logicalEncoderID]==4) encoder_status_leds[logicalEncoderID]=0;
                //ShowEncoderStatus(logicalEncoderID,1,encoder_status_leds[logicalEncoderID]);
                SetEncoderStatusLEDs(logicalEncoderID, 
                    encoder_status_leds[logicalEncoderID]==3, 
                    encoder_status_leds[logicalEncoderID]==2,
                    encoder_status_leds[logicalEncoderID]==1);

                // Send MIDI based on configured button map
                SendEncoderButtonMIDI(logicalEncoderID, true);

                // Update switch status display if viewing it
                if (IsInSettingsMode() && currentMenuLevel == MENU_SWITCH_STATUS) {
                    DisplaySwitchStatus();
                }
            }

            if (EncodersBtnState[encoderIndex] == ENC_BTN_RELEASE) {
                Serial.print("Encoder ");
                Serial.print(logicalEncoderID);
                Serial.println(": BUTTON RELEASE");

                // Send MIDI based on configured button map
                SendEncoderButtonMIDI(logicalEncoderID, false);

                EncodersBtnState[encoderIndex] = ENC_STATE_NONE;
                EncodersBtnState_Logical[logicalEncoderID] = ENC_STATE_NONE;

                // Update switch status display if viewing it
                if (IsInSettingsMode() && currentMenuLevel == MENU_SWITCH_STATUS) {
                    DisplaySwitchStatus();
                }
            }

            p_EncodersBtnState[encoderIndex] = EncodersBtnState[encoderIndex];
        }
    }

    return encoderChanged;
}

//==============================================================================
// KEY READING FUNCTIONS
//==============================================================================

byte read165_keys() {
    byte incoming = 0;

    // Read each of the 8 bits from shift register
    for (int i = 0; i < 8; i++) {
        incoming <<= 1;
        if (digitalRead(PIN_KEY_DATAIN)) {
            incoming |= 1;
        }
        // Clock pulse
        digitalWrite(PIN_KEY_CLK, HIGH);
        delayMicroseconds(CLOCK_DELAY);
        digitalWrite(PIN_KEY_CLK, LOW);
    }

    return incoming;
}

byte ReadAllKeys() {
    #ifdef _GSIMULATOR
        return 0 ;
    #endif

    byte changed = 0;

    // Load the current input state into the shift register
    digitalWrite(PIN_KEY_LAT, LOW);
    delayMicroseconds(LATCH_DELAY);
    digitalWrite(PIN_KEY_LAT, HIGH);

    

    // Read all shift registers (4 registers for 32 keys)
    for (byte i = 0; i < SHIFT_REG_COUNT; i++) {
        byte data = read165_keys();
        p_idata_keys[i] = idata_keys[i];
        idata_keys[i] = data;

        if (idata_keys[i] != p_idata_keys[i]) {
            changed = 1;
        }
    }

    return changed;
}

// Key event handlers (OnKey Press, OnKey Release, ToggleKey LED) are now in keys.h

void ProcessKeys() {
    if (ReadAllKeys()) {
        // Iterate through all shift registers
        for (byte i = 0; i < SHIFT_REG_COUNT; i++) {
            byte currentData = idata_keys[i];
            byte previousData = p_idata_keys[i];

            // Check each bit in the register
            for (byte j = 0; j < 8; j++) {
                int physicalKeyIndex = (i * 8) + j;

                
                // Map physical key to logical key
                int logicalKeyID = KeyMap[physicalKeyIndex % 16];

                if (physicalKeyIndex >= 16) {
                    logicalKeyID += 16;  // Second set of keys
                }
                logicalKeyID = logicalKeyID % NUM_KEYS;  // Safety check


                // Detect key press (transition from 1 to 0)
                if (((previousData & 0x01) == 1) && ((currentData & 0x01) == 0)) {
                    OnKeyPress(logicalKeyID ,digitalRead(KEYBOARD_SHIFT_0)==0
                                            ,digitalRead(KEYBOARD_SHIFT_1)==0
                                            ,digitalRead(KEYBOARD_SHIFT_2)==0
                                        );


                //Serial.print("PHY:");Serial.print(physicalKeyIndex%16);
                //Serial.print("LOG:");Serial.println(logicalKeyID%16);

                Serial.print("Shift 0:");Serial.println(digitalRead(KEYBOARD_SHIFT_0));
                Serial.print("Shift 1:");Serial.println(digitalRead(KEYBOARD_SHIFT_1));
                Serial.print("Shift 2:");Serial.println(digitalRead(KEYBOARD_SHIFT_2));

                }
                // Detect key release (transition from 0 to 1)
                else if (((previousData & 0x01) == 0) && ((currentData & 0x01) == 1)) {
                    OnKeyRelease(logicalKeyID
                                                ,digitalRead(KEYBOARD_SHIFT_0)==0
                                                ,digitalRead(KEYBOARD_SHIFT_1)==0
                                                ,digitalRead(KEYBOARD_SHIFT_2)==0
                                            );
                }

                // Shift to next bit
                currentData >>= 1;
                previousData >>= 1;
            }
        }
    }
}


void InitUSBHost()
{
#ifndef _GSIMULATOR        
    myusb.begin();
	keyboard1.attachPress(OnPress);
	keyboard1.attachRawPress(OnRawPress);
	keyboard1.attachRawRelease(OnRawRelease);
	keyboard2.attachPress(OnPress);
	midi1.setHandleNoteOff(OnNoteOff);
	midi1.setHandleNoteOn(OnNoteOn);
	midi1.setHandleControlChange(OnControlChange);
#endif
}

//==============================================================================
// MAIN SETUP
//==============================================================================

void setup() {
    // Initialize all peripherals in order
    InitSerial();
    InitFastLED();
    //InitMAX7219();
    InitEncoderLED();
    InitShiftRegisters();
    InitEncoders();
    InitEncoderLED();
    LoadAllMidiMapsFromEEPROM();  // Load MIDI mappings from EEPROM (or initialize defaults)
    InitializeNoteSequences();    // Initialize note sequence storage
    InitializeSequencePlayback(); // Initialize sequence playback engine
    InitializeMidiRelay();        // Initialize MIDI relay with scale mapping and quantization
    InitUSBMIDI();
    InitUSBHost();

    InitTFT();

    // Display ready screen with LED animation
    ShowReadyAnimation();

    Serial.println("===========================================");
    Serial.println("  System Ready - Waiting for input...     ");
    Serial.println("===========================================");

    // Initialize LED rings to show initial encoder positions
    for (int i = 0; i < NUM_ENCODERS; i++) {
        UpdateEncoderLEDRing(i, EncoderPosition[i]);
    }

    // Home screen will remain showing "READY" message
    // Encoder/switch status can be viewed in Settings -> General menu
    delay(1000);
}

void ProcessSerialData()
{
if (Serial.available()) 
	{
		for(;;)
		{
			if (Serial.available())
			{
				ReadSerialData();
			}
			else
			{
				break;
			}
		}
	}    
}

//==============================================================================
// MAIN LOOP
//==============================================================================

void loop() {

    static uint32_t last1ms = 0;
    uint32_t now = millis();

#ifdef _GSIMULATOR    
    delay(5);
#else
	myusb.Task();
	midi1.read();
#endif

// Run these every 1 ms
    if ((now - last1ms) >= 1)
    {
        last1ms = now;
        // Main processing loop
        ProcessEncoders();  // Now updates display internally for each changed encoder
        ProcessKeys();
        UpdateSequencePlaybacks();      // Update active sequence playbacks
        ProcessScheduledNoteOffs();     // Process scheduled note-off events
        ProcessMidiRelay(); // Process quantized MIDI relay events
        ProcessSerialData();


        //delay(1);
    }
}
