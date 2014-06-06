//////////////////////////////////////////////////////////////////////////
// hd3.c
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

#include "hd3.h"

const char* Hd3GetLbaName(const HD3* lba, const void* data)
{
	assert(lba != NULL);
	assert(data != NULL);
	return (const char*)data + lba->pName;
}
int Hd3IsLbaEmtpy(const HD3* lba)
{
	assert(lba != NULL);
	return lba->size == 0 &&
		lba->blockPos == 0 && lba->blockSize == 0;
}

void Hd2toHd3(HD3* hd3, const HD2* hd2)
{
	hd3->pName = hd2->pName;
	hd3->size = hd2->size;
	hd3->blockPos = hd2->blockPos;
	hd3->blockSize = hd2->blockSize;
}
void Hd3toHd2(HD2* hd2, const HD3* hd3)
{
	hd2->pName = hd3->pName;
	hd2->dummy1 = 0;
	hd2->dummy2 = 0;
	hd2->dummy3 = 0;
	hd2->pos = hd3->blockPos * ISO_ALIGN;
	hd2->size = hd3->size;
	hd2->blockPos = hd3->blockPos;
	hd2->blockSize = hd3->blockSize;
}

int Hd3Unpack(const char* szFileDat, const char* szFileHd3, const char* szExport)
{
	FILE* fDat;
	FILE* fHd3;
	int nHd3Lenght;
	void* pHd3;
	HD3* lba;

	fDat = fopen(szFileDat, "rb");
	if (fDat == NULL)
	{
		printf("Unable to open %s\n", szFileDat);
		return -2;
	}
	fHd3 = fopen(szFileHd3, "rb");
	if (fHd3 == NULL)
	{
		fclose(fDat);
		printf("Unable to open %s\n", szFileHd3);
		return -3;
	}

	nHd3Lenght = CalculateFileSize(fHd3);
	pHd3 = malloc(nHd3Lenght);
	assert(pHd3 != NULL);
	fread(pHd3, 1, nHd3Lenght, fHd3);
	fclose(fHd3);

	lba = (HD3*)pHd3;
	while (Hd3IsLbaEmtpy(lba) == 0)
	{
		HD2 hd2;
		Hd3toHd2(&hd2, lba);
		Hd2ExtractFile(&hd2, pHd3, fDat, szExport);
		Hd2toHd3(lba, &hd2);
		lba++;
	}

	free(pHd3);
	fclose(fDat);
	return 0;
}

int Hd3Repack(const char* szFileDat, const char* szFileHd3, const char* szImport)
{
	// .DAT file that will be created
	FILE* fDat;
	// .HD2 file that will be used and updated
	FILE* fHd3;
	// length of HD3 file
	int nHd3Lenght;
	// pointer to the entire HD3 file in memory
	void* pHd3;
	// same as pHd3
	HD3* lba;

	// try to open the .HD3 file first
	fHd3 = fopen(szFileHd3, "r+b");
	if (fHd3 == NULL)
	{
		printf("Unable to open %s\n", szFileHd3);
		return -3;
	}

	// try to create the .DAT file
	fDat = fopen(szFileDat, "wb");
	if (fDat == NULL)
	{
		fclose(fHd3);
		printf("Unable to open %s\n", szFileDat);
		return -2;
	}

	// read .HD3 in memory
	nHd3Lenght = CalculateFileSize(fHd3);
	pHd3 = malloc(nHd3Lenght);
	assert(pHd3 != NULL);
	fread(pHd3, 1, nHd3Lenght, fHd3);

	// update the HD3 in memory and create the .DAT file
	lba = (HD3*)pHd3;
	while (Hd3IsLbaEmtpy(lba) == 0)
	{
		HD2 hd2;
		Hd3toHd2(&hd2, lba);
		Hd2ReinsertFile(&hd2, pHd3, fDat, szImport);
		Hd2toHd3(lba, &hd2);
		lba++;
	}

	// update .HD2 file
	fseek(fHd3, 0, SEEK_SET);
	fwrite(pHd3, 1, nHd3Lenght, fHd3);

	// release resources
	fclose(fHd3);
	free(pHd3);
	fclose(fDat);

	// return positive
	return 0;
}