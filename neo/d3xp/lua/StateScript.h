#pragma once

class idStateScript
{
public:
	idStateScript( idEntity* _owner );

	void SetName( const char* name )
	{
		scriptName = name;
	}

	virtual ~idStateScript() {}

	void Construct();

	void Destroy();

	void Think();

	void Reload();

	void SendEvent( int entityNumber, const char* eventName );

	const char* GetName()
	{
		return scriptName.c_str();
	}

private:

	idEntity*	owner;
	idStr		scriptName;
};