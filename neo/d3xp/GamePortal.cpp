#include "precompiled.h"
#pragma hdrstop

#include "Game_local.h"

#include <lua.hpp>
#include <luaconf.h>

const idEventDef EV_Opened("<portalopened>", NULL);
const idEventDef EV_Closed("<portalclosed>", NULL);
const idEventDef EV_PortalSpark("<portalSpark>", NULL);
const idEventDef EV_PortalSparkEnd("<portalSparkEnd>", NULL);
const idEventDef EV_ShowGlowPortal("showGlowPortal", NULL);
const idEventDef EV_HideGlowPortal("hideGlowPortal", NULL);
const idEventDef EV_ResetGravity("resetGravityPortal", NULL);

CLASS_DECLARATION(idEntity, spGamePortal)
END_CLASS

spGamePortal::spGamePortal()
	: areaPortal(0)
	, portalState(portalState_t::PORTAL_OPENED)
	, bNoTeleport(false)
	, slavePortals()
	, masterPortal()
	, proximityEntities()
{
}

spGamePortal::~spGamePortal()
{
}

static void* l_alloc(void* ud, void* ptr, size_t osize,
	size_t nsize) {
	(void)ud;  (void)osize;  /* not used */
	if (nsize == 0) {
		free(ptr);
		return NULL;
	}
	else
		return realloc(ptr, nsize);
}

static int l_commonPrintf(lua_State* L)
{
	// get argument
	const char* str = lua_tostring(L, -1);
	common->Printf(str);
	return 0;
}

void spGamePortal::Spawn()
{
	idBounds bounds;

	bNoTeleport = spawnArgs.GetBool("noTeleport");

	areaPortal = 1;

	if (bNoTeleport) {
		GetPhysics()->SetContents(0);
	}
	else {
		GetPhysics()->SetContents(CONTENTS_SOLID);
	}

	// Setup the initial state for the portal
	if (spawnArgs.GetBool("startActive")) { // Start Active
		portalState = portalState_t::PORTAL_OPENED;
	}
	else { // Start closed
		Hide();
		portalState = portalState_t::PORTAL_CLOSED;
		GetPhysics()->SetContents(0); //rww - if closed, ensure things will not hit and go through
	}

	BecomeActive(TH_THINK | TH_UPDATEVISUALS | TH_ANIMATE);

	UpdateVisuals();

	PostEventMS(&EV_PostSpawn, 0);

	fl.networkSync = true; //rww

	proximityEntities.Clear(); // Clear the list of potential entities to be portalled

	// Set up the lua stuff.
	const char* luaScript = spawnArgs.GetString("lua_script");

	if (luaScript)
	{
		char* src;
		if (fileSystem->ReadFile(luaScript, (void**)&src, NULL) < 0)
		{
			gameLocal.Error("Couldn't load %s\n", luaScript);
		}
		else
		{
			lua_State* L = lua_newstate(l_alloc, nullptr);
			luaL_openlibs(L);
			int sz = fileSystem->GetFileLength(luaScript);
			if (luaL_loadbuffer(L, src, sz, luaScript) != LUA_OK || lua_pcall(L, 0, 0, 0))
			{
				common->Error("Failed to load lua script %s : %s\n", luaScript, lua_tostring(L, -1));
				lua_pop(L, -1);
			}
			fileSystem->FreeFile(src);
			lua_pushcfunction(L, l_commonPrintf);
			lua_setglobal(L, "comPrintf");
			// call main
			lua_getglobal(L, "main");
			if (lua_pcall(L, 0, 0, 0) != LUA_OK)
			{
				common->Error("Error running function 'main': %s\n", lua_tostring(L, -1));
				lua_pop(L, -1);
			}
			lua_close(L);
		}
	}
}

void spGamePortal::Save(idSaveGame* savefile) const
{
}

void spGamePortal::Restore(idRestoreGame* savefile)
{
}

void spGamePortal::ClientPredictionThink(void)
{
	Think();
}

void spGamePortal::CheckPlayerDistances(void)
{
}

void PortalRotate(idVec3& vec, const idMat3& sourceTranspose, const idMat3& dest, const bool flipX) {
	vec *= sourceTranspose;
	vec.y *= -1;
	if (flipX) {
		vec.x *= -1;
	}
	vec *= dest;
}

void spGamePortal::Think(void)
{
	int				i;
	idPlane			plane;

	idEntity::Think();

	if (portalState == portalState_t::PORTAL_CLOSED || bNoTeleport) {
		return;
	}

	// Force visuals to update until a remote renderview has been created for this portal
	if (!renderEntity.remoteRenderView) {
		BecomeActive(TH_UPDATEVISUALS);
	}

	idBounds bounds = GetPhysics()->GetClipModel()->GetBounds();
	bounds.RotateSelf(GetPhysics()->GetAxis());
	bounds.TranslateSelf(GetPhysics()->GetOrigin());
	bounds.ExpandSelf(1.0f);

	idEntity* entityList[MAX_GENTITIES];
	int numListedEntities = gameLocal.clip.EntitiesTouchingBounds(bounds, -1, entityList, MAX_GENTITIES);
	for (int i = 0; i < numListedEntities; i++)
	{
		idEntity* ent = entityList[i];
		if (ent != this && !ent->IsHidden() && !ent->IsType(idWorldspawn::Type))
		{
			idEntityPtr<idEntity> traceEntPtr;
			traceEntPtr = ent;
			if (!ignoredEntities.Find(traceEntPtr))
			{
				AddProximityEntity(ent);
			}
		}
	}

	ignoredEntities.Clear();

	for (int i = 0; i < numListedEntities; i++)
	{
		idEntity* ent = entityList[i];
		if (ent != this && !ent->IsHidden() && !ent->IsType(idWorldspawn::Type))
		{
			idEntityPtr<idEntity> traceEntPtr;
			traceEntPtr = ent;

			ignoredEntities.AddUnique(traceEntPtr);

			if (cameraTarget)
			{
				spGamePortal* targetPortal = static_cast<spGamePortal*>(cameraTarget);
				targetPortal->ignoredEntities.AddUnique(traceEntPtr);
			}
		}
	}

	// Bit of a hack for noclipping players:  If they are close to the portal, then add them to the proximity list automatically
	//idPlayer* player = gameLocal.GetLocalPlayer();
	//if (player && player->noclip) { //rww - note that the local player is NULL for dedicated servers.
	//	if ((player->GetPhysics()->GetOrigin() - GetPhysics()->GetOrigin()).LengthFast() < kDistanceToPlayer) {
	//		AddProximityEntity(player);
	//	}
	//}

	for (i = 0; i < proximityEntities.Num(); i++)
	{
		if (!proximityEntities[i].entity.IsValid())
		{
			// Remove this entity from the list
			proximityEntities.RemoveIndex(i);
			continue;
		}

		idEntity* hit = proximityEntities[i].entity.GetEntity();

		if (cameraTarget && cameraTarget->IsType(spGamePortal::Type))
		{
			idMat3 sourceAxis = GetPhysics()->GetAxis().Transpose();
			idMat3 destAxis = cameraTarget->GetPhysics()->GetAxis();

			// Compute new location
			idVec3 point = hit->GetPhysics()->GetOrigin();

			int side = plane.Side(point);
			if (side == PLANESIDE_ON || side == PLANESIDE_CROSS) {
				side = PLANESIDE_BACK;
			}

			idVec3 nextLocation = hit->GetPhysics()->GetOrigin() + hit->GetPhysics()->GetLinearVelocity();
			int nextSide = plane.Side(nextLocation);
			if (nextSide == PLANESIDE_ON || nextSide == PLANESIDE_CROSS) {
				nextSide = PLANESIDE_BACK;
			}

			if (side == PLANESIDE_BACK && nextSide == PLANESIDE_FRONT)
			{
				continue;
			}

			idVec3 newLocation = (GetPhysics()->GetOrigin() - point) * sourceAxis;
			newLocation.z = -newLocation.z;
			newLocation = cameraTarget->GetPhysics()->GetOrigin() + (newLocation * destAxis);

			// Compute new axis
			idMat3 newEntAxis = hit->GetPhysics()->GetAxis();
			if (hit->IsType(idPlayer::Type))
			{
				newEntAxis = static_cast<idPlayer*>(hit)->firstPersonViewAxis;
			}

			idAngles angleDiff = GetPhysics()->GetAxis().ToAngles() - newEntAxis.ToAngles();
			idAngles origAngle = destAxis.ToAngles();
			origAngle.yaw = origAngle.yaw - angleDiff.yaw;
			origAngle.pitch = origAngle.pitch - angleDiff.pitch;
			origAngle.yaw -= 180;
			origAngle.Normalize180();

			idVec3 vel = hit->GetPhysics()->GetLinearVelocity();
			PortalRotate(vel, sourceAxis, destAxis, true);
			hit->GetPhysics()->SetLinearVelocity(vel);

			if (hit->IsType(idPlayer::Type))
			{
				idPlayer* player = static_cast<idPlayer*>(hit);
				//player->Teleport(newLocation, origAngle, nullptr);
				player->SetOrigin(newLocation + idVec3(0, 0, CM_CLIP_EPSILON));
				player->SetViewAngles(origAngle);
				player->CalculateFirstPersonView();
				player->UpdateVisuals();
				//player->GetPhysics()->SetLinearVelocity(vel);
			}
			else
			{
				hit->SetOrigin(newLocation + idVec3(0, 0, CM_CLIP_EPSILON));
				hit->SetAngles(origAngle);
			}

			proximityEntities.RemoveIndex(i);
		}
	}
}

void spGamePortal::AddProximityEntity(const idEntity* other)
{
	// Go through the list and guarantee that this entity isn't in multiple times
// note:  cannot use IdList::AddUnique, because the lastPortalPoint might be different during this add
	for (int i = 0; i < proximityEntities.Num(); i++) {
		if (proximityEntities[i].entity.GetEntity() == other) {
			return;
		}
	}

	// Add this entity to the potential portal list
	ProximityEntity prox;
	prox.entity = other;
	prox.lastPortalPoint = ((idEntity*)(other))->GetPhysics()->GetOrigin();

	proximityEntities.Append(prox);

	// If the entity is a player, then inform the player that they are close to this portal
	// needed for weapon projectile firing
	if (other->IsType(idPlayer::Type)) {
		idPlayer* player = (idPlayer*)(other);
		//player->SetPortalColliding(true);
	}
}
