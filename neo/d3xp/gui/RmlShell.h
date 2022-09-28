#ifndef __GUI_RMLSHELL_H__
#define __GUI_RMLSHELL_H__

#ifndef __TYPEINFOGEN__
	#include "../../renderer/RenderCommon.h"
	#include "RmlUi/Core.h"
#endif

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
	IDLE,
	WAITING,
	START,
	LOADING,
	GAME,
	PAUSED,
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

	bool Init( const char* filename, idSoundWorld* sw );

	// Called every frame.
	void Update();

	void HandleScreenChange();

	void HandleStateChange();

	void SetState( ShellState _nextState );

	void SetupDataBinding();

	void ActivateMenu( bool show );

	void SetNextScreen( const char* nextScreen );

	void ShowScreen( RmlDocHandle handle, bool show );

	void UpdateSavedGames();

	int FindVidModeIndex( int windowSizeIndex, int displayIndex ) const;

	RmlUserInterface* Ui()
	{
		return ui;
	}

	void SetGameCompleted( bool completed )
	{
		gameComplete = completed;
	}

	bool GetGameComplete()
	{
		return gameComplete;
	}

	void SetInGame( bool _inGame )
	{
		inGame = _inGame;
	}

	bool InGame()
	{
		return inGame;
	}

	bool IsPausingGame();

	ShellState State() const
	{
		return state;
	}

private:
	// The container of the ui.
	RmlUserInterface*		ui;
	idSoundWorld*			soundWorld;

	ShellState				nextState;
	ShellState				state;

	idList<RmlDocHandle>	docStack;
	RmlDocHandle			activeDoc;
	RmlDocHandle			nextDoc;

	bool					isInitialized;
	bool					gameComplete;
	bool					inGame;

	EventHandlerOptions		eventHandlerOptions;
	RmlGameEventHandler		baseEventHandler;

	RmlDocHandle			errorDoc;
	RmlDocHandle			waitingDoc;
	RmlDocHandle			startDoc;
	RmlDocHandle			loadingDoc;
	RmlDocHandle			optionsDoc;
	RmlDocHandle			pauseDoc;
	RmlDocHandle			creditsDoc;
	RmlDocHandle			quitDoc;
};

#endif
