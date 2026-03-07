#ifndef SERIALHANDLER_H
#define SERIALHANDLER_H

#define SET_RS485_WRITE
#define SET_RS485_READ

// External references from settings.h
struct EncoderMidiMap;
struct EncoderButtonMidiMap;
struct KeyMidiMap;
extern EncoderMidiMap encoderMidiMaps[];
extern EncoderButtonMidiMap encoderButtonMidiMaps[];
extern KeyMidiMap keyMidiMaps[];

// External references from notesequence.h
struct NoteSequence;
struct NoteEvent;
struct NoteSequenceHeader;
extern NoteSequence noteSequences[];
extern void ClearNoteSequence(int sequenceIndex);

// External references from eeprom_storage.h
extern void SaveEncoderMidiMap(int encoderIndex);
extern void SaveEncoderButtonMidiMap(int encoderIndex);
extern void SaveKeyMidiMap(int keyIndex);
extern void SaveAllMidiMapsToEEPROM();

// External references from encoderled.h
extern void ShowEncoderStatus(int encoder, int status, bool onOff);
extern void SetEncoderStatusLEDs(int encoder, bool led13, bool led14, bool led15);
extern void ClearEncoderStatus(int encoder);
extern void ClearAllEncoderStatus();

// External references from GrooviXBoxMIDIFirmware.ino
extern void UpdateEncoderLEDRing(int encoderID, int position);
extern int EncoderPosition[];

// External references from display.h
extern void SetLabel(int slot, const char* text, uint16_t color, uint8_t fontSize);
extern void ClearLabel(int slot);
extern void ClearAllLabels();
extern void DisplayLabelGrid();
extern void UpdateLabelGrid();
extern uint16_t RGB888toRGB565(uint8_t r, uint8_t g, uint8_t b);

// External references for Key RGB LEDs (from GrooviXBoxMIDIFirmware.ino)
extern CRGB key_rgb_leds[];
extern const int KeyLedMap[];

int SerialDataPos=0;
byte IsGBoxProtocol;

#define MAX_SERIAL_BUF_SIZE	1024
char SerialData[MAX_SERIAL_BUF_SIZE+100];

byte DEVICE_ID_1='0';
byte DEVICE_ID_2='1';

// Helper function to parse comma-separated integers from SerialData
// Returns number of values parsed
int ParseCSVIntegers(int startPos, int values[], int maxValues) {
	int count = 0;
	int currentValue = 0;
	bool hasDigit = false;

	for (int i = startPos; i < SerialDataPos - 2 && count < maxValues; i++) {
		char ch = SerialData[i];

		if (ch >= '0' && ch <= '9') {
			currentValue = currentValue * 10 + (ch - '0');
			hasDigit = true;
		}
		else if (ch == ',' || ch == 'X') {
			if (hasDigit) {
				values[count++] = currentValue;
				currentValue = 0;
				hasDigit = false;
			}
			if (ch == 'X') break;
		}
	}

	// Handle last value if no trailing comma
	if (hasDigit && count < maxValues) {
		values[count++] = currentValue;
	}

	return count;
}

// Helper function to extract text from CSV format
// Format: [01DL,slot,r,g,b,text hereXX]
// Returns pointer to the text portion after the last comma before XX
char* ExtractTextFromCommand(int afterCommaCount) {
	int commasSeen = 0;
	int textStart = 0;

	// Find position after the Nth comma
	for (int i = 0; i < SerialDataPos - 2; i++) {
		if (SerialData[i] == ',') {
			commasSeen++;
			if (commasSeen == afterCommaCount) {
				textStart = i + 1;
				break;
			}
		}
	}

	if (textStart == 0) return NULL;

	// Find end of text (before XX)
	int textEnd = SerialDataPos - 2;

	// Copy text to a static buffer
	static char textBuffer[64];
	int len = textEnd - textStart;
	if (len >= 64) len = 63;
	if (len < 0) len = 0;

	strncpy(textBuffer, &SerialData[textStart], len);
	textBuffer[len] = '\0';

	return textBuffer;
}

// Set encoder MIDI mapping from parsed values
// Format: [01EW,idx,P,CH,T,CC,M,MIN,MAX,NCW,NCCWXX]
// Example: [01EW,0,1,1,0,12,0,0,127,60,61XX]
void SetEncoderConfig() {
	int values[10];
	int count = ParseCSVIntegers(5, values, 10); // Start after "01EW,"

	if (count < 10) {
		SET_RS485_WRITE
		Serial.println(F("ERROR: Invalid encoder config format"));
		SET_RS485_READ
		return;
	}

	int idx = values[0];
	if (idx < 0 || idx >= NUM_ENCODERS) {
		SET_RS485_WRITE
		Serial.println(F("ERROR: Invalid encoder index"));
		SET_RS485_READ
		return;
	}

	EncoderMidiMap &map = encoderMidiMaps[idx];
	map.port = values[1];
	map.channel = values[2];
	map.msgType = (MidiMessageType)values[3];
	map.ccNumber = values[4];
	map.mode = (EncoderMode)values[5];
	map.minValue = values[6];
	map.maxValue = values[7];
	map.noteCW = values[8];
	map.noteCCW = values[9];

	// Update LED ring to reflect new mode
	UpdateEncoderLEDRing(idx, EncoderPosition[idx]);

	SET_RS485_WRITE
	Serial.print(F("OK: Encoder "));
	Serial.print(idx);
	Serial.println(F(" config updated"));
	SET_RS485_READ
}

// Set encoder button MIDI mapping from parsed values
// Format: [01BW,idx,P,CH,T,NOTE,CC,VAL,PC,MODEXX]
void SetEncoderButtonConfig() {
	int values[9];
	int count = ParseCSVIntegers(5, values, 9); // Start after "01BW,"

	if (count < 9) {
		SET_RS485_WRITE
		Serial.println(F("ERROR: Invalid encoder button config format"));
		SET_RS485_READ
		return;
	}

	int idx = values[0];
	if (idx < 0 || idx >= NUM_ENCODERS) {
		SET_RS485_WRITE
		Serial.println(F("ERROR: Invalid encoder index"));
		SET_RS485_READ
		return;
	}

	EncoderButtonMidiMap &map = encoderButtonMidiMaps[idx];
	map.port = values[1];
	map.channel = values[2];
	map.msgType = (MidiMessageType)values[3];
	map.noteNumber = values[4];
	map.ccNumber = values[5];
	map.ccValue = values[6];
	map.programNumber = values[7];
	map.buttonMode = (ButtonMode)values[8];

	SET_RS485_WRITE
	Serial.print(F("OK: Encoder button "));
	Serial.print(idx);
	Serial.println(F(" config updated"));
	SET_RS485_READ
}

// Set key press MIDI mapping from parsed values
// Format: [01KP,idx,P,CH,T,NOTE,CC,VAL,PCXX]
void SetKeyPressConfig() {
	int values[8];
	int count = ParseCSVIntegers(5, values, 8); // Start after "01KP,"

	if (count < 8) {
		SET_RS485_WRITE
		Serial.println(F("ERROR: Invalid key press config format"));
		SET_RS485_READ
		return;
	}

	int idx = values[0];
	if (idx < 0 || idx >= NUM_KEYS) {
		SET_RS485_WRITE
		Serial.println(F("ERROR: Invalid key index"));
		SET_RS485_READ
		return;
	}

	KeyMidiMap &map = keyMidiMaps[idx];
	map.pressPort = values[1];
	map.pressChannel = values[2];
	map.pressMsgType = (MidiMessageType)values[3];
	map.pressNoteNumber = values[4];
	map.pressCcNumber = values[5];
	map.pressCcValue = values[6];
	map.pressProgramNumber = values[7];

	SET_RS485_WRITE
	Serial.print(F("OK: Key "));
	Serial.print(idx);
	Serial.println(F(" press config updated"));
	SET_RS485_READ
}

// Set key release MIDI mapping from parsed values
// Format: [01KR,idx,P,CH,T,NOTE,CC,VAL,PC,MODEXX]
void SetKeyReleaseConfig() {
	int values[9];
	int count = ParseCSVIntegers(5, values, 9); // Start after "01KR,"

	if (count < 9) {
		SET_RS485_WRITE
		Serial.println(F("ERROR: Invalid key release config format"));
		SET_RS485_READ
		return;
	}

	int idx = values[0];
	if (idx < 0 || idx >= NUM_KEYS) {
		SET_RS485_WRITE
		Serial.println(F("ERROR: Invalid key index"));
		SET_RS485_READ
		return;
	}

	KeyMidiMap &map = keyMidiMaps[idx];
	map.releasePort = values[1];
	map.releaseChannel = values[2];
	map.releaseMsgType = (MidiMessageType)values[3];
	map.releaseNoteNumber = values[4];
	map.releaseCcNumber = values[5];
	map.releaseCcValue = values[6];
	map.releaseProgramNumber = values[7];
	map.buttonMode = (ButtonMode)values[8];

	SET_RS485_WRITE
	Serial.print(F("OK: Key "));
	Serial.print(idx);
	Serial.println(F(" release config updated"));
	SET_RS485_READ
}

//==============================================================================
// NOTE SEQUENCE HANDLERS
//==============================================================================

// Set note sequence header
// Format: [01NH,keyIdx,ppqn,tempo,eventCount,channelXX]
void HandleSequenceHeader() {
	int values[5];
	int count = ParseCSVIntegers(5, values, 5); // Start after "01NH,"

	if (count < 5) {
		SET_RS485_WRITE
		Serial.println(F("ERROR: Invalid sequence header format"));
		SET_RS485_READ
		return;
	}

	int keyIdx = values[0];
	if (keyIdx < 0 || keyIdx >= 32) {
		SET_RS485_WRITE
		Serial.println(F("ERROR: Invalid key index (0-31)"));
		SET_RS485_READ
		return;
	}

	// Get sequence index from key mapping (or use keyIdx as sequence index)
	int seqIdx = keyMidiMaps[keyIdx].sequenceIndex;

	// If no sequence assigned yet, assign one based on keyIdx
	if (seqIdx == 255) {
		// Use key index as sequence index (limited to 16 sequences)
		if (keyIdx < 16) {
			seqIdx = keyIdx;
			keyMidiMaps[keyIdx].sequenceIndex = seqIdx;
		} else {
			SET_RS485_WRITE
			Serial.println(F("ERROR: No sequence slot available (max 16)"));
			SET_RS485_READ
			return;
		}
	}

	NoteSequence &seq = noteSequences[seqIdx];
	seq.header.ppqn = values[1];
	seq.header.tempo = values[2];
	seq.header.eventCount = values[3];
	seq.header.channel = values[4];
	seq.isActive = true;

	SET_RS485_WRITE
	Serial.print(F("OK: Sequence "));
	Serial.print(seqIdx);
	Serial.println(F(" header set"));
	SET_RS485_READ
}

// Set note sequence event
// Format: [01NE,keyIdx,eventIdx,deltaTime,note,velocity,durationXX]
void HandleSequenceEvent() {
	int values[6];
	int count = ParseCSVIntegers(5, values, 6); // Start after "01NE,"

	if (count < 6) {
		SET_RS485_WRITE
		Serial.println(F("ERROR: Invalid sequence event format"));
		SET_RS485_READ
		return;
	}

	int keyIdx = values[0];
	if (keyIdx < 0 || keyIdx >= 32) {
		SET_RS485_WRITE
		Serial.println(F("ERROR: Invalid key index (0-31)"));
		SET_RS485_READ
		return;
	}

	int seqIdx = keyMidiMaps[keyIdx].sequenceIndex;
	if (seqIdx == 255 || seqIdx >= 16) {
		SET_RS485_WRITE
		Serial.println(F("ERROR: No sequence assigned to this key"));
		SET_RS485_READ
		return;
	}

	int eventIdx = values[1];
	if (eventIdx < 0 || eventIdx >= 256) {
		SET_RS485_WRITE
		Serial.println(F("ERROR: Invalid event index (0-255)"));
		SET_RS485_READ
		return;
	}

	NoteSequence &seq = noteSequences[seqIdx];
	NoteEvent &evt = seq.events[eventIdx];

	evt.deltaTime = values[2];
	evt.noteNumber = values[3];
	evt.velocity = values[4];
	evt.duration = values[5];

	SET_RS485_WRITE
	Serial.print(F("OK: Sequence "));
	Serial.print(seqIdx);
	Serial.print(F(" event "));
	Serial.print(eventIdx);
	Serial.println(F(" set"));
	SET_RS485_READ
}

// Clear note sequence
// Format: [01NC,keyIdxXX]
void HandleSequenceClear() {
	int values[1];
	int count = ParseCSVIntegers(5, values, 1); // Start after "01NC,"

	if (count < 1) {
		SET_RS485_WRITE
		Serial.println(F("ERROR: Invalid sequence clear format"));
		SET_RS485_READ
		return;
	}

	int keyIdx = values[0];
	if (keyIdx < 0 || keyIdx >= 32) {
		SET_RS485_WRITE
		Serial.println(F("ERROR: Invalid key index (0-31)"));
		SET_RS485_READ
		return;
	}

	int seqIdx = keyMidiMaps[keyIdx].sequenceIndex;
	if (seqIdx != 255 && seqIdx < 16) {
		ClearNoteSequence(seqIdx);
		keyMidiMaps[keyIdx].sequenceIndex = 255; // Unassign
	}

	SET_RS485_WRITE
	Serial.print(F("OK: Sequence for key "));
	Serial.print(keyIdx);
	Serial.println(F(" cleared"));
	SET_RS485_READ
}

// Dump note sequence
// Format: [01ND,keyIdxXX] or [01NDXX] for all
void HandleSequenceDump() {
	SET_RS485_WRITE

	// Check if specific key or all sequences
	if (SerialDataPos > 6) {
		// Specific key
		int values[1];
		int count = ParseCSVIntegers(5, values, 1);

		if (count < 1) {
			Serial.println(F("ERROR: Invalid sequence dump format"));
			SET_RS485_READ
			return;
		}

		int keyIdx = values[0];
		if (keyIdx < 0 || keyIdx >= 32) {
			Serial.println(F("ERROR: Invalid key index (0-31)"));
			SET_RS485_READ
			return;
		}

		int seqIdx = keyMidiMaps[keyIdx].sequenceIndex;
		if (seqIdx == 255 || seqIdx >= 16 || !noteSequences[seqIdx].isActive) {
			Serial.print(F("[SEQ:"));
			Serial.print(keyIdx);
			Serial.println(F("]NONE"));
			SET_RS485_READ
			return;
		}

		NoteSequence &seq = noteSequences[seqIdx];

		// Dump header
		Serial.print(F("[SEQ:"));
		Serial.print(keyIdx);
		Serial.print(F("]PPQN="));
		Serial.print(seq.header.ppqn);
		Serial.print(F(",TEMPO="));
		Serial.print(seq.header.tempo);
		Serial.print(F(",CH="));
		Serial.print(seq.header.channel);
		Serial.print(F(",CNT="));
		Serial.println(seq.header.eventCount);

		// Dump events
		for (int i = 0; i < seq.header.eventCount; i++) {
			Serial.print(F("[SEV:"));
			Serial.print(keyIdx);
			Serial.print(F(","));
			Serial.print(i);
			Serial.print(F("]DT="));
			Serial.print(seq.events[i].deltaTime);
			Serial.print(F(",N="));
			Serial.print(seq.events[i].noteNumber);
			Serial.print(F(",V="));
			Serial.print(seq.events[i].velocity);
			Serial.print(F(",DUR="));
			Serial.println(seq.events[i].duration);
		}
	} else {
		// Dump all sequences
		Serial.println(F("=== NOTE SEQUENCES ==="));
		for (int keyIdx = 0; keyIdx < 32; keyIdx++) {
			int seqIdx = keyMidiMaps[keyIdx].sequenceIndex;
			if (seqIdx != 255 && seqIdx < 16 && noteSequences[seqIdx].isActive) {
				NoteSequence &seq = noteSequences[seqIdx];
				Serial.print(F("Key "));
				Serial.print(keyIdx);
				Serial.print(F(" -> Seq "));
				Serial.print(seqIdx);
				Serial.print(F(": "));
				Serial.print(seq.header.eventCount);
				Serial.print(F(" events, PPQN="));
				Serial.print(seq.header.ppqn);
				Serial.print(F(", Tempo="));
				Serial.println(seq.header.tempo);
			}
		}
		Serial.println(F("=== END SEQUENCES ==="));
	}

	SET_RS485_READ
}

char hex2asc(byte ch)
{
	if ((ch>=0)&&(ch<=9)) return (ch+'0');
	if ((ch>=10)&&(ch<=15)) return (ch+'A'-10);
	return '0';
}


byte asc2hex(char ch)
{
	if ((ch>='0')&&(ch<='9')) return (ch-'0');
	if ((ch>='A')&&(ch<='F')) return (ch-'A'+10);
	if ((ch>='a')&&(ch<='f')) return (ch-'a'+10);
	return 0;
}


bool CheckCRC()
{
	byte i,CalcCRC,SentCRC;
	CalcCRC=0;
	
	i=SerialDataPos-2;
	if ((SerialData[i]=='X')&&(SerialData[i+1]=='X')) return 1;
	
	SentCRC=(asc2hex(SerialData[i])<<4)+asc2hex(SerialData[i+1]);

	for(i=0;i<SerialDataPos-2;i++) 
	{
		CalcCRC=CalcCRC ^ SerialData[i];
		
		/*
		Serial.print('.');print_hex(CalcCRC);
		Serial.print('>');print_hex(SerialData[i]);
		Serial.print('\n');Serial.print('\r');
		*/
	}
	
	
	if (SentCRC!=CalcCRC) 
	{
		SET_RS485_WRITE
		Serial.println(F("EE"));
		//Serial.print('E');Serial.print('E');Serial.print('\n');Serial.print('\r');
		Serial.print("1>");Serial.println(SentCRC,HEX);
		Serial.print("2>");Serial.println(CalcCRC,HEX);
		SET_RS485_READ
		return 0;
	}
	
	return 1;
}


// Dump encoder MIDI configuration to serial
void DumpEncoderConfig()
{
	SET_RS485_WRITE
	Serial.println(F("=== ENCODER MIDI CONFIG ==="));

	for (int i = 0; i < NUM_ENCODERS; i++) {
		EncoderMidiMap &map = encoderMidiMaps[i];

		// Format: [EC:idx]P=port,CH=channel,T=msgType,CC=ccNum,M=mode,MIN=min,MAX=max,NCW=noteCW,NCCW=noteCCW
		Serial.print(F("[EC:"));
		Serial.print(i);
		Serial.print(F("]P="));
		Serial.print(map.port);
		Serial.print(F(",CH="));
		Serial.print(map.channel);
		Serial.print(F(",T="));
		Serial.print((int)map.msgType);
		Serial.print(F(",CC="));
		Serial.print(map.ccNumber);
		Serial.print(F(",M="));
		Serial.print((int)map.mode);
		Serial.print(F(",MIN="));
		Serial.print(map.minValue);
		Serial.print(F(",MAX="));
		Serial.print(map.maxValue);
		Serial.print(F(",NCW="));
		Serial.print(map.noteCW);
		Serial.print(F(",NCCW="));
		Serial.println(map.noteCCW);
	}

	Serial.println(F("=== END ENCODER CONFIG ==="));
	SET_RS485_READ
}

// Dump encoder button MIDI configuration to serial
void DumpEncoderButtonConfig()
{
	SET_RS485_WRITE
	Serial.println(F("=== ENCODER BUTTON MIDI CONFIG ==="));

	for (int i = 0; i < NUM_ENCODERS; i++) {
		EncoderButtonMidiMap &map = encoderButtonMidiMaps[i];

		// Format: [EB:idx]P=port,CH=channel,T=msgType,NOTE=noteNum,CC=ccNum,VAL=ccValue,PC=programNum,MODE=buttonMode
		Serial.print(F("[EB:"));
		Serial.print(i);
		Serial.print(F("]P="));
		Serial.print(map.port);
		Serial.print(F(",CH="));
		Serial.print(map.channel);
		Serial.print(F(",T="));
		Serial.print((int)map.msgType);
		Serial.print(F(",NOTE="));
		Serial.print(map.noteNumber);
		Serial.print(F(",CC="));
		Serial.print(map.ccNumber);
		Serial.print(F(",VAL="));
		Serial.print(map.ccValue);
		Serial.print(F(",PC="));
		Serial.print(map.programNumber);
		Serial.print(F(",MODE="));
		Serial.println((int)map.buttonMode);
	}

	Serial.println(F("=== END ENCODER BUTTON CONFIG ==="));
	SET_RS485_READ
}

// Dump key MIDI configuration to serial
void DumpKeyConfig()
{
	SET_RS485_WRITE
	Serial.println(F("=== KEY MIDI CONFIG ==="));

	for (int i = 0; i < NUM_KEYS; i++) {
		KeyMidiMap &map = keyMidiMaps[i];

		// Press configuration
		Serial.print(F("[KP:"));
		Serial.print(i);
		Serial.print(F("]P="));
		Serial.print(map.pressPort);
		Serial.print(F(",CH="));
		Serial.print(map.pressChannel);
		Serial.print(F(",T="));
		Serial.print((int)map.pressMsgType);
		Serial.print(F(",NOTE="));
		Serial.print(map.pressNoteNumber);
		Serial.print(F(",CC="));
		Serial.print(map.pressCcNumber);
		Serial.print(F(",VAL="));
		Serial.print(map.pressCcValue);
		Serial.print(F(",PC="));
		Serial.println(map.pressProgramNumber);

		// Release configuration
		Serial.print(F("[KR:"));
		Serial.print(i);
		Serial.print(F("]P="));
		Serial.print(map.releasePort);
		Serial.print(F(",CH="));
		Serial.print(map.releaseChannel);
		Serial.print(F(",T="));
		Serial.print((int)map.releaseMsgType);
		Serial.print(F(",NOTE="));
		Serial.print(map.releaseNoteNumber);
		Serial.print(F(",CC="));
		Serial.print(map.releaseCcNumber);
		Serial.print(F(",VAL="));
		Serial.print(map.releaseCcValue);
		Serial.print(F(",PC="));
		Serial.print(map.releaseProgramNumber);
		Serial.print(F(",MODE="));
		Serial.println((int)map.buttonMode);
	}

	Serial.println(F("=== END KEY CONFIG ==="));
	SET_RS485_READ
}

// Dump display label configuration to serial
void DumpLabelConfig()
{
	SET_RS485_WRITE
	Serial.println(F("=== DISPLAY LABEL CONFIG ==="));

	extern LabelSlot labelSlots[];

	for (int i = 0; i < 16; i++) {
		LabelSlot &slot = labelSlots[i];

		// Extract RGB from RGB565 color (approximate reverse conversion)
		uint16_t color565 = slot.color;
		uint8_t r = ((color565 >> 11) & 0x1F) << 3;  // 5 bits -> 8 bits
		uint8_t g = ((color565 >> 5) & 0x3F) << 2;   // 6 bits -> 8 bits
		uint8_t b = (color565 & 0x1F) << 3;          // 5 bits -> 8 bits

		// Format: [LBL:idx]R=r,G=g,B=b,SIZE=fontSize,TEXT=text
		Serial.print(F("[LBL:"));
		Serial.print(i);
		Serial.print(F("]R="));
		Serial.print(r);
		Serial.print(F(",G="));
		Serial.print(g);
		Serial.print(F(",B="));
		Serial.print(b);
		Serial.print(F(",SIZE="));
		Serial.print(slot.fontSize);
		Serial.print(F(",TEXT="));
		Serial.println(slot.text);
	}

	Serial.println(F("=== END LABEL CONFIG ==="));
	SET_RS485_READ
}

// Dump all key RGB LED colors
// Format: [KLED:idx]R=r,G=g,B=b
void DumpKeyLEDConfig()
{
	SET_RS485_WRITE
	Serial.println(F("=== KEY RGB LED CONFIG ==="));

	for (int keyID = 0; keyID < 32; keyID++) {
		// Calculate LED index using same logic as ToggleKeyLED
		int ledIndex = KeyLedMap[keyID % 16];
		if (keyID >= 16) {
			ledIndex += 16;
		}
		ledIndex = ledIndex % NUM_KEY_LEDS;

		CRGB &led = key_rgb_leds[ledIndex];

		// Format: [KLED:idx]R=r,G=g,B=b
		Serial.print(F("[KLED:"));
		Serial.print(keyID);
		Serial.print(F("]R="));
		Serial.print(led.r);
		Serial.print(F(",G="));
		Serial.print(led.g);
		Serial.print(F(",B="));
		Serial.println(led.b);
	}

	Serial.println(F("=== END KEY LED CONFIG ==="));
	SET_RS485_READ
}

// Set RGB color for a specific key (sets both LEDs for that key)
// keyID: 0-31
// r, g, b: 0-255
void SetKeyLEDColor(int keyID, uint8_t r, uint8_t g, uint8_t b)
{
	if (keyID < 0 || keyID >= 32) return;

	// Calculate LED index using same logic as ToggleKeyLED
	int ledIndex = KeyLedMap[keyID % 16];
	if (keyID >= 16) {
		ledIndex += 16;
	}
	ledIndex = ledIndex % NUM_KEY_LEDS;

	// Set the LED color
	key_rgb_leds[ledIndex].r = r;
	key_rgb_leds[ledIndex].g = g;
	key_rgb_leds[ledIndex].b = b;

	#ifndef _GSIMULATOR
	FastLED.show();
	#endif
}

// Set all key LEDs to specific color
void SetAllKeyLEDs(uint8_t r, uint8_t g, uint8_t b)
{
	for (int i = 0; i < NUM_KEY_LEDS; i++) {
		key_rgb_leds[i].r = r;
		key_rgb_leds[i].g = g;
		key_rgb_leds[i].b = b;
	}

	#ifndef _GSIMULATOR
	FastLED.show();
	#endif
}

// Clear all key LEDs (set to black/off)
void ClearAllKeyLEDs()
{
	for (int i = 0; i < NUM_KEY_LEDS; i++) {
		key_rgb_leds[i] = CRGB::Black;
	}

	#ifndef _GSIMULATOR
	FastLED.show();
	#endif
}

void ParsePacket()
{

		if (!CheckCRC())
		{
			SerialDataPos=0;
			return;
		}
		else
		{
			if ((SerialData[0]=='X')&&(SerialData[1]=='X'))
			{
				// Broad cast to all device
			}
			else
			{
				if ((SerialData[0]!=DEVICE_ID_1)||
					(SerialData[1]!=DEVICE_ID_2))
					{
						SerialDataPos=0;
						return;
					}
			}

			if (SerialData[2]=='R') // Remote Commands
			{
				//  012345678
				// [01R1XX]
				//HandleRemoteCommands(SerialData[3]);
                Serial.println("HandleRemoteCommands Received.");

				return;
			}

			if (SerialData[2]=='E') // Encoder/Configuration Commands
			{
				//  0123456
				// [01ECXX] - Dump Encoder Config
				// [01EBXX] - Dump Encoder Button Config
				// [01EW,idx,P,CH,T,CC,M,MIN,MAX,NCW,NCCWXX] - Write Encoder Config

				if (SerialData[3]=='C') {
					// Encoder rotation config dump
					DumpEncoderConfig();
				}
				else if (SerialData[3]=='B') {
					// Encoder button config dump
					DumpEncoderButtonConfig();
				}
				else if (SerialData[3]=='W') {
					// Write encoder rotation config
					SetEncoderConfig();
				}

				SerialDataPos=0;
				return;
			}

			if (SerialData[2]=='B') // Button Configuration Commands
			{
				//  0123456
				// [01BW,idx,P,CH,T,NOTE,CC,VAL,PC,MODEXX] - Write Encoder Button Config

				if (SerialData[3]=='W') {
					// Write encoder button config
					SetEncoderButtonConfig();
				}

				SerialDataPos=0;
				return;
			}

			if (SerialData[2]=='K') // Key Configuration Commands
			{
				//  0123456
				// [01KCXX] - Dump Key Config
				// [01KP,idx,P,CH,T,NOTE,CC,VAL,PCXX] - Write Key Press Config
				// [01KR,idx,P,CH,T,NOTE,CC,VAL,PC,MODEXX] - Write Key Release Config (MODE added here)

				if (SerialData[3]=='C') {
					// Dump key config
					DumpKeyConfig();
				}
				else if (SerialData[3]=='P') {
					// Write key press config
					SetKeyPressConfig();
				}
				else if (SerialData[3]=='R') {
					// Write key release config
					SetKeyReleaseConfig();
				}

				SerialDataPos=0;
				return;
			}

			if (SerialData[2]=='N') // Note Sequence Commands
			{
				//  0123456
				// [01NH,keyIdx,ppqn,tempo,eventCount,channelXX] - Set Sequence Header
				// [01NE,keyIdx,eventIdx,deltaTime,note,velocity,durationXX] - Set Sequence Event
				// [01NC,keyIdxXX] - Clear Sequence
				// [01ND,keyIdxXX] - Dump Sequence

				if (SerialData[3]=='H') {
					// Set sequence header
					HandleSequenceHeader();
				}
				else if (SerialData[3]=='E') {
					// Set sequence event
					HandleSequenceEvent();
				}
				else if (SerialData[3]=='C') {
					// Clear sequence
					HandleSequenceClear();
				}
				else if (SerialData[3]=='D') {
					// Dump sequence
					HandleSequenceDump();
				}

				SerialDataPos=0;
				return;
			}

			if (SerialData[2]=='S') // Save to EEPROM Commands
			{
				//  0123456
				// [01SAXX] - Save All to EEPROM
				// [01SE,idxXX] - Save Encoder idx to EEPROM
				// [01SB,idxXX] - Save Encoder Button idx to EEPROM
				// [01SK,idxXX] - Save Key idx to EEPROM

				if (SerialData[3]=='A') {
					// Save all to EEPROM
					SET_RS485_WRITE
					Serial.println(F("Saving all to EEPROM..."));
					SET_RS485_READ
					SaveAllMidiMapsToEEPROM();
					SET_RS485_WRITE
					Serial.println(F("OK: All saved"));
					SET_RS485_READ
				}
				else if (SerialData[3]=='E') {
					// Save individual encoder
					int values[1];
					int count = ParseCSVIntegers(5, values, 1);
					if (count > 0 && values[0] >= 0 && values[0] < NUM_ENCODERS) {
						SaveEncoderMidiMap(values[0]);
						SET_RS485_WRITE
						Serial.println(F("OK: Saved"));
						SET_RS485_READ
					}
				}
				else if (SerialData[3]=='B') {
					// Save individual encoder button
					int values[1];
					int count = ParseCSVIntegers(5, values, 1);
					if (count > 0 && values[0] >= 0 && values[0] < NUM_ENCODERS) {
						SaveEncoderButtonMidiMap(values[0]);
						SET_RS485_WRITE
						Serial.println(F("OK: Saved"));
						SET_RS485_READ
					}
				}
				else if (SerialData[3]=='K') {
					// Save individual key
					int values[1];
					int count = ParseCSVIntegers(5, values, 1);
					if (count > 0 && values[0] >= 0 && values[0] < NUM_KEYS) {
						SaveKeyMidiMap(values[0]);
						SET_RS485_WRITE
						Serial.println(F("OK: Saved"));
						SET_RS485_READ
					}
				}

				SerialDataPos=0;
				return;
			}

			if (SerialData[2]=='L') // LED Control Commands
			{
				//  0123456
				// Encoder Status LEDs:
				// [01LS,encoder,led13,led14,led15XX] - Set Status LEDs
				// [01LC,encoderXX] - Clear Status LEDs for encoder
				// [01LAXX] - Clear All Status LEDs
				//
				// Key RGB LEDs:
				// [01LKDXX] - Dump Key LED Config
				// [01LK,keyID,r,g,bXX] - Set Key LED Color
				// [01LKA,r,g,bXX] - Set All Key LEDs
				// [01LKCXX] - Clear All Key LEDs

				if (SerialData[3]=='S') {
					// Set status LEDs
					int values[4];
					int count = ParseCSVIntegers(5, values, 4);
					if (count >= 4) {
						int encoder = values[0];
						if (encoder >= 0 && encoder < 8) {
							SetEncoderStatusLEDs(encoder, values[1] != 0, values[2] != 0, values[3] != 0);
							SET_RS485_WRITE
							Serial.print(F("OK: Encoder "));
							Serial.print(encoder);
							Serial.println(F(" status LEDs set"));
							SET_RS485_READ
						}
					}
				}
				else if (SerialData[3]=='C') {
					// Clear status LEDs for specific encoder
					int values[1];
					int count = ParseCSVIntegers(5, values, 1);
					if (count > 0 && values[0] >= 0 && values[0] < 8) {
						ClearEncoderStatus(values[0]);
						SET_RS485_WRITE
						Serial.print(F("OK: Encoder "));
						Serial.print(values[0]);
						Serial.println(F(" status LEDs cleared"));
						SET_RS485_READ
					}
				}
				else if (SerialData[3]=='A') {
					// Clear all status LEDs
					ClearAllEncoderStatus();
					SET_RS485_WRITE
					Serial.println(F("OK: All status LEDs cleared"));
					SET_RS485_READ
				}
				else if (SerialData[3]=='K') {
					// Key RGB LED commands
					if (SerialData[4]=='D') {
						// Dump key LED config: [01LKDXX]
						DumpKeyLEDConfig();
					}
					else if (SerialData[4]=='A') {
						// Set all key LEDs: [01LKA,r,g,bXX]
						int values[3];
						int count = ParseCSVIntegers(6, values, 3);
						if (count >= 3) {
							SetAllKeyLEDs((uint8_t)values[0], (uint8_t)values[1], (uint8_t)values[2]);
							SET_RS485_WRITE
							Serial.print(F("OK: All key LEDs set to RGB("));
							Serial.print(values[0]);
							Serial.print(F(","));
							Serial.print(values[1]);
							Serial.print(F(","));
							Serial.print(values[2]);
							Serial.println(F(")"));
							SET_RS485_READ
						}
					}
					else if (SerialData[4]=='C') {
						// Clear all key LEDs: [01LKCXX]
						ClearAllKeyLEDs();
						SET_RS485_WRITE
						Serial.println(F("OK: All key LEDs cleared"));
						SET_RS485_READ
					}
					else {
						// Set individual key LED: [01LK,keyID,r,g,bXX]
						int values[4];
						int count = ParseCSVIntegers(5, values, 4);
						if (count >= 4) {
							int keyID = values[0];
							if (keyID >= 0 && keyID < 32) {
								SetKeyLEDColor(keyID, (uint8_t)values[1], (uint8_t)values[2], (uint8_t)values[3]);
								SET_RS485_WRITE
								Serial.print(F("OK: Key "));
								Serial.print(keyID);
								Serial.print(F(" LED set to RGB("));
								Serial.print(values[1]);
								Serial.print(F(","));
								Serial.print(values[2]);
								Serial.print(F(","));
								Serial.print(values[3]);
								Serial.println(F(")"));
								SET_RS485_READ
							}
						}
					}
				}

				SerialDataPos=0;
				return;
			}

			if (SerialData[2]=='D') // Display Label Commands
			{
				//  0123456
				// [01DDXX] - Dump Label Config
				// [01DL,slot,r,g,b,fontSize,textXX] - Set Label
				// [01DC,slotXX] - Clear Label
				// [01DAXX] - Clear All Labels
				// [01DGXX] - Display Label Grid
				// [01DUXX] - Update Label Grid (partial refresh)

				if (SerialData[3]=='D') {
					// Dump label config
					DumpLabelConfig();
					SerialDataPos=0;
					return;
				}
				else if (SerialData[3]=='L') {
					// Set label: [01DL,slot,r,g,b,fontSize,textXX]
					int values[5];
					int count = ParseCSVIntegers(5, values, 5);
					if (count >= 5) {
						int slot = values[0];
						if (slot >= 0 && slot < 16) {
							uint8_t r = (uint8_t)values[1];
							uint8_t g = (uint8_t)values[2];
							uint8_t b = (uint8_t)values[3];
							uint8_t fontSize = (uint8_t)values[4];
							uint16_t color = RGB888toRGB565(r, g, b);

							char* text = ExtractTextFromCommand(6);  // 6 commas before text: after DL, slot, r, g, b, fontSize
							if (text) {
								SetLabel(slot, text, color, fontSize);
								UpdateLabelGrid();  // Automatically update display
								SET_RS485_WRITE
								Serial.print(F("OK: Label "));
								Serial.print(slot);
								Serial.print(F(" set to: "));
								Serial.print(text);
								Serial.print(F(" RGB("));
								Serial.print(r);
								Serial.print(F(","));
								Serial.print(g);
								Serial.print(F(","));
								Serial.print(b);
								Serial.print(F(") Size="));
								Serial.print(fontSize);
								Serial.print(F(" = 0x"));
								Serial.println(color, HEX);
								SET_RS485_READ
							}
						}
					}
				}
				else if (SerialData[3]=='C') {
					// Clear specific label
					int values[1];
					int count = ParseCSVIntegers(5, values, 1);
					if (count > 0 && values[0] >= 0 && values[0] < 16) {
						ClearLabel(values[0]);
						UpdateLabelGrid();  // Automatically update display
						SET_RS485_WRITE
						Serial.print(F("OK: Label "));
						Serial.print(values[0]);
						Serial.println(F(" cleared"));
						SET_RS485_READ
					}
				}
				else if (SerialData[3]=='A') {
					// Clear all labels
					ClearAllLabels();
					UpdateLabelGrid();  // Automatically update display
					SET_RS485_WRITE
					Serial.println(F("OK: All labels cleared"));
					SET_RS485_READ
				}
				else if (SerialData[3]=='G') {
					// Display label grid (full redraw)
					DisplayLabelGrid();
					SET_RS485_WRITE
					Serial.println(F("OK: Label grid displayed"));
					SET_RS485_READ
				}
				else if (SerialData[3]=='U') {
					// Update label grid (partial refresh)
					UpdateLabelGrid();
					SET_RS485_WRITE
					Serial.println(F("OK: Label grid updated"));
					SET_RS485_READ
				}

				SerialDataPos=0;
				return;
			}
        }
}

byte ReadSerialData()
{
    char ch=Serial.read();
    
    if (ch=='[') 
	{
		SerialDataPos=0;
		return 0;
	}

    
	if (SerialDataPos>=MAX_SERIAL_BUF_SIZE) 
    {
        SerialDataPos=0;
    }

    SerialData[SerialDataPos]=ch;
	
	if ((ch==']')&&(SerialDataPos>1))
	{
		ParsePacket();
		return 1;
	}

	SerialDataPos++;
	SerialData[SerialDataPos]=0;

    return 1;
}


#endif