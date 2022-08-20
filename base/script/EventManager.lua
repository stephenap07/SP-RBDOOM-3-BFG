local ____lualib = require("lualib_bundle")
local __TS__Class = ____lualib.__TS__Class
local Set = ____lualib.Set
local __TS__New = ____lualib.__TS__New
local __TS__Iterator = ____lualib.__TS__Iterator
local ____exports = {}
____exports.EventManager = __TS__Class()
local EventManager = ____exports.EventManager
EventManager.name = "EventManager"
function EventManager.prototype.____constructor(self)
    self.listeners = {}
end
function EventManager.prototype.subscribe(self, event, func)
    if not self.listeners[event.name] then
        self.listeners[event.name] = __TS__New(Set)
    end
    local ____table_listeners_event_name_add_result_0 = self.listeners[event.name]
    if ____table_listeners_event_name_add_result_0 ~= nil then
        ____table_listeners_event_name_add_result_0 = ____table_listeners_event_name_add_result_0:add(func)
    end
end
function EventManager.prototype.unsubscribe(self, event, listener)
    self.listeners[event.name] = nil
end
function EventManager.prototype.sendEvent(self, sender, event, data)
    local listenersForEvent = self.listeners[event.name]
    if listenersForEvent then
        for ____, listener in __TS__Iterator(listenersForEvent) do
            listener(nil, data)
        end
    end
end
function EventManager.prototype.clear(self)
    self.listeners = {}
end
return ____exports
