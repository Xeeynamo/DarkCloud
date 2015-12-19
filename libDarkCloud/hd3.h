//////////////////////////////////////////////////////////////////////////
// hd3.h
// Part of LightCloud, HD2 format for Dark Cloud's (TM) PS2 game
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
#include "hd2.h"

// HD3 file entry
typedef struct HD3
{
	// position on current file where the file name is found
	u32 pName;
	// real size of the file
	u32 size;
	// block position (ISO aligned) of file
	u32 blockPos;
	// how blocks the file fills
	u32 blockSize;
} HD3;

//! \brief get a pointer to the file name of specified LBA entry
//! \param[in] lba to check
//! \param[in] data in memory of the entire .HD3 file
const char* Hd3GetLbaName(const HD3* lba, const void* data);

//! \brief check if the specified LBA entry doesn't reefer to any real file
//! \param[in] lba to check
//! \return 1 if it's empty, 0 if it isn't
//! \details this is usually used to check the EOF of an HD2 file
int Hd3IsLbaEmtpy(const HD3* lba);

//! \brief repack an HD3 file
//! \param[in] szFileDat file name of the .DAT file to create
//! \param[in] szFileHd3 file name of the original .HD3 file
//! \param[in] szImport folder where the files are located
//! \return 0 is positive, -3 szFileHd3 not opened, -2 szFileDat not created
int Hd3Unpack(const char* szFileDat, const char* szFileHd3, const char* szImport);

//! \brief repack an HD3 file
//! \param[in] szFileDat file name of the .DAT file to create
//! \param[in] szFileHd3 file name of the original .HD3 file
//! \param[in] szImport folder where the files are located
//! \return 0 is positive, -3 szFileHd3 not opened, -2 szFileDat not created
int Hd3Repack(const char* szFileDat, const char* szFileHd3, const char* szImport);