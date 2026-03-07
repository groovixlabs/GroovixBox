/***************************************************
  GlowDial MIDI Controller - EEPROM Storage

  This file contains EEPROM storage functions for
  persisting MIDI mapping configurations.

  MIT License
 ****************************************************/

#ifndef EEPROM_STORAGE_H
#define EEPROM_STORAGE_H

#ifndef _GSIMULATOR

#include <EEPROM.h>

#endif

#include "settings.h"

//==============================================================================
// EEPROM MEMORY MAP
//==============================================================================

// Magic number to verify EEPROM has been initialized
#define EEPROM_MAGIC_NUMBER 0x474C4F57  // "GLOW" in ASCII

// EEPROM addresses
#define EEPROM_ADDR_MAGIC           0
#define EEPROM_ADDR_ENCODER_MAPS    (EEPROM_ADDR_MAGIC + sizeof(uint32_t))
#define EEPROM_ADDR_ENCODER_BTN_MAPS (EEPROM_ADDR_ENCODER_MAPS + (sizeof(EncoderMidiMap) * NUM_ENCODERS))
#define EEPROM_ADDR_KEY_MAPS        (EEPROM_ADDR_ENCODER_BTN_MAPS + (sizeof(EncoderButtonMidiMap) * NUM_ENCODERS))
#define EEPROM_ADDR_QUANTIZE        (EEPROM_ADDR_KEY_MAPS + (sizeof(KeyMidiMap) * NUM_KEYS))

// Total EEPROM size used
#define EEPROM_TOTAL_SIZE           (EEPROM_ADDR_QUANTIZE + sizeof(bool) + sizeof(uint16_t) + sizeof(MusicalScale) + sizeof(NoteQuantization))

//==============================================================================
// EEPROM FUNCTIONS
//==============================================================================

// Check if EEPROM has been initialized
bool IsEEPROMInitialized() {
    uint32_t magic;
    EEPROM.get(EEPROM_ADDR_MAGIC, magic);
    return (magic == EEPROM_MAGIC_NUMBER);
}

// Initialize EEPROM with magic number
void InitializeEEPROM() {
    uint32_t magic = EEPROM_MAGIC_NUMBER;
    EEPROM.put(EEPROM_ADDR_MAGIC, magic);
    Serial.println("EEPROM: Initialized with magic number");
}

// Save all MIDI maps to EEPROM
void SaveAllMidiMapsToEEPROM() {
    Serial.println("EEPROM: Saving all MIDI maps...");

    // Save encoder MIDI maps
    int addr = EEPROM_ADDR_ENCODER_MAPS;
    for (int i = 0; i < NUM_ENCODERS; i++) {
        EEPROM.put(addr, encoderMidiMaps[i]);
        addr += sizeof(EncoderMidiMap);
    }
    Serial.print("EEPROM: Saved ");
    Serial.print(NUM_ENCODERS);
    Serial.println(" encoder maps");

    // Save encoder button MIDI maps
    addr = EEPROM_ADDR_ENCODER_BTN_MAPS;
    for (int i = 0; i < NUM_ENCODERS; i++) {
        EEPROM.put(addr, encoderButtonMidiMaps[i]);
        addr += sizeof(EncoderButtonMidiMap);
    }
    Serial.print("EEPROM: Saved ");
    Serial.print(NUM_ENCODERS);
    Serial.println(" encoder button maps");

    // Save key MIDI maps
    addr = EEPROM_ADDR_KEY_MAPS;
    for (int i = 0; i < NUM_KEYS; i++) {
        EEPROM.put(addr, keyMidiMaps[i]);
        addr += sizeof(KeyMidiMap);
    }
    Serial.print("EEPROM: Saved ");
    Serial.print(NUM_KEYS);
    Serial.println(" key maps");

    // Save MIDI Quantize settings
    addr = EEPROM_ADDR_QUANTIZE;
    EEPROM.put(addr, globalQuantizeEnabled);
    addr += sizeof(bool);
    EEPROM.put(addr, globalBPM);
    addr += sizeof(uint16_t);
    EEPROM.put(addr, globalScale);
    addr += sizeof(MusicalScale);
    EEPROM.put(addr, globalQuantization);
    Serial.println("EEPROM: Saved quantize settings");

    Serial.print("EEPROM: Total bytes used: ");
    Serial.println(EEPROM_TOTAL_SIZE);
}

// Load all MIDI maps from EEPROM
void LoadAllMidiMapsFromEEPROM() {
    if (!IsEEPROMInitialized()) {
        Serial.println("EEPROM: Not initialized, using defaults");
        InitializeEncoderMidiMaps();
        InitializeEncoderButtonMidiMaps();
        InitializeKeyMidiMaps();
        // Reset quantize settings to defaults
        globalQuantizeEnabled = true;
        globalBPM = 120;
        globalScale = SCALE_CHROMATIC;
        globalQuantization = QUANT_1_16;
        InitializeEEPROM();
        SaveAllMidiMapsToEEPROM();
        return;
    }

    Serial.println("EEPROM: Loading all MIDI maps...");

    // Load encoder MIDI maps
    int addr = EEPROM_ADDR_ENCODER_MAPS;
    for (int i = 0; i < NUM_ENCODERS; i++) {
        EEPROM.get(addr, encoderMidiMaps[i]);
        addr += sizeof(EncoderMidiMap);
    }
    Serial.print("EEPROM: Loaded ");
    Serial.print(NUM_ENCODERS);
    Serial.println(" encoder maps");

    // Load encoder button MIDI maps
    addr = EEPROM_ADDR_ENCODER_BTN_MAPS;
    for (int i = 0; i < NUM_ENCODERS; i++) {
        EEPROM.get(addr, encoderButtonMidiMaps[i]);
        addr += sizeof(EncoderButtonMidiMap);
    }
    Serial.print("EEPROM: Loaded ");
    Serial.print(NUM_ENCODERS);
    Serial.println(" encoder button maps");

    // Load key MIDI maps
    addr = EEPROM_ADDR_KEY_MAPS;
    for (int i = 0; i < NUM_KEYS; i++) {
        EEPROM.get(addr, keyMidiMaps[i]);
        addr += sizeof(KeyMidiMap);
    }
    Serial.print("EEPROM: Loaded ");
    Serial.print(NUM_KEYS);
    Serial.println(" key maps");

    // Load MIDI Quantize settings
    addr = EEPROM_ADDR_QUANTIZE;
    EEPROM.get(addr, globalQuantizeEnabled);
    addr += sizeof(bool);
    EEPROM.get(addr, globalBPM);
    addr += sizeof(uint16_t);
    EEPROM.get(addr, globalScale);
    addr += sizeof(MusicalScale);
    EEPROM.get(addr, globalQuantization);

    // Validate loaded values and set to defaults if invalid
    // Note: bool doesn't need validation as it will be 0 or 1
    if (globalBPM < 20 || globalBPM > 300) {
        globalBPM = 120;
        Serial.println("EEPROM: Invalid BPM, set to default 120");
    }
    if (globalScale > SCALE_B_MINOR) {
        globalScale = SCALE_CHROMATIC;
        Serial.println("EEPROM: Invalid scale, set to default Chromatic");
    }
    if (globalQuantization > QUANT_1) {
        globalQuantization = QUANT_1_16;
        Serial.println("EEPROM: Invalid quantization, set to default 1/16");
    }

    Serial.println("EEPROM: Loaded quantize settings");
}

// Save individual encoder MIDI map
void SaveEncoderMidiMap(int encoderIndex) {
    if (encoderIndex < 0 || encoderIndex >= NUM_ENCODERS) return;

    int addr = EEPROM_ADDR_ENCODER_MAPS + (encoderIndex * sizeof(EncoderMidiMap));
    EEPROM.put(addr, encoderMidiMaps[encoderIndex]);

    Serial.print("EEPROM: Saved encoder ");
    Serial.print(encoderIndex);
    Serial.println(" map");
}

// Save individual encoder button MIDI map
void SaveEncoderButtonMidiMap(int encoderIndex) {
    if (encoderIndex < 0 || encoderIndex >= NUM_ENCODERS) return;

    int addr = EEPROM_ADDR_ENCODER_BTN_MAPS + (encoderIndex * sizeof(EncoderButtonMidiMap));
    EEPROM.put(addr, encoderButtonMidiMaps[encoderIndex]);

    Serial.print("EEPROM: Saved encoder button ");
    Serial.print(encoderIndex);
    Serial.println(" map");
}

// Save individual key MIDI map
void SaveKeyMidiMap(int keyIndex) {
    if (keyIndex < 0 || keyIndex >= NUM_KEYS) return;

    int addr = EEPROM_ADDR_KEY_MAPS + (keyIndex * sizeof(KeyMidiMap));
    EEPROM.put(addr, keyMidiMaps[keyIndex]);

    Serial.print("EEPROM: Saved key ");
    Serial.print(keyIndex);
    Serial.println(" map");
}

// Save MIDI Quantize settings
void SaveQuantizeSettings() {
    int addr = EEPROM_ADDR_QUANTIZE;
    EEPROM.put(addr, globalQuantizeEnabled);
    addr += sizeof(bool);
    EEPROM.put(addr, globalBPM);
    addr += sizeof(uint16_t);
    EEPROM.put(addr, globalScale);
    addr += sizeof(MusicalScale);
    EEPROM.put(addr, globalQuantization);
    Serial.println("EEPROM: Saved quantize settings");
}

// Reset EEPROM to defaults
void ResetEEPROMToDefaults() {
    Serial.println("EEPROM: Resetting to defaults...");
    InitializeEncoderMidiMaps();
    InitializeEncoderButtonMidiMaps();
    InitializeKeyMidiMaps();
    // Reset quantize settings to defaults
    globalQuantizeEnabled = true;
    globalBPM = 120;
    globalScale = SCALE_CHROMATIC;
    globalQuantization = QUANT_1_16;
    InitializeEEPROM();
    SaveAllMidiMapsToEEPROM();
    Serial.println("EEPROM: Reset complete");
}

#endif // EEPROM_STORAGE_H
