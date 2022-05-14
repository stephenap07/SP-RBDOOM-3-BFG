#pragma once

#ifdef USE_PHYSX
#include "task/PxCpuDispatcher.h"

class GamePXCpuDispatcher : public physx::PxCpuDispatcher
{
public:

	GamePXCpuDispatcher( idParallelJobList* theJobList, idParallelJobList* theSubJobList );

	~GamePXCpuDispatcher( );

	void submitTask( physx::PxBaseTask& task ) override;

	uint32_t getWorkerCount( ) const override;

private:
	idParallelJobList* jobList;
	idParallelJobList* subJobList;
};

#endif