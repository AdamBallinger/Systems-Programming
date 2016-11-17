#include <command.h>
#include <console.h>
#include <keyboard.h>
#include <hal.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <floppydisk.h>

char* cmd_prompt = "Command";

uint32_t inputLength = 0;
char currentInput[255];

bool running = true;

void Run() 
{
	ConsoleWriteString(cmd_prompt);
	ConsoleWriteString(">");
	
	while(running)
	{
		// Get ASCII character for the key pressed.
		char keycode = KeyboardGetCharacter();
		char keyChar = KeyboardConvertKeyToASCII(keycode);
		
		if(keycode == KEY_UNKNOWN) continue;
		
		int currentConsoleX, currentConsoleY;
		ConsoleGetXY(&currentConsoleX, &currentConsoleY);
		
		if(keycode == KEY_BACKSPACE)
		{
			if(inputLength > 0 && currentConsoleX > 0)
			{
				ConsoleGotoXY(--currentConsoleX, currentConsoleY);
				currentInput[--inputLength] = 0;
				ConsoleWriteCharacter(' ');
				ConsoleGotoXY(currentConsoleX, currentConsoleY);
			}

			continue;
		}
		
		if(keycode == KEY_RETURN)
		{
			if(inputLength > 0)
			{
				ProcessCMD(currentInput);
				
				if(running)
				{
					PrintPrompt();
				}
				
				currentInput[0] = 0;
				inputLength = 0;
			}

			continue;
		}
		
		Append(currentInput, keyChar);
		ConsoleWriteCharacter(keyChar);
	}	
}

void Append(char* destination, char source)
{
	size_t len = strlen(destination);
	destination[len] = source;
	destination[len + 1] = '\0';
	inputLength++;
}

void ProcessCMD(char* cmd)
{
	// If two strings are exactly the same.
	if(strcmp("cls", cmd) == 0)
	{
		ConsoleClearScreen(0x1F);
		return;
	}
	
	if(strcmp("exit", cmd) == 0)
	{
		ConsoleWriteString("\n");
		ConsoleWriteString("Operating system shutting down.");
		running = false;
		return;
	}
}

void PrintPrompt()
{
	ConsoleWriteString("\n");
	ConsoleWriteString(cmd_prompt);
	ConsoleWriteString(">");
}