//	Basic Console Output.

#include <stdint.h>
#include <string.h>
#include <console.h>

// Video memory
uint16_t *_videoMemory = (uint16_t *)0xB8000;

#define CONSOLE_HEIGHT		25
#define CONSOLE_WIDTH		80

// Current cursor position
uint8_t _cursorX = 0;
uint8_t _cursorY = 0;

// Current color
uint8_t	_colour = 0;

char stringBuffer[40];
char hexChars[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

// Write byte to device through port mapped io
void  OutputByteToVideoController(unsigned short portid, unsigned char value)
{
	asm volatile("movb	%1, %%al \n\t"
				 "movw	%0, %%dx \n\t"
				  "out	%%al, %%dx"
				 : : "m" (portid), "m" (value));

}

// Returns colour for console display.
uint8_t GetColour(uint8_t background, uint8_t foreground, bool flashText, bool forebright)
{
	//
	//	Colour bits format:
	//		
	//		Bit 7 - ON: Sets text to flash  OFF: Sets background colour to brighter version.
	//		Bits 6 to 4 - Control the background colour.
	//		Bit 3 - ON: Sets foreground colour to bright.
	//		Bits 2 to 0 - Control the foreground colour.
	//

    uint8_t colour = 0x0;

	// Set the background colour.
    colour |= (background << 4);

	// Set text to flash if flag is set.
    if (flashText)
    {
		// Set 7th bit to on
		colour |= (1 << 7);
	}

	// If true then set the text colour to the brighter version.
    if (forebright)
	{
		colour |= (foreground + 0x8);
	}
    else
	{
		colour |= (foreground);
	}
        
    return colour;
}

// Write a given value as binary to console.
void ConsoleWriteBinary(size_t val)
{
    ConsoleWriteCharacter('[');

    int bitsDone = 0;

	// Display the state of each bit in the value (1 or 0)
    for(int64_t bit = (sizeof(val) * 8) - 1; bit >= 0; bit--)
    {
        int64_t bitVal = val >> bit;

		// Place a space between each set of 8 bits
        if (bitsDone > 0 && bitsDone % 8 == 0)
        {
            ConsoleWriteCharacter(' ');
            bitsDone = 0;
        }

        if(bitVal & 1)
        {
            ConsoleWriteString("1");
        }
        else
        {
            ConsoleWriteString("0");
        }

        bitsDone++;
    }

    ConsoleWriteCharacter(']');

    ConsoleWriteCharacter('\n');
}

// Update hardware cursor position

void UpdateCursorPosition(int x, int y)
{
    uint16_t cursorLocation = y * 80 + x;

	// Send location to VGA controller to set cursor
	// Send the high byte
	OutputByteToVideoController(0x3D4, 14);
	OutputByteToVideoController(0x3D5, cursorLocation >> 8);
	// Send the low byte
	OutputByteToVideoController(0x3D4, 15);
	OutputByteToVideoController(0x3D5, cursorLocation);
}

void Scroll()
{
	if (_cursorY >= CONSOLE_HEIGHT)
	{
		uint16_t attribute = _colour << 8;

		// Move current display up one line
		int line25 = (CONSOLE_HEIGHT - 1) * CONSOLE_WIDTH;
		for (int i = 0; i < line25; i++)
		{
			_videoMemory[i] = _videoMemory[i + CONSOLE_WIDTH];
		}
		// Clear the bottom line
		for (int i = line25; i < line25 + 80; i++)
		{
			_videoMemory[i] = attribute | ' ';
		}
		_cursorY = 25;
	}
}

// Displays a character
void ConsoleWriteCharacter(unsigned char c)
{
    uint16_t attribute = _colour << 8;

    if (c == 0x08 && _cursorX)
	{
		// Backspace character
        _cursorX--;
	}
    else if (c == 0x09)
	{
		// Tab character
        _cursorX = (_cursorX + 8) & ~(8 - 1);
	}
    else if (c == '\r')
	{
		// Carriage return
        _cursorX = 0;
	}
	else if (c == '\n')
	{
		// New line
        _cursorX = 0;
        _cursorY++;
	}
    else if (c >= ' ')
	{
		// Printable characters

		// Display character on screen
        uint16_t* location = _videoMemory + (_cursorY * CONSOLE_WIDTH + _cursorX);
        *location = c | attribute;
        _cursorX++;
    }
    // If we are at edge of row, go to new line
    if (_cursorX >= CONSOLE_WIDTH)
	{
        _cursorX = 0;
        _cursorY++;
    }
	// If we are at the last line, scroll up
	if (_cursorY >= CONSOLE_HEIGHT)
	{
		Scroll();
		_cursorY = CONSOLE_HEIGHT - 1;
	}
    //! update hardware cursor
	UpdateCursorPosition (_cursorX,_cursorY);
}

void ConsoleWriteInt(unsigned int i, unsigned int base)
{
    int pos = 0;

    if (i == 0 || base > 16)
    {
		ConsoleWriteCharacter('0');
    }
	else
	{
		while (i != 0)
		{
			stringBuffer[pos] = hexChars[i % base];
			pos++;
			i /= base;
		}
		while (--pos >= 0)
		{
			ConsoleWriteCharacter(stringBuffer[pos]);
		}
	}
}

// Sets new font colour and returns the old colour
unsigned int ConsoleSetColour(const uint8_t c)
{
	unsigned int t = (unsigned int)_colour;
	_colour = c;
	return t;
}

// Set new cursor position
void ConsoleGotoXY(unsigned int x, unsigned int y)
{
	if (_cursorX <= CONSOLE_WIDTH - 1)
	{
	    _cursorX = x;
	}
	if (_cursorY <= CONSOLE_HEIGHT - 1)
	    _cursorY = y;

	//! update hardware cursor to new position
	UpdateCursorPosition(_cursorX, _cursorY);
}

// Returns cursor position
void ConsoleGetXY(unsigned* x, unsigned* y)
{
	if (x == 0 || y == 0)
	{
		return;
	}
	*x = _cursorX;
	*y = _cursorY;
}

// Return horizontal width
int ConsoleGetWidth()
{
	return CONSOLE_WIDTH;
}

// Return vertical height
int ConsoleGetHeight()
{
	return CONSOLE_HEIGHT;
}

//! Clear screen
void ConsoleClearScreen(const uint8_t c)
{
	_colour = c;
	uint16_t blank = ' ' | (c << 8);
	for (int i = 0; i < 80*25; i++)
	{
        _videoMemory[i] = blank;
	}
    ConsoleGotoXY(0,0);
}

// Display specified string

void ConsoleWriteString(char* str)
{
	if (!str)
	{
		return;
	}
	while (*str)
	{
		ConsoleWriteCharacter(*str++);
	}
}



