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
	void					Save( idSaveGame* savefile_ ) const;
	void					Restore( idRestoreGame* savefile_ );

	void					ClientPredictionThink() override;

	void					CheckPlayerDistances();
	void					Think() override;

	[[nodiscard]] bool		IsActive() const
	{
		return _portalState < PORTAL_CLOSING;
	}

	void					AddProximityEntity( const idEntity* other_ );

private:

	qhandle_t							_areaPortal;	//!< 0 = no portal
	int									_portalState;	//!< Current portal state
	bool								_noTeleport;	//!< Is purely a visual portal.  Will not try to teleport anything near it
	bool								_flipX;			//!< If this is true, it flips the teleportation mechanism so the player is facing opposite face of the surface.

	idEntityPtr<spGamePortal>			_partner;
	idList<ProximityEntity>		        _proximityEntities;
	idList<idEntityPtr<idEntity> >      _ignoredEntities;
};


#endif