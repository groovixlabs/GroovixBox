/***************************************************
  GlowDial MIDI Controller - Key Functions

  This file contains all key handling functions including
  key press/release events and key mapping.

  MIT License
 ****************************************************/

#ifndef KEYS_H
#define KEYS_H

#ifndef _GSIMULATOR
#include <FastLED.h>
#endif
#include "settings.h"
#include "sequenceplayback.h"

// External references
extern void SetKeyLEDColor(int keyID, uint8_t r, uint8_t g, uint8_t b);

//==============================================================================
// KEY MAPPING FUNCTIONS
//==============================================================================

const char* GetKeyName(int gkey) {
    
    switch (gkey) {
        case GKEYS_PLAY:        return "Play";
        case GKEYS_COPY:        return "Copy";
        case GKEYS_PASTE:       return "Paste";
        case GKEYS_SETTINGS:    return "Settings";

        case GKEYS_PGUP:        return "PGUP";
        case GKEYS_PGDN:        return "PGDN";
        case GKEYS_BACK:        return "BACK";
        case GKEYS_ENTER:       return "ENTER";

        case GKEYS_SONG:        return "Song";
        case GKEYS_SCENE:       return "Scene";
        case GKEYS_TRACK:       return "Track";
        case GKEYS_PATTERN:     return "Pattern";

        case GKEYS_ADD:         return "ADD";
        case GKEYS_DEL:         return "DEL";
        case GKEYS_MUTE:        return "Mute";
        case GKEYS_PROPS:       return "Props";

        case GKEYS_BANK_A:      return "Bank A";
        case GKEYS_BANK_B:      return "Bank B";
        case GKEYS_BANK_C:      return "Bank C";
        case GKEYS_BANK_D:      return "Bank D";

        case GKEYS_UP:          return "Up";
        case GKEYS_DOWN:        return "Down";
        case GKEYS_LEFT:        return "Left";
        case GKEYS_RIGHT:       return "Right";

        case GKEYS_KEY_1:       return "P1";
        case GKEYS_KEY_2:       return "P2";
        case GKEYS_KEY_3:       return "P3";
        case GKEYS_KEY_4:       return "P4";
        case GKEYS_KEY_5:       return "P5";
        case GKEYS_KEY_6:       return "P6";
        case GKEYS_KEY_7:       return "P7";
        case GKEYS_KEY_8:       return "P8";
    }
    
    return "Unknown";
}

//==============================================================================
// KEY LED FUNCTIONS
//==============================================================================

void ToggleKeyLED(int logicalKeyID) {
    int ledIndex = KeyLedMap[logicalKeyID % 16];
    if (logicalKeyID >= 16) {
        ledIndex += 16;  // Second set of keys
    }

    ledIndex = ledIndex % NUM_KEY_LEDS;  // Safety check

    // Toggle the LED
    if (
        key_rgb_leds[ledIndex].r == 0 &&
        key_rgb_leds[ledIndex].g == 0 &&
        key_rgb_leds[ledIndex].b == 0
    ) {
        key_rgb_leds[ledIndex] = CRGB::Orange;
    } else {
        key_rgb_leds[ledIndex] = CRGB::Black;
    }

    FastLED.show();
}

//==============================================================================
// KEY MIDI SENDING FUNCTIONS
//==============================================================================

// Send MIDI message for key press/release based on configured map
void SendKeyMIDI(int logicalKeyID, bool isPress) {
    if (logicalKeyID < 0 || logicalKeyID >= NUM_KEYS) return;

    KeyMidiMap &keyMap = keyMidiMaps[logicalKeyID];
    ButtonMode mode = keyMap.buttonMode;

    // Check if this key has a note sequence assigned (press only)
    if (isPress && keyMap.pressMsgType == MIDI_MSG_NOTE_SEQUENCE) {
        if (keyMap.sequenceIndex != 255 && keyMap.sequenceIndex < 16) {
            StartSequencePlayback(logicalKeyID, keyMap.sequenceIndex);
            Serial.print("Triggered note sequence ");
            Serial.print(keyMap.sequenceIndex);
            Serial.print(" from key ");
            Serial.println(logicalKeyID);
        } else {
            Serial.print("WARNING: Key ");
            Serial.print(logicalKeyID);
            Serial.println(" has NOTE_SEQUENCE type but no valid sequence assigned");
        }
        return; // Don't process other MIDI types
    }

    // Handle different button modes
    if (mode == BTN_MODE_NORMAL) {
        // Normal mode: NoteOn on press, NoteOff on release
        uint8_t port = isPress ? keyMap.pressPort : keyMap.releasePort;
        uint8_t channel = isPress ? keyMap.pressChannel : keyMap.releaseChannel;
        MidiMessageType msgType = isPress ? keyMap.pressMsgType : keyMap.releaseMsgType;
        uint8_t noteNumber = isPress ? keyMap.pressNoteNumber : keyMap.releaseNoteNumber;
        uint8_t ccNumber = isPress ? keyMap.pressCcNumber : keyMap.releaseCcNumber;
        uint8_t ccValue = isPress ? keyMap.pressCcValue : keyMap.releaseCcValue;
        uint8_t programNumber = isPress ? keyMap.pressProgramNumber : keyMap.releaseProgramNumber;

        switch (msgType) {
            case MIDI_MSG_NOTE:
                if (isPress) {
                    usbMIDI.sendNoteOn(noteNumber, 127, channel);
                } else {
                    usbMIDI.sendNoteOff(noteNumber, 0, channel);
                }
                break;

            case MIDI_MSG_CC:
                usbMIDI.sendControlChange(ccNumber, ccValue, channel);
                break;

            case MIDI_MSG_PROGRAM_CHANGE:
                if (isPress) {
                    usbMIDI.sendProgramChange(programNumber, channel);
                }
                break;

            default:
                break;
        }
    }
    else if (mode == BTN_MODE_HOLD_SINGLE || mode == BTN_MODE_HOLD_MULTIPLE) {
        // Hold modes: Only act on key press (ignore release)
        if (!isPress) return;

        // Toggle the hold state
        bool wasHeld = keyHoldState[logicalKeyID];
        keyHoldState[logicalKeyID] = !wasHeld;

        // In HOLD_SINGLE mode, release the previously held key
        if (mode == BTN_MODE_HOLD_SINGLE && !wasHeld && lastHeldKey != -1 && lastHeldKey != logicalKeyID) {
            // Release the previous key
            KeyMidiMap &prevKeyMap = keyMidiMaps[lastHeldKey];
            if (prevKeyMap.releaseMsgType == MIDI_MSG_NOTE) {
                usbMIDI.sendNoteOff(prevKeyMap.releaseNoteNumber, 0, prevKeyMap.releaseChannel);
            }
            keyHoldState[lastHeldKey] = false;

            // Update LED for previous key
            SetKeyLEDColor(lastHeldKey, 0, 0, 0);  // Turn off LED
        }

        // Send MIDI message
        if (keyHoldState[logicalKeyID]) {
            // Turning on - use press config
            if (keyMap.pressMsgType == MIDI_MSG_NOTE) {
                usbMIDI.sendNoteOn(keyMap.pressNoteNumber, 127, keyMap.pressChannel);
            } else if (keyMap.pressMsgType == MIDI_MSG_CC) {
                usbMIDI.sendControlChange(keyMap.pressCcNumber, keyMap.pressCcValue, keyMap.pressChannel);
            }

            // Update LED to indicate held state (use green)
            SetKeyLEDColor(logicalKeyID, 0, 255, 0);

            // Update last held key for HOLD_SINGLE mode
            if (mode == BTN_MODE_HOLD_SINGLE) {
                lastHeldKey = logicalKeyID;
            }
        } else {
            // Turning off - use release config
            if (keyMap.releaseMsgType == MIDI_MSG_NOTE) {
                usbMIDI.sendNoteOff(keyMap.releaseNoteNumber, 0, keyMap.releaseChannel);
            } else if (keyMap.releaseMsgType == MIDI_MSG_CC) {
                usbMIDI.sendControlChange(keyMap.releaseCcNumber, keyMap.releaseCcValue, keyMap.releaseChannel);
            }

            // Update LED to turn off
            SetKeyLEDColor(logicalKeyID, 0, 0, 0);

            // Clear last held key if this was it
            if (mode == BTN_MODE_HOLD_SINGLE && lastHeldKey == logicalKeyID) {
                lastHeldKey = -1;
            }
        }
    }
}

//==============================================================================
// KEY EVENT HANDLERS
//==============================================================================

void OnKeyPress(int logicalKeyID,int Shift0,int Shift1,int Shift2) {

    // Check if we're in settings mode
    if (IsInSettingsMode()) {
        // Handle settings navigation keys
        switch (logicalKeyID) {
            case GKEYS_UP:
                MenuNavigateUp();
                DisplaySettingsMenu();
                return;
            case GKEYS_DOWN:
                MenuNavigateDown();
                DisplaySettingsMenu();
                return;
            case GKEYS_LEFT:
                MenuNavigateLeft();
                DisplaySettingsMenu();
                return;
            case GKEYS_RIGHT:
                MenuNavigateRight();
                DisplaySettingsMenu();
                return;
            case GKEYS_ENTER:
                MenuEnter();
                DisplaySettingsMenu();
                return;
            case GKEYS_BACK:
                MenuBack();
                // Update display after menu back (either new menu level or home screen)
                if (IsInSettingsMode()) {
                    DisplaySettingsMenu();
                } else {
                    // Exited settings mode, show static home screen
                    DisplayReadyScreen();
                }
                return;

            case GKEYS_SETTINGS:
                ExitSettingsMode();
                DisplayReadyScreen();
                return;

            default:
                // Other keys do nothing in settings mode
                Serial.println("Key press ignored in settings mode");
                return;
        }
    }

    // Normal mode key handling
    Serial.print("Key ");
    Serial.print(logicalKeyID);
    Serial.print(" (");
    Serial.print(GetKeyName(logicalKeyID));
    Serial.print(") ");
    Serial.println("): PRESS");

    // Check for SETTINGS key to enter settings mode

    if (
        #ifndef _GSIMULATOR
        (Shift2)&&
        #endif
        (logicalKeyID == GKEYS_SETTINGS)) 
    {
        EnterSettingsMode();
        DisplaySettingsMenu();
        return;
    }

    if (Shift0)
    {
        SetKeyLEDColor(logicalKeyID, 0, 255, 255);  // Red LED
    }
    else if (Shift1)
    {
        SetKeyLEDColor(logicalKeyID, 255, 0, 255);  // Red LED
    }
    else
    {
        SetKeyLEDColor(logicalKeyID, 0, 0, 0);  // Red LED
    }


    // Check for PLAY key to toggle play mode
    if (logicalKeyID == GKEYS_PLAY) {
        TogglePlayMode();

        // Update LED: Red when playing, off when stopped
        if (IsInPlayMode()) {
            SetKeyLEDColor(GKEYS_PLAY, 255, 0, 0);  // Red LED
        } else {
            SetKeyLEDColor(GKEYS_PLAY, 0, 0, 0);    // LED off
        }
        return;
    }

    // Send MIDI based on configured key map
    SendKeyMIDI(logicalKeyID, true);  // true = press

    // Toggle LED
    //ToggleKeyLED(logicalKeyID);
}

void OnKeyRelease(int logicalKeyID,int Shift0,int Shift1,int Shift2) {

    // In settings mode, ignore key releases (navigation is on key press)
    if (IsInSettingsMode()) {
        return;
    }

    Serial.print("Key ");
    Serial.print(logicalKeyID);
    Serial.print(" (");
    Serial.print(GetKeyName(logicalKeyID));
    Serial.print(") ");
    Serial.println(": RELEASE");

    // Send MIDI based on configured key map
    SendKeyMIDI(logicalKeyID, false);  // false = release
}

#endif // KEYS_H
