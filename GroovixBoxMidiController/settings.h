/***************************************************
  GlowDial MIDI Controller - Settings Menu

  This file contains the settings menu system including
  navigation, display, and configuration options.

  MIT License
 ****************************************************/

#ifndef SETTINGS_H
#define SETTINGS_H

#ifndef _GSIMULATOR
#include "Adafruit_ILI9341.h"
extern Adafruit_ILI9341 tft;
#endif
//==============================================================================
// EXTERNAL REFERENCES
//==============================================================================


//==============================================================================
// SETTINGS MODE STATE
//==============================================================================

enum DisplayMode {
    MODE_NORMAL,
    MODE_SETTINGS
};

DisplayMode currentDisplayMode = MODE_NORMAL;

//==============================================================================
// PLAY MODE STATE
//==============================================================================

bool isPlayMode = false;  // Global play/stop state

//==============================================================================
// MIDI MAPPING DATA STRUCTURES
//==============================================================================

enum MidiMessageType {
    MIDI_MSG_CC,
    MIDI_MSG_NOTE,
    MIDI_MSG_PROGRAM_CHANGE,
    MIDI_MSG_PITCH_BEND,
    MIDI_MSG_NOTE_SEQUENCE
};

enum EncoderMode {
    ENC_MODE_ABSOLUTE,
    ENC_MODE_RELATIVE,
    ENC_MODE_CENTER
};

enum ButtonMode {
    BTN_MODE_NORMAL,         // Standard: NoteOn on press, NoteOff on release
    BTN_MODE_HOLD_SINGLE,    // Toggle: Only one note can be held at a time
    BTN_MODE_HOLD_MULTIPLE   // Toggle: Multiple notes can be held
};

enum MusicalScale {
    SCALE_CHROMATIC,
    SCALE_C_MAJOR,
    SCALE_C_MINOR,
    SCALE_C_DORIAN,
    SCALE_C_PHRYGIAN,
    SCALE_C_LYDIAN,
    SCALE_C_MIXOLYDIAN,
    SCALE_C_LOCRIAN,
    SCALE_D_MAJOR,
    SCALE_D_MINOR,
    SCALE_E_MAJOR,
    SCALE_E_MINOR,
    SCALE_F_MAJOR,
    SCALE_F_MINOR,
    SCALE_G_MAJOR,
    SCALE_G_MINOR,
    SCALE_A_MAJOR,
    SCALE_A_MINOR,
    SCALE_B_MAJOR,
    SCALE_B_MINOR
};

enum NoteQuantization {
    QUANT_1_32,   // 1/32 note
    QUANT_1_16,   // 1/16 note
    QUANT_1_8,    // 1/8 note
    QUANT_1_4,    // 1/4 note
    QUANT_1_2,    // 1/2 note
    QUANT_1       // Whole note
};

struct EncoderMidiMap {
    uint8_t port;              // MIDI port (0-15)
    uint8_t channel;           // MIDI channel (1-16)
    MidiMessageType msgType;   // Message type
    uint8_t ccNumber;          // CC number (0-127) or base note
    EncoderMode mode;          // Encoder mode
    uint8_t minValue;          // Minimum value (0-127)
    uint8_t maxValue;          // Maximum value (0-127)
    uint8_t noteCW;            // Note for clockwise (in NOTE mode)
    uint8_t noteCCW;           // Note for counter-clockwise (in NOTE mode)
};

// Encoder button MIDI mapping
struct EncoderButtonMidiMap {
    uint8_t port;              // MIDI port (0-15)
    uint8_t channel;           // MIDI channel (1-16)
    MidiMessageType msgType;   // Message type (CC, Note, or Program Change)
    uint8_t noteNumber;        // Note number (0-127) for NOTE mode
    uint8_t ccNumber;          // CC number (0-127) for CC mode
    uint8_t ccValue;           // CC value to send (0-127) for CC mode
    uint8_t programNumber;     // Program number (0-127) for PC mode
    ButtonMode buttonMode;     // Button mode (Normal, Hold Single, Hold Multiple)
};

// Key MIDI mapping (supports separate press and release)
struct KeyMidiMap {
    // Press configuration
    uint8_t pressPort;
    uint8_t pressChannel;
    MidiMessageType pressMsgType;
    uint8_t pressNoteNumber;
    uint8_t pressCcNumber;
    uint8_t pressCcValue;
    uint8_t pressProgramNumber;

    // Release configuration
    uint8_t releasePort;
    uint8_t releaseChannel;
    MidiMessageType releaseMsgType;
    uint8_t releaseNoteNumber;
    uint8_t releaseCcNumber;
    uint8_t releaseCcValue;
    uint8_t releaseProgramNumber;

    // Button mode
    ButtonMode buttonMode;     // Button mode (Normal, Hold Single, Hold Multiple)

    // Note sequence assignment
    uint8_t sequenceIndex;     // Index into noteSequences array (0-15, or 255 = no sequence)
};

// Default MIDI mapping for all encoders
EncoderMidiMap encoderMidiMaps[NUM_ENCODERS];

// Default MIDI mapping for all encoder buttons
EncoderButtonMidiMap encoderButtonMidiMaps[NUM_ENCODERS];

// Default MIDI mapping for all keys (32 keys)
#define NUM_KEYS 32
KeyMidiMap keyMidiMaps[NUM_KEYS];

// Hold state tracking for buttons and keys
bool encoderButtonHoldState[NUM_ENCODERS] = {false};  // Track which encoder buttons are held
bool keyHoldState[NUM_KEYS] = {false};                // Track which keys are held
int lastHeldEncoderButton = -1;                       // Last held encoder button for HOLD_SINGLE mode
int lastHeldKey = -1;                                 // Last held key for HOLD_SINGLE mode

// MIDI Quantize settings
bool globalQuantizeEnabled = true;                    // Global quantize enable/disable
uint16_t globalBPM = 120;                             // Global BPM (default 120)
MusicalScale globalScale = SCALE_CHROMATIC;           // Global musical scale
NoteQuantization globalQuantization = QUANT_1_16;     // Global note quantization

//==============================================================================
// MENU STRUCTURE
//==============================================================================

enum MenuLevel {
    MENU_TOP_LEVEL,
    MENU_GENERAL,
    MENU_GENERAL_LIST,
    MENU_ENCODER_STATUS,
    MENU_SWITCH_STATUS,
    MENU_MIDI_QUANTIZE_LIST,
    MENU_MIDI_QUANTIZE_ENABLE,
    MENU_MIDI_QUANTIZE_BPM,
    MENU_MIDI_QUANTIZE_SCALE,
    MENU_MIDI_QUANTIZE_QUANT,
    MENU_ENCODER_MIDI,
    MENU_ENCODER_MIDI_LIST,
    MENU_ENCODER_MIDI_DETAIL,
    MENU_ENCODER_BTN_MIDI_LIST,
    MENU_ENCODER_BTN_MIDI_DETAIL,
    MENU_KEYS_MIDI,
    MENU_KEYS_MIDI_LIST,
    MENU_KEYS_MIDI_DETAIL,
    MENU_ADVANCED,
    MENU_ADVANCED_LIST,
    MENU_FACTORY_RESET_CONFIRM,
    MENU_REBOOT_CONFIRM,
};

const char* topLevelMenuItems[] = {
    "General",
    "Midi Quantize",
    "Encoder MIDI Map",
    "Encoder Btn MIDI Map",
    "Keys MIDI Map",
    "Advanced"
};

const int topLevelMenuCount = 6;

const char* generalMenuItems[] = {
    "Encoder Status",
    "Switch Status"
};

const int generalMenuCount = 2;

const char* midiQuantizeMenuItems[] = {
    "Quantize",
    "BPM",
    "Scale",
    "Quantization"
};

const int midiQuantizeMenuCount = 4;

const char* advancedMenuItems[] = {
    "Reboot",
    "Factory Reset"
};

const int advancedMenuCount = 2;

MenuLevel currentMenuLevel = MENU_TOP_LEVEL;
int currentMenuItem = 0;

// Encoder MIDI mapping state
int selectedEncoderIndex = 0;       // Which encoder (0-15) in list view
int selectedDetailParameter = 0;    // Which parameter in detail view
bool isEditingParameter = false;    // Whether we're actively editing a value

// Key MIDI mapping state
int selectedKeyIndex = 0;           // Which key (0-31) in list view
bool isConfiguringPress = true;     // true = press, false = release

// Parameter indices for detail view
enum EncoderDetailParam {
    PARAM_PORT,
    PARAM_CHANNEL,
    PARAM_MSG_TYPE,
    PARAM_CC_NUMBER,
    PARAM_MODE,
    PARAM_MIN_VALUE,
    PARAM_MAX_VALUE,
    PARAM_NOTE_CW,       // Only for NOTE mode
    PARAM_NOTE_CCW,      // Only for NOTE mode
    PARAM_COUNT
};

// Parameter indices for encoder button detail view
enum EncoderButtonDetailParam {
    BTN_PARAM_PORT,
    BTN_PARAM_CHANNEL,
    BTN_PARAM_MSG_TYPE,
    BTN_PARAM_NOTE_NUMBER,   // For NOTE mode
    BTN_PARAM_CC_NUMBER,     // For CC mode
    BTN_PARAM_CC_VALUE,      // For CC mode
    BTN_PARAM_PROGRAM_NUMBER, // For PC mode
    BTN_PARAM_COUNT
};

// Parameter indices for key detail view (same structure as button params)
enum KeyDetailParam {
    KEY_PARAM_PORT,
    KEY_PARAM_CHANNEL,
    KEY_PARAM_MSG_TYPE,
    KEY_PARAM_NOTE_NUMBER,
    KEY_PARAM_CC_NUMBER,
    KEY_PARAM_CC_VALUE,
    KEY_PARAM_PROGRAM_NUMBER,
    KEY_PARAM_COUNT
};

//==============================================================================
// MIDI MAP INITIALIZATION
//==============================================================================

void InitializeEncoderMidiMaps() {
    for (int i = 0; i < NUM_ENCODERS; i++) {
        encoderMidiMaps[i].port = 0;
        encoderMidiMaps[i].channel = 1;
        encoderMidiMaps[i].msgType = MIDI_MSG_CC;
        encoderMidiMaps[i].ccNumber = MIDI_CC_START + i;
        encoderMidiMaps[i].mode = ENC_MODE_ABSOLUTE;
        encoderMidiMaps[i].minValue = 0;
        encoderMidiMaps[i].maxValue = 127;
        encoderMidiMaps[i].noteCW = MIDI_NOTE_START_C4 + i;   // C4 + offset
        encoderMidiMaps[i].noteCCW = MIDI_NOTE_START_C3 + i;  // C3 + offset
    }
}

void InitializeEncoderButtonMidiMaps() {
    for (int i = 0; i < NUM_ENCODERS; i++) {
        encoderButtonMidiMaps[i].port = 0;
        encoderButtonMidiMaps[i].channel = 1;
        encoderButtonMidiMaps[i].msgType = MIDI_MSG_NOTE;
        encoderButtonMidiMaps[i].noteNumber = MIDI_NOTE_START_C2 + i;
        encoderButtonMidiMaps[i].ccNumber = i;
        encoderButtonMidiMaps[i].ccValue = 127;
        encoderButtonMidiMaps[i].programNumber = i;
        encoderButtonMidiMaps[i].buttonMode = BTN_MODE_NORMAL;  // Default to normal mode
    }
}

void InitializeKeyMidiMaps() {
    for (int i = 0; i < NUM_KEYS; i++) {
        // Press configuration - default to Note mode
        keyMidiMaps[i].pressPort = 0;
        keyMidiMaps[i].pressChannel = 2;  // MIDI_CHANNEL_KEYS
        keyMidiMaps[i].pressMsgType = MIDI_MSG_NOTE;
        keyMidiMaps[i].pressNoteNumber = MIDI_NOTE_START_C2 + i;  // C4 + offset
        keyMidiMaps[i].pressCcNumber = i;
        keyMidiMaps[i].pressCcValue = 127;
        keyMidiMaps[i].pressProgramNumber = i;

        // Release configuration - default to Note Off
        keyMidiMaps[i].releasePort = 0;
        keyMidiMaps[i].releaseChannel = 2;  // MIDI_CHANNEL_KEYS
        keyMidiMaps[i].releaseMsgType = MIDI_MSG_NOTE;
        keyMidiMaps[i].releaseNoteNumber = MIDI_NOTE_START_C2 + i;  // Same note
        keyMidiMaps[i].releaseCcNumber = i;
        keyMidiMaps[i].releaseCcValue = 0;  // CC value 0 for release
        keyMidiMaps[i].releaseProgramNumber = i;

        // Button mode
        keyMidiMaps[i].buttonMode = BTN_MODE_NORMAL;  // Default to normal mode

        // No sequence assigned by default
        keyMidiMaps[i].sequenceIndex = 255;  // NO_SEQUENCE_ASSIGNED
    }
}

//==============================================================================
// SETTINGS MODE FUNCTIONS
//==============================================================================

bool IsInSettingsMode() {
    return currentDisplayMode == MODE_SETTINGS;
}

void ExitSettingsMode() {
    currentDisplayMode = MODE_NORMAL;
    currentMenuLevel = MENU_TOP_LEVEL;
    currentMenuItem = 0;

    Serial.println("=== Exiting Settings Mode ===");
}


void EnterSettingsMode() {

    currentDisplayMode = MODE_SETTINGS;
    currentMenuLevel = MENU_TOP_LEVEL;
    currentMenuItem = 0;

    Serial.println("=== Entering Settings Mode ===");
}


void doReboot() {
#ifndef _GSIMULATOR
    USB1_USBCMD = 0; // disconnect USB
    delay(50);       // enough time for USB hubs/ports to detect disconnect
    SCB_AIRCR = 0x05FA0004;
#endif
}

//==============================================================================
// PLAY MODE FUNCTIONS
//==============================================================================

void EnterPlayMode() {
    isPlayMode = true;

    // Reset quantization reference time when entering play mode
    // The first note will establish the grid (set to 0 to indicate "waiting for first note")
    extern unsigned long quantizationStartTime;
    quantizationStartTime = 0;

    Serial.println("=== Play Mode: STARTED ===");
    Serial.println("=== Waiting for first MIDI note to establish quantization grid ===");
}

void ExitPlayMode() {
    isPlayMode = false;
    Serial.println("=== Play Mode: STOPPED ===");
}

void TogglePlayMode() {
    if (isPlayMode) {
        ExitPlayMode();
    } else {
        EnterPlayMode();
    }
}

bool IsInPlayMode() {
    return isPlayMode;
}

//==============================================================================
// MENU NAVIGATION
//==============================================================================

void MenuNavigateUp() {
    if (currentMenuLevel == MENU_TOP_LEVEL) {
        currentMenuItem--;
        if (currentMenuItem < 0) {
            currentMenuItem = topLevelMenuCount - 1;
        }
        Serial.print("Menu: Selected ");
        Serial.println(topLevelMenuItems[currentMenuItem]);
    }
    else if (currentMenuLevel == MENU_GENERAL_LIST) {
        currentMenuItem--;
        if (currentMenuItem < 0) {
            currentMenuItem = generalMenuCount - 1;
        }
        Serial.print("General Menu: Selected ");
        Serial.println(generalMenuItems[currentMenuItem]);
    }
    else if (currentMenuLevel == MENU_MIDI_QUANTIZE_LIST) {
        currentMenuItem--;
        if (currentMenuItem < 0) {
            currentMenuItem = midiQuantizeMenuCount - 1;
        }
        Serial.print("MIDI Quantize Menu: Selected ");
        Serial.println(midiQuantizeMenuItems[currentMenuItem]);
    }
    else if (currentMenuLevel == MENU_ADVANCED_LIST) {
        currentMenuItem--;
        if (currentMenuItem < 0) {
            currentMenuItem = advancedMenuCount - 1;
        }
        Serial.print("Advanced Menu: Selected ");
        Serial.println(advancedMenuItems[currentMenuItem]);
    }
    else if (currentMenuLevel == MENU_ENCODER_MIDI_LIST || currentMenuLevel == MENU_ENCODER_BTN_MIDI_LIST) {
        selectedEncoderIndex--;
        if (selectedEncoderIndex < 0) {
            selectedEncoderIndex = NUM_ENCODERS - 1;
        }
        Serial.print("Encoder List: Selected E");
        Serial.println(selectedEncoderIndex);
    }
    else if (currentMenuLevel == MENU_KEYS_MIDI_LIST) {
        selectedKeyIndex--;
        if (selectedKeyIndex < 0) {
            selectedKeyIndex = NUM_KEYS - 1;
        }
        Serial.print("Key List: Selected K");
        Serial.println(selectedKeyIndex);
    }
    else if (currentMenuLevel == MENU_ENCODER_MIDI_DETAIL && !isEditingParameter) {
        selectedDetailParameter--;

        // Skip Note CW/CCW params if not in NOTE mode
        EncoderMidiMap &map = encoderMidiMaps[selectedEncoderIndex];
        if (map.msgType != MIDI_MSG_NOTE) {
            if (selectedDetailParameter == PARAM_NOTE_CCW) {
                selectedDetailParameter = PARAM_MAX_VALUE;
            } else if (selectedDetailParameter == PARAM_NOTE_CW) {
                selectedDetailParameter = PARAM_MAX_VALUE;
            }
        }

        if (selectedDetailParameter < 0) {
            selectedDetailParameter = (map.msgType == MIDI_MSG_NOTE) ? PARAM_NOTE_CCW : PARAM_MAX_VALUE;
        }
    }
    else if (currentMenuLevel == MENU_ENCODER_BTN_MIDI_DETAIL && !isEditingParameter) {
        selectedDetailParameter--;

        // Skip parameters based on message type
        EncoderButtonMidiMap &map = encoderButtonMidiMaps[selectedEncoderIndex];

        if (selectedDetailParameter < 0) {
            // Wrap to last valid parameter based on message type
            if (map.msgType == MIDI_MSG_NOTE) {
                selectedDetailParameter = BTN_PARAM_NOTE_NUMBER;
            } else if (map.msgType == MIDI_MSG_CC) {
                selectedDetailParameter = BTN_PARAM_CC_VALUE;
            } else if (map.msgType == MIDI_MSG_PROGRAM_CHANGE) {
                selectedDetailParameter = BTN_PARAM_PROGRAM_NUMBER;
            }
        }
    }
    else if (currentMenuLevel == MENU_KEYS_MIDI_DETAIL && !isEditingParameter) {
        selectedDetailParameter--;

        // Get current configuration (press or release)
        KeyMidiMap &map = keyMidiMaps[selectedKeyIndex];
        MidiMessageType msgType = isConfiguringPress ? map.pressMsgType : map.releaseMsgType;

        if (selectedDetailParameter < 0) {
            // Wrap to last valid parameter based on message type
            if (msgType == MIDI_MSG_NOTE) {
                selectedDetailParameter = KEY_PARAM_NOTE_NUMBER;
            } else if (msgType == MIDI_MSG_CC) {
                selectedDetailParameter = KEY_PARAM_CC_VALUE;
            } else if (msgType == MIDI_MSG_PROGRAM_CHANGE) {
                selectedDetailParameter = KEY_PARAM_PROGRAM_NUMBER;
            }
        }
    }
}

void MenuNavigateDown() {
    if (currentMenuLevel == MENU_TOP_LEVEL) {
        currentMenuItem++;
        if (currentMenuItem >= topLevelMenuCount) {
            currentMenuItem = 0;
        }
        Serial.print("Menu: Selected ");
        Serial.println(topLevelMenuItems[currentMenuItem]);
    }
    else if (currentMenuLevel == MENU_GENERAL_LIST) {
        currentMenuItem++;
        if (currentMenuItem >= generalMenuCount) {
            currentMenuItem = 0;
        }
        Serial.print("General Menu: Selected ");
        Serial.println(generalMenuItems[currentMenuItem]);
    }
    else if (currentMenuLevel == MENU_MIDI_QUANTIZE_LIST) {
        currentMenuItem++;
        if (currentMenuItem >= midiQuantizeMenuCount) {
            currentMenuItem = 0;
        }
        Serial.print("MIDI Quantize Menu: Selected ");
        Serial.println(midiQuantizeMenuItems[currentMenuItem]);
    }
    else if (currentMenuLevel == MENU_ADVANCED_LIST) {
        currentMenuItem++;
        if (currentMenuItem >= advancedMenuCount) {
            currentMenuItem = 0;
        }
        Serial.print("Advanced Menu: Selected ");
        Serial.println(advancedMenuItems[currentMenuItem]);
    }
    else if (currentMenuLevel == MENU_ENCODER_MIDI_LIST || currentMenuLevel == MENU_ENCODER_BTN_MIDI_LIST) {
        selectedEncoderIndex++;
        if (selectedEncoderIndex >= NUM_ENCODERS) {
            selectedEncoderIndex = 0;
        }
        Serial.print("Encoder List: Selected E");
        Serial.println(selectedEncoderIndex);
    }
    else if (currentMenuLevel == MENU_KEYS_MIDI_LIST) {
        selectedKeyIndex++;
        if (selectedKeyIndex >= NUM_KEYS) {
            selectedKeyIndex = 0;
        }
        Serial.print("Key List: Selected K");
        Serial.println(selectedKeyIndex);
    }
    else if (currentMenuLevel == MENU_ENCODER_MIDI_DETAIL && !isEditingParameter) {
        selectedDetailParameter++;

        // Skip Note CW/CCW params if not in NOTE mode
        EncoderMidiMap &map = encoderMidiMaps[selectedEncoderIndex];
        int maxParam = (map.msgType == MIDI_MSG_NOTE) ? PARAM_NOTE_CCW : PARAM_MAX_VALUE;

        if (selectedDetailParameter > maxParam) {
            selectedDetailParameter = 0;
        }
    }
    else if (currentMenuLevel == MENU_ENCODER_BTN_MIDI_DETAIL && !isEditingParameter) {
        selectedDetailParameter++;

        // Determine max parameter based on message type
        EncoderButtonMidiMap &map = encoderButtonMidiMaps[selectedEncoderIndex];
        int maxParam;
        if (map.msgType == MIDI_MSG_NOTE) {
            maxParam = BTN_PARAM_NOTE_NUMBER;
        } else if (map.msgType == MIDI_MSG_CC) {
            maxParam = BTN_PARAM_CC_VALUE;
        } else if (map.msgType == MIDI_MSG_PROGRAM_CHANGE) {
            maxParam = BTN_PARAM_PROGRAM_NUMBER;
        } else {
            maxParam = BTN_PARAM_MSG_TYPE;
        }

        if (selectedDetailParameter > maxParam) {
            selectedDetailParameter = 0;
        }
    }
    else if (currentMenuLevel == MENU_KEYS_MIDI_DETAIL && !isEditingParameter) {
        selectedDetailParameter++;

        // Get current configuration (press or release)
        KeyMidiMap &map = keyMidiMaps[selectedKeyIndex];
        MidiMessageType msgType = isConfiguringPress ? map.pressMsgType : map.releaseMsgType;

        int maxParam;
        if (msgType == MIDI_MSG_NOTE) {
            maxParam = KEY_PARAM_NOTE_NUMBER;
        } else if (msgType == MIDI_MSG_CC) {
            maxParam = KEY_PARAM_CC_VALUE;
        } else if (msgType == MIDI_MSG_PROGRAM_CHANGE) {
            maxParam = KEY_PARAM_PROGRAM_NUMBER;
        } else {
            maxParam = KEY_PARAM_MSG_TYPE;
        }

        if (selectedDetailParameter > maxParam) {
            selectedDetailParameter = 0;
        }
    }
}

void MenuNavigateLeft() {
    // Switch between Press/Release in key detail view
    if (currentMenuLevel == MENU_KEYS_MIDI_DETAIL && !isEditingParameter) {
        isConfiguringPress = true;
        selectedDetailParameter = 0;  // Reset to first parameter
        Serial.println("Switched to PRESS configuration");
        return;
    }

    // MIDI Quantize parameter adjustment
    if (currentMenuLevel == MENU_MIDI_QUANTIZE_ENABLE && isEditingParameter) {
        globalQuantizeEnabled = !globalQuantizeEnabled;  // Toggle
        Serial.print("Quantize: ");
        Serial.println(globalQuantizeEnabled ? "ON" : "OFF");
    }
    else if (currentMenuLevel == MENU_MIDI_QUANTIZE_BPM && isEditingParameter) {
        if (globalBPM > 20) {
            globalBPM--;
        } else {
            globalBPM = 20;  // Clamp to minimum
        }
    }
    else if (currentMenuLevel == MENU_MIDI_QUANTIZE_SCALE && isEditingParameter) {
        if (globalScale > SCALE_CHROMATIC) {
            globalScale = (MusicalScale)((int)globalScale - 1);
        }
    }
    else if (currentMenuLevel == MENU_MIDI_QUANTIZE_QUANT && isEditingParameter) {
        if (globalQuantization > QUANT_1_32) {
            globalQuantization = (NoteQuantization)((int)globalQuantization - 1);
        }
    }
    // Used for encoder rotation when editing parameter values
    else if (currentMenuLevel == MENU_ENCODER_MIDI_DETAIL && isEditingParameter) {
        EncoderMidiMap &map = encoderMidiMaps[selectedEncoderIndex];

        switch (selectedDetailParameter) {
            case PARAM_PORT:
                if (map.port > 0) map.port--;
                break;
            case PARAM_CHANNEL:
                if (map.channel > 1) map.channel--;
                break;
            case PARAM_MSG_TYPE:
                if (map.msgType > MIDI_MSG_CC) {
                    map.msgType = (MidiMessageType)((int)map.msgType - 1);
                }
                break;
            case PARAM_CC_NUMBER:
                if (map.ccNumber > 0) map.ccNumber--;
                break;
            case PARAM_MODE:
                if (map.mode > ENC_MODE_ABSOLUTE) {
                    map.mode = (EncoderMode)((int)map.mode - 1);
                }
                break;
            case PARAM_MIN_VALUE:
                if (map.minValue > 0) map.minValue--;
                break;
            case PARAM_MAX_VALUE:
                if (map.maxValue > 0) map.maxValue--;
                break;
            case PARAM_NOTE_CW:
                if (map.noteCW > 0) map.noteCW--;
                break;
            case PARAM_NOTE_CCW:
                if (map.noteCCW > 0) map.noteCCW--;
                break;
        }
    }
    else if (currentMenuLevel == MENU_ENCODER_BTN_MIDI_DETAIL && isEditingParameter) {
        EncoderButtonMidiMap &map = encoderButtonMidiMaps[selectedEncoderIndex];

        switch (selectedDetailParameter) {
            case BTN_PARAM_PORT:
                if (map.port > 0) map.port--;
                break;
            case BTN_PARAM_CHANNEL:
                if (map.channel > 1) map.channel--;
                break;
            case BTN_PARAM_MSG_TYPE:
                if (map.msgType > MIDI_MSG_CC) {
                    map.msgType = (MidiMessageType)((int)map.msgType - 1);
                    // Skip PITCH_BEND for button mapping
                    if (map.msgType == MIDI_MSG_PITCH_BEND) {
                        map.msgType = MIDI_MSG_PROGRAM_CHANGE;
                    }
                }
                break;
            case BTN_PARAM_NOTE_NUMBER:
                if (map.noteNumber > 0) map.noteNumber--;
                break;
            case BTN_PARAM_CC_NUMBER:
                if (map.ccNumber > 0) map.ccNumber--;
                break;
            case BTN_PARAM_CC_VALUE:
                if (map.ccValue > 0) map.ccValue--;
                break;
            case BTN_PARAM_PROGRAM_NUMBER:
                if (map.programNumber > 0) map.programNumber--;
                break;
        }
    }
    else if (currentMenuLevel == MENU_KEYS_MIDI_DETAIL && isEditingParameter) {
        KeyMidiMap &map = keyMidiMaps[selectedKeyIndex];

        // Edit press or release configuration based on current state
        if (isConfiguringPress) {
            switch (selectedDetailParameter) {
                case KEY_PARAM_PORT:
                    if (map.pressPort > 0) map.pressPort--;
                    break;
                case KEY_PARAM_CHANNEL:
                    if (map.pressChannel > 1) map.pressChannel--;
                    break;
                case KEY_PARAM_MSG_TYPE:
                    if (map.pressMsgType > MIDI_MSG_CC) {
                        map.pressMsgType = (MidiMessageType)((int)map.pressMsgType - 1);
                        if (map.pressMsgType == MIDI_MSG_PITCH_BEND) {
                            map.pressMsgType = MIDI_MSG_PROGRAM_CHANGE;
                        }
                    }
                    break;
                case KEY_PARAM_NOTE_NUMBER:
                    if (map.pressNoteNumber > 0) map.pressNoteNumber--;
                    break;
                case KEY_PARAM_CC_NUMBER:
                    if (map.pressCcNumber > 0) map.pressCcNumber--;
                    break;
                case KEY_PARAM_CC_VALUE:
                    if (map.pressCcValue > 0) map.pressCcValue--;
                    break;
                case KEY_PARAM_PROGRAM_NUMBER:
                    if (map.pressProgramNumber > 0) map.pressProgramNumber--;
                    break;
            }
        } else {
            // Editing release configuration
            switch (selectedDetailParameter) {
                case KEY_PARAM_PORT:
                    if (map.releasePort > 0) map.releasePort--;
                    break;
                case KEY_PARAM_CHANNEL:
                    if (map.releaseChannel > 1) map.releaseChannel--;
                    break;
                case KEY_PARAM_MSG_TYPE:
                    if (map.releaseMsgType > MIDI_MSG_CC) {
                        map.releaseMsgType = (MidiMessageType)((int)map.releaseMsgType - 1);
                        if (map.releaseMsgType == MIDI_MSG_PITCH_BEND) {
                            map.releaseMsgType = MIDI_MSG_PROGRAM_CHANGE;
                        }
                    }
                    break;
                case KEY_PARAM_NOTE_NUMBER:
                    if (map.releaseNoteNumber > 0) map.releaseNoteNumber--;
                    break;
                case KEY_PARAM_CC_NUMBER:
                    if (map.releaseCcNumber > 0) map.releaseCcNumber--;
                    break;
                case KEY_PARAM_CC_VALUE:
                    if (map.releaseCcValue > 0) map.releaseCcValue--;
                    break;
                case KEY_PARAM_PROGRAM_NUMBER:
                    if (map.releaseProgramNumber > 0) map.releaseProgramNumber--;
                    break;
            }
        }
    }
}

void MenuNavigateRight() {
    // Switch between Press/Release in key detail view
    if (currentMenuLevel == MENU_KEYS_MIDI_DETAIL && !isEditingParameter) {
        isConfiguringPress = false;
        selectedDetailParameter = 0;  // Reset to first parameter
        Serial.println("Switched to RELEASE configuration");
        return;
    }

    // MIDI Quantize parameter adjustment
    if (currentMenuLevel == MENU_MIDI_QUANTIZE_ENABLE && isEditingParameter) {
        globalQuantizeEnabled = !globalQuantizeEnabled;  // Toggle
        Serial.print("Quantize: ");
        Serial.println(globalQuantizeEnabled ? "ON" : "OFF");
    }
    else if (currentMenuLevel == MENU_MIDI_QUANTIZE_BPM && isEditingParameter) {
        if (globalBPM < 300) {
            globalBPM++;
        } else {
            globalBPM = 300;  // Clamp to maximum
        }
    }
    else if (currentMenuLevel == MENU_MIDI_QUANTIZE_SCALE && isEditingParameter) {
        if (globalScale < SCALE_B_MINOR) {
            globalScale = (MusicalScale)((int)globalScale + 1);
        }
    }
    else if (currentMenuLevel == MENU_MIDI_QUANTIZE_QUANT && isEditingParameter) {
        if (globalQuantization < QUANT_1) {
            globalQuantization = (NoteQuantization)((int)globalQuantization + 1);
        }
    }
    // Used for encoder rotation when editing parameter values
    else if (currentMenuLevel == MENU_ENCODER_MIDI_DETAIL && isEditingParameter) {
        EncoderMidiMap &map = encoderMidiMaps[selectedEncoderIndex];

        switch (selectedDetailParameter) {
            case PARAM_PORT:
                if (map.port < 15) map.port++;
                break;
            case PARAM_CHANNEL:
                if (map.channel < 16) map.channel++;
                break;
            case PARAM_MSG_TYPE:
                if (map.msgType < MIDI_MSG_PITCH_BEND) {
                    map.msgType = (MidiMessageType)((int)map.msgType + 1);
                }
                break;
            case PARAM_CC_NUMBER:
                if (map.ccNumber < 127) map.ccNumber++;
                break;
            case PARAM_MODE:
                if (map.mode < ENC_MODE_CENTER) {
                    map.mode = (EncoderMode)((int)map.mode + 1);
                }
                break;
            case PARAM_MIN_VALUE:
                if (map.minValue < 127) map.minValue++;
                break;
            case PARAM_MAX_VALUE:
                if (map.maxValue < 127) map.maxValue++;
                break;
            case PARAM_NOTE_CW:
                if (map.noteCW < 127) map.noteCW++;
                break;
            case PARAM_NOTE_CCW:
                if (map.noteCCW < 127) map.noteCCW++;
                break;
        }
    }
    else if (currentMenuLevel == MENU_ENCODER_BTN_MIDI_DETAIL && isEditingParameter) {
        EncoderButtonMidiMap &map = encoderButtonMidiMaps[selectedEncoderIndex];

        switch (selectedDetailParameter) {
            case BTN_PARAM_PORT:
                if (map.port < 15) map.port++;
                break;
            case BTN_PARAM_CHANNEL:
                if (map.channel < 16) map.channel++;
                break;
            case BTN_PARAM_MSG_TYPE:
                if (map.msgType < MIDI_MSG_PROGRAM_CHANGE) {
                    map.msgType = (MidiMessageType)((int)map.msgType + 1);
                    // Skip PITCH_BEND for button mapping
                    if (map.msgType == MIDI_MSG_PITCH_BEND) {
                        map.msgType = MIDI_MSG_PROGRAM_CHANGE;
                    }
                }
                break;
            case BTN_PARAM_NOTE_NUMBER:
                if (map.noteNumber < 127) map.noteNumber++;
                break;
            case BTN_PARAM_CC_NUMBER:
                if (map.ccNumber < 127) map.ccNumber++;
                break;
            case BTN_PARAM_CC_VALUE:
                if (map.ccValue < 127) map.ccValue++;
                break;
            case BTN_PARAM_PROGRAM_NUMBER:
                if (map.programNumber < 127) map.programNumber++;
                break;
        }
    }
    else if (currentMenuLevel == MENU_KEYS_MIDI_DETAIL && isEditingParameter) {
        KeyMidiMap &map = keyMidiMaps[selectedKeyIndex];

        // Edit press or release configuration based on current state
        if (isConfiguringPress) {
            switch (selectedDetailParameter) {
                case KEY_PARAM_PORT:
                    if (map.pressPort < 15) map.pressPort++;
                    break;
                case KEY_PARAM_CHANNEL:
                    if (map.pressChannel < 16) map.pressChannel++;
                    break;
                case KEY_PARAM_MSG_TYPE:
                    if (map.pressMsgType < MIDI_MSG_PROGRAM_CHANGE) {
                        map.pressMsgType = (MidiMessageType)((int)map.pressMsgType + 1);
                        if (map.pressMsgType == MIDI_MSG_PITCH_BEND) {
                            map.pressMsgType = MIDI_MSG_PROGRAM_CHANGE;
                        }
                    }
                    break;
                case KEY_PARAM_NOTE_NUMBER:
                    if (map.pressNoteNumber < 127) map.pressNoteNumber++;
                    break;
                case KEY_PARAM_CC_NUMBER:
                    if (map.pressCcNumber < 127) map.pressCcNumber++;
                    break;
                case KEY_PARAM_CC_VALUE:
                    if (map.pressCcValue < 127) map.pressCcValue++;
                    break;
                case KEY_PARAM_PROGRAM_NUMBER:
                    if (map.pressProgramNumber < 127) map.pressProgramNumber++;
                    break;
            }
        } else {
            // Editing release configuration
            switch (selectedDetailParameter) {
                case KEY_PARAM_PORT:
                    if (map.releasePort < 15) map.releasePort++;
                    break;
                case KEY_PARAM_CHANNEL:
                    if (map.releaseChannel < 16) map.releaseChannel++;
                    break;
                case KEY_PARAM_MSG_TYPE:
                    if (map.releaseMsgType < MIDI_MSG_PROGRAM_CHANGE) {
                        map.releaseMsgType = (MidiMessageType)((int)map.releaseMsgType + 1);
                        if (map.releaseMsgType == MIDI_MSG_PITCH_BEND) {
                            map.releaseMsgType = MIDI_MSG_PROGRAM_CHANGE;
                        }
                    }
                    break;
                case KEY_PARAM_NOTE_NUMBER:
                    if (map.releaseNoteNumber < 127) map.releaseNoteNumber++;
                    break;
                case KEY_PARAM_CC_NUMBER:
                    if (map.releaseCcNumber < 127) map.releaseCcNumber++;
                    break;
                case KEY_PARAM_CC_VALUE:
                    if (map.releaseCcValue < 127) map.releaseCcValue++;
                    break;
                case KEY_PARAM_PROGRAM_NUMBER:
                    if (map.releaseProgramNumber < 127) map.releaseProgramNumber++;
                    break;
            }
        }
    }
}

// Direct encoder-to-parameter adjustment (without entering edit mode)
// encoderIndex: 0-15 maps to parameter index 0-N
// increment: true = increase value, false = decrease value
void AdjustParameterByEncoder(int encoderIndex, bool increment) {
    // Encoder MIDI Detail View - Map encoder to parameter
    if (currentMenuLevel == MENU_ENCODER_MIDI_DETAIL) {
        // Limit to 9 parameters (PARAM_COUNT)
        if (encoderIndex >= PARAM_COUNT) return;

        EncoderMidiMap &map = encoderMidiMaps[selectedEncoderIndex];
        int paramIndex = encoderIndex;

        // Skip NOTE_CW and NOTE_CCW parameters if not in NOTE mode
        if (map.msgType != MIDI_MSG_NOTE && paramIndex >= PARAM_NOTE_CW) {
            return;
        }

        // Adjust the parameter based on encoder index
        if (increment) {
            switch (paramIndex) {
                case PARAM_PORT:
                    if (map.port < 15) map.port++;
                    break;
                case PARAM_CHANNEL:
                    if (map.channel < 16) map.channel++;
                    break;
                case PARAM_MSG_TYPE:
                    if (map.msgType < MIDI_MSG_PITCH_BEND) {
                        map.msgType = (MidiMessageType)((int)map.msgType + 1);
                    }
                    break;
                case PARAM_CC_NUMBER:
                    if (map.ccNumber < 127) map.ccNumber++;
                    break;
                case PARAM_MODE:
                    if (map.mode < ENC_MODE_CENTER) {
                        map.mode = (EncoderMode)((int)map.mode + 1);
                    }
                    break;
                case PARAM_MIN_VALUE:
                    if (map.minValue < 127) map.minValue++;
                    break;
                case PARAM_MAX_VALUE:
                    if (map.maxValue < 127) map.maxValue++;
                    break;
                case PARAM_NOTE_CW:
                    if (map.noteCW < 127) map.noteCW++;
                    break;
                case PARAM_NOTE_CCW:
                    if (map.noteCCW < 127) map.noteCCW++;
                    break;
            }
        } else {
            // Decrement
            switch (paramIndex) {
                case PARAM_PORT:
                    if (map.port > 0) map.port--;
                    break;
                case PARAM_CHANNEL:
                    if (map.channel > 1) map.channel--;
                    break;
                case PARAM_MSG_TYPE:
                    if (map.msgType > MIDI_MSG_CC) {
                        map.msgType = (MidiMessageType)((int)map.msgType - 1);
                    }
                    break;
                case PARAM_CC_NUMBER:
                    if (map.ccNumber > 0) map.ccNumber--;
                    break;
                case PARAM_MODE:
                    if (map.mode > ENC_MODE_ABSOLUTE) {
                        map.mode = (EncoderMode)((int)map.mode - 1);
                    }
                    break;
                case PARAM_MIN_VALUE:
                    if (map.minValue > 0) map.minValue--;
                    break;
                case PARAM_MAX_VALUE:
                    if (map.maxValue > 0) map.maxValue--;
                    break;
                case PARAM_NOTE_CW:
                    if (map.noteCW > 0) map.noteCW--;
                    break;
                case PARAM_NOTE_CCW:
                    if (map.noteCCW > 0) map.noteCCW--;
                    break;
            }
        }

        Serial.print("Direct encoder adjust: Encoder ");
        Serial.print(encoderIndex);
        Serial.print(" -> Param ");
        Serial.println(paramIndex);
    }
    // Encoder Button MIDI Detail View
    else if (currentMenuLevel == MENU_ENCODER_BTN_MIDI_DETAIL) {
        // Limit to 7 parameters (BTN_PARAM_COUNT)
        if (encoderIndex >= BTN_PARAM_COUNT) return;

        EncoderButtonMidiMap &map = encoderButtonMidiMaps[selectedEncoderIndex];
        int paramIndex = encoderIndex;

        // Adjust the parameter based on encoder index
        if (increment) {
            switch (paramIndex) {
                case BTN_PARAM_PORT:
                    if (map.port < 15) map.port++;
                    break;
                case BTN_PARAM_CHANNEL:
                    if (map.channel < 16) map.channel++;
                    break;
                case BTN_PARAM_MSG_TYPE:
                    if (map.msgType < MIDI_MSG_PROGRAM_CHANGE) {
                        map.msgType = (MidiMessageType)((int)map.msgType + 1);
                        if (map.msgType == MIDI_MSG_PITCH_BEND) {
                            map.msgType = MIDI_MSG_PROGRAM_CHANGE;
                        }
                    }
                    break;
                case BTN_PARAM_NOTE_NUMBER:
                    if (map.noteNumber < 127) map.noteNumber++;
                    break;
                case BTN_PARAM_CC_NUMBER:
                    if (map.ccNumber < 127) map.ccNumber++;
                    break;
                case BTN_PARAM_CC_VALUE:
                    if (map.ccValue < 127) map.ccValue++;
                    break;
                case BTN_PARAM_PROGRAM_NUMBER:
                    if (map.programNumber < 127) map.programNumber++;
                    break;
            }
        } else {
            // Decrement
            switch (paramIndex) {
                case BTN_PARAM_PORT:
                    if (map.port > 0) map.port--;
                    break;
                case BTN_PARAM_CHANNEL:
                    if (map.channel > 1) map.channel--;
                    break;
                case BTN_PARAM_MSG_TYPE:
                    if (map.msgType > MIDI_MSG_CC) {
                        int newType = (int)map.msgType - 1;
                        if (newType == MIDI_MSG_PITCH_BEND) {
                            map.msgType = MIDI_MSG_NOTE;
                        } else {
                            map.msgType = (MidiMessageType)newType;
                        }
                    }
                    break;
                case BTN_PARAM_NOTE_NUMBER:
                    if (map.noteNumber > 0) map.noteNumber--;
                    break;
                case BTN_PARAM_CC_NUMBER:
                    if (map.ccNumber > 0) map.ccNumber--;
                    break;
                case BTN_PARAM_CC_VALUE:
                    if (map.ccValue > 0) map.ccValue--;
                    break;
                case BTN_PARAM_PROGRAM_NUMBER:
                    if (map.programNumber > 0) map.programNumber--;
                    break;
            }
        }

        Serial.print("Direct encoder adjust (button): Encoder ");
        Serial.print(encoderIndex);
        Serial.print(" -> Param ");
        Serial.println(paramIndex);
    }
    // Key MIDI Detail View
    else if (currentMenuLevel == MENU_KEYS_MIDI_DETAIL) {
        // Limit to 7 parameters (KEY_PARAM_COUNT)
        if (encoderIndex >= KEY_PARAM_COUNT) return;

        KeyMidiMap &map = keyMidiMaps[selectedKeyIndex];
        int paramIndex = encoderIndex;

        // Edit press or release configuration based on current state
        if (isConfiguringPress) {
            if (increment) {
                switch (paramIndex) {
                    case KEY_PARAM_PORT:
                        if (map.pressPort < 15) map.pressPort++;
                        break;
                    case KEY_PARAM_CHANNEL:
                        if (map.pressChannel < 16) map.pressChannel++;
                        break;
                    case KEY_PARAM_MSG_TYPE:
                        if (map.pressMsgType < MIDI_MSG_PROGRAM_CHANGE) {
                            map.pressMsgType = (MidiMessageType)((int)map.pressMsgType + 1);
                            if (map.pressMsgType == MIDI_MSG_PITCH_BEND) {
                                map.pressMsgType = MIDI_MSG_PROGRAM_CHANGE;
                            }
                        }
                        break;
                    case KEY_PARAM_NOTE_NUMBER:
                        if (map.pressNoteNumber < 127) map.pressNoteNumber++;
                        break;
                    case KEY_PARAM_CC_NUMBER:
                        if (map.pressCcNumber < 127) map.pressCcNumber++;
                        break;
                    case KEY_PARAM_CC_VALUE:
                        if (map.pressCcValue < 127) map.pressCcValue++;
                        break;
                    case KEY_PARAM_PROGRAM_NUMBER:
                        if (map.pressProgramNumber < 127) map.pressProgramNumber++;
                        break;
                }
            } else {
                // Decrement
                switch (paramIndex) {
                    case KEY_PARAM_PORT:
                        if (map.pressPort > 0) map.pressPort--;
                        break;
                    case KEY_PARAM_CHANNEL:
                        if (map.pressChannel > 1) map.pressChannel--;
                        break;
                    case KEY_PARAM_MSG_TYPE:
                        if (map.pressMsgType > MIDI_MSG_CC) {
                            int newType = (int)map.pressMsgType - 1;
                            if (newType == MIDI_MSG_PITCH_BEND) {
                                map.pressMsgType = MIDI_MSG_NOTE;
                            } else {
                                map.pressMsgType = (MidiMessageType)newType;
                            }
                        }
                        break;
                    case KEY_PARAM_NOTE_NUMBER:
                        if (map.pressNoteNumber > 0) map.pressNoteNumber--;
                        break;
                    case KEY_PARAM_CC_NUMBER:
                        if (map.pressCcNumber > 0) map.pressCcNumber--;
                        break;
                    case KEY_PARAM_CC_VALUE:
                        if (map.pressCcValue > 0) map.pressCcValue--;
                        break;
                    case KEY_PARAM_PROGRAM_NUMBER:
                        if (map.pressProgramNumber > 0) map.pressProgramNumber--;
                        break;
                }
            }
        } else {
            // Editing release configuration
            if (increment) {
                switch (paramIndex) {
                    case KEY_PARAM_PORT:
                        if (map.releasePort < 15) map.releasePort++;
                        break;
                    case KEY_PARAM_CHANNEL:
                        if (map.releaseChannel < 16) map.releaseChannel++;
                        break;
                    case KEY_PARAM_MSG_TYPE:
                        if (map.releaseMsgType < MIDI_MSG_PROGRAM_CHANGE) {
                            map.releaseMsgType = (MidiMessageType)((int)map.releaseMsgType + 1);
                            if (map.releaseMsgType == MIDI_MSG_PITCH_BEND) {
                                map.releaseMsgType = MIDI_MSG_PROGRAM_CHANGE;
                            }
                        }
                        break;
                    case KEY_PARAM_NOTE_NUMBER:
                        if (map.releaseNoteNumber < 127) map.releaseNoteNumber++;
                        break;
                    case KEY_PARAM_CC_NUMBER:
                        if (map.releaseCcNumber < 127) map.releaseCcNumber++;
                        break;
                    case KEY_PARAM_CC_VALUE:
                        if (map.releaseCcValue < 127) map.releaseCcValue++;
                        break;
                    case KEY_PARAM_PROGRAM_NUMBER:
                        if (map.releaseProgramNumber < 127) map.releaseProgramNumber++;
                        break;
                }
            } else {
                // Decrement
                switch (paramIndex) {
                    case KEY_PARAM_PORT:
                        if (map.releasePort > 0) map.releasePort--;
                        break;
                    case KEY_PARAM_CHANNEL:
                        if (map.releaseChannel > 1) map.releaseChannel--;
                        break;
                    case KEY_PARAM_MSG_TYPE:
                        if (map.releaseMsgType > MIDI_MSG_CC) {
                            int newType = (int)map.releaseMsgType - 1;
                            if (newType == MIDI_MSG_PITCH_BEND) {
                                map.releaseMsgType = MIDI_MSG_NOTE;
                            } else {
                                map.releaseMsgType = (MidiMessageType)newType;
                            }
                        }
                        break;
                    case KEY_PARAM_NOTE_NUMBER:
                        if (map.releaseNoteNumber > 0) map.releaseNoteNumber--;
                        break;
                    case KEY_PARAM_CC_NUMBER:
                        if (map.releaseCcNumber > 0) map.releaseCcNumber--;
                        break;
                    case KEY_PARAM_CC_VALUE:
                        if (map.releaseCcValue > 0) map.releaseCcValue--;
                        break;
                    case KEY_PARAM_PROGRAM_NUMBER:
                        if (map.releaseProgramNumber > 0) map.releaseProgramNumber--;
                        break;
                }
            }
        }

        Serial.print("Direct encoder adjust (key): Encoder ");
        Serial.print(encoderIndex);
        Serial.print(" -> Param ");
        Serial.println(paramIndex);
    }
}

void MenuEnter() {
    if (currentMenuLevel == MENU_TOP_LEVEL) {
        Serial.print("Menu: Entering ");
        Serial.println(topLevelMenuItems[currentMenuItem]);

        // Enter submenu based on selection
        switch (currentMenuItem) {
            case 0:
                currentMenuLevel = MENU_GENERAL_LIST;
                currentMenuItem = 0;
                break;
            case 1:
                currentMenuLevel = MENU_MIDI_QUANTIZE_LIST;
                currentMenuItem = 0;
                break;
            case 2:
                currentMenuLevel = MENU_ENCODER_MIDI_LIST;
                selectedEncoderIndex = 0;
                break;
            case 3:
                currentMenuLevel = MENU_ENCODER_BTN_MIDI_LIST;
                selectedEncoderIndex = 0;
                break;
            case 4:
                currentMenuLevel = MENU_KEYS_MIDI_LIST;
                selectedKeyIndex = 0;
                break;
            case 5:
                currentMenuLevel = MENU_ADVANCED_LIST;
                currentMenuItem = 0;
                break;
        }
    }
    else if (currentMenuLevel == MENU_GENERAL_LIST) {
        // Enter general submenu based on selection
        switch (currentMenuItem) {
            case 0:
                currentMenuLevel = MENU_ENCODER_STATUS;
                Serial.println("Entering Encoder Status");
                break;
            case 1:
                currentMenuLevel = MENU_SWITCH_STATUS;
                Serial.println("Entering Switch Status");
                break;
        }
    }
    else if (currentMenuLevel == MENU_MIDI_QUANTIZE_LIST) {
        // Enter MIDI Quantize submenu based on selection
        switch (currentMenuItem) {
            case 0:
                currentMenuLevel = MENU_MIDI_QUANTIZE_ENABLE;
                isEditingParameter = true;
                Serial.println("Entering Quantize Enable/Disable");
                break;
            case 1:
                currentMenuLevel = MENU_MIDI_QUANTIZE_BPM;
                isEditingParameter = true;
                Serial.println("Entering BPM Edit");
                break;
            case 2:
                currentMenuLevel = MENU_MIDI_QUANTIZE_SCALE;
                isEditingParameter = true;
                Serial.println("Entering Scale Select");
                break;
            case 3:
                currentMenuLevel = MENU_MIDI_QUANTIZE_QUANT;
                isEditingParameter = true;
                Serial.println("Entering Quantization Select");
                break;
        }
    }
    else if (currentMenuLevel == MENU_ADVANCED_LIST) {
        // Enter advanced submenu based on selection
        switch (currentMenuItem) {
            case 0:
                currentMenuLevel = MENU_REBOOT_CONFIRM;
                Serial.println("Entering Reboot Confirmation");
                break;
            case 1:
                currentMenuLevel = MENU_FACTORY_RESET_CONFIRM;
                Serial.println("Entering Factory Reset Confirmation");
                break;
        }
    }
    else if (currentMenuLevel == MENU_REBOOT_CONFIRM) 
    {
        Serial.println("Performing Reboot...");
        doReboot();
    }
    else if (currentMenuLevel == MENU_FACTORY_RESET_CONFIRM) {
        // Perform factory reset
        Serial.println("Performing Factory Reset...");
        ResetEEPROMToDefaults();
        // Go back to top level
        currentMenuLevel = MENU_TOP_LEVEL;
        currentMenuItem = 0;
    }
    else if (currentMenuLevel == MENU_ENCODER_MIDI_LIST) {
        // Enter encoder detail view
        currentMenuLevel = MENU_ENCODER_MIDI_DETAIL;
        selectedDetailParameter = 0;
        isEditingParameter = false;
        Serial.print("Entering detail view for Encoder ");
        Serial.println(selectedEncoderIndex);
    }
    else if (currentMenuLevel == MENU_ENCODER_BTN_MIDI_LIST) {
        // Enter encoder button detail view
        currentMenuLevel = MENU_ENCODER_BTN_MIDI_DETAIL;
        selectedDetailParameter = 0;
        isEditingParameter = false;
        Serial.print("Entering button detail view for Encoder ");
        Serial.println(selectedEncoderIndex);
    }
    else if (currentMenuLevel == MENU_KEYS_MIDI_LIST) {
        // Enter key detail view
        currentMenuLevel = MENU_KEYS_MIDI_DETAIL;
        selectedDetailParameter = 0;
        isEditingParameter = false;
        isConfiguringPress = true;  // Start with press configuration
        Serial.print("Entering detail view for Key ");
        Serial.println(selectedKeyIndex);
    }
    else if (currentMenuLevel == MENU_ENCODER_MIDI_DETAIL || currentMenuLevel == MENU_ENCODER_BTN_MIDI_DETAIL || currentMenuLevel == MENU_KEYS_MIDI_DETAIL) {
        // Toggle editing mode for the selected parameter
        isEditingParameter = !isEditingParameter;
        Serial.print("Editing parameter: ");
        Serial.println(isEditingParameter ? "ON" : "OFF");
    }
    else if (currentMenuLevel == MENU_MIDI_QUANTIZE_ENABLE || currentMenuLevel == MENU_MIDI_QUANTIZE_BPM || currentMenuLevel == MENU_MIDI_QUANTIZE_SCALE || currentMenuLevel == MENU_MIDI_QUANTIZE_QUANT) {
        // Toggle editing mode for quantize parameters
        isEditingParameter = !isEditingParameter;
        Serial.print("Editing quantize parameter: ");
        Serial.println(isEditingParameter ? "ON" : "OFF");
    }
}

void MenuBack() {
    if (currentMenuLevel == MENU_TOP_LEVEL) {
        // Exit settings mode
        ExitSettingsMode();
    }
    else if (currentMenuLevel == MENU_GENERAL_LIST) {
        // Go back to top level
        currentMenuLevel = MENU_TOP_LEVEL;
        currentMenuItem = 0;
    }
    else if (currentMenuLevel == MENU_MIDI_QUANTIZE_LIST) {
        // Go back to top level
        currentMenuLevel = MENU_TOP_LEVEL;
        currentMenuItem = 1;  // Return to "Midi Quantize" item
    }
    else if (currentMenuLevel == MENU_MIDI_QUANTIZE_ENABLE || currentMenuLevel == MENU_MIDI_QUANTIZE_BPM || currentMenuLevel == MENU_MIDI_QUANTIZE_SCALE || currentMenuLevel == MENU_MIDI_QUANTIZE_QUANT) {
        if (isEditingParameter) {
            // Exit editing mode
            isEditingParameter = false;
        } else {
            // Save quantize settings to EEPROM before going back
            SaveQuantizeSettings();
            // Go back to MIDI Quantize list
            MenuLevel prevLevel = currentMenuLevel;  // Store before changing
            currentMenuLevel = MENU_MIDI_QUANTIZE_LIST;
            // Set menu item based on which parameter we were editing
            if (prevLevel == MENU_MIDI_QUANTIZE_ENABLE) {
                currentMenuItem = 0;
            } else if (prevLevel == MENU_MIDI_QUANTIZE_BPM) {
                currentMenuItem = 1;
            } else if (prevLevel == MENU_MIDI_QUANTIZE_SCALE) {
                currentMenuItem = 2;
            } else {
                currentMenuItem = 3;
            }
        }
    }
    else if (currentMenuLevel == MENU_ADVANCED_LIST) {
        // Go back to top level
        currentMenuLevel = MENU_TOP_LEVEL;
        currentMenuItem = 5;  // Return to "Advanced" item (updated index)
    }
    else if (currentMenuLevel == MENU_REBOOT_CONFIRM) {
        // Go back to advanced list without resetting
        currentMenuLevel = MENU_ADVANCED_LIST;
        currentMenuItem = 0;
    }
    else if (currentMenuLevel == MENU_FACTORY_RESET_CONFIRM) {
        // Go back to advanced list without resetting
        currentMenuLevel = MENU_ADVANCED_LIST;
        currentMenuItem = 0;
    }
    else if (currentMenuLevel == MENU_ENCODER_STATUS || currentMenuLevel == MENU_SWITCH_STATUS) {
        // Go back to general list
        currentMenuLevel = MENU_GENERAL_LIST;
        currentMenuItem = (currentMenuLevel == MENU_ENCODER_STATUS) ? 0 : 1;
    }
    else if (currentMenuLevel == MENU_ENCODER_MIDI_DETAIL) {
        if (isEditingParameter) {
            // Exit editing mode
            isEditingParameter = false;
        } else {
            // Save encoder map to EEPROM before going back
            SaveEncoderMidiMap(selectedEncoderIndex);
            // Go back to encoder list
            currentMenuLevel = MENU_ENCODER_MIDI_LIST;
        }
    }
    else if (currentMenuLevel == MENU_ENCODER_BTN_MIDI_DETAIL) {
        if (isEditingParameter) {
            // Exit editing mode
            isEditingParameter = false;
        } else {
            // Save encoder button map to EEPROM before going back
            SaveEncoderButtonMidiMap(selectedEncoderIndex);
            // Go back to encoder button list
            currentMenuLevel = MENU_ENCODER_BTN_MIDI_LIST;
        }
    }
    else if (currentMenuLevel == MENU_ENCODER_MIDI_LIST) {
        // Go back to top level
        currentMenuLevel = MENU_TOP_LEVEL;
        currentMenuItem = 2;  // Return to "Encoder MIDI Map" item
    }
    else if (currentMenuLevel == MENU_ENCODER_BTN_MIDI_LIST) {
        // Go back to top level
        currentMenuLevel = MENU_TOP_LEVEL;
        currentMenuItem = 3;  // Return to "Encoder Btn MIDI Map" item
    }
    else if (currentMenuLevel == MENU_KEYS_MIDI_DETAIL) {
        if (isEditingParameter) {
            // Exit editing mode
            isEditingParameter = false;
        } else {
            // Save key map to EEPROM before going back
            SaveKeyMidiMap(selectedKeyIndex);
            // Go back to key list
            currentMenuLevel = MENU_KEYS_MIDI_LIST;
        }
    }
    else if (currentMenuLevel == MENU_KEYS_MIDI_LIST) {
        // Go back to top level
        currentMenuLevel = MENU_TOP_LEVEL;
        currentMenuItem = 4;  // Return to "Keys MIDI Map" item
    }
    else {
        // Go back to top level
        Serial.println("Menu: Back to main menu");
        currentMenuLevel = MENU_TOP_LEVEL;
        currentMenuItem = 0;
    }
}

//==============================================================================
// SETTINGS DISPLAY FUNCTIONS
//==============================================================================

void DisplayTopLevelMenu() {
    tft.fillScreen(ILI9341_BLACK);

    // Title
    tft.setTextColor(ILI9341_CYAN);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.println("SETTINGS");

    // Draw horizontal line
    tft.drawFastHLine(10, 35, 300, ILI9341_WHITE);

    // Menu items
    tft.setTextSize(2);
    int yPos = 50;

    for (int i = 0; i < topLevelMenuCount; i++) {
        if (i == currentMenuItem) {
            // Highlight selected item with outlined rectangle
            tft.drawRect(5, yPos - 2, 310, 24, ILI9341_CYAN);
            tft.setTextColor(ILI9341_CYAN);
            tft.setCursor(15, yPos);
            tft.print("> ");
        } else {
            tft.setTextColor(ILI9341_WHITE);
            tft.setCursor(25, yPos);
        }

        tft.println(topLevelMenuItems[i]);
        yPos += 30;
    }

    // Instructions at bottom
    tft.setTextSize(1);
    tft.setTextColor(ILI9341_DARKGREY);
    tft.setCursor(10, 210);
    tft.print("UP/DOWN: Navigate  ENTER: Select");
    tft.setCursor(10, 225);
    tft.print("BACK: Exit Settings");
}

void DisplayGeneralList() {
    tft.fillScreen(ILI9341_BLACK);

    // Title
    tft.setTextColor(ILI9341_CYAN);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.println("GENERAL");

    // Draw horizontal line
    tft.drawFastHLine(10, 35, 300, ILI9341_WHITE);

    // Menu items
    tft.setTextSize(2);
    int yPos = 50;

    for (int i = 0; i < generalMenuCount; i++) {
        if (i == currentMenuItem) {
            // Highlight selected item with outlined rectangle
            tft.drawRect(5, yPos - 2, 310, 24, ILI9341_CYAN);
            tft.setTextColor(ILI9341_CYAN);
            tft.setCursor(15, yPos);
            tft.print("> ");
        } else {
            tft.setTextColor(ILI9341_WHITE);
            tft.setCursor(25, yPos);
        }

        tft.println(generalMenuItems[i]);
        yPos += 30;
    }

    // Instructions at bottom
    tft.setTextSize(1);
    tft.setTextColor(ILI9341_DARKGREY);
    tft.setCursor(10, 210);
    tft.print("UP/DOWN: Navigate  ENTER: Select");
    tft.setCursor(10, 225);
    tft.print("BACK: Return to menu");
}

void DisplayEncoderStatus() {
    // Use the existing UpdateEncoderDisplay function from display.h
    UpdateEncoderDisplay();

    // Add back button hint at the bottom
    tft.setTextColor(ILI9341_DARKGREY);
    tft.setTextSize(1);
    tft.setCursor(200, 225);
    tft.print("BACK: Return");
}

void DisplaySwitchStatus() {
    tft.fillScreen(ILI9341_BLACK);

    // Title
    tft.setTextColor(ILI9341_CYAN);
    tft.setTextSize(2);
    tft.setCursor(10, 5);
    tft.println("Switch Status");

    // Draw encoder button status grid (4x4 layout)
    tft.setTextSize(1);
    int xOffset = 10;
    int yOffset = 30;
    int cellWidth = 75;
    int cellHeight = 50;

    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            int encoderID = row * 4 + col;  // Logical encoder ID
            int x = xOffset + (col * cellWidth);
            int y = yOffset + (row * cellHeight);

            // Encoder button number
            tft.setTextColor(ILI9341_YELLOW);
            tft.setCursor(x, y);
            tft.print("SW");
            tft.print(encoderID);

            // Show button state with large indicator
            if (EncodersBtnState_Logical[encoderID] == ENC_BTN_PRESS) {
                tft.fillCircle(x + 30, y + 15, 8, ILI9341_RED);
                tft.setTextColor(ILI9341_WHITE);
                tft.setCursor(x, y + 30);
                tft.print("PRESSED");
            } else {
                tft.drawCircle(x + 30, y + 15, 8, ILI9341_DARKGREY);
                tft.setTextColor(ILI9341_DARKGREY);
                tft.setCursor(x, y + 30);
                tft.print("Released");
            }
        }
    }

    // Footer
    tft.setTextColor(ILI9341_DARKGREY);
    tft.setTextSize(1);
    tft.setCursor(10, 225);
    tft.print("BACK: Return to menu");
}

void DisplayMidiQuantizeList() {
    tft.fillScreen(ILI9341_BLACK);

    // Title
    tft.setTextColor(ILI9341_CYAN);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.println("MIDI QUANTIZE");

    // Draw horizontal line
    tft.drawFastHLine(10, 35, 300, ILI9341_WHITE);

    // Menu items
    tft.setTextSize(2);
    int yPos = 50;

    for (int i = 0; i < midiQuantizeMenuCount; i++) {
        if (i == currentMenuItem) {
            // Highlight selected item with outlined rectangle
            tft.drawRect(5, yPos - 2, 310, 24, ILI9341_CYAN);
            tft.setTextColor(ILI9341_CYAN);
            tft.setCursor(15, yPos);
            tft.print("> ");
        } else {
            tft.setTextColor(ILI9341_WHITE);
            tft.setCursor(25, yPos);
        }

        tft.println(midiQuantizeMenuItems[i]);
        yPos += 30;
    }

    // Instructions at bottom
    tft.setTextSize(1);
    tft.setTextColor(ILI9341_DARKGREY);
    tft.setCursor(10, 210);
    tft.print("UP/DOWN: Navigate  ENTER: Edit");
    tft.setCursor(10, 225);
    tft.print("BACK: Return to menu");
}

void DisplayMidiQuantizeEnable() {
    tft.fillScreen(ILI9341_BLACK);

    // Title
    tft.setTextColor(ILI9341_CYAN);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.println("Quantize Setting");

    tft.drawFastHLine(10, 35, 300, ILI9341_WHITE);

    // Enable/Disable display (large)
    tft.setTextSize(4);
    if (isEditingParameter) {
        tft.setTextColor(ILI9341_GREEN);
    } else {
        tft.setTextColor(ILI9341_YELLOW);
    }
    tft.setCursor(80, 100);
    tft.print(globalQuantizeEnabled ? "ON" : "OFF");

    // Instructions
    tft.setTextColor(ILI9341_DARKGREY);
    tft.setTextSize(1);
    tft.setCursor(10, 210);
    if (isEditingParameter) {
        tft.print("LEFT/RIGHT: Toggle ON/OFF");
    } else {
        tft.print("ENTER: Edit value");
    }
    tft.setCursor(10, 225);
    tft.print("BACK: ");
    tft.print(isEditingParameter ? "Save" : "Return");
}

void DisplayMidiQuantizeBPM() {
    tft.fillScreen(ILI9341_BLACK);

    // Title
    tft.setTextColor(ILI9341_CYAN);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.println("BPM Setting");

    tft.drawFastHLine(10, 35, 300, ILI9341_WHITE);

    // BPM value display (large)
    tft.setTextSize(4);
    if (isEditingParameter) {
        tft.setTextColor(ILI9341_GREEN);
    } else {
        tft.setTextColor(ILI9341_YELLOW);
    }
    tft.setCursor(80, 100);
    tft.print(globalBPM);
    tft.setTextSize(2);
    tft.print(" BPM");

    // Instructions
    tft.setTextColor(ILI9341_DARKGREY);
    tft.setTextSize(1);
    tft.setCursor(10, 210);
    if (isEditingParameter) {
        tft.print("LEFT/RIGHT: Adjust (20-300)");
    } else {
        tft.print("ENTER: Edit value");
    }
    tft.setCursor(10, 225);
    tft.print("BACK: ");
    tft.print(isEditingParameter ? "Save" : "Return");
}

void DisplayMidiQuantizeScale() {
    tft.fillScreen(ILI9341_BLACK);

    // Title
    tft.setTextColor(ILI9341_CYAN);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.println("Scale Setting");

    tft.drawFastHLine(10, 35, 300, ILI9341_WHITE);

    // Scale name display
    tft.setTextSize(3);
    if (isEditingParameter) {
        tft.setTextColor(ILI9341_GREEN);
    } else {
        tft.setTextColor(ILI9341_YELLOW);
    }

    const char* scaleNames[] = {
        "Chromatic",
        "C Major", "C Minor",
        "C Dorian", "C Phrygian",
        "C Lydian", "C Mixolydian",
        "C Locrian",
        "D Major", "D Minor",
        "E Major", "E Minor",
        "F Major", "F Minor",
        "G Major", "G Minor",
        "A Major", "A Minor",
        "B Major", "B Minor"
    };

    tft.setCursor(40, 100);
    tft.print(scaleNames[globalScale]);

    // Instructions
    tft.setTextColor(ILI9341_DARKGREY);
    tft.setTextSize(1);
    tft.setCursor(10, 210);
    if (isEditingParameter) {
        tft.print("LEFT/RIGHT: Change scale");
    } else {
        tft.print("ENTER: Edit scale");
    }
    tft.setCursor(10, 225);
    tft.print("BACK: ");
    tft.print(isEditingParameter ? "Save" : "Return");
}

void DisplayMidiQuantizeQuant() {
    tft.fillScreen(ILI9341_BLACK);

    // Title
    tft.setTextColor(ILI9341_CYAN);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.println("Quantization");

    tft.drawFastHLine(10, 35, 300, ILI9341_WHITE);

    // Quantization value display
    tft.setTextSize(4);
    if (isEditingParameter) {
        tft.setTextColor(ILI9341_GREEN);
    } else {
        tft.setTextColor(ILI9341_YELLOW);
    }

    const char* quantNames[] = {
        "1/32",
        "1/16",
        "1/8",
        "1/4",
        "1/2",
        "1"
    };

    tft.setCursor(100, 100);
    tft.print(quantNames[globalQuantization]);

    // Instructions
    tft.setTextColor(ILI9341_DARKGREY);
    tft.setTextSize(1);
    tft.setCursor(10, 210);
    if (isEditingParameter) {
        tft.print("LEFT/RIGHT: Change quantization");
    } else {
        tft.print("ENTER: Edit quantization");
    }
    tft.setCursor(10, 225);
    tft.print("BACK: ");
    tft.print(isEditingParameter ? "Save" : "Return");
}

void DisplayEncoderMidiList() {
    tft.fillScreen(ILI9341_BLACK);

    // Title
    tft.setTextColor(ILI9341_CYAN);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.println("Encoder MIDI Map");

    tft.drawFastHLine(10, 35, 300, ILI9341_WHITE);

    // Display encoder list (show 8 at a time)
    tft.setTextSize(1);
    int yPos = 45;
    int startIdx = (selectedEncoderIndex / 8) * 8;  // Show 8 encoders per page
    int endIdx = min(startIdx + 8, NUM_ENCODERS);

    for (int i = startIdx; i < endIdx; i++) {
        EncoderMidiMap &map = encoderMidiMaps[i];

        if (i == selectedEncoderIndex) {
            // Highlight selected encoder with outlined rectangle
            tft.drawRect(5, yPos - 2, 310, 18, ILI9341_CYAN);
            tft.setTextColor(ILI9341_CYAN);
            tft.setCursor(10, yPos);
            tft.print("> ");
        } else {
            tft.setTextColor(ILI9341_WHITE);
            tft.setCursor(20, yPos);
        }

        // Encoder number and summary
        tft.print("E");
        tft.print(i);
        tft.print(": CH");
        tft.print(map.channel);
        tft.print(" ");

        // Show message type
        switch (map.msgType) {
            case MIDI_MSG_CC:
                tft.print("CC");
                tft.print(map.ccNumber);
                break;
            case MIDI_MSG_NOTE:
                tft.print("Note ");
                tft.print(map.noteCW);
                tft.print("/");
                tft.print(map.noteCCW);
                break;
            case MIDI_MSG_PROGRAM_CHANGE:
                tft.print("PC");
                break;
            case MIDI_MSG_PITCH_BEND:
                tft.print("PB");
                break;
        }

        yPos += 20;
    }

    // Instructions
    tft.setTextColor(ILI9341_DARKGREY);
    tft.setTextSize(1);
    tft.setCursor(10, 210);
    tft.print("UP/DOWN: Navigate  ENTER: Configure");
    tft.setCursor(10, 225);
    tft.print("BACK: Return to menu");
}

void DisplayEncoderMidiDetail() {
    tft.fillScreen(ILI9341_BLACK);

    EncoderMidiMap &map = encoderMidiMaps[selectedEncoderIndex];

    // Title
    tft.setTextColor(ILI9341_CYAN);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.print("Encoder ");
    tft.print(selectedEncoderIndex);
    tft.println(" Config");

    tft.drawFastHLine(10, 35, 300, ILI9341_WHITE);

    // Display parameters
    tft.setTextSize(1);
    int yPos = 45;

    const char* paramNames[] = {
        "Port", "Channel", "Type", "CC/Note", "Mode",
        "Min", "Max", "Note CW", "Note CCW"
    };

    for (int i = 0; i < PARAM_COUNT; i++) {
        // Skip Note CW/CCW if not in NOTE mode
        if ((i == PARAM_NOTE_CW || i == PARAM_NOTE_CCW) && map.msgType != MIDI_MSG_NOTE) {
            continue;
        }

        // Highlight selected parameter with outlined rectangle
        if (i == selectedDetailParameter) {
            if (isEditingParameter) {
                tft.drawRect(5, yPos - 2, 310, 14, ILI9341_GREEN);  // Green outline when editing
                tft.setTextColor(ILI9341_GREEN);
            } else {
                tft.drawRect(5, yPos - 2, 310, 14, ILI9341_CYAN);   // Cyan outline when selected
                tft.setTextColor(ILI9341_CYAN);
            }
            tft.setCursor(10, yPos);
            tft.print("> ");
        } else {
            tft.setTextColor(ILI9341_WHITE);
            tft.setCursor(20, yPos);
        }

        // Parameter name with encoder index
        tft.print(i);
        tft.print(". ");
        tft.print(paramNames[i]);
        tft.print(": ");

        // Parameter value
        tft.setTextColor(ILI9341_YELLOW);
        switch (i) {
            case PARAM_PORT:
                tft.print(map.port);
                break;
            case PARAM_CHANNEL:
                tft.print(map.channel);
                break;
            case PARAM_MSG_TYPE:
                switch (map.msgType) {
                    case MIDI_MSG_CC: tft.print("CC"); break;
                    case MIDI_MSG_NOTE: tft.print("Note"); break;
                    case MIDI_MSG_PROGRAM_CHANGE: tft.print("PC"); break;
                    case MIDI_MSG_PITCH_BEND: tft.print("PitchBend"); break;
                }
                break;
            case PARAM_CC_NUMBER:
                tft.print(map.ccNumber);
                break;
            case PARAM_MODE:
                switch (map.mode) {
                    case ENC_MODE_ABSOLUTE: tft.print("Absolute"); break;
                    case ENC_MODE_RELATIVE: tft.print("Relative"); break;
                    case ENC_MODE_CENTER: tft.print("Center"); break;
                }
                break;
            case PARAM_MIN_VALUE:
                tft.print(map.minValue);
                break;
            case PARAM_MAX_VALUE:
                tft.print(map.maxValue);
                break;
            case PARAM_NOTE_CW:
                tft.print(map.noteCW);
                break;
            case PARAM_NOTE_CCW:
                tft.print(map.noteCCW);
                break;
        }

        yPos += 16;
    }

    // Instructions
    tft.setTextColor(ILI9341_DARKGREY);
    tft.setTextSize(1);
    tft.setCursor(10, 210);
    if (isEditingParameter) {
        tft.print("ENCODER: Change value  ENTER: Done");
    } else {
        tft.print("ENC 0-N: Direct adjust  ENTER: Edit");
    }
    tft.setCursor(10, 225);
    tft.print("BACK: ");
    tft.print(isEditingParameter ? "Cancel edit" : "Return to list");
}

void DisplayEncoderButtonMidiList() {
    tft.fillScreen(ILI9341_BLACK);

    // Title
    tft.setTextColor(ILI9341_CYAN);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.println("Encoder Btn MIDI");

    tft.drawFastHLine(10, 35, 300, ILI9341_WHITE);

    // Display encoder button list (show 8 at a time)
    tft.setTextSize(1);
    int yPos = 45;
    int startIdx = (selectedEncoderIndex / 8) * 8;  // Show 8 encoders per page
    int endIdx = min(startIdx + 8, NUM_ENCODERS);

    for (int i = startIdx; i < endIdx; i++) {
        EncoderButtonMidiMap &map = encoderButtonMidiMaps[i];

        if (i == selectedEncoderIndex) {
            // Highlight selected encoder with outlined rectangle
            tft.drawRect(5, yPos - 2, 310, 18, ILI9341_CYAN);
            tft.setTextColor(ILI9341_CYAN);
            tft.setCursor(10, yPos);
            tft.print("> ");
        } else {
            tft.setTextColor(ILI9341_WHITE);
            tft.setCursor(20, yPos);
        }

        // Encoder number and summary
        tft.print("E");
        tft.print(i);
        tft.print(" Btn: CH");
        tft.print(map.channel);
        tft.print(" ");

        // Show message type
        switch (map.msgType) {
            case MIDI_MSG_CC:
                tft.print("CC");
                tft.print(map.ccNumber);
                tft.print("=");
                tft.print(map.ccValue);
                break;
            case MIDI_MSG_NOTE:
                tft.print("Note ");
                tft.print(map.noteNumber);
                break;
            case MIDI_MSG_PROGRAM_CHANGE:
                tft.print("PC ");
                tft.print(map.programNumber);
                break;
            default:
                break;
        }

        yPos += 20;
    }

    // Instructions
    tft.setTextColor(ILI9341_DARKGREY);
    tft.setTextSize(1);
    tft.setCursor(10, 210);
    tft.print("UP/DOWN: Navigate  ENTER: Configure");
    tft.setCursor(10, 225);
    tft.print("BACK: Return to menu");
}

void DisplayEncoderButtonMidiDetail() {
    tft.fillScreen(ILI9341_BLACK);

    EncoderButtonMidiMap &map = encoderButtonMidiMaps[selectedEncoderIndex];

    // Title
    tft.setTextColor(ILI9341_CYAN);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.print("E");
    tft.print(selectedEncoderIndex);
    tft.println(" Btn Config");

    tft.drawFastHLine(10, 35, 300, ILI9341_WHITE);

    // Display parameters
    tft.setTextSize(1);
    int yPos = 45;

    const char* btnParamNames[] = {
        "Port", "Channel", "Type", "Note", "CC Number", "CC Value", "Program"
    };

    for (int i = 0; i < BTN_PARAM_COUNT; i++) {
        // Skip parameters based on message type
        if (map.msgType == MIDI_MSG_NOTE && (i == BTN_PARAM_CC_NUMBER || i == BTN_PARAM_CC_VALUE || i == BTN_PARAM_PROGRAM_NUMBER)) {
            continue;
        }
        if (map.msgType == MIDI_MSG_CC && (i == BTN_PARAM_NOTE_NUMBER || i == BTN_PARAM_PROGRAM_NUMBER)) {
            continue;
        }
        if (map.msgType == MIDI_MSG_PROGRAM_CHANGE && (i == BTN_PARAM_NOTE_NUMBER || i == BTN_PARAM_CC_NUMBER || i == BTN_PARAM_CC_VALUE)) {
            continue;
        }

        // Highlight selected parameter with outlined rectangle
        if (i == selectedDetailParameter) {
            if (isEditingParameter) {
                tft.drawRect(5, yPos - 2, 310, 14, ILI9341_GREEN);  // Green outline when editing
                tft.setTextColor(ILI9341_GREEN);
            } else {
                tft.drawRect(5, yPos - 2, 310, 14, ILI9341_CYAN);   // Cyan outline when selected
                tft.setTextColor(ILI9341_CYAN);
            }
            tft.setCursor(10, yPos);
            tft.print("> ");
        } else {
            tft.setTextColor(ILI9341_WHITE);
            tft.setCursor(20, yPos);
        }

        // Parameter name with encoder index
        tft.print(i);
        tft.print(". ");
        tft.print(btnParamNames[i]);
        tft.print(": ");

        // Parameter value
        tft.setTextColor(ILI9341_YELLOW);
        switch (i) {
            case BTN_PARAM_PORT:
                tft.print(map.port);
                break;
            case BTN_PARAM_CHANNEL:
                tft.print(map.channel);
                break;
            case BTN_PARAM_MSG_TYPE:
                switch (map.msgType) {
                    case MIDI_MSG_CC: tft.print("CC"); break;
                    case MIDI_MSG_NOTE: tft.print("Note"); break;
                    case MIDI_MSG_PROGRAM_CHANGE: tft.print("PC"); break;
                    default: break;
                }
                break;
            case BTN_PARAM_NOTE_NUMBER:
                tft.print(map.noteNumber);
                break;
            case BTN_PARAM_CC_NUMBER:
                tft.print(map.ccNumber);
                break;
            case BTN_PARAM_CC_VALUE:
                tft.print(map.ccValue);
                break;
            case BTN_PARAM_PROGRAM_NUMBER:
                tft.print(map.programNumber);
                break;
        }

        yPos += 16;
    }

    // Instructions
    tft.setTextColor(ILI9341_DARKGREY);
    tft.setTextSize(1);
    tft.setCursor(10, 210);
    if (isEditingParameter) {
        tft.print("ENCODER: Change value  ENTER: Done");
    } else {
        tft.print("ENC 0-N: Direct adjust  ENTER: Edit");
    }
    tft.setCursor(10, 225);
    tft.print("BACK: ");
    tft.print(isEditingParameter ? "Cancel edit" : "Return to list");
}

void DisplayKeysMidiList() {
    tft.fillScreen(ILI9341_BLACK);

    // Title
    tft.setTextColor(ILI9341_CYAN);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.println("Keys MIDI Map");

    tft.drawFastHLine(10, 35, 300, ILI9341_WHITE);

    // Display key list (show 8 at a time)
    tft.setTextSize(1);
    int yPos = 45;
    int startIdx = (selectedKeyIndex / 8) * 8;  // Show 8 keys per page
    int endIdx = min(startIdx + 8, NUM_KEYS);

    for (int i = startIdx; i < endIdx; i++) {
        KeyMidiMap &map = keyMidiMaps[i];

        if (i == selectedKeyIndex) {
            // Highlight selected key with outlined rectangle
            tft.drawRect(5, yPos - 2, 310, 18, ILI9341_CYAN);
            tft.setTextColor(ILI9341_CYAN);
            tft.setCursor(10, yPos);
            tft.print("> ");
        } else {
            tft.setTextColor(ILI9341_WHITE);
            tft.setCursor(20, yPos);
        }

        // Key number and summary
        tft.print("K");
        tft.print(i);
        tft.print(": CH");
        tft.print(map.pressChannel);
        tft.print(" ");

        // Show press message type
        switch (map.pressMsgType) {
            case MIDI_MSG_CC:
                tft.print("CC");
                tft.print(map.pressCcNumber);
                break;
            case MIDI_MSG_NOTE:
                tft.print("Note ");
                tft.print(map.pressNoteNumber);
                break;
            case MIDI_MSG_PROGRAM_CHANGE:
                tft.print("PC ");
                tft.print(map.pressProgramNumber);
                break;
            default:
                break;
        }

        yPos += 20;
    }

    // Instructions
    tft.setTextColor(ILI9341_DARKGREY);
    tft.setTextSize(1);
    tft.setCursor(10, 210);
    tft.print("UP/DOWN: Navigate  ENTER: Configure");
    tft.setCursor(10, 225);
    tft.print("BACK: Return to menu");
}

void DisplayKeysMidiDetail() {
    tft.fillScreen(ILI9341_BLACK);

    KeyMidiMap &map = keyMidiMaps[selectedKeyIndex];

    // Title
    tft.setTextColor(ILI9341_CYAN);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.print("Key ");
    tft.print(selectedKeyIndex);
    tft.print(" ");
    tft.println(isConfiguringPress ? "PRESS" : "RELEASE");

    tft.drawFastHLine(10, 35, 300, ILI9341_WHITE);

    // Get current configuration (press or release)
    uint8_t port = isConfiguringPress ? map.pressPort : map.releasePort;
    uint8_t channel = isConfiguringPress ? map.pressChannel : map.releaseChannel;
    MidiMessageType msgType = isConfiguringPress ? map.pressMsgType : map.releaseMsgType;
    uint8_t noteNumber = isConfiguringPress ? map.pressNoteNumber : map.releaseNoteNumber;
    uint8_t ccNumber = isConfiguringPress ? map.pressCcNumber : map.releaseCcNumber;
    uint8_t ccValue = isConfiguringPress ? map.pressCcValue : map.releaseCcValue;
    uint8_t programNumber = isConfiguringPress ? map.pressProgramNumber : map.releaseProgramNumber;

    // Display parameters
    tft.setTextSize(1);
    int yPos = 45;

    const char* keyParamNames[] = {
        "Port", "Channel", "Type", "Note", "CC Number", "CC Value", "Program"
    };

    for (int i = 0; i < KEY_PARAM_COUNT; i++) {
        // Skip parameters based on message type
        if (msgType == MIDI_MSG_NOTE && (i == KEY_PARAM_CC_NUMBER || i == KEY_PARAM_CC_VALUE || i == KEY_PARAM_PROGRAM_NUMBER)) {
            continue;
        }
        if (msgType == MIDI_MSG_CC && (i == KEY_PARAM_NOTE_NUMBER || i == KEY_PARAM_PROGRAM_NUMBER)) {
            continue;
        }
        if (msgType == MIDI_MSG_PROGRAM_CHANGE && (i == KEY_PARAM_NOTE_NUMBER || i == KEY_PARAM_CC_NUMBER || i == KEY_PARAM_CC_VALUE)) {
            continue;
        }

        // Highlight selected parameter with outlined rectangle
        if (i == selectedDetailParameter) {
            if (isEditingParameter) {
                tft.drawRect(5, yPos - 2, 310, 14, ILI9341_GREEN);  // Green outline when editing
                tft.setTextColor(ILI9341_GREEN);
            } else {
                tft.drawRect(5, yPos - 2, 310, 14, ILI9341_CYAN);   // Cyan outline when selected
                tft.setTextColor(ILI9341_CYAN);
            }
            tft.setCursor(10, yPos);
            tft.print("> ");
        } else {
            tft.setTextColor(ILI9341_WHITE);
            tft.setCursor(20, yPos);
        }

        // Parameter name with encoder index
        tft.print(i);
        tft.print(". ");
        tft.print(keyParamNames[i]);
        tft.print(": ");

        // Parameter value
        tft.setTextColor(ILI9341_YELLOW);
        switch (i) {
            case KEY_PARAM_PORT:
                tft.print(port);
                break;
            case KEY_PARAM_CHANNEL:
                tft.print(channel);
                break;
            case KEY_PARAM_MSG_TYPE:
                switch (msgType) {
                    case MIDI_MSG_CC: tft.print("CC"); break;
                    case MIDI_MSG_NOTE: tft.print("Note"); break;
                    case MIDI_MSG_PROGRAM_CHANGE: tft.print("PC"); break;
                    default: break;
                }
                break;
            case KEY_PARAM_NOTE_NUMBER:
                tft.print(noteNumber);
                break;
            case KEY_PARAM_CC_NUMBER:
                tft.print(ccNumber);
                break;
            case KEY_PARAM_CC_VALUE:
                tft.print(ccValue);
                break;
            case KEY_PARAM_PROGRAM_NUMBER:
                tft.print(programNumber);
                break;
        }

        yPos += 16;
    }

    // Instructions
    tft.setTextColor(ILI9341_DARKGREY);
    tft.setTextSize(1);
    tft.setCursor(10, 195);
    tft.print("LEFT/RIGHT: Switch Press/Release");
    tft.setCursor(10, 210);
    if (isEditingParameter) {
        tft.print("ENCODER: Change value  ENTER: Done");
    } else {
        tft.print("ENC 0-N: Direct adjust  ENTER: Edit");
    }
    tft.setCursor(10, 225);
    tft.print("BACK: ");
    tft.print(isEditingParameter ? "Cancel edit" : "Return to list");
}

void DisplayAdvancedList() {
    tft.fillScreen(ILI9341_BLACK);

    // Title
    tft.setTextColor(ILI9341_CYAN);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.println("ADVANCED");

    // Draw horizontal line
    tft.drawFastHLine(10, 35, 300, ILI9341_WHITE);

    // Menu items
    tft.setTextSize(2);
    int yPos = 50;

    for (int i = 0; i < advancedMenuCount; i++) {
        if (i == currentMenuItem) {
            // Highlight selected item with outlined rectangle
            tft.drawRect(5, yPos - 2, 310, 24, ILI9341_CYAN);
            tft.setTextColor(ILI9341_CYAN);
            tft.setCursor(15, yPos);
            tft.print("> ");
        } else {
            tft.setTextColor(ILI9341_WHITE);
            tft.setCursor(25, yPos);
        }

        tft.println(advancedMenuItems[i]);
        yPos += 30;
    }

    // Instructions at bottom
    tft.setTextSize(1);
    tft.setTextColor(ILI9341_DARKGREY);
    tft.setCursor(10, 210);
    tft.print("UP/DOWN: Navigate  ENTER: Select");
    tft.setCursor(10, 225);
    tft.print("BACK: Return to menu");
}

void DisplayFactoryResetConfirm() {
    tft.fillScreen(ILI9341_BLACK);

    // Warning title
    tft.setTextColor(ILI9341_RED);
    tft.setTextSize(3);
    tft.setCursor(40, 20);
    tft.println("WARNING!");

    // Draw horizontal line
    tft.drawFastHLine(10, 55, 300, ILI9341_RED);

    // Warning message
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_YELLOW);
    tft.setCursor(10, 70);
    tft.println("Factory Reset");

    tft.setTextSize(1);
    tft.setTextColor(ILI9341_WHITE);
    tft.setCursor(10, 100);
    tft.println("This will erase ALL MIDI");
    tft.setCursor(10, 115);
    tft.println("mappings and restore");
    tft.setCursor(10, 130);
    tft.println("factory defaults.");

    tft.setCursor(10, 155);
    tft.setTextColor(ILI9341_YELLOW);
    tft.println("This action cannot be undone!");

    // Instructions
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_GREEN);
    tft.setCursor(10, 185);
    tft.println("ENTER: Confirm");

    tft.setTextColor(ILI9341_RED);
    tft.setCursor(10, 210);
    tft.println("BACK: Cancel");
}

void DisplayFactoryRebootConfirm() {
    tft.fillScreen(ILI9341_BLACK);

    // Warning title
    tft.setTextColor(ILI9341_RED);
    tft.setTextSize(3);
    tft.setCursor(40, 20);
    tft.println("WARNING!");

    // Draw horizontal line
    tft.drawFastHLine(10, 55, 300, ILI9341_RED);

    // Warning message
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_YELLOW);
    tft.setCursor(10, 70);
    tft.println("Reboot ?");

    // Instructions
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_GREEN);
    tft.setCursor(10, 185);
    tft.println("ENTER: Confirm");

    tft.setTextColor(ILI9341_RED);
    tft.setCursor(10, 210);
    tft.println("BACK: Cancel");
}

void DisplaySettingsMenu() {
    switch (currentMenuLevel) {
        case MENU_TOP_LEVEL:
            DisplayTopLevelMenu();
            break;
        case MENU_GENERAL_LIST:
            DisplayGeneralList();
            break;
        case MENU_ENCODER_STATUS:
            DisplayEncoderStatus();
            break;
        case MENU_SWITCH_STATUS:
            DisplaySwitchStatus();
            break;
        case MENU_MIDI_QUANTIZE_LIST:
            DisplayMidiQuantizeList();
            break;
        case MENU_MIDI_QUANTIZE_ENABLE:
            DisplayMidiQuantizeEnable();
            break;
        case MENU_MIDI_QUANTIZE_BPM:
            DisplayMidiQuantizeBPM();
            break;
        case MENU_MIDI_QUANTIZE_SCALE:
            DisplayMidiQuantizeScale();
            break;
        case MENU_MIDI_QUANTIZE_QUANT:
            DisplayMidiQuantizeQuant();
            break;
        case MENU_ENCODER_MIDI_LIST:
            DisplayEncoderMidiList();
            break;
        case MENU_ENCODER_MIDI_DETAIL:
            DisplayEncoderMidiDetail();
            break;
        case MENU_ENCODER_BTN_MIDI_LIST:
            DisplayEncoderButtonMidiList();
            break;
        case MENU_ENCODER_BTN_MIDI_DETAIL:
            DisplayEncoderButtonMidiDetail();
            break;
        case MENU_KEYS_MIDI_LIST:
            DisplayKeysMidiList();
            break;
        case MENU_KEYS_MIDI_DETAIL:
            DisplayKeysMidiDetail();
            break;
        case MENU_ADVANCED_LIST:
            DisplayAdvancedList();
            break;
        case MENU_FACTORY_RESET_CONFIRM:
            DisplayFactoryResetConfirm();
            break;
        case MENU_REBOOT_CONFIRM:
            DisplayFactoryRebootConfirm();
            break;
            
    }
}

#endif // SETTINGS_H
