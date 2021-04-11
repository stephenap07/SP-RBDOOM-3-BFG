#include "precompiled.h"

#include "renderer/CubeAtlas.h"

#include "renderer/Image.h"

#include <vector>

static void writeUV( idDrawVert* _vertexBuffer, float _x, float _y, float _z, float _w )
{
	_vertexBuffer->SetTexCoord( _x, _y );
	//xyzw[3] = _w;
}

class RectanglePacker
{
public:
	RectanglePacker();
	RectanglePacker( uint32_t _width, uint32_t _height );

	/// non constructor initialization
	void init( uint32_t _width, uint32_t _height );

	/// find a suitable position for the given rectangle
	/// @return true if the rectangle can be added, false otherwise
	bool addRectangle( uint16_t _width, uint16_t _height, uint16_t& _outX, uint16_t& _outY );

	/// return the used surface in squared unit
	uint32_t getUsedSurface()
	{
		return m_usedSpace;
	}

	/// return the total available surface in squared unit
	uint32_t getTotalSurface()
	{
		return m_width * m_height;
	}

	/// return the usage ratio of the available surface [0:1]
	float getUsageRatio();

	/// reset to initial state
	void clear();

private:
	int32_t fit( uint32_t _skylineNodeIndex, uint16_t _width, uint16_t _height );

	/// Merges all skyline nodes that are at the same level.
	void merge();

	struct Node
	{
		Node() : x( 0 ), y( 0 ), width( 0 )
		{
		}

		Node( int16_t _x, int16_t _y, int16_t _width ) : x( _x ), y( _y ), width( _width )
		{
		}

		int16_t x;     //< The starting x-coordinate (leftmost).
		int16_t y;     //< The y-coordinate of the skyline level line.
		int32_t width; //< The line _width. The ending coordinate (inclusive) will be x+width-1.
	};


	uint32_t m_width;            //< width (in pixels) of the underlying texture
	uint32_t m_height;           //< height (in pixels) of the underlying texture
	uint32_t m_usedSpace;        //< Surface used in squared pixel
	idList<Node> m_skyline; //< node of the skyline algorithm
};

RectanglePacker::RectanglePacker()
	: m_width( 0 )
	, m_height( 0 )
	, m_usedSpace( 0 )
{
}

RectanglePacker::RectanglePacker( uint32_t _width, uint32_t _height )
	: m_width( _width )
	, m_height( _height )
	, m_usedSpace( 0 )
{
	// We want a one pixel border around the whole atlas to avoid any artefact when
	// sampling texture
	m_skyline.Append( Node( 1, 1, uint16_t( _width - 2 ) ) );
}

void RectanglePacker::init( uint32_t _width, uint32_t _height )
{
	assert( _width > 2 );
	assert( _height > 2 );

	m_width = _width;
	m_height = _height;
	m_usedSpace = 0;

	m_skyline.Clear();
	// We want a one pixel border around the whole atlas to avoid any artifact when
	// sampling texture
	m_skyline.Append( Node( 1, 1, uint16_t( _width - 2 ) ) );
}

bool RectanglePacker::addRectangle( uint16_t _width, uint16_t _height, uint16_t& _outX, uint16_t& _outY )
{
	int best_height, best_index;
	int32_t best_width;
	Node* node;
	Node* prev;
	_outX = 0;
	_outY = 0;

	best_height = INT_MAX;
	best_index = -1;
	best_width = INT_MAX;
	for( uint16_t ii = 0, num = uint16_t( m_skyline.Num() ); ii < num; ++ii )
	{
		int32_t yy = fit( ii, _width, _height );
		if( yy >= 0 )
		{
			node = &m_skyline[ii];
			if( ( ( yy + _height ) < best_height )
					|| ( ( ( yy + _height ) == best_height ) && ( node->width < best_width ) ) )
			{
				best_height = uint16_t( yy ) + _height;
				best_index = ii;
				best_width = node->width;
				_outX = node->x;
				_outY = uint16_t( yy );
			}
		}
	}

	if( best_index == -1 )
	{
		return false;
	}

	Node newNode( _outX, _outY + _height, _width );
	m_skyline.Insert( newNode, best_index );

	for( uint16_t ii = uint16_t( best_index + 1 ), num = uint16_t( m_skyline.Num() ); ii < num; ++ii )
	{
		node = &m_skyline[ii];
		prev = &m_skyline[ii - 1];
		if( node->x < ( prev->x + prev->width ) )
		{
			uint16_t shrink = uint16_t( prev->x + prev->width - node->x );
			node->x += shrink;
			node->width -= shrink;
			if( node->width <= 0 )
			{
				m_skyline.RemoveIndex( ii );
				--ii;
				--num;
			}
			else
			{
				break;
			}
		}
		else
		{
			break;
		}
	}

	merge();
	m_usedSpace += _width * _height;
	return true;
}

float RectanglePacker::getUsageRatio()
{
	uint32_t total = m_width * m_height;
	if( total > 0 )
	{
		return ( float )m_usedSpace / ( float )total;
	}

	return 0.0f;
}

void RectanglePacker::clear()
{
	m_skyline.Clear();
	m_usedSpace = 0;

	// We want a one pixel border around the whole atlas to avoid any artefact when
	// sampling texture
	m_skyline.Append( Node( 1, 1, uint16_t( m_width - 2 ) ) );
}

int32_t RectanglePacker::fit( uint32_t _skylineNodeIndex, uint16_t _width, uint16_t _height )
{
	int32_t width = _width;
	int32_t height = _height;

	const Node& baseNode = m_skyline[_skylineNodeIndex];

	int32_t xx = baseNode.x, yy;
	int32_t widthLeft = width;
	int32_t ii = _skylineNodeIndex;

	if( ( xx + width ) > ( int32_t )( m_width - 1 ) )
	{
		return -1;
	}

	yy = baseNode.y;
	while( widthLeft > 0 )
	{
		const Node& node = m_skyline[ii];
		if( node.y > yy )
		{
			yy = node.y;
		}

		if( ( yy + height ) > ( int32_t )( m_height - 1 ) )
		{
			return -1;
		}

		widthLeft -= node.width;
		++ii;
	}

	return yy;
}

void RectanglePacker::merge()
{
	Node* node;
	Node* next;
	uint32_t ii;

	for( ii = 0; ii < m_skyline.Num() - 1; ++ii )
	{
		node = ( Node* )&m_skyline[ii];
		next = ( Node* )&m_skyline[ii + 1];
		if( node->y == next->y )
		{
			node->width += next->width;
			m_skyline.RemoveIndex( ii + 1 );
			--ii;
		}
	}
}

struct Atlas::PackedLayer
{
	RectanglePacker packer;
	AtlasRegion faceRegion;
};

struct ImgData
{
	byte* data;
	int width;
	int height;
};

void GenerateAtlasImage( idImage* image )
{
	image->GenerateImage(
		renderSystem->GetFontManager()->getAtlas()->getTextureBuffer(),
		renderSystem->GetFontManager()->getAtlas()->getTextureSize(),
		renderSystem->GetFontManager()->getAtlas()->getTextureSize(),
		textureFilter_t::TF_LINEAR,
		textureRepeat_t::TR_CLAMP,
		textureUsage_t::TD_LOOKUP_TABLE_RGBA );
}

Atlas::Atlas( uint16_t aTextureSize, uint16_t aMaxRegionsCount )
	: m_usedLayers( 0 )
	, m_usedFaces( 0 )
	, m_textureSize( aTextureSize )
	, m_regionCount( 0 )
	, m_maxRegionCount( aMaxRegionsCount )
{
	assert( aTextureSize >= 64 && aTextureSize <= 4096 );
	assert( aMaxRegionsCount >= 64 && aMaxRegionsCount <= 32000 );

	m_layers = new PackedLayer[24];
	for( int ii = 0; ii < 24; ++ii )
	{
		m_layers[ii].packer.init( aTextureSize, aTextureSize );
	}

	m_regions = new AtlasRegion[aMaxRegionsCount];
	m_textureBuffer = new byte[getTextureBufferSize()];
	memset( m_textureBuffer, 0, getTextureBufferSize() );
}

Atlas::Atlas( uint16_t aTextureSize, const uint8_t* aTextureBuffer, uint16_t aRegionCount, const uint8_t* aRegionBuffer, uint16_t aMaxRegionsCount )
	: m_usedLayers( 24 )
	, m_usedFaces( 6 )
	, m_textureSize( aTextureSize )
	, m_regionCount( aRegionCount )
	, m_maxRegionCount( aRegionCount < aMaxRegionsCount ? aRegionCount : aMaxRegionsCount )
{
	assert( aRegionCount <= 64 && aMaxRegionsCount <= 4096 );

	m_regions = new AtlasRegion[aRegionCount];
	m_textureBuffer = new byte[getTextureBufferSize()];

	memcpy( m_regions, aRegionBuffer, aRegionCount * sizeof( AtlasRegion ) );
	memcpy( m_textureBuffer, aTextureBuffer, getTextureBufferSize() );
}

Atlas::~Atlas()
{
	delete[] m_layers;
	delete[] m_regions;
	delete[] m_textureBuffer;
}

void Atlas::init()
{
	m_texelSize = float( UINT16_MAX ) / float( m_textureSize );
	float texelHalf = m_texelSize / 2.0f;

	m_texelOffset[0] = texelHalf;
	m_texelOffset[1] = -texelHalf;

	m_image = globalImages->ImageFromFunction( "_fontAtlas", GenerateAtlasImage );
}

uint16_t Atlas::addRegion( uint16_t _width, uint16_t _height, const uint8_t* _bitmapBuffer, AtlasRegion::Type _type, uint16_t outline )
{
	if( m_regionCount >= m_maxRegionCount )
	{
		return UINT16_MAX;
	}

	uint16_t xx = 0;
	uint16_t yy = 0;
	uint32_t idx = 0;
	while( idx < m_usedLayers )
	{
		if( m_layers[idx].faceRegion.getType() == _type
				&& m_layers[idx].packer.addRectangle( _width + 1, _height + 1, xx, yy ) )
		{
			break;
		}

		idx++;
	}

	if( idx >= m_usedLayers )
	{
		if( ( idx + _type ) > 24
				|| m_usedFaces >= 6 )
		{
			return UINT16_MAX;
		}

		for( int ii = 0; ii < _type; ++ii )
		{
			AtlasRegion& region = m_layers[idx + ii].faceRegion;
			region._x = 0;
			region._y = 0;
			region._width = m_textureSize;
			region._height = m_textureSize;
			region.setMask( _type, m_usedFaces, ii );
		}

		m_usedLayers += _type;
		m_usedFaces++;

		if( !m_layers[idx].packer.addRectangle( _width + 1, _height + 1, xx, yy ) )
		{
			return UINT16_MAX;
		}
	}

	AtlasRegion& region = m_regions[m_regionCount];
	region._x = xx;
	region._y = yy;
	region._width = _width;
	region._height = _height;
	region._mask = m_layers[idx].faceRegion._mask;

	updateRegion( region, _bitmapBuffer );

	region._x += outline;
	region._y += outline;
	region._width -= ( outline * 2 );
	region._height -= ( outline * 2 );

	return m_regionCount++;
}

void Atlas::updateRegion( const AtlasRegion& _region, const uint8_t* _bitmapBuffer )
{
	uint32_t size = _region._width * _region._height * 4;

	if( 0 < size )
	{
		const byte* mem = ( byte* )Mem_ClearedAlloc( size, TAG_FONT );

		if( _region.getType() == AtlasRegion::TYPE_RGBA8 )
		{
			const uint8_t* inLineBuffer = _bitmapBuffer;
			uint8_t* outLineBuffer = m_textureBuffer + ( uintptr_t )_region.getFaceIndex() * ( ( int32_t )m_textureSize * ( int32_t )m_textureSize * 4 ) + ( ( ( ( int32_t )_region._y * ( int32_t )m_textureSize ) + ( int32_t )_region._x ) * 4 );

			for( int yy = 0; yy < _region._height; ++yy )
			{
				memcpy( outLineBuffer, inLineBuffer, ( size_t )_region._width * 4 );
				inLineBuffer += ( size_t )_region._width * 4;
				outLineBuffer += ( size_t )m_textureSize * 4;
			}

			memcpy( ( void* )mem, _bitmapBuffer, size );
		}
		else
		{
			uint32_t layer = _region.getComponentIndex();
			const uint8_t* inLineBuffer = _bitmapBuffer;
			uint8_t* outLineBuffer = ( m_textureBuffer + ( uintptr_t )_region.getFaceIndex() * ( ( int32_t )m_textureSize * ( int32_t )m_textureSize * 4 ) + ( ( ( ( int32_t )_region._y * ( int32_t )m_textureSize ) + ( int32_t )_region._x ) * 4 ) );

			for( int yy = 0; yy < _region._height; ++yy )
			{
				for( int xx = 0; xx < _region._width; ++xx )
				{
					byte val = 0;
					if( inLineBuffer[xx] > 0 )
					{
						val = 255;
					}

					outLineBuffer[( xx * 4 ) + layer + 0] = val;
					outLineBuffer[( xx * 4 ) + layer + 1] = val;
					outLineBuffer[( xx * 4 ) + layer + 2] = val;
					outLineBuffer[( xx * 4 ) + layer + 3] = inLineBuffer[xx];
				}

				memcpy( ( void* )( mem + yy * ( int32_t )_region._width * 4 ), outLineBuffer, ( int32_t )_region._width * 4 );
				inLineBuffer += _region._width;
				outLineBuffer += ( int32_t )m_textureSize * 4;
			}
		}

		// TODO: This messes up renderdoc.
		m_image->SubImageUpload( 0, _region._x, _region._y, 0, _region._width, _region._height, mem );
		Mem_Free( ( void* )mem );
	}
}

void Atlas::packFaceLayerUV( uint32_t _idx, idDrawVert* _vertexBuffer ) const
{
	packUV( m_layers[_idx].faceRegion, _vertexBuffer );
}

void Atlas::packUV( uint16_t _regionHandle, idDrawVert* _vertexBuffer ) const
{
	const AtlasRegion& region = m_regions[_regionHandle];
	packUV( region, _vertexBuffer );
}

void Atlas::packUV( const AtlasRegion& _region, idDrawVert* _vertexBuffer ) const
{
	//float x0 = ((float)_region._x / (m_textureSize / 2.0f)) - 1.0f;
	//float y0 = ((float)_region._y / (m_textureSize / 2.0f)) - 1.0f;
	//float x1 = ((float)(_region._x + _region._width)) / (m_textureSize / 2.0f) - 1.0f;
	//float y1 = ((float)(_region._y + _region._height)) / (m_textureSize / 2.0f) - 1.0f;
	//int16_t ww = (int16_t)((float(INT16_MAX) / 4.0f) * (float)_region.getComponentIndex());

	//float x0 = ((float)_region._x / (m_textureSize / 2.0f));
	//float y0 = ((float)_region._y / (m_textureSize / 2.0f));
	//float x1 = ((float)(_region._x + _region._width)) / (m_textureSize / 2.0f);
	//float y1 = ((float)(_region._y + _region._height)) / (m_textureSize / 2.0f);
	//int16_t ww = (int16_t)((float(INT16_MAX) / 4.0f) * (float)_region.getComponentIndex());

	float x0 = ( float )_region._x / ( float )m_textureSize;
	float y0 = ( float )_region._y / ( float )m_textureSize;
	float x1 = ( float )( _region._x + _region._width ) / ( float )m_textureSize;
	float y1 = ( float )( _region._y + _region._height ) / ( float )m_textureSize;
	int16_t ww = ( int16_t )( ( float( INT16_MAX ) / 4.0f ) * ( float )_region.getComponentIndex() );

	switch( _region.getFaceIndex() )
	{
		case 0: // +X

			/*
			x0 = -x0;
			x1 = -x1;
			y0 = -y0;
			y1 = -y1;
			*/

			//writeUV(&_vertexBuffer[0], 1.0f, y0, x0, ww);
			//writeUV(&_vertexBuffer[1], 1.0f, y0, x1, ww);
			//writeUV(&_vertexBuffer[2], 1.0f, y1, x1, ww);
			//writeUV(&_vertexBuffer[3], 1.0f, y1, x0, ww);

			writeUV( &_vertexBuffer[0], x0, y0, 0, ww );
			writeUV( &_vertexBuffer[1], x1, y0, 0, ww );
			writeUV( &_vertexBuffer[2], x1, y1, 0, ww );
			writeUV( &_vertexBuffer[3], x0, y1, 0, ww );

			break;

		case 1: // -X
			y0 = -y0;
			y1 = -y1;
			writeUV( &_vertexBuffer[0], INT16_MIN, y0, x0, ww );
			writeUV( &_vertexBuffer[1], INT16_MIN, y1, x0, ww );
			writeUV( &_vertexBuffer[2], INT16_MIN, y1, x1, ww );
			writeUV( &_vertexBuffer[3], INT16_MIN, y0, x1, ww );
			break;

		case 2: // +Y
			writeUV( &_vertexBuffer[0], x0, INT16_MAX, y0, ww );
			writeUV( &_vertexBuffer[1], x0, INT16_MAX, y1, ww );
			writeUV( &_vertexBuffer[2], x1, INT16_MAX, y1, ww );
			writeUV( &_vertexBuffer[3], x1, INT16_MAX, y0, ww );
			break;

		case 3: // -Y
			y0 = -y0;
			y1 = -y1;
			writeUV( &_vertexBuffer[0], x0, INT16_MIN, y0, ww );
			writeUV( &_vertexBuffer[1], x0, INT16_MIN, y1, ww );
			writeUV( &_vertexBuffer[2], x1, INT16_MIN, y1, ww );
			writeUV( &_vertexBuffer[3], x1, INT16_MIN, y0, ww );
			break;

		case 4: // +Z
			y0 = -y0;
			y1 = -y1;
			writeUV( &_vertexBuffer[0], x0, y0, INT16_MAX, ww );
			writeUV( &_vertexBuffer[1], x0, y1, INT16_MAX, ww );
			writeUV( &_vertexBuffer[2], x1, y1, INT16_MAX, ww );
			writeUV( &_vertexBuffer[3], x1, y0, INT16_MAX, ww );
			break;

		case 5: // -Z
			x0 = -x0;
			x1 = -x1;
			y0 = -y0;
			y1 = -y1;
			writeUV( &_vertexBuffer[0], x0, y0, INT16_MIN, ww );
			writeUV( &_vertexBuffer[1], x0, y1, INT16_MIN, ww );
			writeUV( &_vertexBuffer[2], x1, y1, INT16_MIN, ww );
			writeUV( &_vertexBuffer[3], x1, y0, INT16_MIN, ww );
			break;
	}
}
