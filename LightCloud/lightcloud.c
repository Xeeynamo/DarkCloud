#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <windows.h>

//////////////////////////////////////////////////////////////////////////
// Shared stuff
typedef signed char		s8;
typedef unsigned char   u8;
typedef signed short	s16;
typedef unsigned short  u16;
typedef signed int		s32;
typedef unsigned int    u32;

int CalculateFileSize(FILE *f)
{
	int prev, size;
	assert(f != NULL);
	prev = ftell(f);
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	fseek(f, prev, SEEK_SET);
	return size;
}
int Align(int n, int align)
{
	return (n % align) == 0 ? n :n + align - (n % align);
}

typedef struct LBA
{
	u32 pName;
	u32 dummy1;
	u32 dummy2;
	u32 dummy3;
	u32 pos;
	u32 size;
	u32 blockPos;
	u32 blockSize;
} LBA;

const char* GetLbaName(const LBA* lba, const void* data)
{
	return (const char*)data + lba->pName;
}

int main()
{
	FILE *fdat = fopen("data.dat", "wb");
	FILE *flba = fopen("data.hd2", "rb");

	if (fdat == 0 || flba == 0)
		return -1;
	int lbaSize = CalculateFileSize(flba);
	void *datalba = malloc(lbaSize);
	fread(datalba, lbaSize, 1, flba);
	LBA *lba = (LBA*)datalba;
	int pos = 0;
	for (int i = 0; i < 5425; i++)
	{
		const char* name = GetLbaName(lba + i, datalba);
		printf("Packing %s...", name);
		char path[MAX_PATH];
		sprintf(path, "export\\%s", GetLbaName(lba + i, datalba));
		FILE *f = fopen(path, "r+b");
		if (f)
		{
			lba[i].size = CalculateFileSize(f);
			lba[i].blockSize = lba[i].size / 0x800;
			lba[i].pos = pos * 0x800;
			lba[i].blockPos = pos;
			lba[i].blockSize = Align(lba[i].blockSize, 0x800);
			void *data = malloc(lba[i].blockSize * 0x800);
			memset(data, 0, lba[i].blockSize * 0x800);
			fread(data, lba[i].size, 1, f);
			fclose(f);
			fwrite(data, lba[i].blockSize * 0x800, 1, fdat);
			printf("Ok.\n");
			free(data);
			pos += lba[i].blockPos;
		}
		else
		{
			printf("Error!!!\n");
			Sleep(100);
		}
	}
	fseek(flba, 0, SEEK_SET);
	fwrite(datalba, lbaSize, 1, flba);
	fclose(fdat);
	fclose(flba);
	free(datalba);
}
