#ifndef _CONSOLE_H
#define _CONSOLE_H
#include <stdint.h>
#include <size_t.h>

#define DECIMAL 10
#define HEX 16

// Console output colours.
#define BLACK 0x0
#define BLUE 0x1
#define GREEN 0x2
#define CYAN 0x3
#define RED 0x4
#define MAGENTA 0x5
#define BROWN 0x6
#define LIGHTGRAY 0x7

// Returns colour for console display.
uint8_t GetColour(uint8_t background, uint8_t foreground, bool flashText, bool forebright);

// Write a given value as binary to console.
void ConsoleWriteBinary(size_t val);

// Output the specified character the current cursor position.
// The attribute at that position remains unchanged.

void ConsoleWriteCharacter(unsigned char c);

// Write the null-terminated string str to the current cursor position on the screen

void ConsoleWriteString(char* str);

// Write the unsigned integer to the screen, using the specified base, e.g. for
// the number to be displayed as a decimal number, you would specify base 10.  For
// a number to be displayed as hexadecimal, you would specify base 16.

void ConsoleWriteInt(unsigned int i, unsigned int base);

// Set the attribute to be used for all subsequent calls to ConsoleWriteXXXX routines

unsigned int ConsoleSetColour(const uint8_t c);

// Position the cursor at the specified X and Y position. All subsequent calls to
// ConsoleWriteXXXX routines will start at this position.

void ConsoleGotoXY(unsigned int x, unsigned int y);

// Get the current X and Y position of the cursor

void ConsoleGetXY(unsigned* x, unsigned* y);

// Get the width of the console window

int ConsoleGetWidth();

// Get the height of the console window

int ConsoleGetHeight();

// Clear the screen.  Set the attribute byte for all characters on the
// screen to be c. Set the cursor position to 0,0.

void ConsoleClearScreen(const uint8_t c);

#endif
