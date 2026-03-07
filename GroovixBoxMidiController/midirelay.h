/***************************************************
  GlowDial MIDI Controller - MIDI Relay

  This file contains MIDI relay functionality that:
  - Maps incoming MIDI notes to the selected musical scale
  - Applies timing quantization based on BPM and quantization settings
  - Relays processed MIDI events to output

  MIT License
 ***************************************************/

#ifndef MIDIRELAY_H
#define MIDIRELAY_H

#include "settings.h"

//==============================================================================
// NOTE NAME CONVERSION
//==============================================================================

const char* NOTE_NAMES[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

// Convert MIDI note number to note name (e.g., 60 -> "C4")
void GetNoteName(byte midiNote, char* buffer, size_t bufferSize) {
    if (midiNote > 127) {
        snprintf(buffer, bufferSize, "INV");
        return;
    }

    int octave = (midiNote / 12) - 1;  // MIDI note 0 = C-1
    int noteIndex = midiNote % 12;

    snprintf(buffer, bufferSize, "%s%d", NOTE_NAMES[noteIndex], octave);
}

//==============================================================================
// MUSICAL SCALE DEFINITIONS
//==============================================================================

// Each scale contains the semitone offsets from the root note (C = 0)
// Chromatic scale contains all 12 notes
const byte SCALE_CHROMATIC_NOTES[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
const byte SCALE_CHROMATIC_COUNT = 12;

// Major scale pattern (Ionian mode): W-W-H-W-W-W-H (W=whole step=2, H=half step=1)
const byte SCALE_MAJOR_NOTES[] = {0, 2, 4, 5, 7, 9, 11};
const byte SCALE_MAJOR_COUNT = 7;

// Natural Minor scale pattern (Aeolian mode): W-H-W-W-H-W-W
const byte SCALE_MINOR_NOTES[] = {0, 2, 3, 5, 7, 8, 10};
const byte SCALE_MINOR_COUNT = 7;

// Dorian mode: W-H-W-W-W-H-W
const byte SCALE_DORIAN_NOTES[] = {0, 2, 3, 5, 7, 9, 10};
const byte SCALE_DORIAN_COUNT = 7;

// Phrygian mode: H-W-W-W-H-W-W
const byte SCALE_PHRYGIAN_NOTES[] = {0, 1, 3, 5, 7, 8, 10};
const byte SCALE_PHRYGIAN_COUNT = 7;

// Lydian mode: W-W-W-H-W-W-H
const byte SCALE_LYDIAN_NOTES[] = {0, 2, 4, 6, 7, 9, 11};
const byte SCALE_LYDIAN_COUNT = 7;

// Mixolydian mode: W-W-H-W-W-H-W
const byte SCALE_MIXOLYDIAN_NOTES[] = {0, 2, 4, 5, 7, 9, 10};
const byte SCALE_MIXOLYDIAN_COUNT = 7;

// Locrian mode: H-W-W-H-W-W-W
const byte SCALE_LOCRIAN_NOTES[] = {0, 1, 3, 5, 6, 8, 10};
const byte SCALE_LOCRIAN_COUNT = 7;

// Scale root note offsets (C=0, D=2, E=4, F=5, G=7, A=9, B=11)
const byte SCALE_ROOTS[] = {
    0,   // SCALE_CHROMATIC (no root needed)
    0,   // SCALE_C_MAJOR (C)
    0,   // SCALE_C_MINOR (C)
    0,   // SCALE_C_DORIAN (C)
    0,   // SCALE_C_PHRYGIAN (C)
    0,   // SCALE_C_LYDIAN (C)
    0,   // SCALE_C_MIXOLYDIAN (C)
    0,   // SCALE_C_LOCRIAN (C)
    2,   // SCALE_D_MAJOR (D)
    2,   // SCALE_D_MINOR (D)
    4,   // SCALE_E_MAJOR (E)
    4,   // SCALE_E_MINOR (E)
    5,   // SCALE_F_MAJOR (F)
    5,   // SCALE_F_MINOR (F)
    7,   // SCALE_G_MAJOR (G)
    7,   // SCALE_G_MINOR (G)
    9,   // SCALE_A_MAJOR (A)
    9,   // SCALE_A_MINOR (A)
    11,  // SCALE_B_MAJOR (B)
    11   // SCALE_B_MINOR (B)
};

//==============================================================================
// MIDI EVENT QUEUE
//==============================================================================

#define MAX_MIDI_EVENTS 64
#define MAX_ACTIVE_NOTES 128  // Track all possible MIDI notes (0-127)

enum MidiEventType {
    MIDI_EVENT_NOTE_ON,
    MIDI_EVENT_NOTE_OFF,
    MIDI_EVENT_CONTROL_CHANGE
};

struct MidiEvent {
    MidiEventType type;
    unsigned long scheduledTime;  // Milliseconds when to send
    byte channel;
    byte data1;  // Note or CC number
    byte data2;  // Velocity or CC value
    bool active;
};

// Track scheduled note-on times for each note to enforce minimum duration
struct ActiveNote {
    unsigned long noteOnTime;  // Quantized time when note-on was scheduled
    byte channel;
    bool active;
};

MidiEvent midiEventQueue[MAX_MIDI_EVENTS];
ActiveNote activeNotes[MAX_ACTIVE_NOTES];  // One entry per MIDI note (0-127)
unsigned long lastQuantizationCheck = 0;
unsigned long quantizationStartTime = 0;  // Reference time for quantization grid

//==============================================================================
// TIMING FUNCTIONS
//==============================================================================

// Calculate milliseconds per beat based on BPM
float GetMillisecondsPerBeat() {
    return 60000.0f / (float)globalBPM;
}

// Calculate quantization interval in milliseconds
float GetQuantizationInterval() {
    float msPerBeat = GetMillisecondsPerBeat();

    switch (globalQuantization) {
        case QUANT_1_32:
            return msPerBeat / 8.0f;  // 1/32 note = 1/8 of a quarter note
        case QUANT_1_16:
            return msPerBeat / 4.0f;  // 1/16 note = 1/4 of a quarter note
        case QUANT_1_8:
            return msPerBeat / 2.0f;  // 1/8 note = 1/2 of a quarter note
        case QUANT_1_4:
            return msPerBeat;         // 1/4 note = 1 beat
        case QUANT_1_2:
            return msPerBeat * 2.0f;  // 1/2 note = 2 beats
        case QUANT_1:
            return msPerBeat * 4.0f;  // Whole note = 4 beats
        default:
            return msPerBeat / 4.0f;  // Default to 1/16
    }
}

// Quantize a timestamp to the nearest grid point
unsigned long QuantizeTime(unsigned long timestamp) {
    float interval = GetQuantizationInterval();
    unsigned long elapsed = timestamp - quantizationStartTime;
    unsigned long gridPoint = (unsigned long)((elapsed / interval) + 0.5f);
    return quantizationStartTime + (unsigned long)(gridPoint * interval);
}

//==============================================================================
// SCALE MAPPING FUNCTIONS
//==============================================================================

// Get the scale notes for the current global scale
void GetCurrentScaleNotes(const byte*& notes, byte& count) {
    switch (globalScale) {
        case SCALE_CHROMATIC:
            notes = SCALE_CHROMATIC_NOTES;
            count = SCALE_CHROMATIC_COUNT;
            break;

        case SCALE_C_DORIAN:
            notes = SCALE_DORIAN_NOTES;
            count = SCALE_DORIAN_COUNT;
            break;

        case SCALE_C_PHRYGIAN:
            notes = SCALE_PHRYGIAN_NOTES;
            count = SCALE_PHRYGIAN_COUNT;
            break;

        case SCALE_C_LYDIAN:
            notes = SCALE_LYDIAN_NOTES;
            count = SCALE_LYDIAN_COUNT;
            break;

        case SCALE_C_MIXOLYDIAN:
            notes = SCALE_MIXOLYDIAN_NOTES;
            count = SCALE_MIXOLYDIAN_COUNT;
            break;

        case SCALE_C_LOCRIAN:
            notes = SCALE_LOCRIAN_NOTES;
            count = SCALE_LOCRIAN_COUNT;
            break;

        default:
            // All other scales use either major or minor pattern
            // For D-B scales: odd indices are major, even indices are minor
            // (after skipping chromatic=0 and C modes=1-7)
            bool isMajor = ((int)globalScale % 2 == 1);
            if (isMajor) {
                notes = SCALE_MAJOR_NOTES;
                count = SCALE_MAJOR_COUNT;
            } else {
                notes = SCALE_MINOR_NOTES;
                count = SCALE_MINOR_COUNT;
            }
            break;
    }
}

// Map a MIDI note to the nearest note in the current scale
byte MapNoteToScale(byte inputNote) {
    // If chromatic scale, return note unchanged
    if (globalScale == SCALE_CHROMATIC) {
        return inputNote;
    }

    // Get current scale notes
    const byte* scaleNotes;
    byte scaleCount;
    GetCurrentScaleNotes(scaleNotes, scaleCount);

    // Get root note for current scale
    byte rootNote = SCALE_ROOTS[globalScale];

    // Calculate octave and note within octave
    byte octave = inputNote / 12;
    byte noteInOctave = inputNote % 12;

    // Find nearest note in scale (stay within the same octave)
    byte nearestScaleNote = scaleNotes[0];
    int minDistance = 12;

    // Convert scale notes to absolute positions by adding root
    for (byte i = 0; i < scaleCount; i++) {
        byte scaleNoteAbsolute = (scaleNotes[i] + rootNote) % 12;
        int distance = abs((int)noteInOctave - (int)scaleNoteAbsolute);

        // Handle wraparound: if distance > 6, the opposite direction is shorter
        if (distance > 6) {
            distance = 12 - distance;
        }

        if (distance < minDistance) {
            minDistance = distance;
            nearestScaleNote = scaleNoteAbsolute;
        }
    }

    // Reconstruct the MIDI note in the same octave
    byte mappedNote = (octave * 12) + nearestScaleNote;

    // Clamp to valid MIDI range (0-127)
    if (mappedNote > 127) mappedNote = 127;

    return mappedNote;
}

//==============================================================================
// EVENT QUEUE MANAGEMENT
//==============================================================================

// Initialize the MIDI relay system
void InitializeMidiRelay() {
    // Clear event queue
    for (int i = 0; i < MAX_MIDI_EVENTS; i++) {
        midiEventQueue[i].active = false;
    }

    // Clear active notes tracking
    for (int i = 0; i < MAX_ACTIVE_NOTES; i++) {
        activeNotes[i].active = false;
    }

    // Set quantization start time
    quantizationStartTime = millis();
    lastQuantizationCheck = quantizationStartTime;

    Serial.println("MIDI Relay: Initialized");
    Serial.print("MIDI Relay: BPM=");
    Serial.print(globalBPM);
    Serial.print(", Interval=");
    Serial.print(GetQuantizationInterval());
    Serial.println("ms");
}

// Add an event to the queue
bool QueueMidiEvent(MidiEventType type, byte channel, byte data1, byte data2) {
    // Find empty slot in queue
    for (int i = 0; i < MAX_MIDI_EVENTS; i++) {
        if (!midiEventQueue[i].active) {
            midiEventQueue[i].type = type;
            midiEventQueue[i].channel = channel;
            midiEventQueue[i].data1 = data1;
            midiEventQueue[i].data2 = data2;
            midiEventQueue[i].scheduledTime = QuantizeTime(millis());
            midiEventQueue[i].active = true;
            return true;
        }
    }

    Serial.println("MIDI Relay: Queue full, event dropped!");
    return false;
}

// Process queued events and send them when their time arrives
void ProcessMidiRelay() {
    // Skip processing if quantization grid hasn't been established yet
    if (quantizationStartTime == 0) {
        return;
    }

    unsigned long currentTime = millis();
    unsigned long timeOffset = currentTime - quantizationStartTime;

    for (int i = 0; i < MAX_MIDI_EVENTS; i++) {
        if (midiEventQueue[i].active && currentTime >= midiEventQueue[i].scheduledTime) {
            unsigned long scheduledOffset = midiEventQueue[i].scheduledTime - quantizationStartTime;

            // Send the MIDI event
            switch (midiEventQueue[i].type) {
                case MIDI_EVENT_NOTE_ON:
                    {
                        char noteName[8];
                        GetNoteName(midiEventQueue[i].data1, noteName, sizeof(noteName));

                        usbMIDI.sendNoteOn(
                            midiEventQueue[i].data1,      // note
                            midiEventQueue[i].data2,      // velocity
                            midiEventQueue[i].channel     // channel
                        );
                        Serial.print("MIDI Relay OUT: NoteOn ch=");
                        Serial.print(midiEventQueue[i].channel);
                        Serial.print(" note=");
                        Serial.print(midiEventQueue[i].data1);
                        Serial.print("(");
                        Serial.print(noteName);
                        Serial.print(") vel=");
                        Serial.print(midiEventQueue[i].data2);
                        Serial.print(" @");
                        Serial.print(timeOffset);
                        Serial.print("ms (scheduled: ");
                        Serial.print(scheduledOffset);
                        Serial.println("ms)");
                    }
                    break;

                case MIDI_EVENT_NOTE_OFF:
                    {
                        char noteName[8];
                        GetNoteName(midiEventQueue[i].data1, noteName, sizeof(noteName));

                        usbMIDI.sendNoteOff(
                            midiEventQueue[i].data1,      // note
                            midiEventQueue[i].data2,      // velocity
                            midiEventQueue[i].channel     // channel
                        );
                        Serial.print("MIDI Relay OUT: NoteOff ch=");
                        Serial.print(midiEventQueue[i].channel);
                        Serial.print(" note=");
                        Serial.print(midiEventQueue[i].data1);
                        Serial.print("(");
                        Serial.print(noteName);
                        Serial.print(") @");
                        Serial.print(timeOffset);
                        Serial.print("ms (scheduled: ");
                        Serial.print(scheduledOffset);
                        Serial.println("ms)");

                        // Clear active note tracking
                        byte noteNum = midiEventQueue[i].data1;
                        if (noteNum < MAX_ACTIVE_NOTES) {
                            activeNotes[noteNum].active = false;
                        }
                    }
                    break;

                case MIDI_EVENT_CONTROL_CHANGE:
                    usbMIDI.sendControlChange(
                        midiEventQueue[i].data1,      // control
                        midiEventQueue[i].data2,      // value
                        midiEventQueue[i].channel     // channel
                    );
                    Serial.print("MIDI Relay OUT: CC ch=");
                    Serial.print(midiEventQueue[i].channel);
                    Serial.print(" cc=");
                    Serial.print(midiEventQueue[i].data1);
                    Serial.print(" val=");
                    Serial.print(midiEventQueue[i].data2);
                    Serial.print(" @");
                    Serial.print(timeOffset);
                    Serial.print("ms (scheduled: ");
                    Serial.print(scheduledOffset);
                    Serial.println("ms)");
                    break;
            }

            // Mark event as processed
            midiEventQueue[i].active = false;
        }
    }
}

//==============================================================================
// MIDI RELAY HANDLERS
//==============================================================================

// Relay incoming Note On with scale mapping and quantization
void RelayNoteOn(byte channel, byte note, byte velocity) {
    unsigned long currentTime = millis();

    // If in play mode and this is the first note (quantizationStartTime == 0),
    // establish the quantization grid starting from this note
    if (isPlayMode && quantizationStartTime == 0) {
        quantizationStartTime = currentTime;
        Serial.println("MIDI Relay: First note received - quantization grid established");
        Serial.print("MIDI Relay: Grid start time: ");
        Serial.print(quantizationStartTime);
        Serial.println("ms");
    }

    unsigned long timeOffset = currentTime - quantizationStartTime;

    // Only apply scale mapping when play mode is active
    byte mappedNote = isPlayMode ? MapNoteToScale(note) : note;

    // Get note names for display
    char inputNoteName[8];
    char mappedNoteName[8];
    GetNoteName(note, inputNoteName, sizeof(inputNoteName));
    GetNoteName(mappedNote, mappedNoteName, sizeof(mappedNoteName));

    Serial.print("MIDI Relay IN: NoteOn ch=");
    Serial.print(channel);
    Serial.print(" note=");
    Serial.print(note);
    Serial.print("(");
    Serial.print(inputNoteName);
    Serial.print(")");

    if (isPlayMode && mappedNote != note) {
        Serial.print(" -> ");
        Serial.print(mappedNote);
        Serial.print("(");
        Serial.print(mappedNoteName);
        Serial.print(")");
    }

    Serial.print(" vel=");
    Serial.print(velocity);
    Serial.print(" @");
    Serial.print(timeOffset);
    Serial.println("ms");

    // Check if quantization should be applied (both play mode and global quantize must be enabled)
    if (!isPlayMode || !globalQuantizeEnabled) {
        // Pass through immediately without quantization or scale mapping
        usbMIDI.sendNoteOn(mappedNote, velocity, channel);
        if (!globalQuantizeEnabled) {
            Serial.print("MIDI Relay OUT: NoteOn (pass-through, quantize disabled) @");
        } else {
            Serial.print("MIDI Relay OUT: NoteOn (pass-through, play mode off) @");
        }
        Serial.print(timeOffset);
        Serial.println("ms");
        return;
    }

    // Both play mode and quantize are active - use quantization and scale mapping
    // Calculate quantized time for this note-on
    unsigned long quantizedTime = QuantizeTime(millis());
    unsigned long scheduledOffset = quantizedTime - quantizationStartTime;

    Serial.print("  -> Scheduled for @");
    Serial.print(scheduledOffset);
    Serial.print("ms (delay: ");
    Serial.print(scheduledOffset - timeOffset);
    Serial.println("ms)");

    // Track this note as active with its scheduled time
    if (mappedNote < MAX_ACTIVE_NOTES) {
        activeNotes[mappedNote].noteOnTime = quantizedTime;
        activeNotes[mappedNote].channel = channel;
        activeNotes[mappedNote].active = true;
    }

    // Queue the event with quantized timing
    QueueMidiEvent(MIDI_EVENT_NOTE_ON, channel, mappedNote, velocity);
}

// Relay incoming Note Off with scale mapping and quantization
void RelayNoteOff(byte channel, byte note, byte velocity) {
    unsigned long currentTime = millis();

    // If in play mode and this is the first MIDI event (quantizationStartTime == 0),
    // establish the quantization grid starting from this event
    if (isPlayMode && quantizationStartTime == 0) {
        quantizationStartTime = currentTime;
        Serial.println("MIDI Relay: First MIDI event (Note Off) received - quantization grid established");
    }

    unsigned long timeOffset = currentTime - quantizationStartTime;

    // Only apply scale mapping when play mode is active
    byte mappedNote = isPlayMode ? MapNoteToScale(note) : note;

    // Get note names for display
    char inputNoteName[8];
    char mappedNoteName[8];
    GetNoteName(note, inputNoteName, sizeof(inputNoteName));
    GetNoteName(mappedNote, mappedNoteName, sizeof(mappedNoteName));

    Serial.print("MIDI Relay IN: NoteOff ch=");
    Serial.print(channel);
    Serial.print(" note=");
    Serial.print(note);
    Serial.print("(");
    Serial.print(inputNoteName);
    Serial.print(")");

    if (isPlayMode && mappedNote != note) {
        Serial.print(" -> ");
        Serial.print(mappedNote);
        Serial.print("(");
        Serial.print(mappedNoteName);
        Serial.print(")");
    }

    Serial.print(" @");
    Serial.print(timeOffset);
    Serial.println("ms");

    // Check if quantization should be applied (both play mode and global quantize must be enabled)
    if (!isPlayMode || !globalQuantizeEnabled) {
        // Pass through immediately without quantization or scale mapping
        usbMIDI.sendNoteOff(mappedNote, velocity, channel);
        if (!globalQuantizeEnabled) {
            Serial.print("MIDI Relay OUT: NoteOff (pass-through, quantize disabled) @");
        } else {
            Serial.print("MIDI Relay OUT: NoteOff (pass-through, play mode off) @");
        }
        Serial.print(timeOffset);
        Serial.println("ms");
        return;
    }

    // Both play mode and quantize are active - use quantization and scale mapping
    // Calculate quantized time for this note-off
    unsigned long quantizedOffTime = QuantizeTime(millis());

    // Check if this note has an active note-on scheduled
    if (mappedNote < MAX_ACTIVE_NOTES && activeNotes[mappedNote].active) {
        unsigned long noteOnTime = activeNotes[mappedNote].noteOnTime;

        // If note-off would be quantized to the same time as note-on,
        // push it to the next quantization period to ensure minimum duration
        if (quantizedOffTime == noteOnTime) {
            float interval = GetQuantizationInterval();
            quantizedOffTime = noteOnTime + (unsigned long)interval;

            Serial.print("  -> Note-off pushed to next period (");
            Serial.print((unsigned long)interval);
            Serial.println("ms minimum duration)");
        }
    }

    unsigned long scheduledOffset = quantizedOffTime - quantizationStartTime;
    Serial.print("  -> Scheduled for @");
    Serial.print(scheduledOffset);
    Serial.print("ms (delay: ");
    Serial.print(scheduledOffset - timeOffset);
    Serial.println("ms)");

    // Find empty slot in queue and add with potentially adjusted time
    for (int i = 0; i < MAX_MIDI_EVENTS; i++) {
        if (!midiEventQueue[i].active) {
            midiEventQueue[i].type = MIDI_EVENT_NOTE_OFF;
            midiEventQueue[i].channel = channel;
            midiEventQueue[i].data1 = mappedNote;
            midiEventQueue[i].data2 = velocity;
            midiEventQueue[i].scheduledTime = quantizedOffTime;
            midiEventQueue[i].active = true;
            return;
        }
    }

    Serial.println("MIDI Relay: Queue full, note-off event dropped!");
}

// Relay incoming Control Change with quantization (no scale mapping for CC)
void RelayControlChange(byte channel, byte control, byte value) {
    unsigned long currentTime = millis();

    // If in play mode and this is the first MIDI event (quantizationStartTime == 0),
    // establish the quantization grid starting from this event
    if (isPlayMode && quantizationStartTime == 0) {
        quantizationStartTime = currentTime;
        Serial.println("MIDI Relay: First MIDI event (CC) received - quantization grid established");
    }

    unsigned long timeOffset = currentTime - quantizationStartTime;

    Serial.print("MIDI Relay IN: CC ch=");
    Serial.print(channel);
    Serial.print(" cc=");
    Serial.print(control);
    Serial.print(" val=");
    Serial.print(value);
    Serial.print(" @");
    Serial.print(timeOffset);
    Serial.println("ms");

    // Check if quantization should be applied (both play mode and global quantize must be enabled)
    if (!isPlayMode || !globalQuantizeEnabled) {
        // Pass through immediately without quantization
        usbMIDI.sendControlChange(control, value, channel);
        if (!globalQuantizeEnabled) {
            Serial.print("MIDI Relay OUT: CC (pass-through, quantize disabled) @");
        } else {
            Serial.print("MIDI Relay OUT: CC (pass-through, play mode off) @");
        }
        Serial.print(timeOffset);
        Serial.println("ms");
        return;
    }

    // Both play mode and quantize are active - use quantization
    unsigned long quantizedTime = QuantizeTime(millis());
    unsigned long scheduledOffset = quantizedTime - quantizationStartTime;

    Serial.print("  -> Scheduled for @");
    Serial.print(scheduledOffset);
    Serial.print("ms (delay: ");
    Serial.print(scheduledOffset - timeOffset);
    Serial.println("ms)");

    // Queue the event with quantized timing
    QueueMidiEvent(MIDI_EVENT_CONTROL_CHANGE, channel, control, value);
}

#endif // MIDIRELAY_H
