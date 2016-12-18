#include <command.h>
#include <console.h>
#include <keyboard.h>
#include <stdint.h>
#include <string.h>
#include <fat12.h>

char cmd_prompt[256] = "Command";

uint32_t inputLength = 0;
char currentInput[256];

_Bool running = true;

// Start in root directory.
char currentDirectory[256] = ".\\";

// Data buffer for read files.
char data[4096];

void Run()
{
	PrintPrompt();

	while (running)
	{
		// Get ASCII character for the key pressed.
		char keycode = KeyboardGetCharacter();
		char keyChar = KeyboardConvertKeyToASCII(keycode);

		if (keycode == KEY_UNKNOWN) continue;

		unsigned int currentConsoleX, currentConsoleY;
		ConsoleGetXY(&currentConsoleX, &currentConsoleY);

		if (keycode == KEY_BACKSPACE)
		{
			if (inputLength > 0 && currentConsoleX > 0)
			{
				ConsoleGotoXY(--currentConsoleX, currentConsoleY);
				RemoveLast(currentInput);
				inputLength--;
				ConsoleWriteCharacter(' ');
				ConsoleGotoXY(currentConsoleX, currentConsoleY);
			}

			continue;
		}

		if (keycode == KEY_RETURN)
		{
			if (inputLength > 0)
			{
				ProcessCMD(currentInput);

				if (running)
				{
					PrintPrompt();
				}

				memset(currentInput, 0, 256);
				inputLength = 0;
			}

			continue;
		}

		if (inputLength == 256)
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

void AppendAll(char* _destination, const char* _source)
{
	while (*_source)
	{
		Append(_destination, *_source);
		*_source++;
	}
}

void RemoveLast(char* _destination)
{
	const char* source = _destination;
	size_t len = strlen(source);
	_destination[len - 1] = 0;
}

void CopyTo(char* _destination, const char* _source)
{
	size_t len = strlen(_source);

	for (size_t i = 0; i < len; i++)
	{
		_destination[i] = _source[i];
	}

	_destination[len] = 0;
}

void DirectoryBack()
{
	size_t len = strlen(currentDirectory);

	// If the length is 2 then the directory is ".\" and at the root so dont try go back a directory.
	if (len == 2) return;

	// Check for '\' at the of the current directory.
	if (currentDirectory[len - 1] == '\\')
	{
		// if the directory ends in a '\' then delete every instance of this char from the current directory
		// until we reach a character that isn't a '\'. This is so a directory such as '.\folder\folder2\\\\\\'
		// will correctly go back to '.\folder\' instead of '.\folder\folder2\\\\\'
		unsigned int i = len - 1;
		while (currentDirectory[i] == '\\')
		{
			currentDirectory[i] = 0;
			i--;
		}
	}

	for (int i = len; i > 0; i--)
	{
		if (currentDirectory[i] == 0) continue;
		if (currentDirectory[i] == '\\') break;

		currentDirectory[i] = 0;
	}
}

void ProcessCMD(char* _cmd)
{
	char cmdArg[256];
	GetStringArgument(0, cmdArg, _cmd);

	// If two strings are exactly the same.
	if (strcmp("cls", cmdArg) == 0)
	{
		ConsoleClearScreen(0x1F);
	}
	else if (strcmp("exit", cmdArg) == 0)
	{
		ConsoleWriteString("\n");
		ConsoleWriteString("Operating system shutting down.");
		running = false;
	}
	else if (strcmp("prompt", cmdArg) == 0)
	{
		char prompt[256];
		GetStringArgument(1, prompt, _cmd);

		if (prompt[0] == 0)
		{
			ConsoleWriteString("\nInvalid command usage. Usage: prompt <word>");
			return;
		}

		CopyTo(cmd_prompt, prompt);
	}
	else if (strcmp("read", cmdArg) == 0)
	{
		char filePath[256];
		GetStringArgument(1, filePath, _cmd);

		if (filePath[0] == 0)
		{
			ConsoleWriteString("\nInvalid command usage. Usage: read <path>");
			return;
		}

		char readDir[256];
		memset(readDir, 0, 256);
		AppendAll(readDir, currentDirectory);
		AppendAll(readDir, filePath);

		ConsoleWriteString("\nReading ");
		ConsoleWriteString(readDir);
		ConsoleWriteString("\n");

		FILE file = FsFat12_Open(readDir);
		if (file.Flags == FS_FILE)
		{
			ConsoleWriteString("\nFile size (bytes): ");
			ConsoleWriteInt(file.FileLength, 10);
			FsFat12_Read(&file, data, file.FileLength);
			ConsoleWriteString("\nFile data: \n");
			for (int i = 0; i < file.FileLength; i++)
			{
				ConsoleWriteCharacter(data[i]);
			}
		}
	}
	else if (strcmp("cd", cmdArg) == 0)
	{
		char dir[256];
		GetStringArgument(1, dir, _cmd);

		if (dir[0] == 0)
		{
			ConsoleWriteString("\nInvalid command usage. Usage cd <path> | . | ..");
			return;
		}

		if (strcmp(dir, ".") == 0)
		{
			ConsoleWriteString("\n");
			ConsoleWriteString(currentDirectory);
		}
		else if (strcmp(dir, "..") == 0)
		{
			DirectoryBack();
			ConsoleWriteString("\n");
			ConsoleWriteString(currentDirectory);
		}
		else
		{
			AppendAll(currentDirectory, dir);
			Append(currentDirectory, '\\');
		}
	}
	else if (strcmp("pwd", cmdArg) == 0)
	{
		ConsoleWriteString("\n");
		ConsoleWriteString(currentDirectory);
	}
	else if (strcmp("dir", cmdArg) == 0)
	{
		ConsoleWriteString("\n");
		// TODO: List all files/folders in the current directory.
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
		// When a space is found or the string ends.
		if (_source[i] == 0 || _source[i] == ' ')
		{
			// Check if the argument counter matches the requested argument index.
			if (argCounter != _argIndex)
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