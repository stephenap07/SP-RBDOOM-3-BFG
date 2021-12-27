// Weapon_rocketlauncher.cpp
//

#pragma hdrstop
#include "precompiled.h"
#include "../Game_local.h"

CLASS_DECLARATION( rvmWeaponObject, idWeaponRocketLauncher )
END_CLASS

#define ROCKETLAUNCHER_FIRERATE			0.75
#define ROCKETLAUNCHER_LOWAMMO			2
#define ROCKETLAUNCHER_RELOADRATE		2
#define	ROCKETLAUNCHER_NUMPROJECTILES	13

// blend times
#define ROCKETLAUNCHER_IDLE_TO_IDLE		0
#define ROCKETLAUNCHER_IDLE_TO_LOWER	4
#define ROCKETLAUNCHER_IDLE_TO_FIRE		4
#define	ROCKETLAUNCHER_IDLE_TO_RELOAD	4
#define	ROCKETLAUNCHER_IDLE_TO_NOAMMO	4
#define ROCKETLAUNCHER_NOAMMO_TO_RELOAD 4
#define ROCKETLAUNCHER_NOAMMO_TO_IDLE	4
#define ROCKETLAUNCHER_RAISE_TO_IDLE	1
#define ROCKETLAUNCHER_FIRE_TO_IDLE		8
#define ROCKETLAUNCHER_RELOAD_TO_IDLE	4
#define	ROCKETLAUNCHER_RELOAD_TO_FIRE	4
#define ROCKETLAUNCHER_RELOAD_TO_LOWER	2

/*
===============
idWeaponRocketLauncher::Init
===============
*/
void idWeaponRocketLauncher::Init( idWeapon* weapon )
{
	rvmWeaponObject::Init( weapon );

	next_attack = 0;
	spread = weapon->GetFloat( "spread" );
	snd_lowammo = FindSound( "snd_lowammo" );
}

/*
===============
idWeaponRocketLauncher::Raise
===============
*/
stateResult_t idWeaponRocketLauncher::Raise( stateParms_t* parms )
{
	enum RisingState
	{
		RISING_NOTSET = 0,
		RISING_WAIT
	};

	switch( parms->stage )
	{
	case RISING_NOTSET:
		owner->Event_PlayAnim( ANIMCHANNEL_ALL, "raise", false );
		parms->stage = RISING_WAIT;
		return SRESULT_WAIT;

	case RISING_WAIT:
		if( owner->Event_AnimDone( ANIMCHANNEL_ALL, ROCKETLAUNCHER_RAISE_TO_IDLE ) )
		{
			return SRESULT_DONE;
		}
		return SRESULT_WAIT;
	}

	return SRESULT_ERROR;
}


/*
===============
idWeaponRocketLauncher::Lower
===============
*/
stateResult_t idWeaponRocketLauncher::Lower( stateParms_t* parms )
{
	enum LoweringState
	{
		LOWERING_NOTSET = 0,
		LOWERING_WAIT
	};

	switch( parms->stage )
	{
	case LOWERING_NOTSET:
		owner->Event_PlayAnim( ANIMCHANNEL_ALL, "putaway", false );
		parms->stage = LOWERING_WAIT;
		return SRESULT_WAIT;

	case LOWERING_WAIT:
		if( owner->Event_AnimDone( ANIMCHANNEL_ALL, 0 ) )
		{
			SetState( "Holstered" );
			return SRESULT_DONE;
		}
		return SRESULT_WAIT;
	}

	return SRESULT_ERROR;
}

/*
===============
idWeaponRocketLauncher::Idle
===============
*/
stateResult_t idWeaponRocketLauncher::Idle( stateParms_t* parms )
{
	enum IdleState
	{
		IDLE_NOTSET = 0,
		IDLE_WAIT
	};

	switch( parms->stage )
	{
	case IDLE_NOTSET:
		owner->Event_WeaponReady( );
		owner->Event_PlayCycle( ANIMCHANNEL_ALL, "idle" );
		parms->stage = IDLE_WAIT;
		return SRESULT_WAIT;

	case IDLE_WAIT:
		// Do nothing.
		return SRESULT_DONE;
	}

	return SRESULT_ERROR;
}
/*
===============
idWeaponRocketLauncher::Fire
===============
*/
stateResult_t idWeaponRocketLauncher::Fire( stateParms_t* parms )
{
	int ammoClip = owner->AmmoInClip( );

	enum FIRE_State
	{
		FIRE_NOTSET = 0,
		FIRE_WAIT
	};

	if( ammoClip == 0 && owner->AmmoAvailable( ) && parms->stage == 0 )
	{
		//owner->WeaponState( WP_RELOAD, PISTOL_IDLE_TO_RELOAD );
		owner->Reload( );
		return SRESULT_DONE;
	}

	switch( parms->stage )
	{
	case FIRE_NOTSET:
		next_attack = gameLocal.realClientTime + SEC2MS( ROCKETLAUNCHER_FIRERATE );

		if( ammoClip == ROCKETLAUNCHER_LOWAMMO )
		{
			int length;
			owner->StartSoundShader( snd_lowammo, SND_CHANNEL_ITEM, 0, false, &length );
		}

		owner->Event_LaunchProjectiles( ROCKETLAUNCHER_NUMPROJECTILES, spread, 0, 1, 1 );

		owner->Event_PlayAnim( ANIMCHANNEL_ALL, "fire", false );
		parms->stage = FIRE_WAIT;

		return SRESULT_WAIT;

	case FIRE_WAIT:
		if( owner->Event_AnimDone( ANIMCHANNEL_ALL, ROCKETLAUNCHER_FIRE_TO_IDLE ) )
		{
			return SRESULT_DONE;
		}
		return SRESULT_WAIT;
	}

	return SRESULT_ERROR;
}

/*
===============
idWeaponRocketLauncher::Reload
===============
*/
stateResult_t idWeaponRocketLauncher::Reload( stateParms_t* parms )
{
	float ammoClip;
	float ammoAvail;
	float clip_size;

	clip_size = owner->ClipSize( );

	enum RELOAD_State
	{
		RELOAD_NOTSET = 0,
		RELOAD_WAIT,
		RELOAD_END
	};

	ammoAvail = owner->AmmoAvailable( );
	ammoClip = owner->AmmoInClip( );

	switch( parms->stage )
	{
	case RELOAD_NOTSET:

		if( ammoClip < clip_size )
		{
			owner->Event_PlayAnim( ANIMCHANNEL_ALL, "reload", false );
			parms->stage = RELOAD_WAIT;
			return SRESULT_WAIT;
		}

	case RELOAD_WAIT:
		if( owner->Event_AnimDone( ANIMCHANNEL_ALL, 0 ) )
		{
			if( ( ammoClip < clip_size ) && ( ammoClip < ammoAvail ) )
			{
				parms->stage = RELOAD_NOTSET;
				owner->Event_AddToClip( ROCKETLAUNCHER_RELOADRATE );
				return SRESULT_WAIT;
			}

			owner->Event_AddToClip( owner->ClipSize( ) );
			return SRESULT_DONE;
		}
		return SRESULT_WAIT;

	case RELOAD_END:
		if( owner->Event_AnimDone( ANIMCHANNEL_ALL, 0 ) )
		{
			return SRESULT_DONE;
		}
		return SRESULT_WAIT;
	}


	return SRESULT_ERROR;
}