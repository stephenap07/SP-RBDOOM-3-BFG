#include "precompiled.h"

#ifdef USE_PHYSX

#include "GamePXCpuDispatcher.h"

#include "task/PxTask.h"

static void RunTask( void* data )
{
	physx::PxBaseTask* task = reinterpret_cast< physx::PxBaseTask* >( data );

	if( task )
	{
		task->run( );
		task->release( );
	}
}

REGISTER_PARALLEL_JOB( RunTask, "PhysxJobTask" );

GamePXCpuDispatcher::GamePXCpuDispatcher( idParallelJobList* theJobList, idParallelJobList* theSubJobList )
	: jobList( theJobList )
	, subJobList( theSubJobList )
{
}

GamePXCpuDispatcher::~GamePXCpuDispatcher( )
{
}

void GamePXCpuDispatcher::submitTask( physx::PxBaseTask& task )
{
	if( jobList->IsSubmitted( ) )
	{
		task.run( );
		task.release( );
	}
	else
	{
		jobList->AddJob( RunTask, &task );
	}
}

uint32_t GamePXCpuDispatcher::getWorkerCount( ) const
{
	return parallelJobManager->GetNumProcessingUnits( );
}

#endif