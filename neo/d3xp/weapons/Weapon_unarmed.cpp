// weapon_fist.cpp
//

#pragma hdrstop
#include "precompiled.h"
#include "../Game_local.h"

CLASS_DECLARATION( rvmWeaponObject, idWeaponUnarmed )
END_CLASS

#define RIFLE_NUMPROJECTILES	1

// blend times
#define FISTS_IDLE_TO_LOWER		4
#define FISTS_IDLE_TO_PUNCH		0
#define FISTS_RAISE_TO_IDLE		4
#define FISTS_PUNCH_TO_IDLE		1


/*
================
idWeaponUnarmed::Init
================
*/
void idWeaponUnarmed::Init( idWeapon* weapon )
{
	rvmWeaponObject::Init( weapon );
}

/*
================
idWeaponUnarmed::Raise
================
*/
stateResult_t idWeaponUnarmed::Raise( stateParms_t* parms )
{
	return SRESULT_DONE;
}

/*
================
idWeaponUnarmed::Lower
================
*/
stateResult_t idWeaponUnarmed::Lower( stateParms_t* parms )
{
	SetState("Holstered");
	return SRESULT_DONE;
}

/*
================
idWeaponUnarmed::Idle
================
*/
stateResult_t idWeaponUnarmed::Idle( stateParms_t* parms )
{
	return SRESULT_DONE;
}

/*
================
idWeaponUnarmed::Fire
================
*/
stateResult_t idWeaponUnarmed::Fire( stateParms_t* parms )
{
	common->Printf("Hello");
	return SRESULT_DONE;
}

/*
================
idWeaponUnarmed::Reload
================
*/
stateResult_t idWeaponUnarmed::Reload( stateParms_t* parms )
{
	return SRESULT_DONE;
}