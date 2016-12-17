#ifndef _COMMAND_H
#define _COMMAND_H

void Run();
void Append(char* _destination, char _source);
void AppendAll(char* _destination, const char* _source);
void RemoveLast(char* _destination, const char* _source);
void CopyTo(char* _destination, const char* _source);
void ProcessCMD(char* _cmd);
void PrintPrompt();
void GetStringArgument(int _argIndex, char* _dest, char* _source);

#endif