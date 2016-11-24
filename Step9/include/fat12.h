#ifndef FAT12_H
#define FAT12_H

#include <floppydisk.h>
#include <filesystem.h>
#include <bpb.h>
#include <console.h>

BootSector* bootSector;

// Initialises the FAT12 file system.
void FsFat12_Initialise();

// Opens a file from the given directory.
FILE FsFat12_Open(const char* _fileName);

// Reads a file from disk, returning number of read bytes and storing the contents
// in given buffer.
unsigned int FsFat12_Read(PFILE _file, unsigned char* _buffer, unsigned int _length);

// Sets EOF flag of given file to 1.
void FsFat12_Close(PFILE _file);

#endif