// Weapon_shotgun.cpp
//

#pragma hdrstop
#include "precompiled.h"
#include "../Game_local.h"

CLASS_DECLARATION( rvmWeaponObject, idWeaponShotgun )
END_CLASS

#define SHOTGUN_FIRERATE		1.333
#define SHOTGUN_LOWAMMO			2
#define SHOTGUN_RELOADRATE		2
#define	SHOTGUN_NUMPROJECTILES	13

// blend times
#define SHOTGUN_IDLE_TO_IDLE	0
#define SHOTGUN_IDLE_TO_LOWER	4
#define SHOTGUN_IDLE_TO_FIRE	1
#define	SHOTGUN_IDLE_TO_RELOAD	4
#define	SHOTGUN_IDLE_TO_NOAMMO	4
#define SHOTGUN_NOAMMO_TO_RELOAD 4
#define SHOTGUN_NOAMMO_TO_IDLE	4
#define SHOTGUN_RAISE_TO_IDLE	1
#define SHOTGUN_FIRE_TO_IDLE	4
#define SHOTGUN_RELOAD_TO_IDLE	4
#define	SHOTGUN_RELOAD_TO_FIRE	4
#define SHOTGUN_RELOAD_TO_LOWER 2

/*
===============
idWeaponShotgun::Init
===============
*/
void idWeaponShotgun::Init( idWeapon* weapon )
{
	rvmWeaponObject::Init( weapon );

	next_attack = 0;
	spread = weapon->GetFloat( "spread" ); // weapon->GetFloat("spread")
	snd_lowammo = FindSound( "snd_lowammo" );
}

/*
===============
idWeaponShotgun::Raise
===============
*/
stateResult_t idWeaponShotgun::Raise( stateParms_t* parms )
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
			if( owner->Event_AnimDone( ANIMCHANNEL_ALL, SHOTGUN_RAISE_TO_IDLE ) )
			{
				return SRESULT_DONE;
			}
			return SRESULT_WAIT;
	}

	return SRESULT_ERROR;
}


/*
===============
idWeaponShotgun::Lower
===============
*/
stateResult_t idWeaponShotgun::Lower( stateParms_t* parms )
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
idWeaponShotgun::Idle
===============
*/
stateResult_t idWeaponShotgun::Idle( stateParms_t* parms )
{
	enum IdleState
	{
		IDLE_NOTSET = 0,
		IDLE_WAIT
	};

	switch( parms->stage )
	{
		case IDLE_NOTSET:
			owner->Event_WeaponReady();
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
idWeaponShotgun::Fire
===============
*/
stateResult_t idWeaponShotgun::Fire( stateParms_t* parms )
{
	int ammoClip = owner->AmmoInClip();

	enum FIRE_State
	{
		FIRE_NOTSET = 0,
		FIRE_WAIT
	};

	if( ammoClip == 0 && owner->AmmoAvailable() && parms->stage == 0 )
	{
		//owner->WeaponState( WP_RELOAD, PISTOL_IDLE_TO_RELOAD );
		owner->Reload();
		return SRESULT_DONE;
	}

	switch( parms->stage )
	{
		case FIRE_NOTSET:
			next_attack = gameLocal.realClientTime + SEC2MS( SHOTGUN_FIRERATE );

			if( ammoClip == SHOTGUN_LOWAMMO )
			{
				int length;
				owner->StartSoundShader( snd_lowammo, SND_CHANNEL_ITEM, 0, false, &length );
			}

			owner->Event_LaunchProjectiles( SHOTGUN_NUMPROJECTILES, spread, 0, 1, 1 );

			owner->Event_PlayAnim( ANIMCHANNEL_ALL, "fire", false );
			parms->stage = FIRE_WAIT;

			return SRESULT_WAIT;

		case FIRE_WAIT:
			if( owner->Event_AnimDone( ANIMCHANNEL_ALL, SHOTGUN_FIRE_TO_IDLE ) )
			{
				return SRESULT_DONE;
			}
			return SRESULT_WAIT;
	}

	return SRESULT_ERROR;
}

/*
===============
idWeaponShotgun::Reload
===============
*/
stateResult_t idWeaponShotgun::Reload( stateParms_t* parms )
{
	float ammoClip;
	float ammoAvail;
	float clip_size;

	clip_size = owner->ClipSize();

	enum RELOAD_State
	{
		RELOAD_NOTSET = 0,
		RELOAD_WAIT,
		RELOAD_END
	};

	ammoAvail = owner->AmmoAvailable();
	ammoClip = owner->AmmoInClip();

	switch( parms->stage )
	{
		case RELOAD_NOTSET:

			if (ammoClip < clip_size)
			{
				owner->Event_PlayAnim(ANIMCHANNEL_ALL, "reload_loop", false);
				parms->stage = RELOAD_WAIT;
				return SRESULT_WAIT;
			}
			else
			{
				owner->Event_PlayAnim(ANIMCHANNEL_ALL, "reload_end", false);
				parms->stage = RELOAD_END;
				return SRESULT_WAIT;
			}

		case RELOAD_WAIT:
			if( owner->Event_AnimDone( ANIMCHANNEL_ALL, 0 ) )
			{
				if( ( ammoClip < clip_size ) && ( ammoClip < ammoAvail ) )
				{
					parms->stage = RELOAD_NOTSET;
					owner->Event_AddToClip( SHOTGUN_RELOADRATE );
					return SRESULT_WAIT;
				}
				else
				{
					parms->stage = RELOAD_END;
					owner->Event_PlayAnim( ANIMCHANNEL_ALL, "reload_end", false );
					return SRESULT_WAIT;
				}
				owner->Event_AddToClip( owner->ClipSize() );
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