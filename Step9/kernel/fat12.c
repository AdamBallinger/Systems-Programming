#include <fat12.h>

void FsFat12_Initialise()
{
	bootSector = (BootSector*)FloppyDriveReadSector(0);
	
	fileSysInfo->numSectors = bootSector->Bpb.NumSectors;
	fileSysInfo->fatOffset = 1;
	fileSysInfo->fatSize = bootSector->Bpb.SectorsPerFat;
	fileSysInfo->fatEntrySize = 8;
	fileSysInfo->numRootEntries = bootSector->Bpb.NumDirEntries;
	fileSysInfo->rootOffset = (bootSector->Bpb.NumberOfFats * bootSector->Bpb.SectorsPerFat) + 1;
	fileSysInfo->rootSize = (bootSector->Bpb.NumDirEntries * 32) / bootSector->Bpb.BytesPerSector;
	
	// 14 sectors
	for(int i = 0; i < 14; i++)
	{
		DirectoryEntry* directory = (DirectoryEntry*)FloppyDriveReadSector(fileSysInfo->rootOffset + i);
		for(int j = 0; j < 16; j++)
		{
			ConsoleWriteString(" , ");
			ConsoleWriteString(directory->Filename);
			directory++;
		}
	}
	
	//PrintBootSectorInfo();
}

FILE FsFat12_Open(const char* _fileName)
{
	if(_fileName)
	{
		
	}
	
	FILE invalidFile;
	invalidFile.Flags = FS_INVALID;
	return invalidFile;
}

unsigned int FsFat12_Read(PFILE _file, unsigned char* _buffer, unsigned int _length)
{
	
	return 0;
}

void FsFat12_Close(PFILE _file)
{
	if(_file)
	{
		_file->Eof = 1;
	}
}

void PrintBootSectorInfo()
{
	ConsoleWriteString("\nWorking Drive: ");
	ConsoleWriteInt(FloppyDriveGetWorkingDrive(), 10);
	
	ConsoleWriteString("\t\t\tOEM ID: ");
	ConsoleWriteInt(bootSector->Bpb.OEMName, 10);
	
	ConsoleWriteString("\nBytes per Sector: ");
	ConsoleWriteInt(bootSector->Bpb.BytesPerSector, 10);
	
	ConsoleWriteString("\t\t\tSectors per Cluster: ");
	ConsoleWriteInt(bootSector->Bpb.SectorsPerCluster, 10);
	
	ConsoleWriteString("\nReserved Sectors: ");
	ConsoleWriteInt(bootSector->Bpb.ReservedSectors, 10);

	ConsoleWriteString("\t\t\tNumber of FAT's: ");
	ConsoleWriteInt(bootSector->Bpb.NumberOfFats, 10);
	
	ConsoleWriteString("\nNumber of Dir Entries: ");
	ConsoleWriteInt(bootSector->Bpb.NumDirEntries, 10);
	
	ConsoleWriteString("\t\tNumber of Sectors: ");
	ConsoleWriteInt(bootSector->Bpb.NumSectors, 10);
	
	ConsoleWriteString("\nMedia: ");
	ConsoleWriteInt(bootSector->Bpb.Media, 16);
	
	ConsoleWriteString("\t\t\t\tSectors per FAT: ");
	ConsoleWriteInt(bootSector->Bpb.SectorsPerFat, 10);
	
	ConsoleWriteString("\nSectors per Track: ");
	ConsoleWriteInt(bootSector->Bpb.SectorsPerTrack, 10);
	
	ConsoleWriteString("\t\t\tHeads per Cyl: ");
	ConsoleWriteInt(bootSector->Bpb.HeadsPerCyl, 10);
	
	ConsoleWriteString("\nHidden Sectors: ");
	ConsoleWriteInt(bootSector->Bpb.HiddenSectors, 10);

	ConsoleWriteString("\t\t\tLong Sectors: ");
	ConsoleWriteInt(bootSector->Bpb.LongSectors, 10);
	
	ConsoleWriteString("\n");
}