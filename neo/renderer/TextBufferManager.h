#pragma once

#include "FontManager.h"

struct TextBufferHandle
{
	uint16_t _id = kInvalidHandle;
};

#define MAX_TEXT_BUFFER_COUNT 64

/// type of vertex and index buffer to use with a TextBuffer
struct BufferType
{
	enum Enum
	{
		Static,
		Dynamic,
		Transient,
	};
};

/// special style effect (can be combined)
enum TextStyleFlags
{
	STYLE_NORMAL = 0,
	STYLE_OVERLINE = 1,
	STYLE_UNDERLINE = 1 << 1,
	STYLE_STRIKE_THROUGH = 1 << 2,
	STYLE_BACKGROUND = 1 << 3,
};

struct TextRectangle
{
	float width, height;
};

class TextBuffer;
class TextBufferManager
{
public:
	TextBufferManager(FontManager* _fontManager);
	~TextBufferManager();

	TextBufferHandle createTextBuffer(uint32_t _type, BufferType::Enum _bufferType);
	void destroyTextBuffer(TextBufferHandle _handle);
	void submitTextBuffer(TextBufferHandle _handle, int32_t _depth = 0);

	void setStyle(TextBufferHandle _handle, uint32_t _flags = STYLE_NORMAL);
	void setTextColor(TextBufferHandle _handle, uint32_t _rgba = 0x000000FF);
	void setBackgroundColor(TextBufferHandle _handle, uint32_t _rgba = 0x000000FF);

	void setOverlineColor(TextBufferHandle _handle, uint32_t _rgba = 0x000000FF);
	void setUnderlineColor(TextBufferHandle _handle, uint32_t _rgba = 0x000000FF);
	void setStrikeThroughColor(TextBufferHandle _handle, uint32_t _rgba = 0x000000FF);

	void setPenPosition(TextBufferHandle _handle, float _x, float _y);

	/// Append an ASCII/utf-8 string to the buffer using current pen position and color.
	void appendText(TextBufferHandle _handle, FontHandle _fontHandle, const idStr& aString);

	/// Append a whole face of the atlas cube, mostly used for debugging and visualizing atlas.
	void appendAtlasFace(TextBufferHandle _handle, uint16_t _faceIndex);

	/// Clear the text buffer and reset its state (pen/color).
	void clearTextBuffer(TextBufferHandle _handle);

	/// Return the rectangular size of the current text buffer (including all its content).
	TextRectangle getRectangle(TextBufferHandle _handle) const;

private:
	struct BufferCache
	{
		vertCacheHandle_t vertexBufferHandle;
		vertCacheHandle_t indexBufferHandle;
		TextBuffer* textBuffer;
		BufferType::Enum bufferType;
		uint32_t fontType;
	};

	BufferCache* m_textBuffers;
	HandleManagerT<MAX_TEXT_BUFFER_COUNT> m_textBufferHandles;
	FontManager* m_fontManager;

	const idMaterial* m_fontMaterial;
};
