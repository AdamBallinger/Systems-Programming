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
	
	FILE file = FsFat12_Open("\\\\folder\\test1.txt");
	if(file.Flags != FS_INVALID)
	{
		ConsoleWriteString("\nFile name: ");
		ConsoleWriteString(file.Name);
		ConsoleWriteString("\nFile size (bytes): ");
		ConsoleWriteInt(file.FileLength, 10);
		ConsoleWriteString("\nFile Cluster: ");
		ConsoleWriteInt(file.CurrentCluster, 10);
	}
	
	//PrintBootSectorInfo();
}

FILE FsFat12_Open(const char* _fileName)
{
	if(_fileName)
	{
		ConsoleWriteString("\nOpening file");
		FILE file;
		char* path = (char*) _fileName;
		char* p;
		bool rootDir = true;
		
		p = strchr(path, '\\');
		if(!p)
		{
			ConsoleWriteString("\nLooking in root for: ");
			ConsoleWriteString(path);
			file = FsFat12_OpenRoot(path);
			
			if(file.Flags == FS_FILE)
			{
				ConsoleWriteString("\nFound file.");
				return file;
			}
			
			ConsoleWriteString("\nFailed to load file.");
			FILE invalidFile;
			invalidFile.Flags = FS_INVALID;
			return invalidFile;
		}
		
		p++;
		
		while(p)
		{
			char pathName[16];
			int i;
			for(i = 0; i < 16; i++)
			{
				if(p[i] == '\\' || p[i] == 0)
				{
					break;
				}	
				
				pathName[i] = p[i];
			}
			
			pathName[i] = 0;
			
			if(rootDir)
			{
				ConsoleWriteString("\nOpening ");
				ConsoleWriteString(pathName);
				ConsoleWriteString(" in root.");
				file = FsFat12_OpenRoot(pathName);
				rootDir = false;
			}
			else
			{
				ConsoleWriteString("\nSearching sub directory ");
				ConsoleWriteString(file.Name);
				file = FsFat12_OpenSubDir(file, pathName);
			}
			
			if(file.Flags == FS_INVALID)
			{
				ConsoleWriteString("\nNo such file or directory.");
				break;
			}
			
			if(file.Flags == FS_FILE)
			{
				return file;
			}
			
			p = strchr(p + 1, '\\');
			if(p)
			{
				p++;
			}
		}
	}
	
	ConsoleWriteString("\nFailed to load file.");
	FILE invalidFile;
	invalidFile.Flags = FS_INVALID;
	return invalidFile;
}

unsigned int FsFat12_Read(PFILE _file, unsigned char* _buffer, unsigned int _length)
{
	if(_file)
	{
		unsigned int physicalSector = 33 + (_file->CurrentCluster - 2);
		unsigned char* sector = (unsigned char*) FloppyDriveReadSector(physicalSector);
		
		memcpy(_buffer, sector, _length);
		
		unsigned int fatOffset = _file->CurrentCluster + (_file->CurrentCluster / 2);
		unsigned int fatSector = 1 + (fatOffset / SECTOR_SIZE);
		unsigned int entryOffset = fatOffset % SECTOR_SIZE;
		
		sector = (unsigned char*) FloppyDriveReadSector(fatSector);
		memcpy(FAT, sector, SECTOR_SIZE);
		
		sector = (unsigned char*) FloppyDriveReadSector(fatSector + 1);
		memcpy(FAT + SECTOR_SIZE, sector, SECTOR_SIZE);
		
		uint16_t nextCluster = *(uint16_t*) &FAT[entryOffset];
		
		if(_file->CurrentCluster & 0x0001)
		{
			nextCluster >>= 4;
		}
		else
		{
			nextCluster &= 0x0FFF;
		}
		
		if(nextCluster >= 0xFF8)
		{
			_file->Eof = 1;
			return 0;
		}
		
		if(nextCluster == 0)
		{
			_file->Eof = 1;
			return 0;
		}
		
		_file->CurrentCluster = nextCluster;
	}
	
	return SECTOR_SIZE;
}

void FsFat12_Close(PFILE _file)
{
	if(_file)
	{
		_file->Eof = 1;
	}
}

FILE FsFat12_OpenRoot(const char* _fileName)
{	
	FILE file;

	if(_fileName)
	{
		pDirectoryEntry directory;
		
		char dosName[12];
		ConvertFileNameToDOS(_fileName, dosName);
		dosName[11] = 0;

		for(int sector = 0; sector < 14; sector++)
		{
			directory = (pDirectoryEntry) FloppyDriveReadSector(fileSysInfo->rootOffset + sector);
			
			for(int i = 0; i < 16; i++)
			{
				char name[12];
				memcpy(&name, directory->Filename, 11);
				name[11] = 0;
				
				ConsoleWriteString(dosName);
				ConsoleWriteString(",");

				if(strcmp(dosName, name) == 0)
				{
					strcpy(file.Name, _fileName);
					file.Id = 0;
					file.Eof = 0;
					file.Position = 0;
					file.CurrentCluster = directory->FirstCluster;
					file.FileLength = directory->FileSize;
					
					if(directory->Attrib == 0x10)
					{
						file.Flags = FS_DIRECTORY;
					}
					else
					{
						ConsoleWriteString("\n erm wut");
						file.Flags = FS_FILE;
					}
					
					return file;
				}
				
				directory++;
			}
		}
	}
	
	file.Flags = FS_INVALID;
	return file;
}

FILE FsFat12_OpenSubDir(FILE _file, const char* _fileName)
{
	FILE file;
	
	char dosName[12];
	ConvertFileNameToDOS(_fileName, dosName);
	dosName[11] = 0;
	
	while(!_file.Eof)
	{
		unsigned char buffer[SECTOR_SIZE];
		FsFat12_Read(&file, buffer, SECTOR_SIZE);
		
		pDirectoryEntry subDir = (pDirectoryEntry) buffer;
		
		for(int i = 0; i < 16; i++)
		{
			char name[12];
			memcpy(name, subDir->Filename, 11);
			name[11] = 0;
			
			if(strcmp(name, dosName) == 0)
			{
				strcpy(file.Name, _fileName);
				file.Id = 0;
				file.Eof = 0;
				file.Position = 0;
				file.CurrentCluster = subDir->FirstCluster;
				file.FileLength = subDir->FileSize;
				
				if(subDir->Attrib == 0x10)
				{
					file.Flags = FS_DIRECTORY;
				}
				else
				{
					file.Flags = FS_FILE;
				}
				
				return file;
			}
			
			subDir++;
		}
	}
	
	file.Flags = FS_INVALID;
	return file;
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

	ConsoleWriteString("\t\t\tNumber of FATS: ");
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

void ConvertFileNameToDOS(const char* source, char* destination)
{	
	if(!destination || !source)
	{
		return;
	}
		
	// Set every character in the destination to space DOS8.3 allows only 11 characters for file names.
	memset(destination, ' ', 11);

	for(int i = 0; i < 8; i++)
	{
		// if the character at i is a "."(extension reached) 
		// then break to add the extension to the file name.
		if(source[i] == '.')
		{
			break;
		}
		
		// Otherwise add the upper-case character to the file name.
		destination[i] = toupper(source[i]);
	}

	// Copy over 3 characters of extension.
	for(int j = 0; j < 3; j++)
	{
		if(source[8 + j])
		{
			destination[8 + j] = source[8 + j];
		}
	}
	 
	// Make extension upper-case.
	for(int i = 0; i < 3; i++)
	{
		destination[8 + i] = toupper(destination[8 + i]);
	}
}