// Weapon_fist.h
//

#pragma once

class idWeaponUnarmed : public rvmWeaponObject
{
public:
	CLASS_PROTOTYPE( idWeaponUnarmed );

	virtual void			Init( idWeapon* weapon );

	stateResult_t			Raise( stateParms_t* parms );
	stateResult_t			Lower( stateParms_t* parms );
	stateResult_t			Idle( stateParms_t* parms );
	stateResult_t			Fire( stateParms_t* parms );
	stateResult_t			Reload( stateParms_t* parms );
};