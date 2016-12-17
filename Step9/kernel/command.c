#include <command.h>
#include <console.h>
#include <keyboard.h>
#include <hal.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <floppydisk.h>
#include <fat12.h>

char cmd_prompt[255] = "Command";

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
				
				memset(currentInput, 0, 255);
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

void Append(char* _destination, char _source)
{
	size_t len = strlen(_destination);
	_destination[len] = _source;
	_destination[len + 1] = 0;
}

void RemoveLast(char* _destination, const char* _source)
{
	size_t len = strlen(_source);
	
	for(size_t i = 0; i < len - 1; i++)
	{
		_destination[i] = _source[i];
	}
	
	_destination[len - 1] = 0;
}

void CopyTo(char* _destination, const char* _source)
{
	size_t len = strlen(_source);
	
	for(size_t i = 0; i < len; i++)
	{
		_destination[i] = _source[i];
	}
	
	_destination[len] = 0;
}

void ProcessCMD(char* _cmd)
{
	char cmdArg[255]; 
	GetStringArgument(0, cmdArg, _cmd);

	// If two strings are exactly the same.
	if(strcmp("cls", cmdArg) == 0)
	{
		ConsoleClearScreen(0x1F);
	}
	else if(strcmp("exit", cmdArg) == 0)
	{
		ConsoleWriteString("\n");
		ConsoleWriteString("Operating system shutting down.");
		running = false;
	}
	else if(strcmp("prompt", cmdArg) == 0)
	{
		char prompt[255];
		GetStringArgument(1, prompt, _cmd);
		
		if(prompt[0] == 0)
		{
			ConsoleWriteString("\nInvalid command usage. Usage: prompt <word>");
			return;
		}
		
		CopyTo(cmd_prompt, prompt);
	}
	else if(strcmp("read", cmdArg) == 0)
	{
		char filePath[255];
		GetStringArgument(1, filePath, _cmd);
		
		if(filePath[0] == 0)
		{
			ConsoleWriteString("\nInvalid command usage. Usage: read <path>");
			return;
		}
	}
	else if(strcmp("cd", cmdArg) == 0)
	{
		char dir[255];
		GetStringArgument(1, dir, _cmd);
		
		if(dir[0] == 0)
		{
			ConsoleWriteString("\nInvalid command usage. Usage cd <path> | . | ..");
			return;
		}
		
		if(strcmp(dir, ".") == 0)
		{
			ConsoleWriteString("\nStaying in current directory.");
		}
		else if(strcmp(dir, "..") == 0)
		{
			ConsoleWriteString("\nMoving back a directory.");
		}
	}
	else
	{
		ConsoleWriteString("\nUnknown command: ");
		ConsoleWriteString(cmdArg);
	}
}

void PrintPrompt()
{
	ConsoleWriteString("\n");
	ConsoleWriteString(cmd_prompt);
	ConsoleWriteString(">");
}

// Get an argument as a string from a given command, and place it in the given destination.
void GetStringArgument(int _argIndex, char* _dest, char* _source)
{
	// Where in the string the last space was detected.
	int last_space_index = 0;
	
	// Current argument being scanned.
	int argCounter = 0;

	for (int i = 0; i <= strlen(_source); ++i)
	{
		// When a space is found,
		if (_source[i] == ' ' || _source[i] == 0)
		{
			// Check if the argument counter matches the requested argument index.
			if(argCounter != _argIndex)
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
				_dest[j] = _source[last_space_index + j];
			}

			// Null terminate the argument.
			_dest[i - last_space_index] = 0;
			return;
		}
	}

	_dest[0] = 0;
}

int GetIntArgument(char* _source)
{
	
	return 0;
}