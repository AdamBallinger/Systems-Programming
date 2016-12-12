#ifndef _COMMAND_H
#define _COMMAND_H

void Run();
void ClearInput();
void Append(char* destination, char source);
void ProcessCMD(char* cmd);
void PrintPrompt();
void GetStringArgument(int argIndex, char* dest, char* source);
int GetIntArgument(char* source);

#endif