#ifndef USBMIDIHOST_H
#define USBMIDIHOST_H

#include "midirelay.h"

void OnNoteOn(byte channel, byte note, byte velocity)
{
	Serial.print("USB Host IN: Note On, ch=");
	Serial.print(channel);
	Serial.print(", note=");
	Serial.print(note);
	Serial.print(", velocity=");
	Serial.print(velocity);
	Serial.println();

	// Relay through MIDI processor with scale mapping and quantization
	RelayNoteOn(channel, note, velocity);
}

void OnNoteOff(byte channel, byte note, byte velocity)
{
	Serial.print("USB Host IN: Note Off, ch=");
	Serial.print(channel);
	Serial.print(", note=");
	Serial.print(note);
	//Serial.print(", velocity=");
	//Serial.print(velocity);
	Serial.println();

	// Relay through MIDI processor with scale mapping and quantization
	RelayNoteOff(channel, note, velocity);
}

void OnControlChange(byte channel, byte control, byte value)
{
	Serial.print("USB Host IN: Control Change, ch=");
	Serial.print(channel);
	Serial.print(", control=");
	Serial.print(control);
	Serial.print(", value=");
	Serial.print(value);
	Serial.println();

	// Relay through MIDI processor with quantization
	RelayControlChange(channel, control, value);
}




#endif