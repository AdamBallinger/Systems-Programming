#ifndef FAT12_H
#define FAT12_H

#include <floppydisk.h>
#include <filesystem.h>
#include <bpb.h>
#include <console.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#define SECTOR_SIZE 512 // Size of a sector in bytes

// Struct for storing important information about the FS.
typedef struct _FileSystemInfo
{
	uint32_t fatOffset;
	uint32_t rootOffset;
	uint32_t rootSize;
	uint32_t dataOffset;
} FileSystemInfo;
typedef FileSystemInfo* pFileSystemInfo;

// Pointer to file system boot sector.
pBootSector bootSector;

// Info about the file system.
pFileSystemInfo fileSysInfo;

// File allocation table
uint8_t FAT[SECTOR_SIZE];


// Initialises the FAT12 file system.
void FsFat12_Initialise();

// Opens a file from the given directory.
FILE FsFat12_Open(const char* _fileName);

// Reads a file from disk and places its contents inside the given buffer.
unsigned int FsFat12_Read(PFILE _file, unsigned char* _buffer, unsigned int _length);

// Sets EOF flag of given file to 1.
void FsFat12_Close(PFILE _file);

// Open file or directory in root.
FILE FsFat12_OpenRoot(const char* _fileName);

// Open a file in a given sub directory.
FILE FsFat12_OpenSubDir(FILE _file, const char* _fileName);

// Print the information about the boot sector to the console.
void PrintBootSectorInfo();

// Converts a given file name, e.g. test1.txt to DOS8.3 standard: TEST1   TXT
void ConvertFileNameToDOS(const char* _source, char* _destination);

#endif