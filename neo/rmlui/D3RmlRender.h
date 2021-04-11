/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.

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
	/// @param[in] vertices The geometry's vertex data.
	/// @param[in] num_vertices The number of vertices passed to the function.
	/// @param[in] indices The geometry's index data.
	/// @param[in] num_indices The number of indices passed to the function. This will always be a multiple of three.
	/// @param[in] texture The texture to be applied to the geometry. This may be nullptr, in which case the geometry is untextured.
	/// @param[in] translation The translation to apply to the geometry.
	void RenderGeometry( Rml::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rml::TextureHandle texture, const Rml::Vector2f& translation ) override;

	/// Called by RmlUi when it wants to enable or disable scissoring to clip content.
	/// @param[in] enable True if scissoring is to enabled, false if it is to be disabled.
	void EnableScissorRegion( bool enable ) override;

	/// Called by RmlUi when it wants to change the scissor region.
	/// @param[in] x The left-most pixel to be rendered. All pixels to the left of this should be clipped.
	/// @param[in] y The top-most pixel to be rendered. All pixels to the top of this should be clipped.
	/// @param[in] width The width of the scissored region. All pixels to the right of (x + width) should be clipped.
	/// @param[in] height The height of the scissored region. All pixels to below (y + height) should be clipped.
	void SetScissorRegion( int x, int y, int width, int height ) override;

	/// Called by RmlUi when a texture is required by the library.
	/// @param[out] texture_handle The handle to write the texture handle for the loaded texture to.
	/// @param[out] texture_dimensions The variable to write the dimensions of the loaded texture.
	/// @param[in] source The application-defined image source, joined with the path of the referencing document.
	/// @return True if the load attempt succeeded and the handle and dimensions are valid, false if not.
	bool LoadTexture( Rml::TextureHandle& texture_handle, Rml::Vector2i& texture_dimensions, const Rml::String& source );

	/// Called by RmlUi when a texture is required to be built from an internally-generated sequence of pixels.
	/// @param[out] texture_handle The handle to write the texture handle for the generated texture to.
	/// @param[in] source The raw 8-bit texture data. Each pixel is made up of four 8-bit values, indicating red, green, blue and alpha in that order.
	/// @param[in] source_dimensions The dimensions, in pixels, of the source data.
	/// @return True if the texture generation succeeded and the handle is valid, false if not.
	bool GenerateTexture( Rml::TextureHandle& texture_handle, const Rml::byte* source, const Rml::Vector2i& source_dimensions );

	/// <summary>
	///
	/// </summary>
	void RenderClipMask();

	/// <summary>
	///
	/// </summary>
	void PreRender();

	/// <summary>
	///
	/// </summary>
	void PostRender();

private:

	bool				_enableScissor;
	idRectangle			_clipRects;
	const idMaterial*	_cursorImages[CURSOR_COUNT];
	const idMaterial*	_guiSolid;

	int					_numMasks;
	idDrawVert*			_verts;
	triIndex_t*			_tris;

	int					_texGen;
	int					_numVerts;
	int					_numIndexes;
};

#endif