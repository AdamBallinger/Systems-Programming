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
	PrintPrompt();
	
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
				RemoveLast(currentInput, currentInput);
				inputLength--;
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
				
				currentInput[0] = '\0';
				inputLength = 0;
			}

			continue;
		}
		
		if(inputLength == 255) 
		{
			continue;
		}
		
		Append(currentInput, keyChar);
		inputLength++;	
		ConsoleWriteCharacter(keyChar);
	}	
}

void Append(char* destination, char source)
{
	size_t len = strlen(destination);
	destination[len] = source;
	destination[len + 1] = '\0';
}

void RemoveLast(char* destination, const char* source)
{
	size_t len = strlen(source);
	
	for(size_t i = 0; i < len - 1; i++)
	{
		destination[i] = source[i];
	}
	
	destination[len - 1] = '\0';
}

void ProcessCMD(char* cmd)
{
	char cmdArg[255]; 
	GetStringArgument(0, cmdArg, cmd);

	// If two strings are exactly the same.
	if(strcmp("cls", cmdArg) == 0)
	{
		ConsoleClearScreen(0x1F);
		return;
	}
	
	if(strcmp("exit", cmdArg) == 0)
	{
		ConsoleWriteString("\n");
		ConsoleWriteString("Operating system shutting down.");
		running = false;
		return;
	}
	
	if(strcmp("prompt", cmdArg) == 0)
	{
		char prompt[255];
		GetStringArgument(1, prompt, cmd);
		
		if(prompt[0] == '\0')
		{
			ConsoleWriteString("\nInvalid command usage. Usage: prompt <string>");
			return;
		}
		
		strcpy(cmd_prompt, prompt);
		return;
	}
	
	if(strcmp("read", cmdArg) == 0)
	{
		char filePath[255];
		GetStringArgument(1, filePath, cmd);
		return;
	}
	
	if(strcmp("cd", cmdArg) == 0)
	{
		char dir[255];
		GetStringArgument(1, dir, cmd);
		return;
	}
	
	ConsoleWriteString("\nUnknown command: ");
	ConsoleWriteString(cmdArg);
}

void PrintPrompt()
{
	ConsoleWriteString("\n");
	ConsoleWriteString(cmd_prompt);
	ConsoleWriteString(">");
}

// Get an argument as a string from a given command, and place it in the given destination.
void GetStringArgument(int argIndex, char* dest, char* source)
{
	// Where in the string the last space was detected.
	int last_space_index = 0;
	
	// Current argument being scanned.
	int argCounter = 0;

	// Add a space on to the end of the source so the end argument can be retrieved.
	//Append(source, ' ');

	for (int i = 0; i <= strlen(source); ++i)
	{
		// When a space is found,
		if (source[i] == ' ' || source[i] == '\0')
		{
			// Check if the argument counter matches the requested argument index.
			if(argCounter != argIndex)
			{
				// If not then increment argument count and record the current string index as 
				// the last detected space position.
				argCounter++;
				last_space_index = i + 1;
				continue;
			}

			// If the arg counter matches the requested argument index 
			for (int j = 0; j < i - last_space_index; j++)
			{
				// Add each character from the last recorded space index to the current.
				dest[j] = source[last_space_index + j];
			}

			// Null terminate the argument.
			dest[i - last_space_index] = '\0';
			return;
		}
	}

	dest[0] = '\0';
}

int GetIntArgument(char* source)
{
	
	return 0;
}