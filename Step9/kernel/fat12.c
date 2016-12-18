#include <fat12.h>

void FsFat12_Initialise()
{
	bootSector = (pBootSector)FloppyDriveReadSector(0);

	fileSysInfo->numSectors = bootSector->Bpb.NumSectors;
	fileSysInfo->fatOffset = 1;
	fileSysInfo->fatSize = bootSector->Bpb.SectorsPerFat;
	fileSysInfo->fatEntrySize = 8;
	fileSysInfo->numRootEntries = bootSector->Bpb.NumDirEntries;
	fileSysInfo->rootOffset = bootSector->Bpb.NumberOfFats * bootSector->Bpb.SectorsPerFat + 1;
	fileSysInfo->rootSize = (bootSector->Bpb.NumDirEntries * 32) / bootSector->Bpb.BytesPerSector;
	fileSysInfo->dataOffset = fileSysInfo->rootOffset + fileSysInfo->rootSize;

	for(unsigned int i = 0; i < fileSysInfo->fatEntrySize; i++)
	{
		unsigned int sector = fileSysInfo->fatOffset + i;
		unsigned char* buffer = (unsigned char*)FloppyDriveReadSector(sector);
		
		FAT[i * SECTOR_SIZE] = (int8_t)buffer;
	}

	ConsoleWriteString("\nRoot offset: ");
	ConsoleWriteInt(fileSysInfo->rootOffset, 10);
	ConsoleWriteString("\nRoot size: ");
	ConsoleWriteInt(fileSysInfo->rootSize, 10);
	ConsoleWriteString("\nData start: ");
	ConsoleWriteInt(fileSysInfo->dataOffset, 10);

	//FILE file = FsFat12_Open(".\\test1.txt");
	//if (file.Flags != FS_INVALID)
	//{
	//	ConsoleWriteString("\nFile name: ");
	//	ConsoleWriteString(file.Name);
	//	ConsoleWriteString("\nFile size (bytes): ");
	//	ConsoleWriteInt(file.FileLength, 10);
	//	ConsoleWriteString("\nFile cluster: ");
	//	ConsoleWriteInt(file.CurrentCluster, 10);
	//	ConsoleWriteString("\nFile data: ");
	//	char* dat = (char*)FloppyDriveReadSector(fileSysInfo->dataOffset + file.CurrentCluster + 3);
	//	for (int i = 0; i < file.FileLength; i++)
	//	{
	//		ConsoleWriteCharacter(dat[i]);
	//	}
	//}

	//PrintBootSectorInfo();
}

FILE FsFat12_Open(const char* _fileName)
{
	if (_fileName)
	{
		//ConsoleWriteString("\nOpening file");
		FILE file;
		char* path = (char*)_fileName;
		char* p;
		_Bool rootDir = true;

		p = strchr(path, '\\');
		if (!p)
		{
			//ConsoleWriteString("\nLooking in root for: ");
			//ConsoleWriteString(path);
			file = FsFat12_OpenRoot(path);

			if (file.Flags == FS_FILE)
			{
				return file;
			}
		}
		else
		{
			p++;

			while (p)
			{
				char pathName[16];
				int i;
				for (i = 0; i < 16; i++)
				{
					if (p[i] == '\\' || p[i] == 0)
					{
						break;
					}

					pathName[i] = p[i];
				}

				pathName[i] = 0;

				if (rootDir)
				{
					//ConsoleWriteString("\nOpening ");
					//ConsoleWriteString(pathName);
					//ConsoleWriteString(" in root.");
					file = FsFat12_OpenRoot(pathName);
					rootDir = false;
				}
				else
				{
					//ConsoleWriteString("\nOpening sub directory for ");
					//ConsoleWriteString(pathName);
					file = FsFat12_OpenSubDir(file, pathName);
				}

				if (file.Flags == FS_INVALID)
				{
					ConsoleWriteString("\nNo such file or directory at ");
					ConsoleWriteString((char*)_fileName);
					break;
				}

				if (file.Flags == FS_FILE)
				{
					return file;
				}

				p = strchr(p + 1, '\\');
				if (p)
				{
					p++;
				}
			}
		}
	}

	//ConsoleWriteString("\nFailed to load file.");
	FILE invalidFile;
	invalidFile.Flags = FS_INVALID;
	return invalidFile;
}


// Read buffer to store sector data for files. Supports only the first 4k of any files.
char readBuffer[4096];

unsigned int FsFat12_Read(PFILE _file, unsigned char* _buffer, unsigned int _length)
{
	// if length is 0 then likely trying to read a directory so just set it to read an entire sector.
	if (_length == 0) _length = SECTOR_SIZE;

	// clear the read buffer of any previous reads data.
	memset(readBuffer, 0, 4096);

	// Calculate how many sectors need to be read based on the value of length.
	unsigned int sectorsToRead = _length % SECTOR_SIZE == 0 ? 1 : _length / SECTOR_SIZE + 1;
	unsigned int bytesRead = 0;

	if (_file)
	{
		//ConsoleWriteString("\nReading file ");
		//ConsoleWriteString(_file->Name);
		//ConsoleWriteString("\n");

		//unsigned int physicalSector = fileSysInfo->rootOffset + fileSysInfo->rootSize + _file->CurrentCluster + 3;
		//unsigned char* sector = (unsigned char*)FloppyDriveReadSector(physicalSector);

		//memcpy(_buffer, sector, _length);
		//FsFat12_Close(_file);

		unsigned int physicalSector = fileSysInfo->dataOffset + _file->CurrentCluster + 3;
		unsigned char* buffer = (unsigned char*)FloppyDriveReadSector(physicalSector);

		memcpy(_buffer, buffer, _length);

		unsigned int fatOffset = _file->CurrentCluster;
		unsigned int fatSector = 1 + fatOffset / _length;
		unsigned int tableOffset = fatOffset % _length;

		buffer = (unsigned char*)FloppyDriveReadSector(fatSector);
		memcpy(FAT, buffer, _length);

		buffer = (unsigned char*)FloppyDriveReadSector(fatSector + 1);
		memcpy(FAT + _length, buffer, _length);

		uint16_t nextCluster = *(uint16_t*)&FAT[tableOffset];

		if (_file->CurrentCluster & 0x0001)
		{
			nextCluster >>= 4;
		}
		else
		{
			nextCluster &= 0x0FFF;
		}

		if (nextCluster >= 0xFF8 || nextCluster == 0)
		{
			FsFat12_Close(_file);
			return bytesRead;
		}

		_file->CurrentCluster = nextCluster;
	}

	return bytesRead;
}

void FsFat12_Close(PFILE _file)
{
	if (_file)
	{
		_file->Eof = 1;
	}
}

FILE FsFat12_OpenRoot(const char* _fileName)
{
	FILE file;

	if (_fileName)
	{
		pDirectoryEntry directory;

		char dosName[12];
		ConvertFileNameToDOS(_fileName, dosName);
		dosName[11] = 0;

		for (int sector = 0; sector < 14; sector++)
		{
			directory = (pDirectoryEntry)FloppyDriveReadSector(fileSysInfo->rootOffset + sector);

			for (int i = 0; i < 16; i++)
			{
				char name[12];
				memcpy(&name, directory->Filename, 11);
				name[11] = 0;

				if (strcmp(dosName, name) == 0)
				{
					strcpy(file.Name, _fileName);
					file.Id = 0;
					file.Eof = 0;
					file.Position = 0;
					file.CurrentCluster = directory->FirstCluster;
					file.FileLength = directory->FileSize;

					if (directory->Attrib == 0x10)
					{
						//ConsoleWriteString("\nFile attrib: directory");
						file.Flags = FS_DIRECTORY;
					}
					else
					{
						//ConsoleWriteString("\nFile attrib: file");
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

	while (!_file.Eof)
	{
		unsigned char buffer[SECTOR_SIZE];
		FsFat12_Read(&_file, buffer, file.FileLength);

		pDirectoryEntry subDir = (pDirectoryEntry)buffer;

		for (int i = 0; i < 16; i++)
		{
			char name[12];
			memcpy(name, subDir->Filename, 11);
			name[11] = 0;

			if (strcmp(name, dosName) == 0)
			{
				strcpy(file.Name, _fileName);
				file.Id = 0;
				file.Eof = 0;
				file.Position = 0;
				file.CurrentCluster = subDir->FirstCluster;
				file.FileLength = subDir->FileSize;

				if (subDir->Attrib == 0x10)
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
	ConsoleWriteInt((unsigned int)bootSector->Bpb.OEMName, 10);

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

void ConvertFileNameToDOS(const char* _source, char* _destination)
{
	if (!_destination || !_source)
	{
		return;
	}

	// Set every character in the destination to space DOS8.3 allows only 11 characters for file names.
	memset(_destination, ' ', 11);

	int i;
	for (i = 0; i < 8; i++)
	{
		// if the character at i is a "."(extension reached) 
		// then break to add the extension to the file name.
		if (_source[i] == 0 || _source[i] == '.')
		{
			break;
		}

		// Otherwise add the upper-case character to the file name.
		_destination[i] = toupper(_source[i]);
	}

	// If we reached the end of the source string then returns as there isn't an ext to add to the end.
	if (_source[i] == 0) return;

	// increment i if character wasnt null as we must have reached a '.'
	i++;

	// Copy over 3 characters of extension.
	for (int j = 0; j < 3; j++)
	{
		// Check the character isn't null
		if (_source[i + j])
		{
			_destination[8 + j] = toupper(_source[i + j]);
		}
	}
}