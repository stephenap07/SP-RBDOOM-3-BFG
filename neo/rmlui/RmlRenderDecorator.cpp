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

#include "precompiled.h"
#pragma hdrstop

#include "RmlRenderDecorator.h"
#include "../renderer/RenderCommon.h"

#include "RmlUi/Core/Element.h"
#include <RmlUi/Core/PropertyDefinition.h>

class SceneRenderHelper
{
public:
	SceneRenderHelper( const char* name, idVec3 rot, idVec3 origin )
	{
		world = renderSystem->AllocRenderWorld();
		needsRender = true;
		lightOrigin = idVec4( -128.0f, 0.0f, 0.0f, 1.0f );
		lightColor = idVec4( 1.0f, 1.0f, 1.0f, 1.0f );
		modelOrigin = idVec4( origin.x, origin.y, origin.z, 0 );
		modelRotate = idVec4( rot.x, rot.y, rot.z, 0 );
		viewOffset = idVec4( -128.0f, 0.0f, 0.0f, 1.0f );
		modelAnim = NULL;
		animLength = 0;
		animEndTime = -1;
		modelDef = -1;
		updateAnimation = true;

		// TODO: Make loadable through rml html
		modelName = name;
	}

	~SceneRenderHelper()
	{
		renderSystem->FreeRenderWorld( world );
	}

	void Draw( int time, idVec4 drawRect )
	{
		PreRender();
		Render( time );

		memset( &refdef, 0, sizeof( refdef ) );
		refdef.vieworg = viewOffset.ToVec3();;
		//refdef.vieworg.Set(-128, 0, 0);

		refdef.viewaxis.Identity();
		refdef.shaderParms[0] = 1;
		refdef.shaderParms[1] = 1;
		refdef.shaderParms[2] = 1;
		refdef.shaderParms[3] = 1;

		refdef.fov_x = 90;
		refdef.fov_y = 2 * atan( ( float )drawRect.w / drawRect.z ) * idMath::M_RAD2DEG;

		refdef.time[0] = time;
		refdef.time[1] = time;

		tr.CropRenderSize( drawRect.x, drawRect.y, drawRect.z, drawRect.w, true );
		world->RenderScene( &refdef );
		tr.UnCrop( );
	}

private:

	void PreRender()
	{
		// Call once
		if( needsRender )
		{
			world->InitFromMap( NULL );

			// Spawn the light
			idDict spawnArgs;
			spawnArgs.Set( "classname", "light" );
			spawnArgs.Set( "name", "light_1" );
			spawnArgs.Set( "origin", lightOrigin.ToVec3().ToString() );
			spawnArgs.Set( "_color", lightColor.ToVec3().ToString() );
			gameEdit->ParseSpawnArgsToRenderLight( &spawnArgs, &rLight );
			lightDef = world->AddLightDef( &rLight );

			// Spawn the model
			if( !modelName[0] )
			{
				common->Warning( "No model set for rml decorator" );
			}
			memset( &worldEntity, 0, sizeof( worldEntity ) );
			spawnArgs.Clear();
			spawnArgs.Set( "classname", "func_static" );
			spawnArgs.Set( "model", modelName );
			spawnArgs.Set( "origin", modelOrigin.ToVec3().ToString() );
			gameEdit->ParseSpawnArgsToRenderEntity( &spawnArgs, &worldEntity );

			if( worldEntity.hModel )
			{
				idVec3 v = modelRotate.ToVec3();
				worldEntity.axis = v.ToMat3();
				worldEntity.shaderParms[0] = 1;
				worldEntity.shaderParms[1] = 1;
				worldEntity.shaderParms[2] = 1;
				worldEntity.shaderParms[3] = 1;
				modelDef = world->AddEntityDef( &worldEntity );
			}

			world->GenerateAllInteractions();
			needsRender = false;
		}
	}

	void Render( int time )
	{
		rLight.origin = lightOrigin.ToVec3();
		rLight.shaderParms[SHADERPARM_RED] = lightColor.x;
		rLight.shaderParms[SHADERPARM_GREEN] = lightColor.y;
		rLight.shaderParms[SHADERPARM_BLUE] = lightColor.z;
		world->UpdateLightDef( lightDef, &rLight );
		if( worldEntity.hModel )
		{
			worldEntity.axis = idAngles( modelRotate.x, modelRotate.y, modelRotate.z ).ToMat3();
			world->UpdateEntityDef( modelDef, &worldEntity );
		}
	}

	renderView_t		refdef;
	idRenderWorld*		world;
	renderEntity_t		worldEntity;
	renderLight_t		rLight;
	const idMD5Anim*	modelAnim;

	qhandle_t	worldModelDef;
	qhandle_t	lightDef;
	qhandle_t   modelDef;
	idStr		modelName;
	idStr		animName;
	idStr		animClass;
	idVec4		lightOrigin;
	idVec4		lightColor;
	idVec4		modelOrigin;
	idVec4		modelRotate;
	idVec4		viewOffset;
	bool		needsRender;
	int			animLength;
	int			animEndTime;
	bool		updateAnimation;
};


idRmlRenderDecorator::idRmlRenderDecorator( const char* name, idVec3 rot, idVec3 origin )
{
	modelName = name;
	modelRotate = rot;
	modelOrigin = origin;
}

idRmlRenderDecorator::~idRmlRenderDecorator()
{
}

Rml::DecoratorDataHandle idRmlRenderDecorator::GenerateElementData( Rml::Element* element ) const
{
	SceneRenderHelper* helper = new SceneRenderHelper( modelName, modelRotate, modelOrigin );
	return Rml::DecoratorDataHandle( helper );
}

void idRmlRenderDecorator::ReleaseElementData( Rml::DecoratorDataHandle element_data ) const
{
	delete reinterpret_cast<SceneRenderHelper*>( element_data );
}

void idRmlRenderDecorator::RenderElement( Rml::Element* element, Rml::DecoratorDataHandle element_data ) const
{
	SceneRenderHelper* helper = reinterpret_cast<SceneRenderHelper*>( element_data );

	Rml::Vector2f pos = element->GetBox().GetPosition( Rml::Box::PADDING );
	Rml::Vector2f box = element->GetBox().GetSize( Rml::Box::PADDING );
	idVec4 drawRect( pos.x, pos.y, box.x, box.y );

	helper->Draw( Sys_Milliseconds(), drawRect );
}

idRmlRenderDecoratorInstancer::idRmlRenderDecoratorInstancer()
	: Rml::DecoratorInstancer()
{
}

idRmlRenderDecoratorInstancer::~idRmlRenderDecoratorInstancer()
{
}

void idRmlRenderDecoratorInstancer::Init()
{
	modelName = RegisterProperty( "model-name", "models/static/Computer.ase" ).AddParser( "string" ).GetId();
	modelRotate = RegisterProperty( "model-rotate", "( 0 0 0 )" ).AddParser( "string" ).GetId();
	modelOrigin = RegisterProperty( "model-origin", "( 0 0 0 )" ).AddParser( "string" ).GetId();
}

Rml::SharedPtr<Rml::Decorator> idRmlRenderDecoratorInstancer::InstanceDecorator( const Rml::String& name, const Rml::PropertyDictionary& properties, const Rml::DecoratorInstancerInterface& instancer_interface )
{
	idStr modelNameStr( properties.GetProperty( modelName )->ToString().c_str() );
	idStr modelRotateStr( properties.GetProperty( modelRotate )->ToString().c_str() );
	idStr modelOriginStr( properties.GetProperty( modelOrigin )->ToString().c_str() );

	idVec3 modelRotVec;
	{
		idParser srcRot;
		srcRot.LoadMemory( modelRotateStr.c_str(), modelRotateStr.Length(), "" );
		srcRot.Parse1DMatrix( 3, &modelRotVec[0] );
	}

	idVec3 modelOriginVec;
	{
		idParser srcRot;
		srcRot.LoadMemory( modelOriginStr.c_str(), modelRotateStr.Length(), "" );
		srcRot.Parse1DMatrix( 3, &modelOriginVec[0] );
	}

	auto decorator = Rml::MakeShared<idRmlRenderDecorator>( modelNameStr.c_str(), modelRotVec, modelOriginVec );
	return decorator;
}