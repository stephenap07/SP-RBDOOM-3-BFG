local ____lualib = require("lualib_bundle")
local __TS__Class = ____lualib.__TS__Class
local __TS__ClassExtends = ____lualib.__TS__ClassExtends
local __TS__New = ____lualib.__TS__New
local ____exports = {}
local ____EntitySystem = require("EntitySystem")
local RegisterStateEntity = ____EntitySystem.RegisterStateEntity
local ____Global = require("Global")
local AnimChannel = ____Global.AnimChannel
local SoundChannel = ____Global.SoundChannel
local ____StateMachine = require("StateMachine")
local StateMachine = ____StateMachine.StateMachine
local ____Weapon = require("Weapon")
local WeaponStateEntity = ____Weapon.WeaponStateEntity
require("imgui")
local SHOTGUN_FIRERATE = 1.333
local SHOTGUN_LOWAMMO = 2
local SHOTGUN_RELOADRATE = 2
local SHOTGUN_NUMPROJECTILES = 13
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
local WeaponShotgun = __TS__Class()
WeaponShotgun.name = "WeaponShotgun"
__TS__ClassExtends(WeaponShotgun, WeaponStateEntity)
function WeaponShotgun.prototype.____constructor(self, entity)
    WeaponStateEntity.prototype.____constructor(self, entity)
    self.spread = 0
    self.nextAttack = 0
    self.endReload = false
    self.isAttacking = false
    self.isNetReload = false
    self.isRaiseWeapon = false
    self.stateMachine = __TS__New(
        StateMachine,
        {
            initial = "Initial",
            states = {
                Initial = {onEnter = function()
                    sys:println("Initial state")
                    self.nextAttack = sys:getTime()
                    self.entity:weaponState("Raise", 0)
                end},
                Raise = {onEnter = function(____, task)
                    sys:println("Raise state")
                    self.entity:weaponRising()
                    self.entity:playAnim(AnimChannel.All, "raise")
                    task:create(function()
                        while self.entity:animDone(AnimChannel.All, SHOTGUN_RAISE_TO_IDLE) ~= 1 do
                            task:waitFrame()
                        end
                        self.entity:weaponState("Idle", 1)
                    end)
                end},
                Lower = {
                    onEnter = function(____, task)
                        sys:println("lowering")
                        self.entity:weaponLowering()
                        self.entity:playAnim(AnimChannel.All, "putaway")
                        task:create(function()
                            while self.entity:animDone(AnimChannel.All, SHOTGUN_RAISE_TO_IDLE) ~= 1 do
                                task:waitFrame()
                            end
                            self.entity:weaponHolstered()
                        end)
                    end,
                    onRaiseWeapon = function()
                        return self.entity:setWeaponState("Raise")
                    end
                },
                Idle = {
                    onEnter = function(____, task)
                        sys:println("Idle state")
                        if self.entity:ammoInClip() == 0 then
                            sys:println("weapon out of ammo")
                            self.entity:weaponOutOfAmmo()
                            return
                        else
                            sys:println("weapon ready")
                            self.entity:weaponReady()
                        end
                        task:create(function()
                            sys:println("Before loop")
                            while true do
                                if imgui.isReadyToRender() then
                                    imgui.beginWindow("Weapon state", imgui.WindowFlags.NoNavFocus)
                                    imgui.text("Idle state")
                                    imgui.endWindow()
                                end
                                task:waitFrame()
                            end
                        end)
                        self.entity:playCycle(AnimChannel.All, "idle")
                    end,
                    onLowerWeapon = function()
                        self.entity:weaponState("Lower", SHOTGUN_IDLE_TO_LOWER)
                        self.isRaiseWeapon = false
                    end,
                    onHolster = function()
                        self.isRaiseWeapon = false
                    end,
                    onBeginAttack = function()
                        sys:println("Attack")
                        local currentTime = sys:getTime()
                        local ammoClip = self.entity:ammoInClip()
                        local ammoAvailable = self.entity:ammoAvailable()
                        if currentTime >= self.nextAttack then
                            if ammoClip > 0 then
                                self.nextAttack = sys:getTime() + SHOTGUN_FIRERATE
                                self.entity:weaponState("Fire", SHOTGUN_IDLE_TO_FIRE)
                            elseif ammoAvailable > 0 then
                                if self.entity:autoReload() then
                                    self.entity:netReload()
                                    self.entity:weaponState("Reload", SHOTGUN_IDLE_TO_RELOAD)
                                else
                                    self.entity:weaponState("NoAmmo", SHOTGUN_IDLE_TO_NOAMMO)
                                end
                            else
                                self.entity:weaponState("NoAmmo", SHOTGUN_IDLE_TO_NOAMMO)
                            end
                        end
                    end,
                    onReload = function()
                        local ammoClip = self.entity:ammoInClip()
                        local clipSize = self.entity:clipSize()
                        local ammoAvailable = self.entity:ammoAvailable()
                        if ammoAvailable > ammoClip and ammoClip < clipSize then
                            self.entity:netReload()
                            self.entity:weaponState("Reload", SHOTGUN_IDLE_TO_RELOAD)
                        end
                    end,
                    onNetReload = function()
                        self.isNetReload = true
                        self.entity:weaponState("Reload", SHOTGUN_IDLE_TO_RELOAD)
                    end
                },
                Fire = {
                    onEnter = function(____, task)
                        sys:println("Start fire")
                        if self.isNetReload then
                            self.isNetReload = false
                            self.entity:weaponState("Reload", SHOTGUN_IDLE_TO_RELOAD)
                            return
                        end
                        local ammoClip = self.entity:ammoInClip()
                        if ammoClip == SHOTGUN_LOWAMMO then
                            self.entity:startSound("snd_lowammo", SoundChannel.Item, 1)
                        end
                        self.entity:launchProjectiles(
                            SHOTGUN_NUMPROJECTILES,
                            self.spread,
                            0,
                            1,
                            1
                        )
                        self.entity:playAnim(AnimChannel.All, "fire")
                        task:create(function()
                            while self.entity:animDone(AnimChannel.All, SHOTGUN_FIRE_TO_IDLE) ~= 1 do
                                task:waitFrame()
                            end
                            self.entity:weaponState("Idle", SHOTGUN_FIRE_TO_IDLE)
                        end)
                        task:create(function()
                            while true do
                                if imgui.isReadyToRender() then
                                    imgui.beginWindow("Poop", imgui.WindowFlags.None)
                                    imgui.text("Fire state")
                                    imgui.endWindow()
                                end
                                task:waitFrame()
                            end
                        end)
                    end,
                    onWeaponNetReload = function()
                        self.entity:weaponState("Reload", SHOTGUN_IDLE_TO_RELOAD)
                    end
                },
                Reload = {
                    onEnter = function(____, task)
                        self.isAttacking = false
                        self.endReload = false
                        sys:println("Start reload")
                        local clipSize = self.entity:clipSize()
                        local ammoAvail = self.entity:ammoAvailable()
                        local ammoClip = self.entity:ammoInClip()
                        self.entity:weaponReloading()
                        self.entity:playAnim(AnimChannel.All, "reload_start")
                        task:create(function()
                            while self.entity:animDone(AnimChannel.All, 0) ~= 1 do
                                task:waitFrame()
                            end
                            while ammoClip < clipSize and ammoClip < ammoAvail do
                                self.entity:weaponReloading()
                                self.entity:playAnim(AnimChannel.All, "reload_loop")
                                self.entity:waitFrame()
                                if self.isAttacking and ammoClip > 0 then
                                    break
                                end
                                if not self.isRaiseWeapon or self.endReload then
                                    break
                                end
                                while self.entity:animDone(AnimChannel.All, 0) ~= 1 do
                                    task:waitFrame()
                                end
                                self.entity:addToClip(SHOTGUN_RELOADRATE)
                                self.entity:weaponReady()
                                task:waitFrame()
                                if self.isAttacking or not self.isRaiseWeapon or self.endReload then
                                    break
                                end
                                ammoAvail = self.entity:ammoAvailable()
                                ammoClip = self.entity:ammoInClip()
                            end
                            self.entity:netEndReload()
                            self.entity:playAnim(AnimChannel.All, "reload_end")
                            task:waitFrame()
                            while self.entity:animDone(AnimChannel.All, SHOTGUN_RELOAD_TO_IDLE) == 0 do
                                if not self.isRaiseWeapon then
                                    self.entity:weaponState("Lower", SHOTGUN_RELOAD_TO_LOWER)
                                    return
                                elseif self.isAttacking then
                                    self.entity:weaponState("Fire", SHOTGUN_RELOAD_TO_FIRE)
                                    return
                                end
                                task:waitFrame()
                            end
                            self.entity:weaponState("Idle", SHOTGUN_RELOAD_TO_IDLE)
                        end)
                    end,
                    onBeginAttack = function()
                        self.isAttacking = true
                    end,
                    onEndAttack = function()
                        self.isAttacking = false
                    end,
                    onLowerWeapon = function()
                        self.isRaiseWeapon = false
                    end,
                    onHolster = function()
                        self.isRaiseWeapon = false
                    end,
                    onEndReload = function()
                        self.endReload = true
                    end
                },
                NoAmmo = {onEnter = function(____, task)
                    if self.isNetReload then
                        self.isNetReload = false
                        self.entity:weaponState("Reload", SHOTGUN_NOAMMO_TO_RELOAD)
                        return
                    end
                    self.entity:playAnim(AnimChannel.All, "noammo")
                    task:create(function()
                        while self.entity:animDone(AnimChannel.All, SHOTGUN_NOAMMO_TO_IDLE) ~= 1 do
                            task:waitFrame()
                        end
                        self.entity:weaponState("Lower", SHOTGUN_NOAMMO_TO_IDLE)
                    end)
                end}
            }
        }
    )
    self.spread = self.entity:getFloatKey("spread")
    sys:println((("Constructed shotgun for " .. self.entity:getName()) .. ", id = ") .. tostring(self.entity:getEntityNum()))
end
RegisterStateEntity(WeaponShotgun)
return ____exports
