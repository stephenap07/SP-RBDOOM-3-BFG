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

#ifndef __RML_RMLRENDER_H_
#define __RML_RMLRENDER_H_

#include "RmlUi/Core/RenderInterface.h"

#include "ui/Rectangle.h"


class idRmlRender : public Rml::RenderInterface
{
public:

	enum
	{
		CURSOR_ARROW,
		CURSOR_HAND,
		CURSOR_HAND_JOY1,
		CURSOR_HAND_JOY2,
		CURSOR_HAND_JOY3,
		CURSOR_HAND_JOY4,
		CURSOR_COUNT
	};

	idRmlRender();

	~idRmlRender() override;

	void Init();

	/// Called by RmlUi when it wants to render geometry that the application does not wish to optimise. Note that
	/// RmlUi renders everything as triangles.
	void RenderGeometry( Rml::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rml::TextureHandle texture, const Rml::Vector2f& translation ) override;

	/// Called by RmlUi when it wants to enable or disable scissoring to clip content.
	void EnableScissorRegion( bool enable ) override;

	/// Called by RmlUi when it wants to change the scissor region.
	void SetScissorRegion( int x, int y, int width, int height ) override;

	/// Called by RmlUi when a texture is required by the library.
	bool LoadTexture( Rml::TextureHandle& texture_handle, Rml::Vector2i& texture_dimensions, const Rml::String& source );

	/// Called by RmlUi when a texture is required to be built from an internally-generated sequence of pixels.
	bool GenerateTexture( Rml::TextureHandle& texture_handle, const Rml::byte* source, const Rml::Vector2i& source_dimensions );

	/// Called by RmlUi when it wants the renderer to use a new transform matrix.
	/// This will only be called if 'transform' properties are encountered. If no transform applies to the current element, nullptr
	/// is submitted. Then it expects the renderer to use an identity matrix or otherwise omit the multiplication with the transform.
	/// @param[in] transform The new transform to apply, or nullptr if no transform applies to the current element.
	void SetTransform( const Rml::Matrix4f* transform ) override;

	void RenderClipMask();

	void PreRender();

	void PostRender();

	void DrawCursor( int x, int y, int w, int h );

	void DrawRect( const idRectangle& rect, const idVec4& color );

private:

	// Generates render state flags. Turns on stencil testing and functions.
	uint64				GenerateGlState() const;

	bool				_enableScissor;
	idRectangle			_clipRects;
	const idMaterial*	_cursorImages[CURSOR_COUNT];
	const idMaterial*	_guiSolid;

	int					_numMasks;
	idDrawVert*			_verts;
	triIndex_t*			_tris;
	idMat3				mat;
	idVec3				origin;

	int					_texGen;
	int					_numVerts;
	int					_numIndexes;
};

#endif