#ifndef __GAME_PORTAL_H__
#define __GAME_PORTAL_H__

struct ProximityEntity
{
	idEntityPtr<idEntity>	entity;
	idVec3					lastPortalPoint;
};


/// I got some of this code from the prey SDK.
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

	virtual					~spGamePortal();

	void					Spawn( void );
	void					Save( idSaveGame* savefile ) const;
	void					Restore( idRestoreGame* savefile );

	virtual void	ClientPredictionThink( void );

	void			CheckPlayerDistances( void );
	void			Think( void );
	bool			IsActive( void )
	{
		return( portalState < PORTAL_CLOSING );
	}

	void			AddProximityEntity( const idEntity* other );

protected:

	// Methods used to sync up portals
	spGamePortal*   GetMasterPortal() const
	{
		return( masterPortal.GetEntity() );
	}
	void			SetMasterPortal( spGamePortal* master )
	{
		masterPortal = master;
	}
	void			AddSlavePortal( const spGamePortal* slave )
	{
		idEntityPtr<spGamePortal> entPtr;
		entPtr = slave;
		slavePortals.Append( entPtr );
	}

private:

	qhandle_t			areaPortal;		// 0 = no portal
	int					portalState;
	bool				bNoTeleport;    // Is purely a visual portal.  Will not try to teleport anything near it

	// If this is true, it flips the teleportation mechanism so the player is facing
	// opposite face of the surface.
	bool                flipX;

	idList<idEntityPtr<spGamePortal> >	slavePortals;
	idEntityPtr<spGamePortal>			masterPortal;
	idList<ProximityEntity>		        proximityEntities;
	idList<idEntityPtr<idEntity> >      ignoredEntities;
};


#endif