#include <fat12.h>

void FsFat12_Initialise()
{
	ConsoleWriteString("Working drive: ");
	ConsoleWriteInt(FloppyDriveGetWorkingDrive(), 10);
	ConsoleWriteString("\n");
}

FILE FsFat12_Open(const char* _fileName)
{
	
}

unsigned int FsFat12_Read(PFILE _file, unsigned char* _buffer, unsigned int _length)
{
	
	return 0;
}

void FsFat12_Close(PFILE _file)
{
	_file->Eof = 1;
}