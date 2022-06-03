#ifndef __GAME_PORTAL_H__
#define __GAME_PORTAL_H__

struct ProximityEntity
{
	idEntityPtr<idEntity>	entity;
	idVec3					lastPortalPoint;
};


/**
 * \brief Object to display Prey style portals. A lot of this was inspired from the Prey SDK.
 */
class spGamePortal : public idEntity
{
public:
	CLASS_PROTOTYPE( spGamePortal );

	enum
	{
		PORTAL_OPENING,
		PORTAL_OPENED,
		PORTAL_CLOSING,
		PORTAL_CLOSED,
	};

	spGamePortal();
	~spGamePortal() override;

	void					Spawn();
	void					Save( idSaveGame* savefile ) const;
	void					Restore( idRestoreGame* savefile );

	void					ClientPredictionThink() override;

	void					CheckPlayerDistances();
	void					Think() override;

	[[nodiscard]] bool		IsActive() const
	{
		return portalState < PORTAL_CLOSING;
	}

	void					AddProximityEntity( const idEntity* other );

private:

	qhandle_t							areaPortal;	//!< 0 = no portal
	int									portalState;	//!< Current portal state
	bool								noTeleport;	//!< Is purely a visual portal.  Will not try to teleport anything near it
	bool								flipX;			//!< If this is true, it flips the teleportation mechanism so the player is facing opposite face of the surface.

	idEntityPtr<spGamePortal>			partner;
	idList<ProximityEntity>		        proximityEntities;
	idList<idEntityPtr<idEntity> >      ignoredEntities;
};


#endif