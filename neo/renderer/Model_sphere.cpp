///////////////////////////////////////////////////////////////////////////////
// Sphere.cpp
// ==========
// Sphere for OpenGL with (radius, sectors, stacks)
// The min number of sectors is 3 and the min number of stacks are 2.
//
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2017-11-01
// UPDATED: 2020-05-20
///////////////////////////////////////////////////////////////////////////////


#include "precompiled.h"

#pragma hdrstop

#include "RenderCommon.h"
#include "Model_local.h"


#include <iostream>
#include <iomanip>
#include <cmath>


// constants //////////////////////////////////////////////////////////////////
const int MIN_SECTOR_COUNT = 3;
const int MIN_STACK_COUNT  = 2;

static const char* sphere_SnapshotName = "_sphere_Snapshot_";

class Sphere_local
{
public:
	// ctor/dtor
	Sphere_local( float radius = 1.0f, int sectorCount = 36, int stackCount = 18, bool smooth = true );
	~Sphere_local( );

	// getters/setters
	float getRadius( ) const
	{
		return radius;
	}
	int getSectorCount( ) const
	{
		return sectorCount;
	}
	int getStackCount( ) const
	{
		return stackCount;
	}
	void set( float radius, int sectorCount, int stackCount, bool smooth = true );
	void setRadius( float radius );
	void setSectorCount( int sectorCount );
	void setStackCount( int stackCount );
	void setSmooth( bool smooth );

	// for vertex data
	unsigned int getVertexCount( ) const
	{
		return ( unsigned int )vertices.Num( ) / 3;
	}
	unsigned int getNormalCount( ) const
	{
		return ( unsigned int )normals.Num( ) / 3;
	}
	unsigned int getTexCoordCount( ) const
	{
		return ( unsigned int )texCoords.Num( ) / 2;
	}
	unsigned int getIndexCount( ) const
	{
		return ( unsigned int )indices.Num( );
	}
	unsigned int getLineIndexCount( ) const
	{
		return ( unsigned int )lineIndices.Num( );
	}
	unsigned int getTriangleCount( ) const
	{
		return getIndexCount( ) / 3;
	}
	unsigned int getVertexSize( ) const
	{
		return ( unsigned int )vertices.Num( ) * sizeof( float );
	}
	unsigned int getNormalSize( ) const
	{
		return ( unsigned int )normals.Num( ) * sizeof( float );
	}
	unsigned int getTexCoordSize( ) const
	{
		return ( unsigned int )texCoords.Num( ) * sizeof( float );
	}
	unsigned int getIndexSize( ) const
	{
		return ( unsigned int )indices.Num( ) * sizeof( triIndex_t );
	}
	unsigned int getLineIndexSize( ) const
	{
		return ( unsigned int )lineIndices.Num( ) * sizeof( unsigned int );
	}
	const float* getVertices( ) const
	{
		return &vertices[0];
	}
	const float* getNormals( ) const
	{
		return &normals[0];
	}
	const float* getTexCoords( ) const
	{
		return &texCoords[0];
	}
	const triIndex_t* getIndices( ) const
	{
		return &indices[0];
	}
	const triIndex_t* getLineIndices( ) const
	{
		return &lineIndices[0];
	}

	// for interleaved vertices: V/N/T
	unsigned int getInterleavedVertexCount( ) const
	{
		return getVertexCount( );    // # of vertices
	}
	unsigned int getInterleavedVertexSize( ) const
	{
		return ( unsigned int )interleavedVertices.Num( ) * sizeof( float );    // # of bytes
	}
	int getInterleavedStride( ) const
	{
		return interleavedStride;    // should be 32 bytes
	}
	const float* getInterleavedVertices( ) const
	{
		return &interleavedVertices[0];
	}

private:
	// member functions
	void buildVerticesSmooth( );
	void buildVerticesFlat( );
	void buildInterleavedVertices( );
	void addVertex( float x, float y, float z );
	void addNormal( float x, float y, float z );
	void addTexCoord( float s, float t );
	void addIndices( unsigned int i1, unsigned int i2, unsigned int i3 );
	idList<float> computeFaceNormal( float x1, float y1, float z1,
									 float x2, float y2, float z2,
									 float x3, float y3, float z3 );

	// memeber vars
	float radius;
	int sectorCount;                        // longitude, # of slices
	int stackCount;                         // latitude, # of stacks
	bool smooth;
	idList<float> vertices;
	idList<float> normals;
	idList<float> texCoords;
	idList<triIndex_t> indices;
	idList<triIndex_t> lineIndices;

	// interleaved
	idList<float> interleavedVertices;
	int interleavedStride;                  // # of bytes to hop to the next vertex (should be 32 bytes)

};

///////////////////////////////////////////////////////////////////////////////
// ctor
///////////////////////////////////////////////////////////////////////////////
Sphere_local::Sphere_local( float radius, int sectors, int stacks, bool smooth ) : interleavedStride( 32 )
{
	set( radius, sectors, stacks, smooth );
}

Sphere_local::~Sphere_local( )
{
	vertices.Clear( );
	normals.Clear( );
	texCoords.Clear( );
	indices.Clear( );
	lineIndices.Clear( );
	interleavedVertices.Clear( );
}



///////////////////////////////////////////////////////////////////////////////
// setters
///////////////////////////////////////////////////////////////////////////////
void Sphere_local::set( float radius, int sectors, int stacks, bool smooth )
{
	this->radius = radius;
	this->sectorCount = sectors;
	if( sectors < MIN_SECTOR_COUNT )
	{
		this->sectorCount = MIN_SECTOR_COUNT;
	}
	this->stackCount = stacks;
	if( sectors < MIN_STACK_COUNT )
	{
		this->sectorCount = MIN_STACK_COUNT;
	}
	this->smooth = smooth;

	if( smooth )
	{
		buildVerticesSmooth();
	}
	else
	{
		buildVerticesFlat();
	}
}

void Sphere_local::setRadius( float radius )
{
	if( radius != this->radius )
	{
		set( radius, sectorCount, stackCount, smooth );
	}
}

void Sphere_local::setSectorCount( int sectors )
{
	if( sectors != this->sectorCount )
	{
		set( radius, sectors, stackCount, smooth );
	}
}

void Sphere_local::setStackCount( int stacks )
{
	if( stacks != this->stackCount )
	{
		set( radius, sectorCount, stacks, smooth );
	}
}

void Sphere_local::setSmooth( bool smooth )
{
	if( this->smooth == smooth )
	{
		return;
	}

	this->smooth = smooth;
	if( smooth )
	{
		buildVerticesSmooth();
	}
	else
	{
		buildVerticesFlat();
	}
}

///////////////////////////////////////////////////////////////////////////////
// build vertices of sphere with smooth shading using parametric equation
// x = r * cos(u) * cos(v)
// y = r * cos(u) * sin(v)
// z = r * sin(u)
// where u: stack(latitude) angle (-90 <= u <= 90)
//       v: sector(longitude) angle (0 <= v <= 360)
///////////////////////////////////////////////////////////////////////////////
void Sphere_local::buildVerticesSmooth()
{
	const float PI = acos( -1 );

	// clear memory of prev arrays
	//clearArrays();

	float x, y, z, xy;                              // vertex position
	float nx, ny, nz, lengthInv = 1.0f / radius;    // normal
	float s, t;                                     // texCoord

	float sectorStep = 2 * PI / sectorCount;
	float stackStep = PI / stackCount;
	float sectorAngle, stackAngle;

	for( int i = 0; i <= stackCount; ++i )
	{
		stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
		xy = radius * cosf( stackAngle );           // r * cos(u)
		z = radius * sinf( stackAngle );            // r * sin(u)

		// add (sectorCount+1) vertices per stack
		// the first and last vertices have same position and normal, but different tex coords
		for( int j = 0; j <= sectorCount; ++j )
		{
			sectorAngle = j * sectorStep;           // starting from 0 to 2pi

			// vertex position
			x = xy * cosf( sectorAngle );           // r * cos(u) * cos(v)
			y = xy * sinf( sectorAngle );           // r * cos(u) * sin(v)
			addVertex( x, y, z );

			// normalized vertex normal
			nx = x * lengthInv;
			ny = y * lengthInv;
			nz = z * lengthInv;
			addNormal( nx, ny, nz );

			// vertex tex coord between [0, 1]
			s = ( float )j / sectorCount;
			t = ( float )i / stackCount;
			addTexCoord( s, t );
		}
	}

	// indices
	//  k1--k1+1
	//  |  / |
	//  | /  |
	//  k2--k2+1
	unsigned int k1, k2;
	for( int i = 0; i < stackCount; ++i )
	{
		k1 = i * ( sectorCount + 1 );   // beginning of current stack
		k2 = k1 + sectorCount + 1;      // beginning of next stack

		for( int j = 0; j < sectorCount; ++j, ++k1, ++k2 )
		{
			// 2 triangles per sector excluding 1st and last stacks
			if( i != 0 )
			{
				addIndices( k1, k2, k1 + 1 ); // k1---k2---k1+1
			}

			if( i != ( stackCount - 1 ) )
			{
				addIndices( k1 + 1, k2, k2 + 1 ); // k1+1---k2---k2+1
			}

			// vertical lines for all stacks
			lineIndices.Append( k1 );
			lineIndices.Append( k2 );
			if( i != 0 ) // horizontal lines except 1st stack
			{
				lineIndices.Append( k1 );
				lineIndices.Append( k1 + 1 );
			}
		}
	}

	// generate interleaved vertex array as well
	buildInterleavedVertices();
}



///////////////////////////////////////////////////////////////////////////////
// generate vertices with flat shading
// each triangle is independent (no shared vertices)
///////////////////////////////////////////////////////////////////////////////
void Sphere_local::buildVerticesFlat()
{
	const float PI = acos( -1 );

	// tmp vertex definition (x,y,z,s,t)
	struct Vertex
	{
		float x, y, z, s, t;
	};

	idList<Vertex> tmpVertices;

	float sectorStep = 2 * PI / sectorCount;
	float stackStep = PI / stackCount;
	float sectorAngle, stackAngle;

	// compute all vertices first, each vertex contains (x,y,z,s,t) except normal
	for( int i = 0; i <= stackCount; ++i )
	{
		stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
		float xy = radius * cosf( stackAngle );     // r * cos(u)
		float z = radius * sinf( stackAngle );      // r * sin(u)

		// add (sectorCount+1) vertices per stack
		// the first and last vertices have same position and normal, but different tex coords
		for( int j = 0; j <= sectorCount; ++j )
		{
			sectorAngle = j * sectorStep;           // starting from 0 to 2pi

			Vertex vertex;
			vertex.x = xy * cosf( sectorAngle );    // x = r * cos(u) * cos(v)
			vertex.y = xy * sinf( sectorAngle );    // y = r * cos(u) * sin(v)
			vertex.z = z;                           // z = r * sin(u)
			vertex.s = ( float )j / sectorCount;    // s
			vertex.t = ( float )i / stackCount;     // t
			tmpVertices.Append( vertex );
		}
	}

	// clear memory of prev arrays
	//clearArrays();

	Vertex v1, v2, v3, v4;                          // 4 vertex positions and tex coords
	idList<float> n;                           // 1 face normal

	int i, j, k, vi1, vi2;
	int index = 0;                                  // index for vertex
	for( i = 0; i < stackCount; ++i )
	{
		vi1 = i * ( sectorCount + 1 );              // index of tmpVertices
		vi2 = ( i + 1 ) * ( sectorCount + 1 );

		for( j = 0; j < sectorCount; ++j, ++vi1, ++vi2 )
		{
			// get 4 vertices per sector
			//  v1--v3
			//  |    |
			//  v2--v4
			v1 = tmpVertices[vi1];
			v2 = tmpVertices[vi2];
			v3 = tmpVertices[vi1 + 1];
			v4 = tmpVertices[vi2 + 1];

			// if 1st stack and last stack, store only 1 triangle per sector
			// otherwise, store 2 triangles (quad) per sector
			if( i == 0 ) // a triangle for first stack ==========================
			{
				// put a triangle
				addVertex( v1.x, v1.y, v1.z );
				addVertex( v2.x, v2.y, v2.z );
				addVertex( v4.x, v4.y, v4.z );

				// put tex coords of triangle
				addTexCoord( v1.s, v1.t );
				addTexCoord( v2.s, v2.t );
				addTexCoord( v4.s, v4.t );

				// put normal
				n = computeFaceNormal( v1.x, v1.y, v1.z, v2.x, v2.y, v2.z, v4.x, v4.y, v4.z );
				for( k = 0; k < 3; ++k ) // same normals for 3 vertices
				{
					addNormal( n[0], n[1], n[2] );
				}

				// put indices of 1 triangle
				addIndices( index, index + 1, index + 2 );

				// indices for line (first stack requires only vertical line)
				lineIndices.Append( index );
				lineIndices.Append( index + 1 );

				index += 3;     // for next
			}
			else if( i == ( stackCount - 1 ) ) // a triangle for last stack =========
			{
				// put a triangle
				addVertex( v1.x, v1.y, v1.z );
				addVertex( v2.x, v2.y, v2.z );
				addVertex( v3.x, v3.y, v3.z );

				// put tex coords of triangle
				addTexCoord( v1.s, v1.t );
				addTexCoord( v2.s, v2.t );
				addTexCoord( v3.s, v3.t );

				// put normal
				n = computeFaceNormal( v1.x, v1.y, v1.z, v2.x, v2.y, v2.z, v3.x, v3.y, v3.z );
				for( k = 0; k < 3; ++k ) // same normals for 3 vertices
				{
					addNormal( n[0], n[1], n[2] );
				}

				// put indices of 1 triangle
				addIndices( index, index + 1, index + 2 );

				// indices for lines (last stack requires both vert/hori lines)
				lineIndices.Append( index );
				lineIndices.Append( index + 1 );
				lineIndices.Append( index );
				lineIndices.Append( index + 2 );

				index += 3;     // for next
			}
			else // 2 triangles for others ====================================
			{
				// put quad vertices: v1-v2-v3-v4
				addVertex( v1.x, v1.y, v1.z );
				addVertex( v2.x, v2.y, v2.z );
				addVertex( v3.x, v3.y, v3.z );
				addVertex( v4.x, v4.y, v4.z );

				// put tex coords of quad
				addTexCoord( v1.s, v1.t );
				addTexCoord( v2.s, v2.t );
				addTexCoord( v3.s, v3.t );
				addTexCoord( v4.s, v4.t );

				// put normal
				n = computeFaceNormal( v1.x, v1.y, v1.z, v2.x, v2.y, v2.z, v3.x, v3.y, v3.z );
				for( k = 0; k < 4; ++k ) // same normals for 4 vertices
				{
					addNormal( n[0], n[1], n[2] );
				}

				// put indices of quad (2 triangles)
				addIndices( index, index + 1, index + 2 );
				addIndices( index + 2, index + 1, index + 3 );

				// indices for lines
				lineIndices.Append( index );
				lineIndices.Append( index + 1 );
				lineIndices.Append( index );
				lineIndices.Append( index + 2 );

				index += 4;     // for next
			}
		}
	}

	// generate interleaved vertex array as well
	buildInterleavedVertices();
}



///////////////////////////////////////////////////////////////////////////////
// generate interleaved vertices: V/N/T
// stride must be 32 bytes
///////////////////////////////////////////////////////////////////////////////
void Sphere_local::buildInterleavedVertices()
{
	interleavedVertices.Clear( );

	std::size_t i, j;
	std::size_t count = vertices.Num();
	for( i = 0, j = 0; i < count; i += 3, j += 2 )
	{
		interleavedVertices.Append( vertices[i] );
		interleavedVertices.Append( vertices[i + 1] );
		interleavedVertices.Append( vertices[i + 2] );

		interleavedVertices.Append( normals[i] );
		interleavedVertices.Append( normals[i + 1] );
		interleavedVertices.Append( normals[i + 2] );

		interleavedVertices.Append( texCoords[j] );
		interleavedVertices.Append( texCoords[j + 1] );
	}
}



///////////////////////////////////////////////////////////////////////////////
// add single vertex to array
///////////////////////////////////////////////////////////////////////////////
void Sphere_local::addVertex( float x, float y, float z )
{
	vertices.Append( x );
	vertices.Append( y );
	vertices.Append( z );
}



///////////////////////////////////////////////////////////////////////////////
// add single normal to array
///////////////////////////////////////////////////////////////////////////////
void Sphere_local::addNormal( float nx, float ny, float nz )
{
	normals.Append( nx );
	normals.Append( ny );
	normals.Append( nz );
}



///////////////////////////////////////////////////////////////////////////////
// add single texture coord to array
///////////////////////////////////////////////////////////////////////////////
void Sphere_local::addTexCoord( float s, float t )
{
	texCoords.Append( s );
	texCoords.Append( t );
}



///////////////////////////////////////////////////////////////////////////////
// add 3 indices to array
///////////////////////////////////////////////////////////////////////////////
void Sphere_local::addIndices( unsigned int i1, unsigned int i2, unsigned int i3 )
{
	indices.Append( i1 );
	indices.Append( i2 );
	indices.Append( i3 );
}



///////////////////////////////////////////////////////////////////////////////
// return face normal of a triangle v1-v2-v3
// if a triangle has no surface (normal length = 0), then return a zero vector
///////////////////////////////////////////////////////////////////////////////
idList<float> Sphere_local::computeFaceNormal( float x1, float y1, float z1, // v1
		float x2, float y2, float z2,  // v2
		float x3, float y3, float z3 ) // v3
{
	const float EPSILON = 0.000001f;

	idList<float> normal;
	normal.Append( 3.0f );
	normal.Append( 0.0f );
	float nx, ny, nz;

	// find 2 edge vectors: v1-v2, v1-v3
	float ex1 = x2 - x1;
	float ey1 = y2 - y1;
	float ez1 = z2 - z1;
	float ex2 = x3 - x1;
	float ey2 = y3 - y1;
	float ez2 = z3 - z1;

	// cross product: e1 x e2
	nx = ey1 * ez2 - ez1 * ey2;
	ny = ez1 * ex2 - ex1 * ez2;
	nz = ex1 * ey2 - ey1 * ex2;

	// normalize only if the length is > 0
	float length = sqrtf( nx * nx + ny * ny + nz * nz );
	if( length > EPSILON )
	{
		// normalize
		float lengthInv = 1.0f / length;
		normal[0] = nx * lengthInv;
		normal[1] = ny * lengthInv;
		normal[2] = nz * lengthInv;
	}

	return normal;
}

dynamicModel_t idRenderModelSphere::IsDynamicModel( ) const
{
	return DM_CACHED;
}

bool idRenderModelSphere::IsLoaded( ) const
{
	return true; // don't ever need to load
}

idRenderModel* idRenderModelSphere::InstantiateDynamicModel( const struct renderEntity_s* renderEntity, const viewDef_t* viewDef, idRenderModel* cachedModel )
{
	idRenderModelStatic* staticModel;
	srfTriangles_t* tri;
	modelSurface_t surf;

	if( cachedModel && !r_useCachedDynamicModels.GetBool( ) )
	{
		delete cachedModel;
		cachedModel = NULL;
	}

	if( renderEntity == NULL || viewDef == NULL )
	{
		delete cachedModel;
		return NULL;
	}

	if( cachedModel != NULL )
	{
		assert( dynamic_cast< idRenderModelStatic* >( cachedModel ) != NULL );
		assert( idStr::Icmp( cachedModel->Name( ), sphere_SnapshotName ) == 0 );

		staticModel = static_cast< idRenderModelStatic* >( cachedModel );
		surf = *staticModel->Surface( 0 );
		tri = surf.geometry;
	}
	else
	{
		// Generate vertex data.
		Sphere_local sphere( 16.0f, 36, 18, true );

		staticModel = new( TAG_MODEL ) idRenderModelStatic;
		staticModel->InitEmpty( sphere_SnapshotName );

		tri = R_AllocStaticTriSurf( );
		R_AllocStaticTriSurfVerts( tri, sphere.getVertexCount() );
		R_AllocStaticTriSurfIndexes( tri, sphere.getIndexCount() );

		// Set vertex data.
		for( int i = 0; i < sphere.getVertexCount( ); i++ )
		{
			tri->verts[i].Clear( );
			idVec3 xyz = *( const idVec3* )&sphere.getVertices()[i * 3];
			//xyz.x = -xyz.x;
			tri->verts[i].xyz = xyz;
			//idVec3 normal = *( const idVec3* )&sphere.getNormals( )[i * 3];
			//normal.x = -normal.x;
			//tri->verts[i].SetNormal( normal );
			//tri->verts[i].SetTangent( 0.0f, 1.0f, 0.0f );
			//tri->verts[i].SetBiTangent( 0.0f, 0.0f, 1.0f );
			const idVec2* texCoord = ( const idVec2* )&sphere.getTexCoords( )[i * 2];
			tri->verts[i].SetTexCoord( *texCoord );
		}

		memcpy( tri->indexes, sphere.getIndices(), sphere.getIndexSize() );

		tri->numVerts = sphere.getVertexCount( );
		tri->numIndexes = sphere.getIndexCount( );
		tri->generateNormals = true;

		surf.geometry = tri;
		surf.id = 0;
		surf.shader = tr.whiteMaterial;

		staticModel->AddSurface( surf );
		staticModel->FinishSurfaces( true );
	}

	int	red = idMath::Ftoi( renderEntity->shaderParms[SHADERPARM_RED] * 255.0f );
	int green = idMath::Ftoi( renderEntity->shaderParms[SHADERPARM_GREEN] * 255.0f );
	int	blue = idMath::Ftoi( renderEntity->shaderParms[SHADERPARM_BLUE] * 255.0f );
	int	alpha = idMath::Ftoi( renderEntity->shaderParms[SHADERPARM_ALPHA] * 255.0f );

	R_BoundTriSurf( tri );

	staticModel->bounds = tri->bounds;

	return staticModel;
}

idBounds idRenderModelSphere::Bounds( const struct renderEntity_s* renderEntity ) const
{
	idBounds	b;

	b.Zero( );
	if( !renderEntity )
	{
		b.ExpandSelf( 8.0f );
	}
	else
	{
		// TODO(Stephen): Hack. I assume the sphere has a radius of 16. With diameter of 32.
		b.ExpandSelf( 32.0f );
	}
	return b;
}