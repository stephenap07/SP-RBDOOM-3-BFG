#ifndef STATESCRIPT_H_
#define STATESCRIPT_H_

/**
 * \class spStateScript
 * Constructs the lua script for an entity and acts as the main communication mechanism between entity and script.
 */
class spStateScript
{
public:
	spStateScript( idEntity* owner );

	virtual		~spStateScript() = default;

	void		Construct();

	void		Destroy();

	void		Think();

	void		Reload();

	void		SendEvent( int entityNumber, const char* eventName );

	void		SetName( const char* name );
	const char* GetName() const;

private:

	idEntity*	owner;			//!< Owner of the script
	idStr		scriptName;		//!< Name of the script
};

#endif