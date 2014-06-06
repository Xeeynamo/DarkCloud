//////////////////////////////////////////////////////////////////////////
// hd2.c
// Part of LightCloud, HD2 format for Dark Cloud's (TM) PS2 game
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

#include "hd2.h"

const char* Hd2GetLbaName(const HD2* lba, const void* data)
{
	assert(lba != NULL);
	assert(data != NULL);
	return (const char*)data + lba->pName;
}
int Hd2IsLbaEmtpy(const HD2* lba)
{
	assert(lba != NULL);
	return lba->pos == 0 && lba->size == 0 &&
		lba->blockPos == 0 && lba->blockSize == 0;
}

int Hd2ExtractFile(const HD2* lba, const void* lbadata, FILE* fDat, const char* szExport)
{
	FILE* fOut;
	char szFileName[MAX_PATH];
	const char* lbaFileName;

	assert(lba != NULL);
	assert(lbadata != NULL);
	assert(fDat != NULL);
	assert(szExport != NULL);

	lbaFileName = Hd2GetLbaName(lba, lbadata);
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

int Hd2ReinsertFile(HD2* lba, const void* lbadata, FILE* fDat, const char* szImport)
{
	FILE* fIn;
	char szFileName[MAX_PATH];
	const char* lbaFileName;

	assert(lba != NULL);
	assert(lbadata != NULL);
	assert(fDat != NULL);
	assert(szImport != NULL);

	lbaFileName = Hd2GetLbaName(lba, lbadata);
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

int Hd2Unpack(const char* szFileDat, const char* szFileHd2, const char* szExport)
{
	FILE* fDat;
	FILE* fHd2;
	int nHd2Lenght;
	void* pHd2;
	HD2* lba;

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

	lba = (HD2*)pHd2;
	while (Hd2IsLbaEmtpy(lba) == 0)
	{
		Hd2ExtractFile(lba, pHd2, fDat, szExport);
		lba++;
	}

	free(pHd2);
	fclose(fDat);
	return 0;
}

int Hd2Repack(const char* szFileDat, const char* szFileHd2, const char* szImport)
{
	// .DAT file that will be created
	FILE* fDat;
	// .HD2 file that will be used and updated
	FILE* fHd2;
	// length of HD2 file
	int nHd2Lenght;
	// pointer to the entire HD2 file in memory
	void* pHd2;
	// same as pHd2
	HD2* lba;

	// try to open the .HD2 file first
	fHd2 = fopen(szFileHd2, "r+b");
	if (fHd2 == NULL)
	{
		printf("Unable to open %s\n", szFileHd2);
		return -3;
	}

	// try to create the .DAT file
	fDat = fopen(szFileDat, "wb");
	if (fDat == NULL)
	{
		fclose(fHd2);
		printf("Unable to open %s\n", szFileDat);
		return -2;
	}

	// read .HD2 in memory
	nHd2Lenght = CalculateFileSize(fHd2);
	pHd2 = malloc(nHd2Lenght);
	assert(pHd2 != NULL);
	fread(pHd2, 1, nHd2Lenght, fHd2);

	// update the HD2 in memory and create the .DAT file
	lba = (HD2*)pHd2;
	while (Hd2IsLbaEmtpy(lba) == 0)
	{
		Hd2ReinsertFile(lba, pHd2, fDat, szImport);
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