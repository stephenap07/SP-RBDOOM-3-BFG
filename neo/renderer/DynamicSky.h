#ifndef RENDERER_DYNAMICSKY_H_
#define RENDERER_DYNAMICSKY_H_

// Represents color. Color-space depends on context.
// In the code below, used to represent color in XYZ, and RGB color-space
typedef idVec3 Color;

/**
*
*/
class DynamicSky
{
public:
	DynamicSky();
	~DynamicSky();

	void				Init( nvrhi::IDevice* newDevice, int verticalCount, int horizontalCount );

	void				SetInvProjMatrix( const idRenderMatrix& mat );

	void				UpdateParams( SkyDef newParams );

	void				WriteParams( nvrhi::ICommandList* commandList );

	srfTriangles_t*		Triangles() const
	{
		return tri;
	}

	nvrhi::IBuffer*		Buffer() const
	{
		return paramBuffer.Get();
	}

	const SkyDef& Params() const
	{
		return params;
	}

private:
	srfTriangles_t*			tri;		//!< Triangles generated in the Init method.
	SkyDef					params;
	nvrhi::BufferHandle		paramBuffer;
	bool					paramsUpdated;
	nvrhi::IDevice*			device;
};

inline bool operator==( const SkyDef& lhs, const SkyDef& rhs )
{
	if( lhs.sunDirection != rhs.sunDirection ||
			lhs.skyLuminanceXYZ != rhs.skyLuminanceXYZ ||
			lhs.parameters != rhs.parameters )
	{
		return false;
	}

	for( int i = 0; i < 5; i++ )
	{
		if( lhs.perezCoeff[i] != rhs.perezCoeff[i] )
		{
			return false;
		}
	}

	return lhs.invViewProj == rhs.invViewProj;
}

#endif