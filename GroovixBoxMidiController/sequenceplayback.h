/***************************************************
  GlowDial MIDI Controller - Sequence Playback Engine

  This file contains the playback engine for note sequences.
  Implements non-blocking, timer-based playback with support
  for concurrent sequences and polyphony.

  MIT License
 ****************************************************/

#ifndef SEQUENCEPLAYBACK_H
#define SEQUENCEPLAYBACK_H

#include <Arduino.h>
#include "notesequence.h"

//==============================================================================
// CONSTANTS
//==============================================================================

#define MAX_CONCURRENT_SEQUENCES 4    // Maximum simultaneous playbacks
#define MAX_SCHEDULED_NOTE_OFFS 32    // Maximum scheduled note-off events

//==============================================================================
// DATA STRUCTURES
//==============================================================================

/**
 * State tracking for an active sequence playback
 */
struct SequencePlaybackState {
    uint8_t keyIndex;           // Which key triggered this sequence
    uint8_t sequenceIndex;      // Which sequence is playing
    uint8_t eventIndex;         // Current event position in sequence
    unsigned long startTime;    // millis() when sequence started
    unsigned long nextEventTime;// millis() when next event should trigger
    unsigned long elapsedTicks; // Accumulated ticks elapsed
    bool active;                // Is this playback slot active?
};

/**
 * Scheduled note-off event
 */
struct ScheduledNoteOff {
    unsigned long offTime;  // millis() when to send NoteOff
    uint8_t channel;        // MIDI channel
    uint8_t noteNumber;     // MIDI note number
    bool active;            // Is this slot active?
};

//==============================================================================
// GLOBAL STATE
//==============================================================================

// Active playback tracking
SequencePlaybackState activePlaybacks[MAX_CONCURRENT_SEQUENCES];

// Scheduled note-off queue
ScheduledNoteOff noteOffQueue[MAX_SCHEDULED_NOTE_OFFS];

//==============================================================================
// TIMING FUNCTIONS
//==============================================================================

/**
 * Convert MIDI ticks to milliseconds
 * Formula: ms = (ticks * 60000) / (BPM * PPQN)
 */
unsigned long TicksToMillis(uint16_t ticks, uint16_t ppqn, uint16_t bpm) {
    if (bpm == 0 || ppqn == 0) return 0;

    float msPerTick = 60000.0f / (float)(bpm * ppqn);
    return (unsigned long)(ticks * msPerTick);
}

//==============================================================================
// NOTE OFF SCHEDULING
//==============================================================================

/**
 * Schedule a note-off event
 */
void ScheduleNoteOff(unsigned long offTime, uint8_t channel, uint8_t noteNumber) {
    // Find free slot
    for (int i = 0; i < MAX_SCHEDULED_NOTE_OFFS; i++) {
        if (!noteOffQueue[i].active) {
            noteOffQueue[i].offTime = offTime;
            noteOffQueue[i].channel = channel;
            noteOffQueue[i].noteNumber = noteNumber;
            noteOffQueue[i].active = true;
            return;
        }
    }

    // No free slots - send note off immediately to prevent stuck notes
    Serial.println("WARNING: Note-off queue full, sending immediate NoteOff");
    usbMIDI.sendNoteOff(noteNumber, 0, channel);
}

/**
 * Process scheduled note-offs
 * Call this from main loop
 */
void ProcessScheduledNoteOffs() {
    unsigned long now = millis();

    for (int i = 0; i < MAX_SCHEDULED_NOTE_OFFS; i++) {
        if (noteOffQueue[i].active && now >= noteOffQueue[i].offTime) {
            // Send note off
            usbMIDI.sendNoteOff(noteOffQueue[i].noteNumber, 0, noteOffQueue[i].channel);

            // Mark slot as free
            noteOffQueue[i].active = false;
        }
    }
}

//==============================================================================
// PLAYBACK CONTROL
//==============================================================================

/**
 * Initialize playback system
 * Call once during setup()
 */
void InitializeSequencePlayback() {
    // Clear all playback slots
    for (int i = 0; i < MAX_CONCURRENT_SEQUENCES; i++) {
        activePlaybacks[i].active = false;
        activePlaybacks[i].keyIndex = 0;
        activePlaybacks[i].sequenceIndex = 0;
        activePlaybacks[i].eventIndex = 0;
        activePlaybacks[i].startTime = 0;
        activePlaybacks[i].nextEventTime = 0;
        activePlaybacks[i].elapsedTicks = 0;
    }

    // Clear note-off queue
    for (int i = 0; i < MAX_SCHEDULED_NOTE_OFFS; i++) {
        noteOffQueue[i].active = false;
        noteOffQueue[i].offTime = 0;
        noteOffQueue[i].channel = 1;
        noteOffQueue[i].noteNumber = 0;
    }

    Serial.println("Sequence playback initialized");
}

/**
 * Find a free playback slot
 * Returns slot index or -1 if all slots occupied
 */
int FindFreePlaybackSlot() {
    for (int i = 0; i < MAX_CONCURRENT_SEQUENCES; i++) {
        if (!activePlaybacks[i].active) {
            return i;
        }
    }
    return -1;
}

/**
 * Start playing a sequence
 * Returns true if started, false if no free slots or invalid sequence
 */
bool StartSequencePlayback(int keyIndex, int sequenceIndex) {
    // Validate parameters
    if (sequenceIndex < 0 || sequenceIndex >= NUM_NOTE_SEQUENCES) {
        Serial.println("ERROR: Invalid sequence index");
        return false;
    }

    if (!noteSequences[sequenceIndex].isActive) {
        Serial.println("ERROR: Sequence not active");
        return false;
    }

    if (noteSequences[sequenceIndex].header.eventCount == 0) {
        Serial.println("ERROR: Sequence is empty");
        return false;
    }

    // Find free playback slot
    int slotIndex = FindFreePlaybackSlot();
    if (slotIndex < 0) {
        Serial.println("ERROR: No free playback slots (max concurrent sequences reached)");
        return false;
    }

    // Initialize playback state
    SequencePlaybackState &state = activePlaybacks[slotIndex];
    state.keyIndex = keyIndex;
    state.sequenceIndex = sequenceIndex;
    state.eventIndex = 0;
    state.startTime = millis();
    state.nextEventTime = state.startTime;  // First event plays immediately
    state.elapsedTicks = 0;
    state.active = true;

    Serial.print("Started sequence ");
    Serial.print(sequenceIndex);
    Serial.print(" on key ");
    Serial.print(keyIndex);
    Serial.print(" (slot ");
    Serial.print(slotIndex);
    Serial.println(")");

    return true;
}

/**
 * Stop a specific playback
 */
void StopSequencePlayback(int slotIndex) {
    if (slotIndex < 0 || slotIndex >= MAX_CONCURRENT_SEQUENCES) return;

    activePlaybacks[slotIndex].active = false;

    Serial.print("Stopped playback slot ");
    Serial.println(slotIndex);
}

/**
 * Stop all playbacks
 */
void StopAllSequencePlaybacks() {
    for (int i = 0; i < MAX_CONCURRENT_SEQUENCES; i++) {
        activePlaybacks[i].active = false;
    }

    Serial.println("Stopped all sequence playbacks");
}

//==============================================================================
// PLAYBACK UPDATE (MAIN LOOP)
//==============================================================================

/**
 * Update all active sequence playbacks
 * Call this from main loop
 */
void UpdateSequencePlaybacks() {
    unsigned long now = millis();

    for (int i = 0; i < MAX_CONCURRENT_SEQUENCES; i++) {
        if (!activePlaybacks[i].active) continue;

        SequencePlaybackState &state = activePlaybacks[i];
        NoteSequence &seq = noteSequences[state.sequenceIndex];

        // Check if it's time to play the next event
        if (now >= state.nextEventTime) {
            // Check if we've finished the sequence
            if (state.eventIndex >= seq.header.eventCount) {
                // Sequence complete
                state.active = false;
                Serial.print("Sequence ");
                Serial.print(state.sequenceIndex);
                Serial.println(" completed");
                continue;
            }

            // Get current event
            NoteEvent &evt = seq.events[state.eventIndex];

            // Send NoteOn
            if (evt.velocity > 0) {
                usbMIDI.sendNoteOn(evt.noteNumber, evt.velocity, seq.header.channel);

                // Schedule NoteOff based on duration
                if (evt.duration > 0) {
                    unsigned long durationMs = TicksToMillis(evt.duration,
                                                             seq.header.ppqn,
                                                             seq.header.tempo);
                    ScheduleNoteOff(now + durationMs, seq.header.channel, evt.noteNumber);
                }
            }

            // Advance to next event
            state.eventIndex++;

            // Calculate next event time
            if (state.eventIndex < seq.header.eventCount) {
                NoteEvent &nextEvt = seq.events[state.eventIndex];
                unsigned long deltaMs = TicksToMillis(nextEvt.deltaTime,
                                                      seq.header.ppqn,
                                                      seq.header.tempo);
                state.nextEventTime = now + deltaMs;
            }
        }
    }
}

#endif // SEQUENCEPLAYBACK_H
