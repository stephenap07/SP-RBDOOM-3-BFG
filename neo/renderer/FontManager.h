/*
 * Copyright 2013 Jeremie Roy.
 * Copyright 2022 Stephen Pridham.
 * All rights reserved.
 * License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
 */

#ifndef FONT_MANAGER_H__
#define FONT_MANAGER_H__

#include "../idlib/HandleManager.h"

class Atlas;


#define MAX_OPENED_FILES 64
#define MAX_OPENED_FONT  1024

#define FONT_TYPE_ALPHA             UINT32_C(0x00000100) // L8
// #define FONT_TYPE_LCD               UINT32_C(0x00000200) // BGRA8
// #define FONT_TYPE_RGBA              UINT32_C(0x00000300) // BGRA8
#define FONT_TYPE_DISTANCE          UINT32_C(0x00000400) // L8
#define FONT_TYPE_DISTANCE_SUBPIXEL UINT32_C(0x00000500) // L8

enum FontStyle
{
	FONT_STYLE_NORMAL = 0,
	FONT_STYLE_ITALIC,
	FONT_STYLE_CONDENSED
};

struct FontInfo
{
	/// The font height in pixel.
	uint16_t pixelSize;
	/// Rendering type used for the font.
	int16_t fontType;

	/// The pixel extents above the baseline in pixels (typically positive).
	float ascender;
	/// The extents below the baseline in pixels (typically negative).
	float descender;
	/// The spacing in pixels between one row's descent and the next row's ascent.
	float lineGap;
	/// This field gives the maximum horizontal cursor advance for all glyphs in the font.
	float maxAdvanceWidth;
	/// The thickness of the under/hover/strike-trough line in pixels.
	float underlineThickness;
	/// The position of the underline relatively to the baseline.
	float underlinePosition;

	/// Scale to apply to glyph data.
	float scale;
};

/// Unicode value of a character
typedef int32_t CodePoint;

/// A structure that describe a glyph.
struct GlyphInfo
{
	/// Index for faster retrieval.
	int32_t glyphIndex;

	/// Glyph's width in pixels.
	float width;

	/// Glyph's height in pixels.
	float height;

	/// Glyph's left offset in pixels
	float offset_x;

	/// Glyph's top offset in pixels.
	///
	/// @remark This is the distance from the baseline to the top-most glyph
	///   scan line, upwards y coordinates being positive.
	float offset_y;

	/// For horizontal text layouts, this is the unscaled horizontal
	/// distance in pixels used to increment the pen position when the
	/// glyph is drawn as part of a string of text.
	float advance_x;

	/// For vertical text layouts, this is the unscaled vertical distance
	/// in pixels used to increment the pen position when the glyph is
	/// drawn as part of a string of text.
	float advance_y;

	/// Region index in the atlas storing textures.
	uint16_t regionIndex;
};

struct Handle
{
	uint16_t id = kInvalidHandle;
};

inline bool operator==( Handle lhs, Handle rhs )
{
	return lhs.id == rhs.id;
}

using TrueTypeHandle = Handle;
using FontHandle = Handle;

class FontManager
{
public:
	/// Create the font manager using an external cube atlas (doesn't take
	/// ownership of the atlas).
	FontManager( Atlas* _atlas );

	/// Create the font manager and create the texture cube as BGRA8 with
	/// linear filtering.
	FontManager( uint16_t _textureSideWidth = 512 );

	~FontManager();

	void init();

	/// Retrieve the atlas used by the font manager (e.g. to add stuff to it)
	const Atlas* getAtlas() const
	{
		return m_atlas;
	}

	/// Load a TrueType font from a given buffer. The buffer is copied and
	/// thus can be freed or reused after this call.
	///
	/// @return invalid handle if the loading fail
	TrueTypeHandle createTtf( const uint8_t* _buffer, uint32_t _size );

	/// Unload a TrueType font (free font memory) but keep loaded glyphs.
	void destroyTtf( TrueTypeHandle _handle );

	/// Return a font whose height is a fixed pixel size.
	FontHandle createFontByPixelSize( TrueTypeHandle _handle, uint32_t _typefaceIndex, uint32_t _pixelSize, uint32_t _fontType = FONT_TYPE_ALPHA );

	/// Return a scaled child font whose height is a fixed pixel size.
	FontHandle createScaledFontToPixelSize( FontHandle _baseFontHandle, uint32_t _pixelSize );

	/// destroy a font (truetype or baked)
	void destroyFont( FontHandle _handle );

	/// Preload a set of glyphs from a TrueType file.
	///
	/// @return True if every glyph could be preloaded, false otherwise if
	///   the Font is a baked font, this only do validation on the characters.
	bool preloadGlyph( FontHandle _handle, const wchar_t* _string );

	/// Preload a single glyph, return true on success.
	bool preloadGlyph( FontHandle _handle, CodePoint _character );

	/// Return the font descriptor of a font.
	///
	/// @remark the handle is required to be valid
	const FontInfo& getFontInfo( FontHandle _handle ) const;

	/// Return the rendering informations about the glyph region. Load the
	/// glyph from a TrueType font if possible
	///
	const GlyphInfo* getGlyphInfo( FontHandle _handle, CodePoint _codePoint );

	const GlyphInfo& getBlackGlyph() const
	{
		return m_blackGlyph;
	}

	const char* getFamilyName( TrueTypeHandle _handle ) const;

private:
	struct CachedFont;
	struct CachedFile
	{
		uint8_t*	buffer;
		uint32_t	bufferSize;
		idStr		familyName;
	};

	bool addBitmap( GlyphInfo& _glyphInfo, const uint8_t* _data );

	bool m_ownAtlas;
	Atlas* m_atlas;

	HandleManagerT<MAX_OPENED_FONT> m_fontHandles;
	CachedFont* m_cachedFonts;

	HandleManagerT<MAX_OPENED_FILES> m_filesHandles;
	CachedFile* m_cachedFiles;

	GlyphInfo m_blackGlyph;

	// temporary buffer to raster glyph
	uint8_t* m_buffer;
};

#endif