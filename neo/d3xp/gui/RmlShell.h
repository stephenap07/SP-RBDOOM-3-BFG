#ifndef __GUI_RMLSHELL_H__
#define __GUI_RMLSHELL_H__

#ifndef __TYPEINFOGEN__
	#include "../../renderer/RenderCommon.h"
#include "RmlUi/Core.h"
#endif

#include <vector>

class RmlUserInterfaceLocal;

namespace Rml
{
class EventListenerInstancer;
class ElementDocument;
}

enum class ShellState : int
{
	// Got through waiting screen and in start menu
	SHELL_ERROR,
	WAITING,
	START,
	LOADING,
	GAME,
	CREDITS,
	QUIT,
	TOTAL
};

enum class ShellScreen : int
{
	NONE,
	START,
	OPTIONS,
	LOADING,
	GAME,
	TEST,
	PAUSE,
	CREDITS,
	TOTAL
};

class UI_Shell
{
public:

	UI_Shell();

	~UI_Shell();

	bool					Init( const char* filename, idSoundWorld* sw );

	// Called every frame.
	void					Update( );

	void					HandleScreenChange( );

	void					HandleStateChange( );

	void					SetState( ShellState _nextState );

	void					SetupDataBinding( );

	void					ActivateMenu( bool show );

	void					SetNextScreen( ShellScreen _nextScreen );

	void					SetNextScreen( const char* _nextScreen );

	void					ShowScreen( const char* _screen );

	void					HideScreen( const char* _screen );

	void					UpdateSavedGames( );

	int						FindVidModeIndex( int windowSizeIndex, int displayIndex ) const;

	RmlUserInterface*		Ui()
	{
		return ui;
	}

	void					SetGameCompleted( bool completed )
	{
		gameComplete = completed;
	}

	bool					GetGameComplete( )
	{
		return gameComplete;
	}

	void					SetInGame( bool _inGame )
	{
		inGame = _inGame;
	}

	bool					InGame( )
	{
		return inGame;
	}

	bool					IsPausingGame( );

	ShellState				State( ) const
	{
		return state;
	}

private:

	// The container of the ui.
	RmlUserInterface*			ui;
	idSoundWorld*				soundWorld;

	ShellState					nextState;
	ShellState					state;

	ShellScreen					activeScreen;
	ShellScreen					nextScreen;

	idStr						nextScreenName;

	bool						isInitialized;
	bool						gameComplete;
	bool						inGame;

	idList<idStrStatic<128>>	screenToName;

	Rml::DataModelHandle		vidModeModel;
	std::vector<vidMode_t>		modeList;
};

#endif
