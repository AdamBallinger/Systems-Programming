#include <fat12.h>

void FsFat12_Initialise()
{
	bootSector = (BootSector*)FloppyDriveReadSector(0);

	ConsoleWriteString("\nWorking Drive: ");
	ConsoleWriteInt(FloppyDriveGetWorkingDrive(), 10);
	
	ConsoleWriteString("\nOEM ID: ");
	ConsoleWriteInt(bootSector->Bpb.OEMName, 10);
	
	ConsoleWriteString("\nBytes per Sector: ");
	ConsoleWriteInt(bootSector->Bpb.BytesPerSector, 10);
	
	ConsoleWriteString("\nSectors per Cluster: ");
	ConsoleWriteInt(bootSector->Bpb.SectorsPerCluster, 10);
	
	ConsoleWriteString("\nReserved Sectors: ");
	ConsoleWriteInt(bootSector->Bpb.ReservedSectors, 10);
	
	ConsoleWriteString("\nNumber of FAT's: ");
	ConsoleWriteInt(bootSector->Bpb.NumberOfFats, 10);
	
	ConsoleWriteString("\nNumber of Dir Entries: ");
	ConsoleWriteInt(bootSector->Bpb.NumDirEntries, 10);
	
	ConsoleWriteString("\nNumber of Sectors: ");
	ConsoleWriteInt(bootSector->Bpb.NumSectors, 10);
	
	ConsoleWriteString("\nMedia: ");
	ConsoleWriteInt(bootSector->Bpb.Media, 16);
	
	ConsoleWriteString("\nSectors per FAT: ");
	ConsoleWriteInt(bootSector->Bpb.SectorsPerFat, 10);
	
	ConsoleWriteString("\nSectors per Track: ");
	ConsoleWriteInt(bootSector->Bpb.SectorsPerTrack, 10);
	
	ConsoleWriteString("\nHeads per Cyl: ");
	ConsoleWriteInt(bootSector->Bpb.HeadsPerCyl, 10);
	
	ConsoleWriteString("\nHidden Sectors: ");
	ConsoleWriteInt(bootSector->Bpb.HiddenSectors, 10);
	
	ConsoleWriteString("\nLong Sectors: ");
	ConsoleWriteInt(bootSector->Bpb.LongSectors, 10);
	
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