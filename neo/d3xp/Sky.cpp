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

#include "precompiled.h"
#pragma hdrstop

#include "Game_local.h"

#include <map>

// HDTV rec. 709 matrix.
static constexpr float M_XYZ2RGB[] =
{
	3.240479f, -0.969256f,  0.055648f,
	-1.53715f,   1.875991f, -0.204043f,
	-0.49853f,   0.041556f,  1.057311f,
};

// Converts color representation from CIE XYZ to RGB color-space.
static Color xyzToRgb( const Color& xyz )
{
	Color rgb;
	rgb.x = M_XYZ2RGB[0] * xyz.x + M_XYZ2RGB[3] * xyz.y + M_XYZ2RGB[6] * xyz.z;
	rgb.y = M_XYZ2RGB[1] * xyz.x + M_XYZ2RGB[4] * xyz.y + M_XYZ2RGB[7] * xyz.z;
	rgb.z = M_XYZ2RGB[2] * xyz.x + M_XYZ2RGB[5] * xyz.y + M_XYZ2RGB[8] * xyz.z;
	return rgb;
};

// Precomputed luminance of sunlight in XYZ colorspace.
// Computed using code from Game Engine Gems, Volume One, chapter 15. Implementation based on Dr. Richard Bird model.
// This table is used for piecewise linear interpolation. Transitions from and to 0.0 at sunset and sunrise are highly inaccurate
static std::map<float, Color> sunLuminanceXYZTable =
{
	{  5.0f, {  0.000000f,  0.000000f,  0.000000f } },
	{  7.0f, { 12.703322f, 12.989393f,  9.100411f } },
	{  8.0f, { 13.202644f, 13.597814f, 11.524929f } },
	{  9.0f, { 13.192974f, 13.597458f, 12.264488f } },
	{ 10.0f, { 13.132943f, 13.535914f, 12.560032f } },
	{ 11.0f, { 13.088722f, 13.489535f, 12.692996f } },
	{ 12.0f, { 13.067827f, 13.467483f, 12.745179f } },
	{ 13.0f, { 13.069653f, 13.469413f, 12.740822f } },
	{ 14.0f, { 13.094319f, 13.495428f, 12.678066f } },
	{ 15.0f, { 13.142133f, 13.545483f, 12.526785f } },
	{ 16.0f, { 13.201734f, 13.606017f, 12.188001f } },
	{ 17.0f, { 13.182774f, 13.572725f, 11.311157f } },
	{ 18.0f, { 12.448635f, 12.672520f,  8.267771f } },
	{ 20.0f, {  0.000000f,  0.000000f,  0.000000f } },
};


// Precomputed luminance of sky in the zenith point in XYZ colorspace.
// Computed using code from Game Engine Gems, Volume One, chapter 15. Implementation based on Dr. Richard Bird model.
// This table is used for piecewise linear interpolation. Day/night transitions are highly inaccurate.
// The scale of luminance change in Day/night transitions is not preserved.
// Luminance at night was increased to eliminate need the of HDR render.
static std::map<float, Color> skyLuminanceXYZTable =
{
	{  0.0f, { 0.308f,    0.308f,    0.411f    } },
	{  1.0f, { 0.308f,    0.308f,    0.410f    } },
	{  2.0f, { 0.301f,    0.301f,    0.402f    } },
	{  3.0f, { 0.287f,    0.287f,    0.382f    } },
	{  4.0f, { 0.258f,    0.258f,    0.344f    } },
	{  5.0f, { 0.258f,    0.258f,    0.344f    } },
	{  7.0f, { 0.962851f, 1.000000f, 1.747835f } },
	{  8.0f, { 0.967787f, 1.000000f, 1.776762f } },
	{  9.0f, { 0.970173f, 1.000000f, 1.788413f } },
	{ 10.0f, { 0.971431f, 1.000000f, 1.794102f } },
	{ 11.0f, { 0.972099f, 1.000000f, 1.797096f } },
	{ 12.0f, { 0.972385f, 1.000000f, 1.798389f } },
	{ 13.0f, { 0.972361f, 1.000000f, 1.798278f } },
	{ 14.0f, { 0.972020f, 1.000000f, 1.796740f } },
	{ 15.0f, { 0.971275f, 1.000000f, 1.793407f } },
	{ 16.0f, { 0.969885f, 1.000000f, 1.787078f } },
	{ 17.0f, { 0.967216f, 1.000000f, 1.773758f } },
	{ 18.0f, { 0.961668f, 1.000000f, 1.739891f } },
	{ 20.0f, { 0.264f,    0.264f,    0.352f    } },
	{ 21.0f, { 0.264f,    0.264f,    0.352f    } },
	{ 22.0f, { 0.290f,    0.290f,    0.386f    } },
	{ 23.0f, { 0.303f,    0.303f,    0.404f    } },
};


// Turbidity tables. Taken from:
// A. J. Preetham, P. Shirley, and B. Smits. A Practical Analytic Model for Daylight. SIGGRAPH '99
// Coefficients correspond to xyY colorspace.
static constexpr Color ABCDE[] =
{
	{ -0.2592f, -0.2608f, -1.4630f },
	{  0.0008f,  0.0092f,  0.4275f },
	{  0.2125f,  0.2102f,  5.3251f },
	{ -0.8989f, -1.6537f, -2.5771f },
	{  0.0452f,  0.0529f,  0.3703f },
};

static constexpr Color ABCDE_t[] =
{
	{ -0.0193f, -0.0167f,  0.1787f },
	{ -0.0665f, -0.0950f, -0.3554f },
	{ -0.0004f, -0.0079f, -0.0227f },
	{ -0.0641f, -0.0441f,  0.1206f },
	{ -0.0033f, -0.0109f, -0.0670f },
};

// Performs piecewise linear interpolation of a Color parameter.
class DynamicValueController
{
	using ValueType = Color;
	using KeyMap = std::map<float, ValueType>;

public:
	DynamicValueController();

	~DynamicValueController();

	void SetMap( const KeyMap& keymap );

	ValueType GetValue( float time ) const;

	void Clear();

private:
	static ValueType Interpolate( float lowerTime, const ValueType& lowerVal, float upperTime,
								  const ValueType& upperVal, float time );

	KeyMap keyMap;
};

// Controls sun position according to time, month, and observer's latitude.
// Sun position computation based on Earth's orbital elements: https://nssdc.gsfc.nasa.gov/planetary/factsheet/earthfact.html
class SunController
{
public:
	enum Month : int
	{
		January = 0,
		February,
		March,
		April,
		May,
		June,
		July,
		August,
		September,
		October,
		November,
		December
	};

	SunController();

	void Update( float _time );

	idVec3	northDir;
	idVec3	sunDir;
	idVec3	upDir;
	float	latitude;
	Month	month;

private:
	void			CalculateSunOrbit();

	void			UpdateSunPosition( float _hour );

	static idQuat	FromAxisAngle( idVec3 axis, const float angle );

	float	eclipticObliquity;
	float	delta;
};

DynamicValueController::DynamicValueController()
{
}

DynamicValueController::~DynamicValueController()
{
}

void DynamicValueController::SetMap( const KeyMap& keymap )
{
	keyMap = keymap;
}

DynamicValueController::ValueType DynamicValueController::GetValue( float time ) const
{
	auto itUpper = keyMap.upper_bound( time + 1e-6f );
	auto itLower = itUpper;
	--itLower;

	if( itLower == keyMap.end() )
	{
		return itUpper->second;
	}

	if( itUpper == keyMap.end() )
	{
		return itLower->second;
	}

	const float lowerTime = itLower->first;
	const ValueType& lowerVal = itLower->second;
	const float upperTime = itUpper->first;
	const ValueType& upperVal = itUpper->second;

	if( lowerTime == upperTime )
	{
		return lowerVal;
	}

	return Interpolate( lowerTime, lowerVal, upperTime, upperVal, time );
}

void DynamicValueController::Clear()
{
	keyMap.clear();
}

DynamicValueController::ValueType DynamicValueController::Interpolate( float lowerTime, const ValueType& lowerVal,
		float upperTime, const ValueType& upperVal, float time )
{
	const float tt = ( time - lowerTime ) / ( upperTime - lowerTime );
	const ValueType result = Lerp( lowerVal, upperVal, tt );
	return result;
}

SunController::SunController() : northDir( 1.0f, 0.0f, 0.0f )
	, sunDir( 0.0f, -1.0f, 0.0f )
	, upDir( 0.0f, 1.0f, 0.0f )
	, latitude( 50.0f )
	, month( June )
	, eclipticObliquity( DEG2RAD( 23.4f ) )
	, delta( 0.0f )
{
}

void SunController::Update( float _time )
{
	CalculateSunOrbit();
	UpdateSunPosition( _time - 12.0f );
}

void SunController::CalculateSunOrbit()
{
	const float day = 30.0f * static_cast<float>( month ) + 15.0f;
	float lambda = 280.46f + 0.9856474f * day;
	lambda = DEG2RAD( lambda );
	delta = idMath::ASin( idMath::Sin( eclipticObliquity ) * idMath::Sin( lambda ) );
}

void SunController::UpdateSunPosition( float hour )
{
	const float latitudeRad = DEG2RAD( latitude );
	const float hh = hour * idMath::PI / 12.0f;
	const float azimuth = idMath::ATan(
							  idMath::Sin( hh )
							  , idMath::Cos( hh ) * idMath::Sin( latitudeRad ) - idMath::Tan( delta ) * idMath::Cos( latitudeRad )
						  );

	const float altitude = idMath::ASin(
							   idMath::Sin( latitudeRad ) * idMath::Sin( delta ) + idMath::Cos( latitudeRad ) * idMath::Cos( delta ) * idMath::Cos( hh )
						   );

	const idQuat rot0 = FromAxisAngle( upDir, -azimuth );
	const idVec3 dir = northDir * rot0;
	const idVec3 uxd = upDir.Cross( dir );

	const idQuat rot1 = FromAxisAngle( uxd, altitude );
	sunDir = dir * rot1;
}

idQuat SunController::FromAxisAngle( idVec3 axis, const float angle )
{
	const float ha = angle * 0.5f;
	const float sa = idMath::Sin( ha );

	return
	{
		axis.x * sa,
		axis.y * sa,
		axis.z * sa,
		idMath::Cos( ha ),
	};
}


static void ComputePerezCoeff( const float turbidity, float* outPerezCoeff )
{
	for( uint32_t ii = 0; ii < 5; ++ii )
	{
		const idVec3 tmp = ( ABCDE_t[ii] * turbidity ) + ABCDE[ii];
		float* out = outPerezCoeff + 4 * ii;
		std::memcpy( out, &tmp, sizeof( idVec3 ) );
		out[3] = 0.0f;
	}
}

CLASS_DECLARATION( idEntity, idSky )
EVENT( EV_PostSpawn, idSky::PostSpawn )
END_CLASS

static DynamicValueController	sunLuminanceXyz;
static DynamicValueController	skyLuminanceXyz;
static SunController			sun;

const int idSky::version = 1;

idSky::idSky()
	: month( 5 )
	, time( 12.0f )
	, latitude( 50.0f )
	, turbidity( 2.150f )
	, sunSize( 0.2f )
	, sunBloom( 3.0f )
	, exposition( 0.1f )
{
}

idSky::~idSky()
{
	gameRenderWorld->FreeSkyDef( skyHandle );
}

void idSky::Spawn()
{
	month = spawnArgs.GetInt( "month", 5 );
	time = spawnArgs.GetFloat( "time", 12.5 );
	latitude = spawnArgs.GetFloat( "latitude", 50.0f );
	turbidity = spawnArgs.GetFloat( "turbidity", 2.150f );
	sunSize = spawnArgs.GetFloat( "sunSize", .02f );
	sunBloom = spawnArgs.GetFloat( "sunBloom", 3.0f );
	exposition = spawnArgs.GetFloat( "exposition", 0.1f );

	sunLuminanceXyz.SetMap( sunLuminanceXYZTable );
	skyLuminanceXyz.SetMap( skyLuminanceXYZTable );

	SkyDef params;
	skyHandle = gameRenderWorld->AddSkyDef( &params );

	PostEventMS( &EV_PostSpawn, 0 );
}

void idSky::Save( idSaveGame* saveFile ) const
{
	saveFile->WriteInt( version );
	saveFile->WriteInt( month );
	saveFile->WriteFloat( time );
	saveFile->WriteFloat( latitude );
	saveFile->WriteFloat( turbidity );
	light.Save( saveFile );
}

void idSky::Restore( idRestoreGame* saveFile )
{
	int savedVersion = version;
	saveFile->ReadInt( savedVersion );
	saveFile->ReadInt( month );
	saveFile->ReadFloat( time );
	saveFile->ReadFloat( latitude );
	saveFile->ReadFloat( turbidity );
	light.Restore( saveFile );
}

void idSky::Think()
{
	//time = idMath::Mod( MS2SEC( gameLocal.GetTime() ), 24.0f );
	time = idMath::Mod( time, 24.0f );
	sun.Update( time );

	const Color colorSunLuminanceXYZ = sunLuminanceXyz.GetValue( time );
	const Color colorSunLuminanceRGB = xyzToRgb( colorSunLuminanceXYZ );

	const Color colorSkyLuminanceXYZ = skyLuminanceXyz.GetValue( time );
	//const Color skyLuminanceRgb = xyzToRgb(colorSkyLuminanceXYZ);
	
	SkyDef params;
	params.skyLuminanceXYZ = idVec4( colorSkyLuminanceXYZ.x, colorSkyLuminanceXYZ.y, colorSkyLuminanceXYZ.z, 1.0f );
	params.sunDirection = idVec4( sun.sunDir.x, sun.sunDir.y, sun.sunDir.z, 1.0f );
	params.parameters = idVec4( sunSize, sunBloom, exposition, time );
	ComputePerezCoeff( turbidity, &params.perezCoeff[0].x );

	if( light.IsValid() )
	{
		idLight* l = light.GetEntity();
		l->SetCenter( idVec3( -sun.sunDir.z, sun.sunDir.x, sun.sunDir.y ) );
		l->SetColor( colorSunLuminanceRGB );
	}

	gameRenderWorld->UpdateSkyDef( skyHandle, &params );
}

void idSky::Present()
{
	if( r_skipSky.GetBool() )
	{
		return;
	}

	// Add to the refresh list
	idEntity::Present();
}

void idSky::PostSpawn()
{
	const char* lightName = spawnArgs.GetString( "light", "" );
	if( !lightName )
	{
		return;
	}
	light = dynamic_cast<idLight*>( gameLocal.FindEntity( lightName ) );
	BecomeActive( TH_THINK );
}
