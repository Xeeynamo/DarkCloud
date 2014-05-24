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
void CreateSubDirs(const char* path)
{
	char newDirName[MAX_PATH];
	memset(&newDirName, 0, MAX_PATH);
	if (path)
	{
		char *pNewDirName = newDirName;
		do
		{
			*pNewDirName = *path;
			if (*path == '\\' || *path == '/')
			{
				if (*(path - 1) != ':')
				{
					CreateDirectory(newDirName, 0);
				}
			}
			pNewDirName++;
		} while (*++path);
	}
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
	FILE *fdat = fopen("data.dat", "rb");
	FILE *flba = fopen("data.hd2", "rb");

	if (fdat == 0 || flba == 0)
		return -1;
	int lbaSize = CalculateFileSize(flba);
	void *datalba = malloc(lbaSize);
	fread(datalba, lbaSize, 1, flba);
	LBA *lba = (LBA*)datalba;
	for (int i = 0; i < 5425; i++)
	{
		const char *name = GetLbaName(lba + i, datalba);
		printf("Extracting %s...", name);
		fseek(fdat, lba[i].pos, SEEK_SET);
		void *data = malloc(lba[i].size);
		fread(data, lba[i].size, 1, fdat);
		char path[MAX_PATH];
		sprintf(path, "export\\%s", GetLbaName(lba + i, datalba));
		CreateSubDirs(path);
		FILE *f = fopen(path, "w+b");
		if (f)
		{
			fwrite(data, lba[i].size, 1, f);
			fclose(f);
			printf("Ok.\n");
		}
		else
		{
			printf("Error!!!\n");
			Sleep(100);
		}
		free(data);
	}
	free(datalba);
}
