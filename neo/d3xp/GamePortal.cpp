#include "precompiled.h"
#pragma hdrstop

#include "Game_local.h"

#include <lua.hpp>
#include <luaconf.h>

const idEventDef EV_Opened( "<portalopened>", NULL );
const idEventDef EV_Closed( "<portalclosed>", NULL );
const idEventDef EV_PortalSpark( "<portalSpark>", NULL );
const idEventDef EV_PortalSparkEnd( "<portalSparkEnd>", NULL );
const idEventDef EV_ShowGlowPortal( "showGlowPortal", NULL );
const idEventDef EV_HideGlowPortal( "hideGlowPortal", NULL );
const idEventDef EV_ResetGravity( "resetGravityPortal", NULL );

CLASS_DECLARATION( idEntity, spGamePortal )
END_CLASS

spGamePortal::spGamePortal(): _areaPortal( 0 ), _portalState( 0 ), _noTeleport( false ), _flipX( false )
{
}

spGamePortal::~spGamePortal()
{
}

void spGamePortal::Spawn()
{
	idBounds bounds;

	_noTeleport = spawnArgs.GetBool( "noTeleport" );
	_flipX = spawnArgs.GetBool( "flipX", true );

	if( const char* partnerName = spawnArgs.GetString( "cameratarget" ) )
	{
		idEntity* ent = gameLocal.FindEntity( partnerName );
		if( ent && ent->IsType( Type ) )
		{
			_partner = static_cast<spGamePortal*>( ent );
		}
	}

	if( !_partner )
	{
		gameLocal.Warning( "Game portal %s has no partner", name.c_str() );
	}

	_areaPortal = gameRenderWorld->FindPortal( GetPhysics()->GetAbsBounds().Expand( 32.0f ) );

	if( _noTeleport )
	{
		GetPhysics()->SetContents( 0 );
	}
	else
	{
		GetPhysics()->SetContents( CONTENTS_TRIGGER );
	}

	// Setup the initial state for the portal
	if( spawnArgs.GetBool( "startActive" ) ) // Start Active
	{
		_portalState = PORTAL_OPENED;
	}
	else   // Start closed
	{
		Hide();
		_portalState = PORTAL_CLOSED;
		GetPhysics()->SetContents( 0 ); // rww - if closed, ensure things will not hit and go through
	}

	int pState = ( _portalState == PORTAL_OPENED ) ? PS_BLOCK_NONE : PS_BLOCK_ALL;
	gameRenderWorld->SetPortalState( _areaPortal, pState );

	BecomeActive( TH_THINK | TH_UPDATEVISUALS );

	UpdateVisuals();

	PostEventMS( &EV_PostSpawn, 0 );

	fl.networkSync = true; // rww
	_proximityEntities.Clear(); // Clear the list of potential entities to be portalled
}

void spGamePortal::Save( idSaveGame* savefile_ ) const
{
}

void spGamePortal::Restore( idRestoreGame* savefile_ )
{
}

void spGamePortal::ClientPredictionThink()
{
	Think();
}

void spGamePortal::CheckPlayerDistances()
{
}

static void PortalRotate( idVec3& vec_, const idMat3& sourceTranspose_, const idMat3& dest_, const bool flipX_ )
{
	vec_ *= sourceTranspose_;
	vec_.y *= -1;
	if( flipX_ )
	{
		vec_.x *= -1;
	}
	vec_ *= dest_;
}

void spGamePortal::Think()
{
	int				i;
	idPlane			plane;

	idEntity::Think();

	if( _portalState == PORTAL_CLOSED || _noTeleport )
	{
		return;
	}

	// Force visuals to update until a remote renderview has been created for this portal
	if( !renderEntity.remoteRenderView )
	{
		BecomeActive( TH_UPDATEVISUALS );
	}

	if( _partner )
	{
		gameRenderWorld->SetPortalState( _partner->_areaPortal, PS_BLOCK_NONE );
	}

	if( _noTeleport )
	{
		return;
	}

	idBounds bounds = GetPhysics()->GetClipModel()->GetBounds();
	bounds.RotateSelf( GetPhysics()->GetAxis() );
	bounds.TranslateSelf( GetPhysics()->GetOrigin() );
	bounds.ExpandSelf( 1.0f );

	idEntity* entityList[MAX_GENTITIES];
	const int numListedEntities = gameLocal.clip.EntitiesTouchingBounds( bounds, -1, entityList, MAX_GENTITIES );
	for( int i = 0; i < numListedEntities; i++ )
	{
		idEntity* ent = entityList[i];
		if( ent != this && !ent->IsHidden() && !ent->IsType( idWorldspawn::Type ) )
		{
			idEntityPtr<idEntity> traceEntPtr;
			traceEntPtr = ent;
			if( !_ignoredEntities.Find( traceEntPtr ) )
			{
				AddProximityEntity( ent );
			}
		}
	}

	_ignoredEntities.Clear();

	for( int i = 0; i < numListedEntities; i++ )
	{
		idEntity* ent = entityList[i];
		if( ent != this && !ent->IsHidden() && !ent->IsType( idWorldspawn::Type ) )
		{
			idEntityPtr<idEntity> traceEntPtr;
			traceEntPtr = ent;

			_ignoredEntities.AddUnique( traceEntPtr );

			//if( cameraTarget )
			//{
			//	auto targetPortal = static_cast<spGamePortal*>( cameraTarget );
			//}
		}
	}

	// Bit of a hack for noclipping players:  If they are close to the portal, then add them to the proximity list automatically
	//idPlayer* player = gameLocal.GetLocalPlayer();
	//if (player && player->noclip) { //rww - note that the local player is NULL for dedicated servers.
	//	if ((player->GetPhysics()->GetOrigin() - GetPhysics()->GetOrigin()).LengthFast() < kDistanceToPlayer) {
	//		AddProximityEntity(player);
	//	}
	//}

	for( i = 0; i < _proximityEntities.Num(); i++ )
	{
		if( !_proximityEntities[i].entity.IsValid() )
		{
			// Remove this entity from the list
			_proximityEntities.RemoveIndex( i );
			continue;
		}

		idEntity* hit = _proximityEntities[i].entity.GetEntity();

		if( cameraTarget && cameraTarget->IsType( spGamePortal::Type ) )
		{
			idMat3 sourceAxis = GetPhysics()->GetAxis().Transpose();
			idMat3 destAxis = cameraTarget->GetPhysics()->GetAxis();

			// Compute new location
			idVec3 point = hit->GetPhysics()->GetOrigin();

			int side = plane.Side( point );
			if( side == PLANESIDE_ON || side == PLANESIDE_CROSS )
			{
				side = PLANESIDE_BACK;
			}

			idVec3 nextLocation = hit->GetPhysics()->GetOrigin() + hit->GetPhysics()->GetLinearVelocity();
			int nextSide = plane.Side( nextLocation );
			if( nextSide == PLANESIDE_ON || nextSide == PLANESIDE_CROSS )
			{
				nextSide = PLANESIDE_BACK;
			}

			if( side == PLANESIDE_BACK && nextSide == PLANESIDE_FRONT )
			{
				continue;
			}

			idVec3 newLocation = ( GetPhysics()->GetOrigin() - point ) * sourceAxis;
			newLocation.z = -newLocation.z;
			newLocation = cameraTarget->GetPhysics()->GetOrigin() + ( newLocation * destAxis );

			// Compute new axis
			idMat3 newEntAxis = hit->GetPhysics()->GetAxis();
			if( hit->IsType( idPlayer::Type ) )
			{
				newEntAxis = static_cast<idPlayer*>( hit )->firstPersonViewAxis;
			}

			const idAngles angleDiff = GetPhysics()->GetAxis().ToAngles() - newEntAxis.ToAngles();
			idAngles origAngle = destAxis.ToAngles();
			origAngle.yaw = origAngle.yaw - angleDiff.yaw;
			origAngle.pitch = origAngle.pitch - angleDiff.pitch;
			origAngle.yaw -= 180;
			origAngle.Normalize180();

			idVec3 vel = hit->GetPhysics()->GetLinearVelocity();
			PortalRotate( vel, sourceAxis, destAxis, _flipX );
			hit->GetPhysics()->SetLinearVelocity( vel );

			if( hit->IsType( idPlayer::Type ) )
			{
				auto player = static_cast<idPlayer*>( hit );
				//player->Teleport(newLocation, origAngle, nullptr);
				player->SetOrigin( newLocation + idVec3( 0, 0, CM_CLIP_EPSILON ) );
				player->SetViewAngles( origAngle );
				player->CalculateFirstPersonView();
				player->UpdateVisuals();
				//player->GetPhysics()->SetLinearVelocity(vel);
			}
			else
			{
				hit->SetOrigin( newLocation + idVec3( 0, 0, CM_CLIP_EPSILON ) );
				hit->SetAngles( origAngle );
			}

			_proximityEntities.RemoveIndex( i );
		}
	}
}

void spGamePortal::AddProximityEntity( const idEntity* other_ )
{
	// Go through the list and guarantee that this entity isn't in multiple times
// note:  cannot use IdList::AddUnique, because the lastPortalPoint might be different during this add
	for( int i = 0; i < _proximityEntities.Num(); i++ )
	{
		if( _proximityEntities[i].entity.GetEntity() == other_ )
		{
			return;
		}
	}

	// Add this entity to the potential portal list
	ProximityEntity prox;
	prox.entity = other_;
	prox.lastPortalPoint = other_->GetPhysics()->GetOrigin();

	_proximityEntities.Append( prox );

	// If the entity is a player, then inform the player that they are close to this portal
	// needed for weapon projectile firing
	if( other_->IsType( idPlayer::Type ) )
	{
		auto player = ( idPlayer* )( other_ );
		//player->SetPortalColliding(true);
	}
}
