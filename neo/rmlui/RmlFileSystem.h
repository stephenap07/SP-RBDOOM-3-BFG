/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2012 Robert Beckebans

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

#ifndef __RML_FILE_SYSTEM_H_
#define __RML_FILE_SYSTEM_H_

#include "RmlUi/Core/FileInterface.h"

class RmlFileSystem : public Rml::FileInterface
{
public:
	RmlFileSystem();
	~RmlFileSystem() override;

	/// Opens a file.
	/// @param path The path to the file to open.
	/// @return A valid file handle, or nullptr on failure
	Rml::FileHandle Open( const Rml::String& path ) override;

	/// Closes a previously opened file.
	/// @param file The file handle previously opened through Open().
	void Close( Rml::FileHandle file ) override;

	/// Reads data from a previously opened file.
	/// @param buffer The buffer to be read into.
	/// @param size The number of bytes to read into the buffer.
	/// @param file The handle of the file.
	/// @return The total number of bytes read into the buffer.
	size_t Read( void* buffer, size_t size, Rml::FileHandle file ) override;

	/// Seeks to a point in a previously opened file.
	/// @param file The handle of the file to seek.
	/// @param offset The number of bytes to seek.
	/// @param origin One of either SEEK_SET (seek from the beginning of the file), SEEK_END (seek from the end of the file) or SEEK_CUR (seek from the current file position).
	/// @return True if the operation completed successfully, false otherwise.
	bool Seek( Rml::FileHandle file, long offset, int origin ) override;

	/// Returns the current position of the file pointer.
	/// @param file The handle of the file to be queried.
	/// @return The number of bytes from the origin of the file.
	size_t Tell( Rml::FileHandle file ) override;

	/// Returns the length of the file.
	/// The default implementation uses Seek & Tell.
	/// @param file The handle of the file to be queried.
	/// @return The length of the file in bytes.
	size_t Length( Rml::FileHandle file ) override;

	/// Load and return a file.
	/// @param path The path to the file to load.
	/// @param out_data The string contents of the file.
	/// @return True on success.
	bool LoadFile( const Rml::String& path, Rml::String& out_data ) override;
};

#endif