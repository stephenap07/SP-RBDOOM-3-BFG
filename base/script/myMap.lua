
sys:print("myMap\n")

-- Register an event handler where the system object sends OnMapLoad events.
local val = RegisterEventHandler(sys, "OnMapLoad", function()
	
	sys:print("Loaded map2\n")

	--[[
	local light1 = sys:getEntity("light_2")

	light1:hide()
	--]]

end)