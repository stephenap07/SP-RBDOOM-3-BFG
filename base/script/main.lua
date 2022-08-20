local ____lualib = require("lualib_bundle")
local __TS__New = ____lualib.__TS__New
local ____exports = {}
local ____Task = require("Task")
local Task = ____Task.Task
require("WeaponShotgun")
local mainTask = __TS__New(Task)
local color = {x = 1, y = 1, z = 1}
local start = {x = 0, y = 0, z = 0}
local ____end = {x = 0, y = 0, z = 64}
mainTask:create(function()
end)
function ____exports.main()
end
function ____exports.mainThink()
end
return ____exports
