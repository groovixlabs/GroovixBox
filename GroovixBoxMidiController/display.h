/***************************************************
  GlowDial MIDI Controller - Display Functions

  This file contains all LCD display related functions
  for showing encoder status and system information.

  MIT License
 ****************************************************/

#ifndef DISPLAY_H
#define DISPLAY_H

#ifndef _GSIMULATOR
#include "Adafruit_ILI9341.h"
extern Adafruit_ILI9341 tft;
#endif



//==============================================================================
// DISPLAY FUNCTIONS
//==============================================================================

// Update just a single encoder's display area (fast partial update)
void UpdateSingleEncoderDisplay(int encoderID) {
    // Display layout constants
    const int xOffset = 10;
    const int yOffset = 30;
    const int cellWidth = 75;
    const int cellHeight = 50;

    // Calculate position in grid
    int row = encoderID / 4;
    int col = encoderID % 4;
    int x = xOffset + (col * cellWidth);
    int y = yOffset + (row * cellHeight);

    // Clear this encoder's cell area
    tft.fillRect(x, y, cellWidth - 2, cellHeight - 2, ILI9341_BLACK);

    // Encoder number
    tft.setTextSize(1);
    tft.setTextColor(ILI9341_YELLOW);
    tft.setCursor(x, y);
    tft.print("E");
    tft.print(encoderID);

    // Encoder value with bar graph
    int value = EncoderPosition[encoderID];
    tft.setTextColor(ILI9341_WHITE);
    tft.setCursor(x + 20, y);

    // Clear value area and print new value (handle 1-2 digit numbers)
    tft.fillRect(x + 20, y, 20, 8, ILI9341_BLACK);
    tft.setCursor(x + 20, y);
    tft.print(value);

    // Draw value bar
    int barWidth = (value * 50) / MAX_ENC_POS;
    tft.fillRect(x, y + 12, 50, 6, ILI9341_BLACK);  // Clear bar area
    tft.fillRect(x, y + 12, barWidth, 6, ILI9341_GREEN);
    tft.drawRect(x, y + 12, 50, 6, ILI9341_WHITE);

    // Show button state
    if (EncodersBtnState_Logical[encoderID] == ENC_BTN_PRESS) {
        tft.fillCircle(x + 55, y + 15, 3, ILI9341_RED);
    } else {
        tft.fillCircle(x + 55, y + 15, 3, ILI9341_BLACK);  // Clear
        tft.drawCircle(x + 55, y + 15, 3, ILI9341_DARKGREY);
    }
}

// Full screen update (used for initial display)
void UpdateEncoderDisplay() {
    tft.fillScreen(ILI9341_BLACK);

    // Title
    tft.setTextColor(ILI9341_CYAN);
    tft.setTextSize(2);
    tft.setCursor(10, 5);
    tft.println("Encoder Status");

    // Draw encoder grid (4x4 layout)
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

            // Encoder number
            tft.setTextColor(ILI9341_YELLOW);
            tft.setCursor(x, y);
            tft.print("E");
            tft.print(encoderID);

            // Encoder value with bar graph
            int value = EncoderPosition[encoderID];
            tft.setTextColor(ILI9341_WHITE);
            tft.setCursor(x + 20, y);
            tft.print(value);

            // Draw value bar
            int barWidth = (value * 50) / MAX_ENC_POS;
            tft.fillRect(x, y + 12, barWidth, 6, ILI9341_GREEN);
            tft.drawRect(x, y + 12, 50, 6, ILI9341_WHITE);

            // Show button state - now using logical encoder ID
            if (EncodersBtnState_Logical[encoderID] == ENC_BTN_PRESS) {
                tft.fillCircle(x + 55, y + 15, 3, ILI9341_RED);
            } else {
                tft.drawCircle(x + 55, y + 15, 3, ILI9341_DARKGREY);
            }
        }
    }

    // Footer - show MIDI info
    tft.setTextColor(ILI9341_DARKGREY);
    tft.setTextSize(1);
    tft.setCursor(10, 225);
    tft.print("MIDI CH:");
    tft.print(MIDI_CHANNEL_ENCODERS);
    tft.print(" CC:");
    tft.print(MIDI_CC_START);
    tft.print("-");
    tft.print(MIDI_CC_START + 15);
}

void DisplayReadyScreen() {
    // Show label grid as the ready screen
    InitLabelGrid();
    DisplayLabelGrid();
    labelGridActive = true;
}

//==============================================================================
// LABEL GRID SYSTEM (4x4 Grid - 16 Slots)
//==============================================================================

#define LABEL_GRID_SLOTS 16
#define LABEL_MAX_LENGTH 32

struct LabelSlot {
    char text[LABEL_MAX_LENGTH + 1];
    uint16_t color;
    uint8_t fontSize;  // 1, 2, 3, etc.
    bool isDirty;  // Flag to track if slot needs redrawing
};

LabelSlot labelSlots[LABEL_GRID_SLOTS];
bool labelGridActive = false;

// Initialize label grid
void InitLabelGrid() {
    for (int i = 0; i < LABEL_GRID_SLOTS; i++) {
        labelSlots[i].text[0] = '\0';  // Empty string
        labelSlots[i].color = ILI9341_WHITE;
        labelSlots[i].fontSize = 1;
        labelSlots[i].isDirty = true;
    }
    labelGridActive = false;
}

// Set a label for a specific slot (0-15)
// slot: 0-15 (4x4 grid, left to right, top to bottom)
// text: Label text (max 32 characters, will wrap)
// color: 16-bit RGB565 color
// fontSize: 1-4 (1=small, 2=medium, 3=large, 4=xlarge)
void SetLabel(int slot, const char* text, uint16_t color, uint8_t fontSize = 1) {
    if (slot < 0 || slot >= LABEL_GRID_SLOTS) return;
    if (fontSize < 1) fontSize = 1;
    if (fontSize > 4) fontSize = 4;

    // Copy text with length limit
    strncpy(labelSlots[slot].text, text, LABEL_MAX_LENGTH);
    labelSlots[slot].text[LABEL_MAX_LENGTH] = '\0';
    labelSlots[slot].color = color;
    labelSlots[slot].fontSize = fontSize;
    labelSlots[slot].isDirty = true;

    labelGridActive = true;
}

// Clear a specific label slot
void ClearLabel(int slot) {
    if (slot < 0 || slot >= LABEL_GRID_SLOTS) return;

    labelSlots[slot].text[0] = '\0';
    labelSlots[slot].isDirty = true;
}

// Clear all labels
void ClearAllLabels() {
    for (int i = 0; i < LABEL_GRID_SLOTS; i++) {
        labelSlots[i].text[0] = '\0';
        labelSlots[i].isDirty = true;
    }
}

// Helper function to draw wrapped text in a slot
void DrawWrappedText(int x, int y, int slotWidth, int slotHeight, const char* text, uint16_t color, uint8_t fontSize) {
    const int padding = 3;
    const int charWidth = 6 * fontSize;   // Approximate character width
    const int charHeight = 8 * fontSize;  // Approximate character height
    const int lineSpacing = charHeight + 1;

    int maxCharsPerLine = (slotWidth - padding * 2) / charWidth;
    if (maxCharsPerLine < 1) maxCharsPerLine = 1;

    int maxLines = (slotHeight - padding * 2 - 10) / lineSpacing;  // -10 for slot number
    if (maxLines < 1) maxLines = 1;

    tft.setTextSize(fontSize);
    tft.setTextColor(color, ILI9341_BLACK);

    int textLen = strlen(text);
    int currentX = x + padding;
    int currentY = y + 12;  // Start below slot number
    int currentLine = 0;
    int currentLineChars = 0;

    for (int i = 0; i < textLen && currentLine < maxLines; i++) {
        char ch = text[i];

        // Check for manual line break or max chars per line
        if (ch == '\n' || currentLineChars >= maxCharsPerLine) {
            currentLine++;
            currentY += lineSpacing;
            currentX = x + padding;
            currentLineChars = 0;

            // Skip the newline character
            if (ch == '\n') continue;
        }

        if (currentLine < maxLines) {
            tft.setCursor(currentX, currentY);
            tft.print(ch);
            currentX += charWidth;
            currentLineChars++;
        }
    }
}

// Display the 4x4 label grid
void DisplayLabelGrid() {
    const int slotWidth = 80;   // 320 / 4
    const int slotHeight = 60;  // 240 / 4

    tft.fillScreen(ILI9341_BLACK);

    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            int slot = row * 4 + col;
            int x = col * slotWidth;
            int y = row * slotHeight;

            // Draw grid border
            tft.drawRect(x, y, slotWidth, slotHeight, ILI9341_DARKGREY);

            // Draw slot number in top-left corner
            tft.setTextSize(1);
            tft.setTextColor(ILI9341_DARKGREY, ILI9341_BLACK);
            tft.setCursor(x + 2, y + 2);
            tft.print(slot);

            // Draw label text with wrapping
            if (labelSlots[slot].text[0] != '\0') {
                DrawWrappedText(x, y, slotWidth, slotHeight,
                               labelSlots[slot].text,
                               labelSlots[slot].color,
                               labelSlots[slot].fontSize);
            }

            labelSlots[slot].isDirty = false;
        }
    }
}

// Update only dirty slots (partial refresh)
void UpdateLabelGrid() {
    if (!labelGridActive) return;

    const int slotWidth = 80;
    const int slotHeight = 60;

    for (int slot = 0; slot < LABEL_GRID_SLOTS; slot++) {
        if (!labelSlots[slot].isDirty) continue;

        int row = slot / 4;
        int col = slot % 4;
        int x = col * slotWidth;
        int y = row * slotHeight;

        // Clear slot interior (keep border)
        tft.fillRect(x + 1, y + 1, slotWidth - 2, slotHeight - 2, ILI9341_BLACK);

        // Redraw slot number
        tft.setTextSize(1);
        tft.setTextColor(ILI9341_DARKGREY, ILI9341_BLACK);
        tft.setCursor(x + 2, y + 2);
        tft.print(slot);

        // Draw label text with wrapping
        if (labelSlots[slot].text[0] != '\0') {
            DrawWrappedText(x, y, slotWidth, slotHeight,
                           labelSlots[slot].text,
                           labelSlots[slot].color,
                           labelSlots[slot].fontSize);
        }

        labelSlots[slot].isDirty = false;
    }
}

// Helper function to convert RGB888 to RGB565
uint16_t RGB888toRGB565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

#endif // DISPLAY_H
