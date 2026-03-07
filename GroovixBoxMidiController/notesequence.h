/***************************************************
  GlowDial MIDI Controller - Note Sequence Storage

  This file contains data structures for storing MIDI
  note sequences using the MIDI file standard format
  (delta time, PPQN, tempo).

  MIT License
 ****************************************************/

#ifndef NOTESEQUENCE_H
#define NOTESEQUENCE_H

#include <Arduino.h>

//==============================================================================
// CONSTANTS
//==============================================================================

#define MAX_SEQUENCE_EVENTS 256       // Maximum notes per sequence
#define NUM_NOTE_SEQUENCES 16         // Total number of sequences (RAM only, no EEPROM limit)
#define NO_SEQUENCE_ASSIGNED 255      // Special value for unused sequence slot

//==============================================================================
// DATA STRUCTURES
//==============================================================================

/**
 * Individual note event in a sequence
 * Uses MIDI file standard format with delta time and duration in ticks
 */
struct NoteEvent {
    uint16_t deltaTime;    // Ticks from previous event (0-65535)
    uint8_t noteNumber;    // MIDI note number (0-127)
    uint8_t velocity;      // Note velocity (1-127, 0 for note off)
    uint16_t duration;     // Note duration in ticks (0-65535)
};

/**
 * Sequence header containing timing and playback parameters
 */
struct NoteSequenceHeader {
    uint16_t ppqn;         // Pulses per quarter note (24-960, typically 96-480)
    uint16_t tempo;        // Tempo in BPM (20-300)
    uint8_t eventCount;    // Number of events in sequence (0-32)
    uint8_t channel;       // MIDI channel for playback (1-16)
};

/**
 * Complete note sequence with header and events
 * Size: 6 + (6 * 256) + 2 = 1,544 bytes
 */
struct NoteSequence {
    NoteSequenceHeader header;
    NoteEvent events[MAX_SEQUENCE_EVENTS];
    bool isActive;         // Is this sequence slot in use?
    uint8_t reserved;      // Padding for alignment
};

//==============================================================================
// GLOBAL STORAGE
//==============================================================================

// Global array of note sequences
NoteSequence noteSequences[NUM_NOTE_SEQUENCES];

//==============================================================================
// INITIALIZATION
//==============================================================================

/**
 * Initialize all note sequences to empty/inactive state
 * Should be called once during setup()
 */
void InitializeNoteSequences() {
    for (int i = 0; i < NUM_NOTE_SEQUENCES; i++) {
        noteSequences[i].isActive = false;
        noteSequences[i].header.ppqn = 96;        // Default PPQN
        noteSequences[i].header.tempo = 120;      // Default tempo
        noteSequences[i].header.eventCount = 0;
        noteSequences[i].header.channel = 1;      // Default channel
        noteSequences[i].reserved = 0;

        // Clear all events
        for (int j = 0; j < MAX_SEQUENCE_EVENTS; j++) {
            noteSequences[i].events[j].deltaTime = 0;
            noteSequences[i].events[j].noteNumber = 0;
            noteSequences[i].events[j].velocity = 0;
            noteSequences[i].events[j].duration = 0;
        }
    }

    Serial.println("Note sequences initialized");
}

/**
 * Clear a specific sequence
 */
void ClearNoteSequence(int sequenceIndex) {
    if (sequenceIndex < 0 || sequenceIndex >= NUM_NOTE_SEQUENCES) return;

    noteSequences[sequenceIndex].isActive = false;
    noteSequences[sequenceIndex].header.eventCount = 0;

    // Clear all events
    for (int j = 0; j < MAX_SEQUENCE_EVENTS; j++) {
        noteSequences[sequenceIndex].events[j].deltaTime = 0;
        noteSequences[sequenceIndex].events[j].noteNumber = 0;
        noteSequences[sequenceIndex].events[j].velocity = 0;
        noteSequences[sequenceIndex].events[j].duration = 0;
    }

    Serial.print("Cleared sequence ");
    Serial.println(sequenceIndex);
}

/**
 * Validate sequence parameters
 * Returns true if valid, false otherwise
 */
bool ValidateSequence(int sequenceIndex) {
    if (sequenceIndex < 0 || sequenceIndex >= NUM_NOTE_SEQUENCES) {
        return false;
    }

    NoteSequence &seq = noteSequences[sequenceIndex];

    // Check PPQN range (24-960)
    if (seq.header.ppqn < 24 || seq.header.ppqn > 960) {
        return false;
    }

    // Check tempo range (20-300 BPM)
    if (seq.header.tempo < 20 || seq.header.tempo > 300) {
        return false;
    }

    // Check event count
    if (seq.header.eventCount > MAX_SEQUENCE_EVENTS) {
        return false;
    }

    // Check channel (1-16)
    if (seq.header.channel < 1 || seq.header.channel > 16) {
        return false;
    }

    // Validate each event
    for (int i = 0; i < seq.header.eventCount; i++) {
        // Check note number (0-127)
        if (seq.events[i].noteNumber > 127) {
            return false;
        }

        // Check velocity (0-127)
        if (seq.events[i].velocity > 127) {
            return false;
        }
    }

    return true;
}

#endif // NOTESEQUENCE_H
