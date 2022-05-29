/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2022 Stephen Pridham

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

#ifndef __GUI_RMLFONTENGINE_H__
#define __GUI_RMLFONTENGINE_H__

#include "rmlui/Core/FontEngineInterface.h"
#include "rmlui/Core/Texture.h"

/**
* IdTech 4 implementation for rendering font glyphs and using them in RML.
*
* @author Stephen Pridham
*/
class RmlFontEngine : public Rml::FontEngineInterface
{
public:
	RmlFontEngine();

	virtual ~RmlFontEngine();

	void Init();

	/// Called by RmlUi when it wants to load a font face from file.
	/// @param[in] file_name The file to load the face from.
	/// @param[in] fallback_face True to use this font face for unknown characters in other font faces.
	/// @param[in] weight The weight to load when the font face contains multiple weights, otherwise the weight to register the font as.
	/// @return True if the face was loaded successfully, false otherwise.
	bool LoadFontFace( const Rml::String& file_name, bool fallback_face, Rml::Style::FontWeight weight ) override;

	/// Called by RmlUi when it wants to load a font face from memory, registered using the provided family, style, and weight.
	/// @param[in] data A pointer to the data.
	/// @param[in] data_size Size of the data in bytes.
	/// @param[in] family The family to register the font as.
	/// @param[in] style The style to register the font as.
	/// @param[in] weight The weight to load when the font face contains multiple weights, otherwise the weight to register the font as.
	/// @param[in] fallback_face True to use this font face for unknown characters in other font faces.
	/// @return True if the face was loaded successfully, false otherwise.
	/// Note: The debugger plugin will load its embedded font faces through this method using the family name 'rmlui-debugger-font'.
	bool LoadFontFace( const byte* data, int data_size, const Rml::String& family, Rml::Style::FontStyle style, Rml::Style::FontWeight weight, bool fallback_face ) override;

	/// Called by RmlUi when a font configuration is resolved for an element. Should return a handle that
	/// can later be used to resolve properties of the face, and generate string geometry to be rendered.
	/// @param[in] family_ The family of the desired font handle.
	/// @param[in] style The style of the desired font handle.
	/// @param[in] weight The weight of the desired font handle.
	/// @param[in] size The size of desired handle, in points.
	/// @return A valid handle if a matching (or closely matching) font face was found, NULL otherwise.
	Rml::FontFaceHandle GetFontFaceHandle( const Rml::String& family_, Rml::Style::FontStyle style, Rml::Style::FontWeight weight, int size ) override;

	/// Called by RmlUi when a list of font effects is resolved for an element with a given font face.
	/// @param[in] handle_ The font handle.
	/// @param[in] fontEffects_ The list of font effects to generate the configuration for.
	/// @return A handle to the prepared font effects which will be used when generating geometry for a string.
	Rml::FontEffectsHandle PrepareFontEffects( Rml::FontFaceHandle handle_, const Rml::FontEffectList& fontEffects_ ) override;

	/// Should return the point size of this font face.
	/// @param[in] handle_ The font handle.
	/// @return The face's point size.
	int GetSize( Rml::FontFaceHandle handle_ ) override;

	/// Should return the pixel height of a lower-case x in this font face.
	/// @param[in] handle The font handle.
	/// @return The height of a lower-case x.
	int GetXHeight( Rml::FontFaceHandle handle ) override;

	/// Should return the default height between this font face's baselines.
	/// @param[in] handle The font handle.
	/// @return The default line height.
	int GetLineHeight( Rml::FontFaceHandle handle ) override;

	/// Should return the font's baseline, as a pixel offset from the bottom of the font.
	/// @param[in] handle The font handle.
	/// @return The font's baseline.
	int GetBaseline( Rml::FontFaceHandle handle ) override;

	/// Should return the font's underline, as a pixel offset from the bottom of the font.
	/// @param[in] handle The font handle.
	/// @param[out] thickness The font's underline thickness in pixels.
	/// @return The underline pixel offset.
	float GetUnderline( Rml::FontFaceHandle handle, float& thickness ) override;

	/// Called by RmlUi when it wants to retrieve the width of a string when rendered with this handle.
	/// @param[in] handle The font handle.
	/// @param[in] string The string to measure.
	/// @param[in] prior_character The optionally-specified character that immediately precedes the string. This may have an impact on the string width due to kerning.
	/// @return The width, in pixels, this string will occupy if rendered with this handle.
	int GetStringWidth( Rml::FontFaceHandle handle, const Rml::String& string, Rml::Character prior_character = Rml::Character::Null ) override;

	/// Called by RmlUi when it wants to retrieve the geometry required to render a single line of text.
	/// @param[in] faceHandle_ The font handle.
	/// @param[in] fontEffectsHandle_ The handle to the prepared font effects for which the geometry should be generated.
	/// @param[in] string_ The string to render.
	/// @param[in] position_ The position of the baseline of the first character to render.
	/// @param[in] colour_ The colour to render the text. Colour alpha is premultiplied with opacity.
	/// @param[in] opacity_ The opacity of the text, should be applied to font effects.
	/// @param[out] geometry_ An array of geometries to generate the geometry into.
	/// @return The width, in pixels, of the string geometry.
	int GenerateString( Rml::FontFaceHandle faceHandle_, Rml::FontEffectsHandle fontEffectsHandle_, const Rml::String& string_, const Rml::Vector2f& position_,
						const Rml::Colourb& colour_, float opacity_, Rml::GeometryList& geometry_ ) override;

	/// Called by RmlUi to determine if the text geometry is required to be re-generated. Whenever the returned version
	/// is changed, all geometry belonging to the given face handle will be re-generated.
	/// @param[in] face_handle The font handle.
	/// @return The version required for using any geometry generated with the face handle.
	int GetVersion( Rml::FontFaceHandle handle_ ) override;

	/// Called by RmlUi when it wants to garbage collect memory used by fonts.
	/// @note All existing FontFaceHandles and FontEffectsHandles are considered invalid after this call.
	void ReleaseFontResources() override;

private:

	FontManager*			_fontManager;
	Rml::Texture			_texture;
	idList<FontHandle>		_fonts;
	idList<TrueTypeHandle>	_fontFaces;
};

#endif