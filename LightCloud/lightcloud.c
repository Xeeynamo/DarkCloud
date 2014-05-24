//////////////////////////////////////////////////////////////////////////
// Light Cloud
// Dark Cloud's .DAT & .HD2 repacker
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

//! \brief align an integer number
//! \param[in] n integer to align
//! \param[in] align alignment value
//! \return aligned value
int Align(int n, int align)
{
	return (n % align) == 0 ? n : n + align - (n % align);
}


//////////////////////////////////////////////////////////////////////////
// Game specific

#define ISO_ALIGN	0x800

// file entry of the game
typedef struct HD2
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
} HD2;

//! \brief get a pointer to the file name of specified LBA entry
//! \param[in] lba to check
//! \param[in] data in memory of the entire .HD2 file
const char* GetLbaName(const HD2* lba, const void* data)
{
	assert(lba != NULL);
	assert(data != NULL);
	return (const char*)data + lba->pName;
}

//! \brief check if the specified LBA entry doesn't reefer to any real file
//! \param[in] lba to check
//! \return 1 if it's empty, 0 if it isn't
//! \details this is usually used to check the EOF of an HD2 file
int IsLbaEmtpy(const HD2* lba)
{
	assert(lba != NULL);
	return lba->pos == 0 && lba->size == 0 &&
		lba->blockPos == 0 && lba->blockSize == 0;
}

int Inject(HD2* lba, const void* lbadata, FILE* fDat, const char* szImport)
{
	FILE* fIn;
	char szFileName[MAX_PATH];
	const char* lbaFileName;

	assert(lba != NULL);
	assert(lbadata != NULL);
	assert(fDat != NULL);
	assert(szImport != NULL);

	lbaFileName = GetLbaName(lba, lbadata);
	strcpy(szFileName, szImport);
	strcat(szFileName, lbaFileName);
	fIn = fopen(szFileName, "rb");
	if (fIn == NULL)
	{
		printf("Unable to open %s\n", szFileName);
		return -1;
	}
	else
	{
		printf("\rInjecting... %64s", lbaFileName);

		lba->pos = Align(ftell(fDat), ISO_ALIGN);
		lba->size = CalculateFileSize(fIn);
		lba->blockPos = lba->pos / ISO_ALIGN;
		lba->blockSize = Align(lba->size, ISO_ALIGN) / ISO_ALIGN;
		fseek(fDat, lba->blockPos * ISO_ALIGN, SEEK_SET);
		FileCopy(fDat, fIn, lba->size);
		fclose(fIn);
	}
	return 0;
}
int Repack(const char* szFileDat, const char* szFileHd2, const char* szImport)
{
	FILE* fDat;
	FILE* fHd2;
	int nHd2Lenght;
	void* pHd2;
	HD2* lba;

	fDat = fopen(szFileDat, "wb");
	if (fDat == NULL)
	{
		printf("Unable to open %s\n", szFileDat);
		return -2;
	}
	fHd2 = fopen(szFileHd2, "r+b");
	if (fHd2 == NULL)
	{
		fclose(fDat);
		printf("Unable to open %s\n", szFileHd2);
		return -3;
	}

	// read .HD2 in memory
	nHd2Lenght = CalculateFileSize(fHd2);
	pHd2 = malloc(nHd2Lenght);
	assert(pHd2 != NULL);
	fread(pHd2, 1, nHd2Lenght, fHd2);

	// replace the original files
	lba = (HD2*)pHd2;
	while (IsLbaEmtpy(lba) == 0)
	{
		Inject(lba, pHd2, fDat, szImport);
		lba++;
	}

	// update .HD2 file
	fseek(fHd2, 0, SEEK_SET);
	fwrite(pHd2, 1, nHd2Lenght, fHd2);
	// release resources
	fclose(fHd2);
	free(pHd2);
	fclose(fDat);
	// return positive
	return 0;
}

int main(int argc, char *argv[])
{
	const char* DEFAULT_EXPORT_DIR = "./export/";

	printf("Light Cloud\n"
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
		return Repack(szFileDat, szFileHd2, DEFAULT_EXPORT_DIR);
	}
	case 3:
		return Repack(argv[1], argv[2], DEFAULT_EXPORT_DIR);
	default:
		printf("Usage:"
			"\tlightcloud <data>\n"
			"\tlightcloud <data.dat> <data.hd2>\n");
		return 1;
	}
	return 0;
}
