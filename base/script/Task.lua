local ____lualib = require("lualib_bundle")
local __TS__Class = ____lualib.__TS__Class
local Map = ____lualib.Map
local __TS__New = ____lualib.__TS__New
local __TS__Iterator = ____lualib.__TS__Iterator
local ____exports = {}
____exports.Task = __TS__Class()
local Task = ____exports.Task
Task.name = "Task"
function Task.prototype.____constructor(self)
    self.waitingThreads = __TS__New(Map)
end
function Task.prototype.create(self, func)
    local co = coroutine.create(func)
    coroutine.resume(co)
end
function Task.prototype.execute(self)
    if self.waitingThreads.size == 0 then
        return
    end
    local threadsToWakeUp = {}
    local currTime = sys:getTime()
    for ____, ____value in __TS__Iterator(self.waitingThreads) do
        local co = ____value[1]
        local wakeUpTime = ____value[2]
        if wakeUpTime <= currTime then
            threadsToWakeUp[#threadsToWakeUp + 1] = co
        end
    end
    do
        local i = 0
        while i < #threadsToWakeUp do
            local co = threadsToWakeUp[i + 1]
            self:deleteWaitingTask(co)
            if co then
                coroutine.resume(co)
            end
            i = i + 1
        end
    end
end
function Task.prototype.wait(self, seconds)
    local co = coroutine.running()
    if not co then
        sys:warning("Attempting to suspend the main thread.")
        return
    end
    self.waitingThreads:set(
        co,
        sys:getTime() + seconds
    )
    coroutine.yield()
end
function Task.prototype.waitFrame(self)
    self:wait(0)
end
function Task.prototype.deleteWaitingTask(self, co)
    self.waitingThreads:delete(co)
end
function Task.prototype.clear(self)
    self.waitingThreads:clear()
end
return ____exports
