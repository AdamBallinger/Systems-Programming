#include <command.h>

char* cmd_prompt = "Command";

uint32_t inputLength = 0;
char* currentInput[255];

void Run() 
{
	while(true)
	{
		// Get ASCII character for the key pressed.
		char keycode = KeyboardGetCharacter();
		char keyChar = KeyboardConvertKeyToASCII(keycode);
		
		if(keycode == KEY_UNKNOWN) continue;
		
		int currentConsoleX, currentConsoleY;
		ConsoleGetXY(&currentConsoleX, &currentConsoleY);
		
		if(keycode == KEY_BACKSPACE && currentConsoleX != 0)
		{
			ConsoleGotoXY(--currentConsoleX, currentConsoleY);
			currentInput[inputLength--] = 0;
			ConsoleWriteCharacter(' ');
			ConsoleGotoXY(currentConsoleX, currentConsoleY);
			continue;
		}
		
		if(keycode == KEY_RETURN)
		{
			ConsoleWriteString("\nENTER -> ");
			ConsoleWriteString(currentInput);
			ConsoleWriteString("\n");
			currentInput = 0;
			inputLength = 0;
			continue;
		}
		
		currentInput[inputLength++] = keyChar;
		ConsoleWriteCharacter(keyChar);
	}	
}
