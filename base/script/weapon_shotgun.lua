local ____lualib = require("lualib_bundle")
local __TS__Class = ____lualib.__TS__Class
local __TS__New = ____lualib.__TS__New
local ____exports = {}
local ____StateMachine = require("StateMachine")
local StateMachine = ____StateMachine.StateMachine
local AnimChannel = AnimChannel or ({})
AnimChannel.All = 0
AnimChannel[AnimChannel.All] = "All"
AnimChannel.Torso = 1
AnimChannel[AnimChannel.Torso] = "Torso"
AnimChannel.Legs = 2
AnimChannel[AnimChannel.Legs] = "Legs"
AnimChannel.Head = 3
AnimChannel[AnimChannel.Head] = "Head"
AnimChannel.Eyelids = 4
AnimChannel[AnimChannel.Eyelids] = "Eyelids"
local SHOTGUN_FIRERATE = 1.333
local SHOTGUN_LOWAMMO = 2
local SHOTGUN_RELOADRATE = 2
local SHOTGUN_NUMPROJECTILES = 13
local SHOTGUN_IDLE_TO_IDLE = 0
local SHOTGUN_IDLE_TO_LOWER = 4
local SHOTGUN_IDLE_TO_FIRE = 1
local SHOTGUN_IDLE_TO_RELOAD = 4
local SHOTGUN_IDLE_TO_NOAMMO = 4
local SHOTGUN_NOAMMO_TO_RELOAD = 4
local SHOTGUN_NOAMMO_TO_IDLE = 4
local SHOTGUN_RAISE_TO_IDLE = 1
local SHOTGUN_FIRE_TO_IDLE = 4
local SHOTGUN_RELOAD_TO_IDLE = 4
local SHOTGUN_RELOAD_TO_FIRE = 4
local SHOTGUN_RELOAD_TO_LOWER = 2
local SND_CHANNEL_ANY = 0
local SND_CHANNEL_VOICE = 1
local SND_CHANNEL_VOICE2 = 2
local SND_CHANNEL_BODY = 3
local SND_CHANNEL_BODY2 = 4
local SND_CHANNEL_BODY3 = 5
local SND_CHANNEL_WEAPON = 6
local SND_CHANNEL_ITEM = 7
local SND_CHANNEL_HEART = 8
local SND_CHANNEL_PDA = 9
local SND_CHANNEL_DEMONIC = 10
local Shotgun = __TS__Class()
Shotgun.name = "Shotgun"
function Shotgun.prototype.____constructor(self)
end
function Shotgun.prototype.init(self)
    local stateMachine = __TS__New(StateMachine)
    local weapon = self
    stateMachine:addState({
        name = "Initial",
        onEnter = function()
            sys:println("Initial state")
            weapon.spread = self:getIntKey("spread")
            self.nextAttack = sys:getTime()
            weapon:weaponState("Raise", 0)
        end
    }).addState({
        name = "Raise",
        onEnter = function(____, task)
            sys:println("Raise state")
            weapon:weaponRising()
            weapon:playAnim(AnimChannel.All, "raise")
            task:create(function()
                while weapon:animDone(AnimChannel.All, SHOTGUN_RAISE_TO_IDLE) ~= 1 do
                    task:wait(0)
                end
                weapon:weaponState("Idle", 1)
            end)
        end
    }).addState({
        name = "Lower",
        onEnter = function(____, task)
            weapon:weaponLowering()
            weapon:playAnim(AnimChannel.All, "putaway")
            task:create(function()
                while weapon:animDone(AnimChannel.All, SHOTGUN_RAISE_TO_IDLE) ~= 1 do
                    task:wait(0)
                end
                weapon:weaponHolstered()
                weapon:weaponState("Idle", 1)
            end)
        end,
        onRaiseWeapon = function()
            weapon:setWeaponState("Raise")
        end
    }).addState({
        name = "Idle",
        onEnter = function(____, task)
            sys:println("Idle state")
            if weapon:ammoInClip() == 0 then
                sys:println("weapon out of ammo")
                weapon:weaponOutOfAmmo()
            else
                sys:println("weapon ready")
                weapon:weaponReady()
            end
            weapon:playCycle(AnimChannel.All, "idle")
        end,
        onLowerWeapon = function()
            weapon:weaponState("Lower", SHOTGUN_IDLE_TO_LOWER)
        end,
        onBeginAttack = function()
            sys:println("Attack")
            local currentTime = sys:getTime()
            local ammoClip = weapon:ammoInClip()
            local ammoAvailable = weapon:ammoAvailable()
            if currentTime >= weapon.nextAttack then
                if ammoClip > 0 then
                    weapon.nextAttack = sys:getTime() + SHOTGUN_FIRERATE
                    weapon:weaponState("Fire", SHOTGUN_IDLE_TO_FIRE)
                elseif ammoAvailable > 0 then
                    if weapon:autoReload() then
                        weapon:netReload()
                        weapon:weaponState("Reload", SHOTGUN_IDLE_TO_RELOAD)
                    else
                        weapon:weaponState("NoAmmo", SHOTGUN_IDLE_TO_NOAMMO)
                    end
                else
                    weapon:weaponState("NoAmmo", SHOTGUN_IDLE_TO_NOAMMO)
                end
            end
        end,
        onReload = function()
            local ammoClip = weapon:ammoInClip()
            local clipSize = weapon:clipSize()
            local ammoAvailable = weapon:ammoAvailable()
            if ammoAvailable > ammoClip and ammoClip < clipSize then
                weapon:netReload()
                weapon:weaponState("Reload", SHOTGUN_IDLE_TO_RELOAD)
            end
        end,
        onNetReload = function()
            weapon:weaponState("Reload", SHOTGUN_IDLE_TO_RELOAD)
        end
    }).addState({
        name = "Fire",
        onEnter = function(____, task)
            sys:println("Start fire")
            local ammoClip = weapon:ammoInClip()
            if ammoClip == SHOTGUN_LOWAMMO then
                weapon:startSound("snd_lowammo", SND_CHANNEL_ITEM, 1)
            end
            weapon:launchProjectiles(
                SHOTGUN_NUMPROJECTILES,
                weapon.spread,
                0,
                1,
                1
            )
            weapon:playAnim(AnimChannel.All, "fire")
            task:create(function()
                while weapon:animDone(AnimChannel.All, SHOTGUN_FIRE_TO_IDLE) ~= 1 do
                    task:wait(0)
                end
                weapon:weaponState("Idle", 1)
            end)
        end,
        onWeaponNetReload = function()
            weapon:weaponState("Reload", SHOTGUN_IDLE_TO_RELOAD)
        end
    }).addState({
        name = "Reload",
        onEnter = function(____, task)
            weapon.isAttacking = false
            weapon.lowerWeapon = false
            weapon.endReload = false
            sys:println("Start reload")
            local clipSize = weapon:clipSize()
            local ammoAvail = weapon:ammoAvailable()
            local ammoClip = weapon:ammoInClip()
            weapon:weaponReloading()
            weapon:playAnim(AnimChannel.All, "reload_start")
            task:create(function()
                while weapon:animDone(AnimChannel.All, 0) ~= 1 do
                    task:wait(0)
                end
                while ammoClip < clipSize and ammoClip < ammoAvail do
                    weapon:weaponReloading()
                    weapon:playAnim(AnimChannel.All, "reload_loop")
                    weapon:waitFrame()
                    if weapon.isAttacking and ammoClip > 0 then
                        break
                    end
                    if weapon.lowerWeapon or weapon.endReload then
                        break
                    end
                    while weapon:animDone(AnimChannel.All, 0) ~= 1 do
                        task:wait(0)
                    end
                    weapon:addToClip(SHOTGUN_RELOADRATE)
                    weapon:weaponReady()
                    task:wait(0)
                    if weapon.isAttacking or weapon.lowerWeapon or weapon.endReload then
                        break
                    end
                    ammoAvail = weapon:ammoAvailable()
                    ammoClip = weapon:ammoInClip()
                end
                weapon:netEndReload()
                weapon:playAnim(AnimChannel.All, "reload_end")
                task:wait(0)
                while weapon:animDone(AnimChannel.All, SHOTGUN_RELOAD_TO_IDLE) == 0 do
                    if weapon.lowerWeapon then
                        weapon:weaponState("Lower", SHOTGUN_RELOAD_TO_LOWER)
                        return
                    elseif weapon.isAttacking then
                        weapon:weaponState("Fire", SHOTGUN_RELOAD_TO_FIRE)
                        return
                    end
                    task:wait(0)
                end
                weapon:weaponState("Idle", SHOTGUN_RELOAD_TO_IDLE)
            end)
        end,
        onBeginAttack = function()
            weapon.isAttacking = true
        end,
        onEndAttack = function()
            weapon.isAttacking = false
        end,
        onLowerWeapon = function()
            weapon.lowerWeapon = true
        end,
        onEndReload = function()
            weapon.endReload = true
        end
    })
end
function Shotgun.prototype.think(self)
end
return ____exports
