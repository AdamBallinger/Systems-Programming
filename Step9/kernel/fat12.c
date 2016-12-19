#include <fat12.h>

unsigned char data[SECTOR_SIZE];

void FsFat12_Initialise()
{
	// Read boot sector from floppy.
	bootSector = (pBootSector)FloppyDriveReadSector(0);

	fileSysInfo->numSectors = bootSector->Bpb.NumSectors;
	fileSysInfo->fatOffset = bootSector->Bpb.ReservedSectors;
	fileSysInfo->fatSize = bootSector->Bpb.SectorsPerFat;
	fileSysInfo->fatEntrySize = 8;
	fileSysInfo->numRootEntries = bootSector->Bpb.NumDirEntries;
	// Root offset = reserved sectors (including boot sector) + the number of sectors occupied by the FATs (both copies) - 1 (0 indexed)
	fileSysInfo->rootOffset = bootSector->Bpb.ReservedSectors + bootSector->Bpb.NumberOfFats * bootSector->Bpb.SectorsPerFat - 1;
	fileSysInfo->rootSize = (bootSector->Bpb.NumDirEntries * 32) / bootSector->Bpb.BytesPerSector;
	fileSysInfo->dataOffset = fileSysInfo->rootOffset + fileSysInfo->rootSize - 1;

	ConsoleWriteString("\nRoot offset: ");
	ConsoleWriteInt(fileSysInfo->rootOffset, 10);
	ConsoleWriteString("\nRoot size: ");
	ConsoleWriteInt(fileSysInfo->rootSize, 10);
	ConsoleWriteString("\nData start: ");
	ConsoleWriteInt(fileSysInfo->dataOffset, 10);
	ConsoleWriteString("\nReserved sectors: ");
	ConsoleWriteInt(bootSector->Bpb.ReservedSectors, 10);

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
		bool rootDir = true;

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

unsigned int FsFat12_Read(PFILE _file, unsigned char* _buffer, unsigned int _length)
{
	// if length is 0 then likely trying to read a directory so just set it to read an entire sector.
	if (_length == 0) _length = SECTOR_SIZE;

	// Set any previous data to 0.
	memset(data, 0, SECTOR_SIZE);

	if (_file)
	{
		// Calculate physical sector position on floppy disk where file data is stored.
		unsigned int physicalSector = fileSysInfo->dataOffset + _file->CurrentCluster;
		// Read file data to buffer.
		unsigned char* buffer = (unsigned char*)FloppyDriveReadSector(physicalSector);

		// Copy the read data into the buffer parameter.
		memcpy(_buffer, buffer, SECTOR_SIZE);

		unsigned int fatOffset = _file->CurrentCluster + (_file->CurrentCluster / 2);
		// Fat sector starts at sector 6 as there are 6 reserved sectors. 0 indexed
		unsigned int fatSector = fileSysInfo->fatOffset + (fatOffset / SECTOR_SIZE);
		// get offset of the sector in the FAT
		unsigned int tableOffset = fatOffset % SECTOR_SIZE;

		// Read the sector that contains the logical sector number for the current physical sector.
		buffer = (unsigned char*)FloppyDriveReadSector(fatSector);
		// Copy this data to the FAT.
		memcpy(FAT, buffer, SECTOR_SIZE);

		// each entry is 12 bits so the cluster need to be loaded into a 2 byte variable.
		uint16_t nextCluster = *(uint16_t*)&FAT[tableOffset];

		// Test if cluster entry in FAT is odd or even (test if bit 1 is set)
		if (_file->CurrentCluster & 0x0001)
		{
			// Higher 12 bits
			nextCluster = nextCluster >> 4;
		}
		else
		{
			// Lower 12 bits
			nextCluster = nextCluster & 0x0FFF;
		}

		// Check if next cluster is at the end of usable sectors or 0 (unused)
		if (nextCluster >= 0xFF8 || nextCluster == 0)
		{
			FsFat12_Close(_file);
			return 0;
		}

		// set file cluster to the next cluster in the chain.
		_file->CurrentCluster = nextCluster;
	}

	return 0;
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

		// Create the DOS 8.3 name for the file we are trying to open.
		char dosName[12];
		ConvertFileNameToDOS(_fileName, dosName);
		// Null terminate the name so it can be used in a strcmp
		dosName[11] = 0;

		// Read each sector in the root directory. (224 total root entries * 32 bytes per entry = 7168. 7168 / 512 (bytes per sector) = 14)
		for (int sector = 0; sector < fileSysInfo->rootSize; sector++)
		{
			directory = (pDirectoryEntry)FloppyDriveReadSector(fileSysInfo->rootOffset + sector);

			// Each sector has 16 entries. (DirectoryEntries are 32 bytes each so 512 (bytes per sector) / 32 = 16)
			for (int i = 0; i < 16; i++)
			{
				// Get the name for this directory.
				char name[12];
				memcpy(&name, directory->Filename, 11);
				// Null terminate it for use in a strcmp wih dos name.
				name[11] = 0;

				// If the 2 names match then the root contains the file we are looking for.
				if (strcmp(dosName, name) == 0)
				{
					// Populate the file struct
					strcpy(file.Name, _fileName);
					file.Id = 0;
					file.Eof = 0;
					file.Position = 0;
					file.CurrentCluster = directory->FirstCluster;
					file.FileLength = directory->FileSize;

					// Set the file flag basd on the directory attributes
					if (directory->Attrib == 0x10)
					{
						file.Flags = FS_DIRECTORY;
					}
					else
					{
						file.Flags = FS_FILE;
					}

					return file;
				}

				// Move to the next entry if the current isn't our file.
				directory++;
			}
		}
	}

	// If no file could be found return an invalid file.
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

	// Set every character in the destination to space DOS8.3 allows only 11 characters for file names that must be padded with spaces.
	memset(_destination, ' ', 11);

	// Add only first 8 characters of file name to the DOS name.
	unsigned int i;
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