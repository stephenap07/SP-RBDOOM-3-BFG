local stateScripts = {}

function DefineStateScript(name, stateScript)
	stateScript[name] = stateScript
end

local stateScript = {}
local currentState = nil
local oldState = nil

function storeState(newState)
  oldState = currentState
  currentState = newState
end

function restoreState()
  currentState = oldState
end

function state(stateName)
  local mt = {}
  mt.__call = function() return stateScript[stateName] end

  stateScript[stateName] = {}
  setmetatable(stateScript[stateName], mt)
  
  stateScript[stateName].on = function(eventName, f)
    sys:print("registered event " ..eventName.."\n")
    stateScript[stateName]["on_" .. eventName] = function()
      storeState(stateName)
      local r = f()
      restoreState()
      return r
    end

    return stateScript[stateName]
  end

  return stateScript[stateName]
end