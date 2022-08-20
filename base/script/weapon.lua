local ____lualib = require("lualib_bundle")
local __TS__Class = ____lualib.__TS__Class
local __TS__New = ____lualib.__TS__New
local ____exports = {}
local ____Task = require("Task")
local Task = ____Task.Task
____exports.WeaponStateEntity = __TS__Class()
local WeaponStateEntity = ____exports.WeaponStateEntity
WeaponStateEntity.name = "WeaponStateEntity"
function WeaponStateEntity.prototype.____constructor(self, entity)
    self.stateMachine = nil
    self.entity = entity
    self.entity.stateEntity = self
    self.mainTask = __TS__New(Task)
    self.mainTask:create(function()
        self:runStateTask()
    end)
end
function WeaponStateEntity.prototype.think(self)
    self.mainTask:execute()
end
function WeaponStateEntity.prototype.sendEvent(self, entity, event)
    local ____table_stateMachine_sendEvent_result_0 = self.stateMachine
    if ____table_stateMachine_sendEvent_result_0 ~= nil then
        ____table_stateMachine_sendEvent_result_0 = ____table_stateMachine_sendEvent_result_0:sendEvent(entity, event)
    end
end
function WeaponStateEntity.prototype.runStateTask(self)
    while true do
        local ____table_stateMachine_execute_result_2 = self.stateMachine
        if ____table_stateMachine_execute_result_2 ~= nil then
            ____table_stateMachine_execute_result_2 = ____table_stateMachine_execute_result_2:execute()
        end
        local weaponState = self.entity:getWeaponState()
        local ____table_stateMachine_currentStateName_result_4 = self.stateMachine
        if ____table_stateMachine_currentStateName_result_4 ~= nil then
            ____table_stateMachine_currentStateName_result_4 = ____table_stateMachine_currentStateName_result_4:currentStateName()
        end
        if ____table_stateMachine_currentStateName_result_4 ~= weaponState then
            local ____table_stateMachine_setState_result_6 = self.stateMachine
            if ____table_stateMachine_setState_result_6 ~= nil then
                ____table_stateMachine_setState_result_6 = ____table_stateMachine_setState_result_6:setState(weaponState)
            end
        end
        self.mainTask:waitFrame()
    end
end
return ____exports
