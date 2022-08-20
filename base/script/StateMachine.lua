local ____lualib = require("lualib_bundle")
local __TS__Class = ____lualib.__TS__Class
local __TS__ClassExtends = ____lualib.__TS__ClassExtends
local __TS__ObjectEntries = ____lualib.__TS__ObjectEntries
local __TS__ArrayForEach = ____lualib.__TS__ArrayForEach
local Map = ____lualib.Map
local __TS__New = ____lualib.__TS__New
local __TS__StringReplace = ____lualib.__TS__StringReplace
local __TS__ObjectKeys = ____lualib.__TS__ObjectKeys
local ____exports = {}
local ____EventManager = require("EventManager")
local EventManager = ____EventManager.EventManager
local ____Task = require("Task")
local Task = ____Task.Task
____exports.StateMachine = __TS__Class()
local StateMachine = ____exports.StateMachine
StateMachine.name = "StateMachine"
__TS__ClassExtends(StateMachine, EventManager)
function StateMachine.prototype.____constructor(self, input)
    EventManager.prototype.____constructor(self)
    self.stateMap = __TS__New(Map)
    self.currentTask = __TS__New(Task)
    if input.states then
        __TS__ArrayForEach(
            __TS__ObjectEntries(input.states),
            function(____, ____bindingPattern0)
                local state
                local name
                name = ____bindingPattern0[1]
                state = ____bindingPattern0[2]
                return self:addState(name, state)
            end
        )
    end
    self:setState(input.initial)
    sys:println("Initialized state machine")
end
function StateMachine.prototype.addState(self, name, state)
    if not self.stateMap:has(name) then
        self.stateMap:set(name, state)
    end
    return self
end
function StateMachine.prototype.setState(self, state)
    if not self.stateMap:has(state) then
        sys:warning(("State " .. state) .. " does not exist")
        return
    end
    local newState = self.stateMap:get(state)
    newState.name = state
    if self.currentState == newState then
        sys:warning("Assigning the same state " .. state)
        return
    end
    local ____this_1
    local ____table_currentState_onExit_2 = self.currentState
    if ____table_currentState_onExit_2 ~= nil then
        ____this_1 = ____table_currentState_onExit_2
        ____table_currentState_onExit_2 = ____this_1.onExit
    end
    local ____table_currentState_onExit_result_0 = ____table_currentState_onExit_2
    if ____table_currentState_onExit_result_0 ~= nil then
        ____table_currentState_onExit_result_0 = ____table_currentState_onExit_result_0(____this_1, self.currentTask)
    end
    self:clear()
    self.currentState = newState
    if self.currentState then
        self:registerState(self.currentState)
    end
    local ____this_5
    local ____table_currentState_onEnter_6 = self.currentState
    if ____table_currentState_onEnter_6 ~= nil then
        ____this_5 = ____table_currentState_onEnter_6
        ____table_currentState_onEnter_6 = ____this_5.onEnter
    end
    local ____table_currentState_onEnter_result_4 = ____table_currentState_onEnter_6
    if ____table_currentState_onEnter_result_4 ~= nil then
        ____table_currentState_onEnter_result_4 = ____table_currentState_onEnter_result_4(____this_5, self.currentTask)
    end
end
function StateMachine.prototype.execute(self)
    self.currentTask:execute()
end
function StateMachine.prototype.currentStateName(self)
    if self.currentState then
        return self.currentState.name
    end
    return nil
end
function StateMachine.prototype.clear(self)
    EventManager.prototype.clear(self)
    self.currentTask:clear()
end
function StateMachine.prototype.registerState(self, state)
    for ____, name in ipairs(__TS__ObjectKeys(state)) do
        do
            if name == "onEnter" or name == "onExit" or name == "name" then
                goto __continue16
            end
            local evName = __TS__StringReplace(name, "on", "")
            local func = state[name]
            self:subscribe({name = evName}, func)
        end
        ::__continue16::
    end
end
return ____exports
