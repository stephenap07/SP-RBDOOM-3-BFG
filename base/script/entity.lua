--[[

This module is used to manage the construction and destruction of an entity
that is accessed from lua.

Ultimately, the entity is owned and managed by the C++ side.
The RegisterEntity and DestroyEntity functions are called from C++.

]]

-- sound channels
SND_CHANNEL_ANY			= 0
SND_CHANNEL_VOICE		= 1
SND_CHANNEL_VOICE2		= 2
SND_CHANNEL_BODY		= 3
SND_CHANNEL_BODY2		= 4
SND_CHANNEL_BODY3		= 5
SND_CHANNEL_WEAPON		= 6
SND_CHANNEL_ITEM		= 7
SND_CHANNEL_HEART		= 8
SND_CHANNEL_PDA			= 9
SND_CHANNEL_DEMONIC		= 10

-- animation channels
ANIMCHANNEL_ALL			= 0
ANIMCHANNEL_TORSO		= 1
ANIMCHANNEL_LEGS		= 2
ANIMCHANNEL_HEAD		= 3
ANIMCHANNEL_EYELIDS		= 4

-- projectile states
PROJECTILE_SPAWNED		= 0
PROJECTILE_CREATED		= 1
PROJECTILE_LAUNCHED		= 2
PROJECTILE_FIZZLED		= 3
PROJECTILE_EXPLODED		= 4

-- entity name : entity object
local REGISTERED_ENTITIES = {}

EntitySystem = {}

function EntitySystem.RegisterEntity(type, entity)
	setmetatable(entity, { __index = type })

	if REGISTERED_ENTITIES[entity:getEntityNum()] ~= nil then
		return
	end

	-- Set up some added support for entity methods
	entity.ws = require("wait_support")
	
	entity.track = function(self, func)
		return self.ws:CreateTrack(func)
	end
	
	entity.signal = function(self, sig)
		return self.ws:Signal(sig)
	end
	
	entity.waitSignal = function(self, sig)
		return self.ws:WaitSignal(sig)
	end

	entity.waitSeconds = function(self, sec)
		return self.ws:WaitSeconds(sec)
	end

	entity.waitFrame = function(self)
		return self.ws:WaitSeconds(0)
	end

	entity.waitAnim = function(self, ch, anim, blendFrames)
		self:playAnim(ch, anim)
		while self:animDone(ch, blendFrames) == 0 do
			self:waitFrame()
		end
	end

	entity.waitUntil = function(self, condFunc)
		while condFunc() == false do self:waitFrame() end
	end

	if entity.Think == nil then
		entity.Think = function(self) end
	end

	REGISTERED_ENTITIES[entity:getEntityNum()] = entity;

	entity.ws:Init(sys:getTime())
	entity:Construct()
end

function EntitySystem.GetEntity(entNum)
	return REGISTERED_ENTITIES[entNum]
end

function EntitySystem.DestroyEntity(entNum)
	local entity = REGISTERED_ENTITIES[entNum]
	if entity == nil then return end
	entity:Destroy()
	REGISTERED_ENTITIES[entNum] = nil
end

-- Called once every frame
function EntitySystem.Think(entNum)
	-- Wake up the waiting threads
	local entity = REGISTERED_ENTITIES[entNum]
	if entity == nil then return end
	entity.ws:WakeUpWaitingThreads(sys:getTime())
	entity:Think()
end

function EntitySystem.ReceiveEvent(entityNum, event)
	local ent = EntitySystem.GetEntity(entityNum)
	
	if ent == nil then
		sys:warning("Failed to find entity with num " .. entityNum)
		return
	end

	if ent.EVENTS_LISTENERS == nil then
		return
	end

	if ent.EVENTS_LISTENERS[ent:getWeaponState()] == nil then
		return
	end

	local func = ent.EVENTS_LISTENERS[ent:getWeaponState()][event]

	if func == nil then
		return
	end

	func()
end