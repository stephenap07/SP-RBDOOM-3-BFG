local ____lualib = require("lualib_bundle")
local __TS__New = ____lualib.__TS__New
local ____exports = {}
local entityTypes = {}
local entities = {}
function ____exports.RegisterStateEntity(theType)
    entityTypes[theType.name] = theType
end
function ____exports.Register(ent)
    local stateObj = ent:getKey("stateScript")
    if entityTypes[stateObj] ~= nil then
        entities[#entities + 1] = __TS__New(entityTypes[stateObj], ent)
    else
        sys:warning("Could not find type for " .. stateObj)
    end
end
function ____exports.Think(ent)
    ent.stateEntity:think()
end
function ____exports.SendEvent(ent, event)
    ent.stateEntity:sendEvent(ent, event)
end
return ____exports
