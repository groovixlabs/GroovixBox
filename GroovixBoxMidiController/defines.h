#ifndef DEFINES_H
#define DEFINES_H

//==============================================================================
// CONSTANTS (needed by both main and display)
//==============================================================================

// LED Rings arround the encoder.
#define LED_RING_COUNT          16
#define LED_RING_LEDS_PER_RING  16
#define LED_RING_LED_COUNT      (LED_RING_COUNT*LED_RING_LEDS_PER_RING)
#define LED_RING_NUM_DEVICES    (LED_RING_COUNT/4) // 2 MAX7219 in series



//==============================================================================
// ENCODER & KEY STATE DEFINITIONS
//==============================================================================

//==============================================================================
// Rotary Encoders

#define NUM_ENCODERS        16
#define MAX_ENC_POS         13

// Encoder Pins (on shift registers)
#define ENC_BOT_BTN         6
#define ENC_BOT_IOA         4
#define ENC_BOT_IOB         5

#define ENC_TOP_BTN         1
#define ENC_TOP_IOA         2
#define ENC_TOP_IOB         3

//********************
// Encoder States.

#define ENC_STATE_NONE      0
#define ENC_STATE_CW        1
#define ENC_STATE_CCW       2

#define ENC_BTN_PRESS       1
#define ENC_BTN_RELEASE     2


//==============================================================================
// Key Matrix

#define NUM_KEY_LEDS        32
#define NUM_KEYS            32
#define SHIFT_REG_COUNT     (NUM_KEYS/8)

// Timing
#define LATCH_DELAY         5
#define CLOCK_DELAY         3

//==============================================================================
// HARDWARE PIN DEFINITIONS
//==============================================================================

// Shift Key
#define KEYBOARD_SHIFT_0    4
#define KEYBOARD_SHIFT_1    5
#define KEYBOARD_SHIFT_2    34


// TFT Display Pins
#define TFT_DC              9
#define TFT_CS              10

// Key RGB LED Strip
#define KEY_LED_DATA_PIN    18 // 32


// Shift Register Pins - Keys
#define PIN_KEY_LAT         36 // 30   // SH/LD (Latch)
#define PIN_KEY_CLK         33 // 31   // Clock
#define PIN_KEY_DATAIN      2  // 8    // Serial Data In (Q7)

// Shift Register Pins - Encoders
#define PIN_ENC_LAT         31 // 28   // Latch
#define PIN_ENC_CLK         32 // 29   // Clock
#define PIN_ENC_DATAIN      3  // 7    // Serial Data In

// MAX7219 LED Matrix Pins (SPI1)
#define LED_RING_PIN_CS     30 //25

//==============================================================================


//NOT USED
//#define NUM_MAX7219_DEVICES 2
//#define PIN_MAX7219_CS      30 //28

//==============================================================================
// MIDI CONFIGURATION
//==============================================================================

const int MIDI_CHANNEL_ENCODERS = 1;
const int MIDI_CHANNEL_KEYS     = 2;
const int MIDI_CC_START         = 12;

//C2: Two octaves below Middle C is MIDI note 36. 
//C3 is one octave below Middle C (MIDI note 48).
//C4 is generally considered Middle C (MIDI note 60).
const int MIDI_NOTE_START_C2       = 36;
const int MIDI_NOTE_START_C3       = 48;
const int MIDI_NOTE_START_C4       = 60;


void ResetEEPROMToDefaults();
void SaveEncoderMidiMap(int encoderIndex);
void SaveEncoderButtonMidiMap(int encoderIndex);
void SaveKeyMidiMap(int encoderIndex);
void SaveQuantizeSettings();
//==============================================================================
// EXTERNAL FUNCTION DECLARATIONS
//==============================================================================

void UpdateEncoderDisplay();  // From display.h
void DisplayReadyScreen();    // From display.h
void InitLabelGrid();
void DisplayLabelGrid();
extern bool labelGridActive;


//==============================================================================
// EXTERNAL REFERENCES
//==============================================================================

// These variables are defined in the main .ino file
extern int EncoderPosition[];
extern byte EncodersBtnState_Logical[];  // Button state by logical ID
extern const int MIDI_CHANNEL_ENCODERS;
extern const int MIDI_CC_START;


//==============================================================================
// KEY DEFINITIONS (1-32)
//==============================================================================

#define GKEYS_PATTERN   7
#define GKEYS_TRACK     6
#define GKEYS_SCENE     5
#define GKEYS_SONG      4

#define GKEYS_PROPS     3
#define GKEYS_ADD       2
#define GKEYS_COPY      1
#define GKEYS_PLAY      0

#define GKEYS_BANK_A    15
#define GKEYS_BANK_B    14
#define GKEYS_BANK_C    13
#define GKEYS_BANK_D    12

#define GKEYS_SETTINGS  11
#define GKEYS_DEL       10
#define GKEYS_PASTE     9
#define GKEYS_MUTE      8


// Bottom Set LOGICAL+16
#define GKEYS_BACK      0+16
#define GKEYS_UP        1+16
#define GKEYS_ENTER     2+16
#define GKEYS_PGUP      3+16

#define GKEYS_KEY_1     4+16
#define GKEYS_KEY_2     5+16
#define GKEYS_KEY_3     6+16
#define GKEYS_KEY_4     7+16


#define GKEYS_LEFT      8+16
#define GKEYS_DOWN      9+16
#define GKEYS_RIGHT     10+16
#define GKEYS_PGDN      11+16


#define GKEYS_KEY_5     12+16
#define GKEYS_KEY_6     13+16
#define GKEYS_KEY_7     14+16
#define GKEYS_KEY_8     15+16



//==============================================================================
// EXTERNAL REFERENCES
//==============================================================================

extern CRGB key_rgb_leds[];
extern const int KeyLedMap[];
extern const int MIDI_CHANNEL_KEYS;

#endif