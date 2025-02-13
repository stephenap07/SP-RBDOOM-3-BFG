/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2012 Robert Beckebans

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/
/*

Base class for all C++ objects.  Provides fast run-time type checking and run-time
instancing of objects.

*/

#include "precompiled.h"
#pragma hdrstop


#include "../Game_local.h"

#include <lua.hpp>


/***********************************************************************

  idTypeInfo

***********************************************************************/

// this is the head of a singly linked list of all the idTypes
static idTypeInfo*				typelist = NULL;
static idHierarchy<idTypeInfo>	classHierarchy;
static int						eventCallbackMemory	= 0;

/*
================
idTypeInfo::idClassType()

Constructor for class.  Should only be called from CLASS_DECLARATION macro.
Handles linking class definition into class hierarchy.  This should only happen
at startup as idTypeInfos are statically defined.  Since static variables can be
initialized in any order, the constructor must handle the case that subclasses
are initialized before superclasses.
================
*/
idTypeInfo::idTypeInfo( const char* classname, const char* superclass, idEventFunc<idClass>* eventCallbacks, idClass * ( *CreateInstance )(),
						void ( idClass::*Spawn )(), void ( idClass::*Save )( idSaveGame* savefile ) const, void ( idClass::*Restore )( idRestoreGame* savefile ) )
{

	idTypeInfo* type;
	idTypeInfo** insert;

	this->classname			= classname;
	this->superclass		= superclass;
	this->eventCallbacks	= eventCallbacks;
	this->eventMap			= NULL;
	this->Spawn				= Spawn;
	this->Save				= Save;
	this->Restore			= Restore;
	this->CreateInstance	= CreateInstance;
	this->super				= idClass::GetClass( superclass );
	this->freeEventMap		= false;
	typeNum					= 0;
	lastChild				= 0;

	// Check if any subclasses were initialized before their superclass
	for( type = typelist; type != NULL; type = type->next )
	{
		if( ( type->super == NULL ) && !idStr::Cmp( type->superclass, this->classname ) &&
				idStr::Cmp( type->classname, "idClass" ) )
		{
			type->super	= this;
		}
	}

	// Insert sorted
	for( insert = &typelist; *insert; insert = &( *insert )->next )
	{
		assert( idStr::Cmp( classname, ( *insert )->classname ) );
		if( idStr::Cmp( classname, ( *insert )->classname ) < 0 )
		{
			next = *insert;
			*insert = this;
			break;
		}
	}
	if( !*insert )
	{
		*insert = this;
		next = NULL;
	}
}

/*
================
idTypeInfo::~idTypeInfo
================
*/
idTypeInfo::~idTypeInfo()
{
	Shutdown();
}

/*
================
idTypeInfo::Init

Initializes the event callback table for the class.  Creates a
table for fast lookups of event functions.  Should only be called once.
================
*/
void idTypeInfo::Init()
{
	idTypeInfo*				c;
	idEventFunc<idClass>*	def;
	int						ev;
	int						i;
	bool*					set;
	int						num;

	if( eventMap )
	{
		// we've already been initialized by a subclass
		return;
	}

	// make sure our superclass is initialized first
	if( super && !super->eventMap )
	{
		super->Init();
	}

	// add to our node hierarchy
	if( super )
	{
		node.ParentTo( super->node );
	}
	else
	{
		node.ParentTo( classHierarchy );
	}
	node.SetOwner( this );

	// keep track of the number of children below each class
	for( c = super; c != NULL; c = c->super )
	{
		c->lastChild++;
	}

	// if we're not adding any new event callbacks, we can just use our superclass's table
	if( ( !eventCallbacks || !eventCallbacks->event ) && super )
	{
		eventMap = super->eventMap;
		return;
	}

	// set a flag so we know to delete the eventMap table
	freeEventMap = true;

	// Allocate our new table.  It has to have as many entries as there
	// are events.  NOTE: could save some space by keeping track of the maximum
	// event that the class responds to and doing range checking.
	num = idEventDef::NumEventCommands();
	eventMap = new( TAG_SYSTEM ) eventCallback_t[ num ];
	memset( eventMap, 0, sizeof( eventCallback_t ) * num );
	eventCallbackMemory += sizeof( eventCallback_t ) * num;

	// allocate temporary memory for flags so that the subclass's event callbacks
	// override the superclass's event callback
	set = new( TAG_SYSTEM ) bool[ num ];
	memset( set, 0, sizeof( bool ) * num );

	// go through the inheritence order and copies the event callback function into
	// a list indexed by the event number.  This allows fast lookups of
	// event functions.
	for( c = this; c != NULL; c = c->super )
	{
		def = c->eventCallbacks;
		if( !def )
		{
			continue;
		}

		// go through each entry until we hit the NULL terminator
		for( i = 0; def[ i ].event != NULL; i++ )
		{
			ev = def[ i ].event->GetEventNum();

			if( set[ ev ] )
			{
				continue;
			}
			set[ ev ] = true;
			eventMap[ ev ] = def[ i ].function;
		}
	}

	delete[] set;
}

/*
================
idTypeInfo::Shutdown

Should only be called when DLL or EXE is being shutdown.
Although it cleans up any allocated memory, it doesn't bother to remove itself
from the class list since the program is shutting down.
================
*/
void idTypeInfo::Shutdown()
{
	// free up the memory used for event lookups
	if( eventMap )
	{
		if( freeEventMap )
		{
			delete[] eventMap;
		}
		eventMap = NULL;
	}
	typeNum = 0;
	lastChild = 0;
}


/***********************************************************************

  idClass

***********************************************************************/

const idEventDef EV_Remove( "<immediateremove>", NULL );
const idEventDef EV_SafeRemove( "remove", NULL );

ABSTRACT_DECLARATION( NULL, idClass )
EVENT( EV_Remove,				idClass::Event_Remove )
EVENT( EV_SafeRemove,			idClass::Event_SafeRemove )
END_CLASS

// alphabetical order
idList<idTypeInfo*, TAG_IDCLASS>	idClass::types;
// typenum order
idList<idTypeInfo*, TAG_IDCLASS>	idClass::typenums;

bool	idClass::initialized	= false;
int		idClass::typeNumBits	= 0;
int		idClass::memused		= 0;
int		idClass::numobjects		= 0;

/*
================
idClass::CallSpawn
================
*/
void idClass::CallSpawn()
{
	idTypeInfo* type;

	type = GetType();
	CallSpawnFunc( type );
}

/*
================
idClass::CallSpawnFunc
================
*/
classSpawnFunc_t idClass::CallSpawnFunc( idTypeInfo* cls )
{
	classSpawnFunc_t func;

	if( cls->super )
	{
		func = CallSpawnFunc( cls->super );
		if( func == cls->Spawn )
		{
			// don't call the same function twice in a row.
			// this can happen when subclasses don't have their own spawn function.
			return func;
		}
	}

	( this->*cls->Spawn )();

	return cls->Spawn;
}

/*
================
idClass::FindUninitializedMemory
================
*/
void idClass::FindUninitializedMemory()
{
#ifdef ID_DEBUG_UNINITIALIZED_MEMORY
	// DG: use int instead of long for 64bit compatibility
	unsigned int* ptr = ( ( unsigned int* )this ) - 1;
	// DG end
	int size = *ptr;
	assert( ( size & 3 ) == 0 );
	size >>= 2;
	for( int i = 0; i < size; i++ )
	{
		if( ptr[i] == 0xcdcdcdcd )
		{
			const char* varName = GetTypeVariableName( GetClassname(), i << 2 );
			gameLocal.Warning( "type '%s' has uninitialized variable %s (offset %d)", GetClassname(), varName, i << 2 );
		}
	}
#endif
}

/*
================
idClass::Spawn
================
*/
void idClass::Spawn()
{
}

/*
================
idClass::~idClass

Destructor for object.  Cancels any events that depend on this object.
================
*/
idClass::~idClass()
{
	idEvent::CancelEvents( this );
}

/*
================
idClass::DisplayInfo_f
================
*/
void idClass::DisplayInfo_f( const idCmdArgs& args )
{
	gameLocal.Printf( "Class memory status: %i bytes allocated in %i objects\n", memused, numobjects );
}

/*
================
idClass::ListClasses_f
================
*/
void idClass::ListClasses_f( const idCmdArgs& args )
{
	int			i;
	idTypeInfo* type;

	gameLocal.Printf( "%-24s %-24s %-6s %-6s\n", "Classname", "Superclass", "Type", "Subclasses" );
	gameLocal.Printf( "----------------------------------------------------------------------\n" );

	for( i = 0; i < types.Num(); i++ )
	{
		type = types[ i ];
		gameLocal.Printf( "%-24s %-24s %6d %6d\n", type->classname, type->superclass, type->typeNum, type->lastChild - type->typeNum );
	}

	gameLocal.Printf( "...%d classes", types.Num() );
}

/*
================
idClass::CreateInstance
================
*/
idClass* idClass::CreateInstance( const char* name )
{
	const idTypeInfo*	type;
	idClass*				obj;

	type = idClass::GetClass( name );
	if( !type )
	{
		return NULL;
	}

	obj = type->CreateInstance();
	return obj;
}

/*
================
idClass::Init

Should be called after all idTypeInfos are initialized, so must be called
manually upon game code initialization.  Tells all the idTypeInfos to initialize
their event callback table for the associated class.  This should only be called
once during the execution of the program or DLL.
================
*/
void idClass::Init()
{
	idTypeInfo*	c;
	int			num;

	gameLocal.Printf( "Initializing class hierarchy\n" );

	if( initialized )
	{
		gameLocal.Printf( "...already initialized\n" );
		return;
	}

	// init the event callback tables for all the classes
	for( c = typelist; c != NULL; c = c->next )
	{
		c->Init();
	}

	// number the types according to the class hierarchy so we can quickly determine if a class
	// is a subclass of another
	num = 0;
	for( c = classHierarchy.GetNext(); c != NULL; c = c->node.GetNext(), num++ )
	{
		c->typeNum = num;
		c->lastChild += num;
	}

	// number of bits needed to send types over network
	typeNumBits = idMath::BitsForInteger( num );

	// create a list of the types so we can do quick lookups
	// one list in alphabetical order, one in typenum order
	types.SetGranularity( 1 );
	types.SetNum( num );
	typenums.SetGranularity( 1 );
	typenums.SetNum( num );
	num = 0;
	for( c = typelist; c != NULL; c = c->next, num++ )
	{
		types[ num ] = c;
		typenums[ c->typeNum ] = c;
	}

	initialized = true;

	gameLocal.Printf( "...%i classes, %i bytes for event callbacks\n", types.Num(), eventCallbackMemory );
}

/*
================
idClass::Shutdown
================
*/
void idClass::Shutdown()
{
	idTypeInfo*	c;

	for( c = typelist; c != NULL; c = c->next )
	{
		c->Shutdown();
	}
	types.Clear();
	typenums.Clear();

	initialized = false;
}

/*
================
idClass::new
================
*/
void* idClass::operator new( size_t s )
{
	int* p;

	s += sizeof( int );
	p = ( int* )Mem_Alloc( s, TAG_IDCLASS );
	*p = s;
	memused += s;
	numobjects++;

	return p + 1;
}

/*
================
idClass::delete
================
*/
void idClass::operator delete( void* ptr )
{
	int* p;

	if( ptr )
	{
		p = ( ( int* )ptr ) - 1;
		memused -= *p;
		numobjects--;
		Mem_Free( p );
	}
}

/*
================
idClass::GetClass

Returns the idTypeInfo for the name of the class passed in.  This is a static function
so it must be called as idClass::GetClass( classname )
================
*/
idTypeInfo* idClass::GetClass( const char* name )
{
	idTypeInfo*	c;
	int			order;
	int			mid;
	int			min;
	int			max;

	if( !initialized )
	{
		// idClass::Init hasn't been called yet, so do a slow lookup
		for( c = typelist; c != NULL; c = c->next )
		{
			if( !idStr::Cmp( c->classname, name ) )
			{
				return c;
			}
		}
	}
	else
	{
		// do a binary search through the list of types
		min = 0;
		max = types.Num() - 1;
		while( min <= max )
		{
			mid = ( min + max ) / 2;
			c = types[ mid ];
			order = idStr::Cmp( c->classname, name );
			if( !order )
			{
				return c;
			}
			else if( order > 0 )
			{
				max = mid - 1;
			}
			else
			{
				min = mid + 1;
			}
		}
	}

	return NULL;
}

/*
================
idClass::GetType
================
*/
idTypeInfo* idClass::GetType( const int typeNum )
{
	idTypeInfo* c;

	if( !initialized )
	{
		for( c = typelist; c != NULL; c = c->next )
		{
			if( c->typeNum == typeNum )
			{
				return c;
			}
		}
	}
	else if( ( typeNum >= 0 ) && ( typeNum < types.Num() ) )
	{
		return typenums[ typeNum ];
	}

	return NULL;
}

/*
================
idClass::GetClassname

Returns the text classname of the object.
================
*/
const char* idClass::GetClassname() const
{
	idTypeInfo* type;

	type = GetType();
	return type->classname;
}

/*
================
idClass::GetSuperclass

Returns the text classname of the superclass.
================
*/
const char* idClass::GetSuperclass() const
{
	idTypeInfo* cls;

	cls = GetType();
	return cls->superclass;
}

/*
================
idClass::CancelEvents
================
*/
void idClass::CancelEvents( const idEventDef* ev )
{
	idEvent::CancelEvents( this, ev );
}

/*
================
idClass::PostEventArgs
================
*/
bool idClass::PostEventArgs( const idEventDef* ev, int time, int numargs, ... )
{
	idTypeInfo*	c;
	idEvent*		event;
	va_list		args;

	assert( ev );

	if( !idEvent::initialized )
	{
		return false;
	}

	c = GetType();
	if( !c->eventMap[ ev->GetEventNum() ] )
	{
		// we don't respond to this event, so ignore it
		return false;
	}

	bool isReplicated = true;
	// If this is an entity with skipReplication, we want to process the event normally even on clients.
	if( IsType( idEntity::Type ) )
	{
		idEntity* thisEnt = static_cast< idEntity* >( this );
		if( thisEnt->fl.skipReplication )
		{
			isReplicated = false;
		}
	}

	// we service events on the client to avoid any bad code filling up the event pool
	// we don't want them processed usually, unless when the map is (re)loading.
	// we allow threads to run fine, though.
	if( common->IsClient() && isReplicated && ( gameLocal.GameState() != GAMESTATE_STARTUP ) && !IsType( idThread::Type ) )
	{
		return true;
	}

	va_start( args, numargs );
	event = idEvent::Alloc( ev, numargs, args );
	va_end( args );

	event->Schedule( this, c, time );

	return true;
}

/*
================
idClass::PostEventMS
================
*/
bool idClass::PostEventMS( const idEventDef* ev, int time )
{
	return PostEventArgs( ev, time, 0 );
}

/*
================
idClass::PostEventMS
================
*/
bool idClass::PostEventMS( const idEventDef* ev, int time, idEventArg arg1 )
{
	return PostEventArgs( ev, time, 1, &arg1 );
}

/*
================
idClass::PostEventMS
================
*/
bool idClass::PostEventMS( const idEventDef* ev, int time, idEventArg arg1, idEventArg arg2 )
{
	return PostEventArgs( ev, time, 2, &arg1, &arg2 );
}

/*
================
idClass::PostEventMS
================
*/
bool idClass::PostEventMS( const idEventDef* ev, int time, idEventArg arg1, idEventArg arg2, idEventArg arg3 )
{
	return PostEventArgs( ev, time, 3, &arg1, &arg2, &arg3 );
}

/*
================
idClass::PostEventMS
================
*/
bool idClass::PostEventMS( const idEventDef* ev, int time, idEventArg arg1, idEventArg arg2, idEventArg arg3, idEventArg arg4 )
{
	return PostEventArgs( ev, time, 4, &arg1, &arg2, &arg3, &arg4 );
}

/*
================
idClass::PostEventMS
================
*/
bool idClass::PostEventMS( const idEventDef* ev, int time, idEventArg arg1, idEventArg arg2, idEventArg arg3, idEventArg arg4, idEventArg arg5 )
{
	return PostEventArgs( ev, time, 5, &arg1, &arg2, &arg3, &arg4, &arg5 );
}

/*
================
idClass::PostEventMS
================
*/
bool idClass::PostEventMS( const idEventDef* ev, int time, idEventArg arg1, idEventArg arg2, idEventArg arg3, idEventArg arg4, idEventArg arg5, idEventArg arg6 )
{
	return PostEventArgs( ev, time, 6, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6 );
}

/*
================
idClass::PostEventMS
================
*/
bool idClass::PostEventMS( const idEventDef* ev, int time, idEventArg arg1, idEventArg arg2, idEventArg arg3, idEventArg arg4, idEventArg arg5, idEventArg arg6, idEventArg arg7 )
{
	return PostEventArgs( ev, time, 7, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7 );
}

/*
================
idClass::PostEventMS
================
*/
bool idClass::PostEventMS( const idEventDef* ev, int time, idEventArg arg1, idEventArg arg2, idEventArg arg3, idEventArg arg4, idEventArg arg5, idEventArg arg6, idEventArg arg7, idEventArg arg8 )
{
	return PostEventArgs( ev, time, 8, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8 );
}

/*
================
idClass::PostEventSec
================
*/
bool idClass::PostEventSec( const idEventDef* ev, float time )
{
	return PostEventArgs( ev, SEC2MS( time ), 0 );
}

/*
================
idClass::PostEventSec
================
*/
bool idClass::PostEventSec( const idEventDef* ev, float time, idEventArg arg1 )
{
	return PostEventArgs( ev, SEC2MS( time ), 1, &arg1 );
}

/*
================
idClass::PostEventSec
================
*/
bool idClass::PostEventSec( const idEventDef* ev, float time, idEventArg arg1, idEventArg arg2 )
{
	return PostEventArgs( ev, SEC2MS( time ), 2, &arg1, &arg2 );
}

/*
================
idClass::PostEventSec
================
*/
bool idClass::PostEventSec( const idEventDef* ev, float time, idEventArg arg1, idEventArg arg2, idEventArg arg3 )
{
	return PostEventArgs( ev, SEC2MS( time ), 3, &arg1, &arg2, &arg3 );
}

/*
================
idClass::PostEventSec
================
*/
bool idClass::PostEventSec( const idEventDef* ev, float time, idEventArg arg1, idEventArg arg2, idEventArg arg3, idEventArg arg4 )
{
	return PostEventArgs( ev, SEC2MS( time ), 4, &arg1, &arg2, &arg3, &arg4 );
}

/*
================
idClass::PostEventSec
================
*/
bool idClass::PostEventSec( const idEventDef* ev, float time, idEventArg arg1, idEventArg arg2, idEventArg arg3, idEventArg arg4, idEventArg arg5 )
{
	return PostEventArgs( ev, SEC2MS( time ), 5, &arg1, &arg2, &arg3, &arg4, &arg5 );
}

/*
================
idClass::PostEventSec
================
*/
bool idClass::PostEventSec( const idEventDef* ev, float time, idEventArg arg1, idEventArg arg2, idEventArg arg3, idEventArg arg4, idEventArg arg5, idEventArg arg6 )
{
	return PostEventArgs( ev, SEC2MS( time ), 6, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6 );
}

/*
================
idClass::PostEventSec
================
*/
bool idClass::PostEventSec( const idEventDef* ev, float time, idEventArg arg1, idEventArg arg2, idEventArg arg3, idEventArg arg4, idEventArg arg5, idEventArg arg6, idEventArg arg7 )
{
	return PostEventArgs( ev, SEC2MS( time ), 7, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7 );
}

/*
================
idClass::PostEventSec
================
*/
bool idClass::PostEventSec( const idEventDef* ev, float time, idEventArg arg1, idEventArg arg2, idEventArg arg3, idEventArg arg4, idEventArg arg5, idEventArg arg6, idEventArg arg7, idEventArg arg8 )
{
	return PostEventArgs( ev, SEC2MS( time ), 8, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8 );
}

/*
================
idClass::ProcessEventArgs
================
*/
bool idClass::ProcessEventArgs( const idEventDef* ev, int numargs, ... )
{
	idTypeInfo*	c;
	int			num;
	// RB: 64 bit fix, changed int to intptr_t
	intptr_t	data[ D_EVENT_MAXARGS ];
	// RB end
	va_list		args;

	assert( ev );
	assert( idEvent::initialized );

	c = GetType();
	num = ev->GetEventNum();
	if( !c->eventMap[ num ] )
	{
		// we don't respond to this event, so ignore it
		return false;
	}

	va_start( args, numargs );
	idEvent::CopyArgs( ev, numargs, args, data );
	va_end( args );

	ProcessEventArgPtr( ev, data );

	return true;
}

/*
================
idClass::ProcessEvent
================
*/
bool idClass::ProcessEvent( const idEventDef* ev )
{
	return ProcessEventArgs( ev, 0 );
}

/*
================
idClass::ProcessEvent
================
*/
bool idClass::ProcessEvent( const idEventDef* ev, idEventArg arg1 )
{
	return ProcessEventArgs( ev, 1, &arg1 );
}

/*
================
idClass::ProcessEvent
================
*/
bool idClass::ProcessEvent( const idEventDef* ev, idEventArg arg1, idEventArg arg2 )
{
	return ProcessEventArgs( ev, 2, &arg1, &arg2 );
}

/*
================
idClass::ProcessEvent
================
*/
bool idClass::ProcessEvent( const idEventDef* ev, idEventArg arg1, idEventArg arg2, idEventArg arg3 )
{
	return ProcessEventArgs( ev, 3, &arg1, &arg2, &arg3 );
}

/*
================
idClass::ProcessEvent
================
*/
bool idClass::ProcessEvent( const idEventDef* ev, idEventArg arg1, idEventArg arg2, idEventArg arg3, idEventArg arg4 )
{
	return ProcessEventArgs( ev, 4, &arg1, &arg2, &arg3, &arg4 );
}

/*
================
idClass::ProcessEvent
================
*/
bool idClass::ProcessEvent( const idEventDef* ev, idEventArg arg1, idEventArg arg2, idEventArg arg3, idEventArg arg4, idEventArg arg5 )
{
	return ProcessEventArgs( ev, 5, &arg1, &arg2, &arg3, &arg4, &arg5 );
}

/*
================
idClass::ProcessEvent
================
*/
bool idClass::ProcessEvent( const idEventDef* ev, idEventArg arg1, idEventArg arg2, idEventArg arg3, idEventArg arg4, idEventArg arg5, idEventArg arg6 )
{
	return ProcessEventArgs( ev, 6, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6 );
}

/*
================
idClass::ProcessEvent
================
*/
bool idClass::ProcessEvent( const idEventDef* ev, idEventArg arg1, idEventArg arg2, idEventArg arg3, idEventArg arg4, idEventArg arg5, idEventArg arg6, idEventArg arg7 )
{
	return ProcessEventArgs( ev, 7, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7 );
}

/*
================
idClass::ProcessEvent
================
*/
bool idClass::ProcessEvent( const idEventDef* ev, idEventArg arg1, idEventArg arg2, idEventArg arg3, idEventArg arg4, idEventArg arg5, idEventArg arg6, idEventArg arg7, idEventArg arg8 )
{
	return ProcessEventArgs( ev, 8, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8 );
}

/*
================
idClass::ProcessEventArgPtr
================
*/
// RB: 64 bit fixes, changed int to intptr_t
bool idClass::ProcessEventArgPtr( const idEventDef* ev, intptr_t* data )
{
// RB end
	idTypeInfo*	c;
	int			num;
	eventCallback_t	callback;

	assert( ev );
	assert( idEvent::initialized );

	SetTimeState ts;

	if( IsType( idEntity::Type ) )
	{
		idEntity* ent = ( idEntity* )this;
		ts.PushState( ent->timeGroup );
	}

	if( g_debugTriggers.GetBool() && ( ev == &EV_Activate ) && IsType( idEntity::Type ) )
	{
		const idEntity* ent = *reinterpret_cast<idEntity**>( data );
		gameLocal.Printf( "%d: '%s' activated by '%s'\n", gameLocal.framenum, static_cast<idEntity*>( this )->GetName(), ent ? ent->GetName() : "NULL" );
	}

	c = GetType();
	num = ev->GetEventNum();
	if( !c->eventMap[ num ] )
	{
		// we don't respond to this event, so ignore it
		return false;
	}

	callback = c->eventMap[ num ];

// RB: I tried first to get CPU_EASYARGS switch running with x86_64
// but it caused many crashes with the Doom scripts.
// The new Callbacks.cpp was generated with intptr_t and it works fine.
#if !CPU_EASYARGS

	/*
	on ppc architecture, floats are passed in a separate set of registers
	the function prototypes must have matching float declaration

	http://developer.apple.com/documentation/DeveloperTools/Conceptual/MachORuntime/2rt_powerpc_abi/chapter_9_section_5.html
	*/

	switch( ev->GetFormatspecIndex() )
	{
		case 1 << D_EVENT_MAXARGS :
			( this->*callback )();
			break;

// generated file - see CREATE_EVENT_CODE
#include "Callbacks.cpp"

		default:
			gameLocal.Warning( "Invalid formatspec on event '%s'", ev->GetName() );
			break;
	}

#else

	assert( D_EVENT_MAXARGS == 8 );

	// RB: 64 bit fixes, changed int to intptr_t
	switch( ev->GetNumArgs() )
	{
		case 0 :
			( this->*callback )();
			break;

		case 1 :
			typedef void ( idClass::*eventCallback_1_t )( const intptr_t );
			( this->*( eventCallback_1_t )callback )( data[ 0 ] );
			break;

		case 2 :
			typedef void ( idClass::*eventCallback_2_t )( const intptr_t, const intptr_t );
			( this->*( eventCallback_2_t )callback )( data[ 0 ], data[ 1 ] );
			break;

		case 3 :
			typedef void ( idClass::*eventCallback_3_t )( const intptr_t, const intptr_t, const intptr_t );
			( this->*( eventCallback_3_t )callback )( data[ 0 ], data[ 1 ], data[ 2 ] );
			break;

		case 4 :
			typedef void ( idClass::*eventCallback_4_t )( const intptr_t, const intptr_t, const intptr_t, const intptr_t );
			( this->*( eventCallback_4_t )callback )( data[ 0 ], data[ 1 ], data[ 2 ], data[ 3 ] );
			break;

		case 5 :
			typedef void ( idClass::*eventCallback_5_t )( const intptr_t, const intptr_t, const intptr_t, const intptr_t, const intptr_t );
			( this->*( eventCallback_5_t )callback )( data[ 0 ], data[ 1 ], data[ 2 ], data[ 3 ], data[ 4 ] );
			break;

		case 6 :
			typedef void ( idClass::*eventCallback_6_t )( const intptr_t, const intptr_t, const intptr_t, const intptr_t, const intptr_t, const intptr_t );
			( this->*( eventCallback_6_t )callback )( data[ 0 ], data[ 1 ], data[ 2 ], data[ 3 ], data[ 4 ], data[ 5 ] );
			break;

		case 7 :
			typedef void ( idClass::*eventCallback_7_t )( const intptr_t, const intptr_t, const intptr_t, const intptr_t, const intptr_t, const intptr_t, const intptr_t );
			( this->*( eventCallback_7_t )callback )( data[ 0 ], data[ 1 ], data[ 2 ], data[ 3 ], data[ 4 ], data[ 5 ], data[ 6 ] );
			break;

		case 8 :
			typedef void ( idClass::*eventCallback_8_t )( const intptr_t, const intptr_t, const intptr_t, const intptr_t, const intptr_t, const intptr_t, const intptr_t, const intptr_t );
			( this->*( eventCallback_8_t )callback )( data[ 0 ], data[ 1 ], data[ 2 ], data[ 3 ], data[ 4 ], data[ 5 ], data[ 6 ], data[ 7 ] );
			break;

		default:
			gameLocal.Warning( "Invalid formatspec on event '%s'", ev->GetName() );
			break;
	}
	// RB end

#endif

	return true;
}

/*
================
idClass::Event_Remove
================
*/
void idClass::Event_Remove()
{
	delete this;
}

static int l_CallEvent( lua_State* L )
{
	// Not thread safe.
	static idArray<idVec3, 6> vectors;

	int numVectors = 0;

	// It's possible that the passed in lua state is a coroutine. Update the lua state just for this call.
	// On return, the lua state will be reverted to the original state before the call
	// to this function.
	ScopedLuaState stateScope( L );

	// Get the EventDef from the upvalue data.
	const idEventDef* evdef = ( const idEventDef* )lua_topointer( L, lua_upvalueindex( 1 ) );

	// Get the idClass from the top of the stack.
	lua_getfield( L, 1, "classPtr" );

	idClass* classPtr = ( idClass* )lua_topointer( L, -1 );

	intptr_t data[D_EVENT_MAXARGS];

	const char* format = evdef->GetArgFormat( );

	for( int i = 0, j = 2; format[i] != 0; i++, j++ )
	{
		switch( format[i] )
		{
			case D_EVENT_INTEGER:
			{
				if( lua_isnumber( L, j ) )
				{
					*( int* )&data[i] = luaL_checkinteger( L, j );
				}
				else if( lua_isboolean( L, j ) )
				{
					*( int* )&data[i] = lua_toboolean( L, j );
				}
				else
				{
					common->Warning( "Failed to parse event argument %s", evdef->GetName() );
				}
				break;
			}
			case D_EVENT_FLOAT:
			{
				*( float* )&data[i] = luaL_checknumber( L, j );
				break;
			}
			case D_EVENT_VECTOR:
			{
				// The vector is represented as a table in lua {x, y, z}
				lua_getfield( L, j, "x" );
				vectors[numVectors].x = ( luaL_checknumber( L, -1 ) );
				lua_pop( L, 1 );
				lua_getfield( L, j, "y" );
				vectors[numVectors].y = ( luaL_checknumber( L, -1 ) );
				lua_pop( L, 1 );
				lua_getfield( L, j, "z" );
				vectors[numVectors].z = ( luaL_checknumber( L, -1 ) );
				lua_pop( L, 1 );
				*( idVec3** )&data[i] = &vectors[numVectors++];
				break;
			}
			case D_EVENT_STRING:
			{
				const char* theStr = luaL_checkstring( L, j );
				*( const char** )&data[i] = theStr;
				break;
			}
			case D_EVENT_ENTITY:
			case D_EVENT_ENTITY_NULL:
			{
				lua_getfield( L, j, "classPtr" );
				data[i] = ( intptr_t )lua_topointer( L, -1 );
				lua_pop( L, 1 );
				break;
			}
			case D_EVENT_TRACE:
			default:
			{
				common->Error( "<unsupported parameter type '%c'>", format[i] );
				break;
			}
		}
	}

	if( classPtr )
	{
		classPtr->ProcessEventArgPtr( evdef, data );
	}
	else
	{
		common->FatalError( "Invalid lua thread passed in" );
	}

	return 1;
}

static void SetEventUpValues( lua_State* L, const idList<const idEventDef*>& events )
{
	for( int i = 0; i < events.Num(); i++ )
	{
		lua_getfield( L, -1, events[i]->GetName() );
		lua_pushlightuserdata( L, ( void* )events[i] );
		lua_setupvalue( L, -2, 1 );

		// pop the function
		lua_pop( L, 1 );
	}
}

static int RegisterLuaClass( lua_State* L, const idList<luaL_Reg>& classLib, const idList<const idEventDef*>& events )
{
	// Can't use luaL_newlib, so use the expanded version to add the upvalues.

	// Create the empty table, pre-allocated to the right size
	lua_createtable( L, 0, classLib.Num( ) );

	// Push a nil upvalue for now
	lua_pushnil( L );

	// Register event functions with 1 upvalue
	luaL_setfuncs( L, &classLib[0], 1 );

	// Set up the upvalues
	SetEventUpValues( L, events );

	return 1;
}

void idClass::ExportLuaFunctions( lua_State* luaState )
{
	// allocate temporary memory for flags so that the subclass's event callbacks
	// override the superclass's event callback
	int numEventDefs = idEventDef::NumEventCommands( );
	bool* set = new bool[numEventDefs];
	memset( set, 0, sizeof( bool ) * numEventDefs );

	for( const idTypeInfo* c = classHierarchy.GetNext( ); c != nullptr; c = c->node.GetNext( ) )
	{
		idList<luaL_Reg> classLib;
		idList<const idEventDef*> eventDefs;

		// Go through each entry until we hit the NULL terminator
		const idTypeInfo* p = c;
		while( p )
		{
			idEventFunc<idClass>* def = p->eventCallbacks;

			if( !def || !def[0].event )
			{
				// No new events or overrides
				p = p->super;
				continue;
			}

			for( int j = 0; def[j].event != nullptr; j++ )
			{
				const idEventDef* ev = def[j].event;
				int evNum = ev->GetEventNum( );

				if( ev->GetName( )[0] == '_' || ev->GetName( )[0] == '<' )
				{
					// internal event
					continue;
				}

				if( set[evNum] )
				{
					// We've run into a method that this class has overriden. Skip.
					continue;
					//gameLocal.Printf( "Class %s is overriding %s of class %s\n", c->classname, ev->GetName( ), p->classname );
				}

				set[evNum] = true;

				eventDefs.Append( ev );
				classLib.Append( { ev->GetName( ), l_CallEvent } );
			}

			p = p->super;
		}

		memset( set, 0, sizeof( bool ) * numEventDefs );
		classLib.Append( { NULL, NULL } );

		// Create a new table with all the event methods for this class
		RegisterLuaClass( luaState, classLib, eventDefs );
		lua_setglobal( luaState, c->classname );
	}

	delete[] set;
}

/*
================
idClass::Event_SafeRemove
================
*/
void idClass::Event_SafeRemove()
{
	// Forces the remove to be done at a safe time
	PostEventMS( &EV_Remove, 0 );
}


// RB: development tool
void idClass::ExportScriptEvents_f( const idCmdArgs& args )
{
	// allocate temporary memory for flags so that the subclass's event callbacks
	// override the superclass's event callback
	int numEventDefs = idEventDef::NumEventCommands();
	bool* set = new bool[ numEventDefs ];

	enum
	{
		EXPORTLANG_DOOMSCRIPT,
		EXPORTLANG_DOOMSHARP,
		NUM_EXPORTLANGS,
	};

	// go through the inheritence order
#if 1 //defined(USE_DOOMSHARP)
	for( int exportLang = 0; exportLang < NUM_EXPORTLANGS; exportLang++ )
#else
	int exportLang = 0;
#endif
	{
		bool threadClassFound = false;
		bool firstEntityClassFound = false;

		memset( set, 0, sizeof( bool ) * numEventDefs );

#if 1 //defined(USE_DOOMSHARP)
		idFile* file = NULL;
		if( exportLang == EXPORTLANG_DOOMSHARP )
		{
			file = fileSystem->OpenFileWrite( "script/doom_events.cs", "fs_basepath" );
		}
		else
		{
			file = fileSystem->OpenFileWrite( "script/doom_events.script", "fs_basepath" );
		}
#else
		idFile* file = fileSystem->OpenFileWrite( "script/doom_events.script", "fs_basepath" );

#endif

		file->Printf( "// generated by RBDOOM-3-BFG\n//\n\n" );

		if( exportLang == EXPORTLANG_DOOMSHARP )
		{
			//file->Printf( "#if CSHARP\n\n" );

			file->Printf( "using vector;\n\n" );
		}

		for( const idTypeInfo* c = classHierarchy.GetNext(); c != NULL; c = c->node.GetNext() )
		{
			idEventFunc<idClass>* def = c->eventCallbacks;
			if( !def || !def[ 0 ].event )
			{
				// no new events or overrides
				continue;
			}


			if( exportLang == EXPORTLANG_DOOMSHARP )
			{
				//if( c->IsType( idEntity::Type ) && threadClassFound )
				//{
				//	file->Printf( "\n}\n\n\n" );
				//}

				file->Printf( "\n/// <summary>\n" );
				file->Printf( "/// %-24s   --->   %-24s\n", c->classname, c->superclass );
				file->Printf( "/// </summary>\n" );

				if( !c->IsType( idThread::Type ) && !c->IsType( idEntity::Type ) )
				{
					continue;
				}
				else if( c->IsType( idThread::Type ) )
				{
					file->Printf( "class sys\n{\n" );

					threadClassFound = true;
				}
				else if( c->IsType( idEntity::Type ) && !firstEntityClassFound )
				{
					file->Printf( "\n}\n\n\n" );

					file->Printf( "class entity\n{\n" );
					file->Printf( "\tpublic void remove() { }\n\n" );

					firstEntityClassFound = true;
				}
			}
			else
			{
				file->Printf( "/*\n\n" );
				file->Printf( "%-24s   --->   %-24s\n\n", c->classname, c->superclass );
				file->Printf( "*/\n\n" );
			}


			// go through each entry until we hit the NULL terminator
			for( int j = 0; def[ j ].event != NULL; j++ )
			{
				const idEventDef* ev = def[ j ].event;
				int evNum = ev->GetEventNum();

				if( set[ evNum ] )
				{
					//continue;
				}

				if( ev->GetName()[0] == '_' || ev->GetName()[0] == '<' )
				{
					// internal event
					continue;
				}

				if( set[ evNum ] )
				{
					file->Printf( "// override " );
				}

				set[ evNum ] = true;

				if( exportLang == EXPORTLANG_DOOMSHARP )
				{
					if( c->IsType( idThread::Type ) )
					{
						file->Printf( "\tpublic static " );
					}
					else
					{
						file->Printf( "\tpublic " );
					}
				}
				else
				{
					file->Printf( "scriptEvent " );
				}

				switch( ev->GetReturnType() )
				{
					case D_EVENT_FLOAT :
						file->Printf( "float " );
						break;

					case D_EVENT_INTEGER :
						file->Printf( "float " );
						break;

					case D_EVENT_VECTOR :
						file->Printf( "vector " );
						break;

					case D_EVENT_STRING :
						file->Printf( "string " );
						break;

					case D_EVENT_ENTITY :
					case D_EVENT_ENTITY_NULL :
						file->Printf( "entity " );
						break;

					case 0:
						file->Printf( "void " );
						break;

					default:
					case D_EVENT_TRACE :
						file->Printf( "<unsupported> " );
						break;
				}

				file->Printf( "%s(", ev->GetName() );

				if( ev->GetNumArgs() )
				{
					file->Printf( " " );
				}

				const char* formatspec = ev->GetArgFormat();
				for( int arg = 0; arg < ev->GetNumArgs(); arg++ )
				{
					if( arg != 0 )
					{
						file->Printf( ", " );
					}

					switch( formatspec[ arg ] )
					{
						case D_EVENT_FLOAT :
							file->Printf( "float parm%d", arg );
							break;

						case D_EVENT_INTEGER :
							file->Printf( "float parm%d", arg );
							break;

						case D_EVENT_VECTOR :
							file->Printf( "vector parm%d", arg );
							break;

						case D_EVENT_STRING :
							file->Printf( "string parm%d", arg );
							break;

						case D_EVENT_ENTITY :
						case D_EVENT_ENTITY_NULL :
							file->Printf( "entity parm%d", arg );
							break;

						case 0:
							// void
							break;

						default:
						case D_EVENT_TRACE :
							file->Printf( "<unsupported> parm%d", arg );
							break;
					}
				}

				if( ev->GetNumArgs() )
				{
					file->Printf( " " );
				}

				if( exportLang == EXPORTLANG_DOOMSHARP )
				{
					file->Printf( ") " );

					switch( ev->GetReturnType() )
					{
						case D_EVENT_FLOAT :
							file->Printf( "{ return 0; }\n\n" );
							break;

						case D_EVENT_INTEGER :
							file->Printf( "{ return 0; }\n\n" );
							break;

						case D_EVENT_VECTOR :
							file->Printf( "{ return new vector( 0, 0, 0 ); }\n\n" );
							break;

						case D_EVENT_STRING :
							file->Printf( "{ return \"\"; }\n\n" );
							break;

						case D_EVENT_ENTITY :
						case D_EVENT_ENTITY_NULL :
							file->Printf( "{ return null; }\n\n" );
							break;

						case 0:
							file->Printf( "{ }\n\n" );
							break;

						default:
						case D_EVENT_TRACE :
							file->Printf( "<unsupported> " );
							break;
					}
				}
				else
				{
					file->Printf( ");\n\n" );
				}
			}
		}

		if( exportLang == EXPORTLANG_DOOMSHARP )
		{
			file->Printf( "\n}\n\n" );
			//file->Printf( "#endif // CSHARP\n\n" );
		}

		fileSystem->CloseFile( file );
	}

	delete[] set;
}

void idClass::ExportTypeScriptEvents_f( const idCmdArgs& args )
{
	// allocate temporary memory for flags so that the subclass's event callbacks
	// override the superclass's event callback
	int numEventDefs = idEventDef::NumEventCommands();
	bool* set = new bool[numEventDefs];

	bool firstEntityClassFound = false;

	memset( set, 0, sizeof( bool ) * numEventDefs );

	idFile* file = fileSystem->OpenFileWrite( "script/tech4-api.d.ts", "fs_basepath" );

	file->Printf( "/** Generated by RBDOOM-3-BFG */\n\n\n" );

	// Declare the types here.
	file->Printf( "interface Vector {\n" );
	file->Printf( "\tx: number;\n" );
	file->Printf( "\ty: number;\n" );
	file->Printf( "\tz: number;\n}\n\n" );

	// TODO(Stephen): Have this declared in a utility script instead.
	//file->Printf( "enum AnimChannel {\n" );
	//file->Printf( "\tAll: 0,\n" );
	//file->Printf( "\tTorso: 1,\n" );
	//file->Printf( "\tLegs: 2,\n" );
	//file->Printf( "\tHead: 3,\n" );
	//file->Printf( "\tEyelids: 4,\n" );
	//file->Printf( "};\n\n" );

	// Declare all the events here.
	for( const idTypeInfo* c = classHierarchy.GetNext(); c != NULL; c = c->node.GetNext() )
	{
		idEventFunc<idClass>* def = c->eventCallbacks;
		if( !def || !def[0].event )
		{
			file->Printf( "declare interface %s extends %s {}\n\n", c->classname, c->superclass );
			// no new events or overrides
			continue;
		}

		const bool isEntityAbstractClass = c->IsType( idEntity::Type ) && !firstEntityClassFound;

		file->Printf( "\n/**\n" );
		file->Printf( "* @type %-24s\n", c->classname );
		file->Printf( "*/\n" );

		file->Printf( "declare" );

		const bool isIdClass = !c->superclass || ( c->superclass && idStr::Cmp( c->superclass, "NULL" ) == 0 );

		file->Printf( " interface %s", c->classname );

		if( !isIdClass && !( c->IsType( idThread::Type ) || c->IsType( idLuaThread::Type ) ) )
		{
			file->Printf( " extends %s", c->superclass );
		}

		file->Printf( " {\n\n" );

		if( isEntityAbstractClass )
		{
			file->Printf( "\nremove(): void;\n\n" );
			firstEntityClassFound = true;
		}

		// go through each entry until we hit the NULL terminator
		for( int j = 0; def[j].event != NULL; j++ )
		{
			const idEventDef* ev = def[j].event;
			int evNum = ev->GetEventNum();

			if( set[evNum] )
			{
				//continue;
			}

			if( ev->GetName()[0] == '_' || ev->GetName()[0] == '<' )
			{
				// internal event
				continue;
			}

			if( ( c->super && c->super->RespondsTo( *ev ) ) &&
					!( c->IsType( idThread::Type ) || c->IsType( idLuaThread::Type ) ) )
			{
				file->Printf( "\t/** @override */\n" );
			}

			set[evNum] = true;

			file->Printf( "\t%s(", ev->GetName() );

			if( ev->GetNumArgs() )
			{
				file->Printf( " " );
			}

			// params
			const char* formatspec = ev->GetArgFormat();
			for( int arg = 0; arg < ev->GetNumArgs(); arg++ )
			{
				if( arg != 0 )
				{
					file->Printf( ", " );
				}

				switch( formatspec[arg] )
				{
					case D_EVENT_FLOAT:
						file->Printf( "parm%d: number", arg );
						break;

					case D_EVENT_INTEGER:
						file->Printf( "parm%d: number", arg );
						break;

					case D_EVENT_VECTOR:
						file->Printf( "parm%d: Vector", arg );
						break;

					case D_EVENT_STRING:
						file->Printf( "parm%d: string", arg );
						break;

					case D_EVENT_ENTITY:
					case D_EVENT_ENTITY_NULL:
						file->Printf( "parm%d: idEntity", arg );
						break;

					case 0:
						// void
						break;

					default:
					case D_EVENT_TRACE:
						file->Printf( "parm%d: <unsupported>", arg );
						break;
				}
			}

			if( ev->GetNumArgs() )
			{
				file->Printf( " " );
			}

			file->Printf( "): " );

			// Return type.
			switch( ev->GetReturnType() )
			{
				case D_EVENT_FLOAT:
					file->Printf( "number" );
					break;

				case D_EVENT_INTEGER:
					file->Printf( "number" );
					break;

				case D_EVENT_VECTOR:
					file->Printf( "Vector" );
					break;

				case D_EVENT_STRING:
					file->Printf( "string" );
					break;

				case D_EVENT_ENTITY:
				case D_EVENT_ENTITY_NULL:
					file->Printf( "idEntity" );
					break;

				case 0:
					file->Printf( "void" );
					break;

				default:
				case D_EVENT_TRACE:
					file->Printf( "<unsupported> " );
					break;
			}

			file->Printf( ";\n\n" );
		}

		file->Printf( "}\n\n" );
	}

	// Declare the global variables here.
	file->Printf( "declare const sys: idLuaThread;" );

	fileSystem->CloseFile( file );

	delete[] set;
}

void idClass::EditLights_f( const idCmdArgs& args )
{
	if( g_editEntityMode.GetInteger() != 1 )
	{
		g_editEntityMode.SetInteger( 1 );

		// turn off com_smp multithreading so we can load and check light textures on main thread
		com_editors |= EDITOR_LIGHT;
	}
	else
	{
		g_editEntityMode.SetInteger( 0 );

		com_editors &= ~EDITOR_LIGHT;
	}
}
// RB end