#pragma once

#include <limits.h>

class idImage;

struct AtlasRegion
{
	enum Type
	{
		TYPE_GRAY = 1, // 1 component
		TYPE_RGBA8 = 4 // 4 components
	};

	uint16_t _x;
	uint16_t _y;
	uint32_t _width;
	uint32_t _height;
	uint32_t _mask;

	Type getType() const
	{
		return (Type)((_mask >> 0) & 0x0000000F);
	}

	uint32_t getFaceIndex() const
	{
		return (_mask >> 4) & 0x0000000F;
	}

	uint32_t getComponentIndex() const
	{
		return (_mask >> 8) & 0x0000000F;
	}

	void setMask(Type aType, uint32_t aFaceIndex, uint32_t aComponentIndex)
	{
		_mask = (aComponentIndex << 8) + (aFaceIndex << 4) + (uint32_t)aType;
	}
};

class CubeAtlas
{
public:
	/// create an empty dynamic atlas (region can be updated and added)
	/// @param textureSize an atlas creates a texture cube of 6 faces with size equal to (textureSize*textureSize * sizeof(RGBA) )
	/// @param maxRegionCount maximum number of region allowed in the atlas
	CubeAtlas(uint16_t _textureSize, uint16_t _maxRegionsCount = 4096);

	/// initialize a static atlas with serialized data	(region can be updated but not added)
	/// @param textureSize an atlas creates a texture cube of 6 faces with size equal to (textureSize*textureSize * sizeof(RGBA) )
	/// @param textureBuffer buffer of size 6*textureSize*textureSize*sizeof(uint32_t) (will be copied)
	/// @param regionCount number of region in the Atlas
	/// @param regionBuffer buffer containing the region (will be copied)
	/// @param maxRegionCount maximum number of region allowed in the atlas
	CubeAtlas(uint16_t _textureSize, const uint8_t* _textureBuffer, uint16_t _regionCount, const uint8_t* _regionBuffer, uint16_t _maxRegionsCount = 4096);
	~CubeAtlas();

	/// add a region to the atlas, and copy the content of mem to the underlying texture
	uint16_t addRegion(uint16_t _width, uint16_t _height, const uint8_t* _bitmapBuffer, AtlasRegion::Type _type = AtlasRegion::TYPE_RGBA8, uint16_t outline = 0);

	/// update a preallocated region
	void updateRegion(const AtlasRegion& _region, const uint8_t* _bitmapBuffer);

	/// Pack the UV coordinates of the four corners of a region to a vertex buffer using the supplied vertex format.
	/// v0 -- v3
	/// |     |     encoded in that order:  v0,v1,v2,v3
	/// v1 -- v2
	/// @remark the UV are four signed short normalized components.
	/// @remark the x,y,z components encode cube uv coordinates. The w component encode the color channel if any.
	/// @param handle handle to the region we are interested in
	/// @param vertexBuffer address of the first vertex we want to update. Must be valid up to vertexBuffer + offset + 3*stride + 4*sizeof(int16_t), which means the buffer must contains at least 4 vertexes including the first.
	/// @param offset byte offset to the first uv coordinate of the vertex in the buffer
	/// @param stride stride between the UV coordinates, usually size of a Vertex.
	void packUV(uint16_t _regionHandle, uint8_t* _vertexBuffer, uint32_t _offset, uint32_t _stride) const;
	void packUV(const AtlasRegion& _region, uint8_t* _vertexBuffer, uint32_t _offset, uint32_t _stride) const;

	/// Same as packUV but pack a whole face of the atlas cube, mostly used for debugging and visualizing atlas
	void packFaceLayerUV(uint32_t _idx, uint8_t* _vertexBuffer, uint32_t _offset, uint32_t _stride) const;

	/// return the TextureHandle (cube) of the atlas
	idImage* getImage() const
	{
		return m_image;
	}

	//retrieve a region info
	const AtlasRegion& getRegion(uint16_t _handle) const
	{
		return m_regions[_handle];
	}

	/// retrieve the size of side of a texture in pixels
	uint16_t getTextureSize() const
	{
		return m_textureSize;
	}

	/// retrieve the usage ratio of the atlas
	//float getUsageRatio() const { return 0.0f; }

	/// retrieve the numbers of region in the atlas
	uint16_t getRegionCount() const
	{
		return m_regionCount;
	}

	/// retrieve a pointer to the region buffer (in order to serialize it)
	const AtlasRegion* getRegionBuffer() const
	{
		return m_regions;
	}

	/// retrieve the byte size of the texture
	uint32_t getTextureBufferSize() const
	{
		return 6 * m_textureSize * m_textureSize * 4;
	}

	/// retrieve the mirrored texture buffer (to serialize it)
	const byte* getTextureBuffer() const
	{
		return m_textureBuffer;
	}

private:
	void init();

	struct PackedLayer;
	/// @owns
	PackedLayer* m_layers;

	/// @owns
	AtlasRegion* m_regions;

	/// @owns
	byte* m_textureBuffer;

	uint32_t m_usedLayers;
	uint32_t m_usedFaces;

	// @notowns
	idImage* m_image;
	uint16_t m_textureSize;
	float m_texelSize;
	float m_texelOffset[2];

	uint16_t m_regionCount;
	uint16_t m_maxRegionCount;
};
