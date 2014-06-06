//////////////////////////////////////////////////////////////////////////
// hd2.h
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

#pragma once
#include "shared.h"

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
const char* Hd2GetLbaName(const HD2* lba, const void* data);

//! \brief check if the specified LBA entry doesn't reefer to any real file
//! \param[in] lba to check
//! \return 1 if it's empty, 0 if it isn't
//! \details this is usually used to check the EOF of an HD2 file
int Hd2IsLbaEmtpy(const HD2* lba);

//! \brief repack an HD2 file
//! \param[in] szFileDat file name of the .DAT file to create
//! \param[in] szFileHd2 file name of the original .HD2 file
//! \param[in] szImport folder where the files are located
//! \return 0 is positive, -3 szFileHd2 not opened, -2 szFileDat not created
int Hd2Repack(const char* szFileDat, const char* szFileHd2, const char* szImport);