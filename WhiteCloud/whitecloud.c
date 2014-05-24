//////////////////////////////////////////////////////////////////////////
// White Cloud
// Dark Cloud's .DAT & .HD2 unpacker
// Copyright(C) 2014  Luciano Ciccariello (Xeeynamo)
// 
// This program is free software; you can redistribute it and / or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or(at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110 - 1301, USA.

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#ifndef MAX_PATH
#define MAX_PATH 260
#endif
#endif


//////////////////////////////////////////////////////////////////////////
// Shared stuff
typedef signed char		s8;
typedef unsigned char   u8;
typedef signed short	s16;
typedef unsigned short  u16;
typedef signed int		s32;
typedef unsigned int    u32;

//! \brief calculate the size of specified file pointer
//! \param[in] file to analyze; must not be NULL
//! \return length of file in bytes
/** \details in order to calculate it, the current position of the file needs
 * to be moved; the previous position will be restored after calculation, but
 * buffered data will be flushed.
 */
int CalculateFileSize(FILE *file)
{
	int prev, size;
	assert(file != NULL);
	// save the previous position
	prev = ftell(file);
	// go to the end of file
	fseek(file, 0, SEEK_END);
	// check the current position
	size = ftell(file);
	// set the previous position
	fseek(file, prev, SEEK_SET);
	// returns the calculated result
	return size;
}

//! \brief copy a file into another
//! \param[in] fDst destination file
//! \param[in] fSrc source file
//! \param[in] length of data to copy in bytes
void FileCopy(FILE* fDst, FILE* fSrc, int length)
{
#define BUFFER_SIZE 0x1000
	unsigned char buffer[BUFFER_SIZE];

	while (length > 0)
	{
		int toCopy = length > BUFFER_SIZE ? BUFFER_SIZE : length;
		length -= toCopy;
		fread(buffer, 1, toCopy, fSrc);
		fwrite(buffer, 1, toCopy, fDst);
	}
#undef BUFFER_SIZE
}

//! \brief create a directory and all its tree
//! \param[in] path of the directory to create; specify a maximum of 260 chars
/** \details if the path specified is, for example, './foo/hello/world'
 * but neither foo and hello directories was previously created, they will
 * be created.
 */
void CreateSubDirs(const char* path)
{
	// create a temporarily string to hold the directory to test and create
	char pDir[MAX_PATH];
	memset(&pDir, 0, MAX_PATH);

	// check if path is NULL
	if (path)
	{
		char* pCur = pDir;
		do
		{
			// store character
			*pCur = *path;
			// check for the directories' symbol separator
			if (*path == '\\' || *path == '/')
			{
				// if it's not a valid path (e.g. C:\, ./), do not value it
				char cPrev = *(path - 1);
				if (cPrev != ':' && cPrev != '.')
				{
#ifdef _WIN32
					// create directory; it will fail if it already exists
					CreateDirectory(pDir, 0);
#else
					struct stat st = {0};
					// check if the directory to create was already created
					if (stat(pDir, &st) == -1)
					{
						// create it
						mkdir(pDir, 0700);
					}
#endif
				}
			}
			pCur++;
		} while (*++path);
	}
}

//////////////////////////////////////////////////////////////////////////
// Game specific

// file entry of the game
typedef struct LBA
{
	// position on current file where the file name is found
	u32 pName;
	u32 dummy1;
	u32 dummy2;
	u32 dummy3;
	// position of file into .DAT (can't use blockPos * 0x800? *facepalm*)
	u32 pos;
	// real size of the file
	u32 size;
	// block position (ISO aligned) of file
	u32 blockPos;
	// how blocks the file fills
	u32 blockSize;
} LBA;

//! \brief get a pointer to the file name of specified LBA entry
//! \param[in] lba to check
//! \param[in] data in memory of the entire .HD2 file
const char* GetLbaName(const LBA* lba, const void* data)
{
	assert(lba != NULL);
	assert(data != NULL);
	return (const char*)data + lba->pName;
}

//! \brief check if the specified LBA entry doesn't reefer to any real file
//! \param[in] lba to check
//! \return 1 if it's empty, 0 if it isn't
//! \details this is usually used to check the EOF of an HD2 file
int IsLbaEmtpy(const LBA* lba)
{
	assert(lba != NULL);
	return lba->pos == 0 && lba->size == 0 &&
		lba->blockPos == 0 && lba->blockSize == 0;
}

int Extract(LBA* lba, const void* lbadata, FILE* fDat, const char* szExport)
{
	FILE* fOut;
	char szFileName[MAX_PATH];
	const char* lbaFileName;

	assert(lba != NULL);
	assert(lbadata != NULL);
	assert(fDat != NULL);
	assert(szExport != NULL);

	lbaFileName = GetLbaName(lba, lbadata);
	strcpy(szFileName, szExport);
	strcat(szFileName, lbaFileName);
	CreateSubDirs(szFileName);
	fOut = fopen(szFileName, "wb");
	if (fOut == NULL)
	{
		printf("Unable to extract %s\n", szFileName);
		return -1;
	}
	else
	{
		printf("\rUnpacking... %64s", lbaFileName);
		fseek(fDat, lba->pos, SEEK_SET);
		FileCopy(fOut, fDat, lba->size);
		fclose(fOut);
	}
	return 0;
}
int Unpack(const char* szFileDat, const char* szFileHd2, const char* szExport)
{
	FILE* fDat;
	FILE* fHd2;
	int nHd2Lenght;
	void* pHd2;
	LBA* lba;

	fDat = fopen(szFileDat, "rb");
	if (fDat == NULL)
	{
		printf("Unable to open %s\n", szFileDat);
		return -2;
	}
	fHd2 = fopen(szFileHd2, "rb");
	if (fHd2 == NULL)
	{
		fclose(fDat);
		printf("Unable to open %s\n", szFileHd2);
		return -3;
	}

	nHd2Lenght = CalculateFileSize(fHd2);
	pHd2 = malloc(nHd2Lenght);
	assert(pHd2 != NULL);
	fread(pHd2, 1, nHd2Lenght, fHd2);
	fclose(fHd2);

	lba = (LBA*)pHd2;
	while (IsLbaEmtpy(lba) == 0)
	{
		Extract(lba, pHd2, fDat, szExport);
		lba++;
	}

	free(fHd2);
	fclose(fDat);
	return 0;
}

int main(int argc, char *argv[])
{
	const char* DEFAULT_EXPORT_DIR = "./export/";

	printf("White Cloud\n"
		"Developed by Luciano Ciccariello (Xeeynamo)\n\n");
	switch (argc)
	{
	case 2:
	{
		char szFileDat[MAX_PATH];
		char szFileHd2[MAX_PATH];
		strcpy(szFileDat, argv[1]);
		strcat(szFileDat, ".DAT");
		strcpy(szFileHd2, argv[1]);
		strcat(szFileHd2, ".HD2");
		return Unpack(szFileDat, szFileHd2, DEFAULT_EXPORT_DIR);
	}
	case 3:
		return Unpack(argv[1], argv[2], DEFAULT_EXPORT_DIR);
	default:
		printf("Usage:"
			"\twhitecloud <data>\n"
			"\twhitecloud <data.dat> <data.hd2>\n");
		return 1;
	}
	return 0;
}
