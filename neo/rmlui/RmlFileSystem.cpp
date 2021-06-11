/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2021 Stephen Pridham

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#pragma hdrstop
#include "precompiled.h"

#include "RmlFileSystem.h"

/*
===============
RmlFileSystem

File system for RML
===============
*/


RmlFileSystem::RmlFileSystem()
{
}

RmlFileSystem::~RmlFileSystem()
{
}

Rml::FileHandle RmlFileSystem::Open( const Rml::String& path )
{
	idFile* file = fileSystem->OpenFileRead( path.c_str() );

	return Rml::FileHandle( file );
}

void RmlFileSystem::Close( Rml::FileHandle file )
{
	if( file )
	{
		fileSystem->CloseFile( reinterpret_cast<idFile*>( file ) );
	}
}

size_t RmlFileSystem::Read( void* buffer, size_t size, Rml::FileHandle file )
{
	idFile* theFile = reinterpret_cast<idFile*>( file );
	if( !theFile )
	{
		return 0;
	}

	return theFile->Read( buffer, size );
}

bool RmlFileSystem::Seek( Rml::FileHandle file, long offset, int origin )
{
	idFile* theFile = reinterpret_cast<idFile*>( file );
	if( !theFile )
	{
		return false;
	}

	return theFile->Seek( offset, ( fsOrigin_t )origin );
}

size_t RmlFileSystem::Tell( Rml::FileHandle file )
{
	idFile* theFile = reinterpret_cast<idFile*>( file );
	if( !theFile )
	{
		return 0;
	}

	return theFile->Tell();
}

size_t RmlFileSystem::Length( Rml::FileHandle file )
{
	idFile* theFile = reinterpret_cast<idFile*>( file );
	if( !theFile )
	{
		return 0;
	}

	return theFile->Length();
}

bool RmlFileSystem::LoadFile( const Rml::String& path, Rml::String& out_data )
{
	idFile* theFile = fileSystem->OpenFileRead( path.c_str() );
	if( !theFile )
	{
		return false;
	}

	int len = theFile->Length();
	out_data.resize( len );
	int ret = theFile->Read( &out_data[0], len );

	return ret != 0;
}