//////////////////////////////////////////////////////////////////////////
// shared.c
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

#include "shared.h"

inline size_t _GetExtensionPosition(const char* str)
{
	const char* tmp = str + strlen(str);
	while (tmp > str)
		if (*tmp == '.')
			return tmp - str;
		else
			tmp--;
	return 0;
}

int CheckExtension(const char* str, const char* ext)
{
	int extPos = _GetExtensionPosition(str);
	if (extPos == 0)
		return -1;
	return strcmp(str + extPos + 1, ext);
}

void GetFilenameWithoutExt(char* dst, size_t len, const char* src)
{
	int extPos, srcLen;
	extPos = _GetExtensionPosition(src);
	srcLen = strlen(src);
	if (srcLen - extPos <= len)
	{
		memcpy(dst, src, srcLen - extPos);
		dst[extPos] = '\0';
	}
	else
	{
		*dst = '\0';
	}
}

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

int Align(int n, int align)
{
	return (n % align) == 0 ? n : n + align - (n % align);
}

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
					struct stat st = { 0 };
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