struct Bone
{
	float4x4 pose;
};

StructuredBuffer<Bone> boneBuffer :
register( b0 );
ByteAddressBuffer vertexBuffer_POS; // T-Pose pos
ByteAddressBuffer vertexBuffer_NOR; // T-Pose normal
ByteAddressBuffer vertexBuffer_WEI; // bone weights
ByteAddressBuffer vertexBuffer_BON; // bone indices
RWByteAddressBuffer streamoutBuffer_POS; // skinned pos
RWByteAddressBuffer streamoutBuffer_NOR; // skinned normal
RWByteAddressBuffer streamoutBuffer_PRE; // previous frame skinned pos

inline void Skinning( inout float4 pos, inout float4 nor, in float4 inBon, in float4 inWei )
{
	float4 p = 0, pp = 0;
	float3 n = 0;
	float4x4 m;
	float3x3 m3;
	float weisum = 0;

	// Force loop to reduce register pressure
	// though this way we can not interleave TEX - ALU operations
	[loop]
	for( uint i = 0; ( ( i < 4 ) && ( weisum < 1.0f ) ); ++i )
	{
		m = boneBuffer[( uint )inBon[i]].pose;
		m3 = ( float3x3 )m;

		p += mul( float4( pos.xyz, 1 ), m ) * inWei[i];
		n += mul( nor.xyz, m3 ) * inWei[i];

		weisum += inWei[i];
	}

	bool w = any( inWei );
	pos.xyz = w ? p.xyz : pos.xyz;
	nor.xyz = w ? n : nor.xyz;
}

[numthreads( 256, 1, 1 )]
void main( uint3 DTid : SV_DispatchThreadID )
{
	const uint fetchAddress = DTid.x * 16; // stride is 16 bytes for each vertex buffer now...

	uint4 pos_u = vertexBuffer_POS.Load4( fetchAddress );
	uint4 nor_u = vertexBuffer_NOR.Load4( fetchAddress );
	uint4 wei_u = vertexBuffer_WEI.Load4( fetchAddress );
	uint4 bon_u = vertexBuffer_BON.Load4( fetchAddress );

	float4 pos = asfloat( pos_u );
	float4 nor = asfloat( nor_u );
	float4 wei = asfloat( wei_u );
	float4 bon = asfloat( bon_u );

	Skinning( pos, nor, bon, wei );

	pos_u = asuint( pos );
	nor_u = asuint( nor );

	// copy prev frame current pos to current frame prev pos
	streamoutBuffer_PRE.Store4( fetchAddress, streamoutBuffer_POS.Load4( fetchAddress ) );

	// write out skinned props:
	streamoutBuffer_POS.Store4( fetchAddress, pos_u );
	streamoutBuffer_NOR.Store4( fetchAddress, nor_u );
}