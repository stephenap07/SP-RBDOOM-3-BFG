-- Main entry point
if os.getenv("LOCAL_LUA_DEBUGGER_VSCODE") == "1" then
    require("lldebugger").start()
end
tech4 = require "main"
EntitySystem = require "EntitySystem"