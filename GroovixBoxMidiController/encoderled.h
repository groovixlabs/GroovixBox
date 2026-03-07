#ifndef ENCODERLED_H
#define ENCODERLED_H



// MAX7219 registers
#define LEDRING_REG_NOOP         0x00
#define LEDRING_REG_DIGIT0       0x01 // row 0
#define LEDRING_REG_DIGIT7       0x08 // row 7
#define LEDRING_REG_DECODEMODE   0x09
#define LEDRING_REG_INTENSITY    0x0A
#define LEDRING_REG_SCANLIMIT    0x0B
#define LEDRING_REG_SHUTDOWN     0x0C
#define LEDRING_REG_DISPLAYTEST  0x0F

// Our framebuffer: 16 columns x 8 rows (two 8x8 modules side-by-side)
// We store per-row bytes for each module.
// ledring[row][0] = left module row bits (columns 0..7)
// ledring[row][1] = right module row bits (columns 8..15)

// Custom mapping: defines the logical shift order of each LED
typedef struct sledringMap {
  int device;  // 0 or 1
  int row;     // DIG pin 0–7
  int col;     // SEG pin 0–7
} ledringMap;

// TODO: Fill this array in the order you want LEDs to shift
// D=0 R=0,4,6,2,3,7,5,1 C=1    E0
// D=0 R=1,5,7,3,2,6,4,0 C=6

// D=0 R=0,4,6,2,3,7,5,1 C=5    E1
// D=0 R=1,5,7,3,2,6,4,0 C=0

// D=1 R=0,4,6,2,3,7,5,1 C=1    E2
// D=1 R=1,5,7,3,2,6,4,0 C=6

// D=1 R=0,4,6,2,3,7,5,1 C=5    E3
// D=1 R=1,5,7,3,2,6,4,0 C=0

// D=0 R=0,4,6,2,3,7,5,1 C=2    E4
// D=0 R=1,5,7,3,2,6,4,0 C=4

// D=0 R=0,4,6,2,3,7,5,1 C=7    E5
// D=0 R=1,5,7,3,2,6,4,0 C=3

// D=1 R=0,4,6,2,3,7,5,1 C=2    E6
// D=1 R=1,5,7,3,2,6,4,0 C=4

// D=1 R=0,4,6,2,3,7,5,1 C=7    E7
// D=1 R=1,5,7,3,2,6,4,0 C=3




const ledringMap ledringOrder[LED_RING_LED_COUNT] =
{
// Example for Device 0 first ring (adjust to match your wiring)

    {0,0,1},{0,4,1},{0,6,1},{0,2,1},{0,3,1},{0,7,1},{0,5,1},{0,1,1}, //1 E0.0-7
    {0,1,6},{0,5,6},{0,7,6},{0,3,6},{0,2,6},{0,6,6},{0,4,6},{0,0,6}, //1 E0.8-15

    {0,0,5},{0,4,5},{0,6,5},{0,2,5},{0,3,5},{0,7,5},{0,5,5},{0,1,5}, //2 E1.0-7
    {0,1,0},{0,5,0},{0,7,0},{0,3,0},{0,2,0},{0,6,0},{0,4,0},{0,0,0}, //3 E1.15-8
    
    {1,0,1},{1,4,1},{1,6,1},{1,2,1},{1,3,1},{1,7,1},{1,5,1},{1,1,1}, //4 E2.7-0
    {1,1,6},{1,5,6},{1,7,6},{1,3,6},{1,2,6},{1,6,6},{1,4,6},{1,0,6}, //5 E2.15-8

    {1,0,5},{1,4,5},{1,6,5},{1,2,5},{1,3,5},{1,7,5},{1,5,5},{1,1,5}, //6 E3.0-7
    {1,1,0},{1,5,0},{1,7,0},{1,3,0},{1,2,0},{1,6,0},{1,4,0},{1,0,0}, //7 E3.15-8
    
    {0,0,2},{0,4,2},{0,6,2},{0,2,2},{0,3,2},{0,7,2},{0,5,2},{0,1,2}, //8  E4.0-7
    {0,1,4},{0,5,4},{0,7,4},{0,3,4},{0,2,4},{0,6,4},{0,4,4},{0,0,4}, //9 E4.8-15

    {0,0,7},{0,4,7},{0,6,7},{0,2,7},{0,3,7},{0,7,7},{0,5,7},{0,1,7}, //10 E5.7-0
    {0,1,3},{0,5,3},{0,7,3},{0,3,3},{0,2,3},{0,6,3},{0,4,3},{0,0,3}, //11 E5.8-15

    {1,0,2},{1,4,2},{1,6,2},{1,2,2},{1,3,2},{1,7,2},{1,5,2},{1,1,2}, //12 E6.0-7
    {1,1,4},{1,5,4},{1,7,4},{1,3,4},{1,2,4},{1,6,4},{1,4,4},{1,0,4}, //13 E6.8-15

    {1,0,7},{1,4,7},{1,6,7},{1,2,7},{1,3,7},{1,7,7},{1,5,7},{1,1,7}, //14 E7.7-0
    {1,1,3},{1,5,3},{1,7,3},{1,3,3},{1,2,3},{1,6,3},{1,4,3},{1,0,3}, //15 E7.8-15

    

};


uint8_t ledring[LED_RING_NUM_DEVICES][8] = {0};

void ledringWriteRow(uint8_t row) {
  digitalWrite(LED_RING_PIN_CS, LOW);

  // IMPORTANT:
  // Shift data for the LAST device first
  /*
  SPI1.transfer(row + 1);      // MAX7219 DIG registers are 1..8
  SPI1.transfer(ledring[1][row]);   // device 1 (farther)

  SPI1.transfer(row + 1);
  SPI1.transfer(ledring[0][row]);   // device 0 (near)
  */
  for(int i=(LED_RING_NUM_DEVICES-1);i>=0;i--)
  {
    SPI1.transfer(row + 1);
    SPI1.transfer(ledring[i][row]);   // device 0 (near)
  }

  digitalWrite(LED_RING_PIN_CS, HIGH);
}


void ledringSetPixelDRC(uint8_t device, uint8_t row,uint8_t col,bool on) {
    // Safety
  if (device >= LED_RING_NUM_DEVICES || row > 7 || col > 7) return;

  uint8_t bitMask = (1 << col);

  if (on)
    ledring[device][row] |= bitMask;
  else
    ledring[device][row] &= ~bitMask;

  // Update only the affected row
  ledringWriteRow(row);
}




void ledringClearAll() {
  memset(ledring, 0, sizeof(ledring));
  for (uint8_t r = 0; r < 8; r++)
    ledringWriteRow(r);
}


//******************************************************************************************* */

void ledringSendCommandAll(uint8_t reg, uint8_t data) {
  digitalWrite(LED_RING_PIN_CS, LOW);
  // When daisy-chained, you shift out pairs (reg,data) for each device.
  // The last pair shifted ends up in the first device in the chain.
  for (int i = 0; i < LED_RING_NUM_DEVICES; i++) {
    SPI1.transfer(reg);
    SPI1.transfer(data);
  }
  digitalWrite(LED_RING_PIN_CS, HIGH);
}

void clearAllEncoderLEDs() {
    for (int row = 0; row < 8; row++) {
        digitalWrite(LED_RING_PIN_CS, LOW);
        for (int dev = 0; dev < LED_RING_NUM_DEVICES; dev++) {
            SPI1.transfer(row + 1);
            SPI1.transfer(0x00);
        }
        digitalWrite(LED_RING_PIN_CS, HIGH);
    }
}

void InitEncoderLED()
{
  pinMode(LED_RING_PIN_CS, OUTPUT);
  digitalWrite(LED_RING_PIN_CS, HIGH);

  delay(1);
  SPI1.begin();
  delay(1);
  #ifndef _GSIMULATOR
    SPI1.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  #endif

  delay(1);
  // Init MAX7219
  ledringSendCommandAll(LEDRING_REG_DISPLAYTEST, 0x00); // normal operation
  ledringSendCommandAll(LEDRING_REG_DECODEMODE,  0x00); // no decode for matrix
  ledringSendCommandAll(LEDRING_REG_SCANLIMIT,   0x07); // scan all 8 rows
  ledringSendCommandAll(LEDRING_REG_INTENSITY,   0x04); // 0x00..0x0F brightness
  ledringSendCommandAll(LEDRING_REG_SHUTDOWN,    0x01); // wake up

  delay(1);
  clearAllEncoderLEDs();
  
}

void ledRingSetLed(int ring, int led, boolean setreset)
{
  if (ring < 0 || ring >= LED_RING_COUNT) return;
  if (led < 0 || led >= LED_RING_LEDS_PER_RING) return;

  // Each group of 8 rings uses 2 MAX7219 devices
  int group = ring / 8;          // 0 for rings 0-7, 1 for rings 8-15
  int ringInGroup = ring % 8;    // 0..7 within the base mapping

  int idx = ringInGroup * LED_RING_LEDS_PER_RING + led; // 0..127

  ledringMap m = ledringOrder[idx];

  // Offset device index for the next pair (devices 2&3)
  m.device += group * 2;

  ledringSetPixelDRC(m.device, m.row, m.col, setreset);
}
// Control status LEDs (13, 14, 15) on each encoder
// encoder: 0-7 (only first 8 encoders have LED rings)
// status: 0, 1, or 2 (corresponds to LED 13, 14, or 15)
// onOff: true=LED on, false=LED off
void ShowEncoderStatus(int encoder, int status, bool onOff)
{
    if (encoder < 0 || encoder >= LED_RING_COUNT) return;

    // Status LEDs are 13, 14, 15 (status parameter is 0, 1, or 2)
    if (status < 0 || status > 2) return;

    int ledIndex = 13 + status; // Convert status 0,1,2 to LED 13,14,15

    ledRingSetLed(encoder, ledIndex, onOff);
}

// Convenience functions for controlling all three status LEDs at once
// encoder: 0-7
// led13, led14, led15: true=on, false=off
void SetEncoderStatusLEDs(int encoder, bool led13, bool led14, bool led15)
{
    if (encoder < 0 || encoder >= LED_RING_COUNT) return;

    ledRingSetLed(encoder, 13, led13);
    ledRingSetLed(encoder, 14, led14);
    ledRingSetLed(encoder, 15, led15);
}

// Clear all status LEDs on a specific encoder
void ClearEncoderStatus(int encoder)
{
    if (encoder < 0 || encoder >= LED_RING_COUNT) return;

    ledRingSetLed(encoder, 13, false);
    ledRingSetLed(encoder, 14, false);
    ledRingSetLed(encoder, 15, false);
}

// Clear all status LEDs on all encoders
void ClearAllEncoderStatus()
{
    for (int i = 0; i < LED_RING_COUNT; i++) {
        ledRingSetLed(i, 13, false);
        ledRingSetLed(i, 14, false);
        ledRingSetLed(i, 15, false);
    }
}


#endif