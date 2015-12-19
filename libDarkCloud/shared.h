//////////////////////////////////////////////////////////////////////////
// shared.h
// Copyright(C) 2016  Luciano Ciccariello (Xeeynamo)
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

#pragma once
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

#define ISO_ALIGN	0x800

typedef signed char		s8;
typedef unsigned char   u8;
typedef signed short	s16;
typedef unsigned short  u16;
typedef signed int		s32;
typedef unsigned int    u32;

//! \brief check the extension of a file's string and compare it
//! \param[in] str file's string to check
//! \param[in] ext extension to verify the comparison
//! \return 0 if it's correct, else != 0
int CheckExtension(const char* str, const char* ext);

//! \brief get the file name without its extension
//! \param[out] dst file name without the extension
//! \param[in] len length of dst in order to avoid overflow
//! \param[in] src original file name with its extension
void GetFilenameWithoutExt(char* dst, size_t len, const char* src);

//! \brief calculate the size of specified file pointer
//! \param[in] file to analyze; must not be NULL
//! \return length of file in bytes
/** \details in order to calculate it, the current position of the file needs
* to be moved; the previous position will be restored after calculation, but
* buffered data will be flushed.
*/
int CalculateFileSize(FILE *file);

//! \brief copy a file into another
//! \param[in] fDst destination file
//! \param[in] fSrc source file
//! \param[in] length of data to copy in bytes
void FileCopy(FILE* fDst, FILE* fSrc, int length);

//! \brief align an integer number
//! \param[in] n integer to align
//! \param[in] align alignment value
//! \return aligned value
int Align(int n, int align);


//! \brief create a directory and all its tree
//! \param[in] path of the directory to create; specify a maximum of 260 chars
/** \details if the path specified is, for example, './foo/hello/world'
* but neither foo and hello directories was previously created, they will
* be created.
*/
void CreateSubDirs(const char* path);