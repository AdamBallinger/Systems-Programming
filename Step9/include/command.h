#ifndef _COMMAND_H
#define _COMMAND_H

void Run();

// Append a character to the end of given destination string.
void Append(char* _destination, char _source);
// Append all characters from given source string to a given destination string.
void AppendAll(char* _destination, const char* _source);
// Remove the last character of a given string.
void RemoveLast(char* _destination);
// Copy given source to the given destination with a null terminate.
void CopyTo(char* _destination, const char* _source);

// Remove the last appended directory from the current directory.
void DirectoryBack();

// Process the given command.
void ProcessCMD(char* _cmd);
// Prints the command prompt to the console.
void PrintPrompt();
// Extracts argument at given index int given destination 
// from the given cmd source.
void GetStringArgument(int _argIndex, char* _dest, char* _source);

#endif