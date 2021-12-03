-- Mr Squid State Script

mrsquid = {}

local on = RegisterEventHandler

mrsquid.init = function(this)
	local state = 'waiting'
	local prevState = nil

	local stateHandlers = {}
	
	sys:print("Start " .. this:getName() .. "\n")

	function go(st)
		state = st
	end
	
	function processState(st)
		if st == 'waiting' then
			table.insert(stateHandlers,
				on(this, 'onUse',
				function(...)
					sys:print('Used!!!\n')
				end))
		end
	end

	local color = { x = 1, y = 1, z = 1 }

	local headJoint = this:getJointHandle("DEF-HEAD")

	id4.CreateTrack(function ()
		while true do
			id4.WaitSeconds(0);
	
			local pos = this:getJointPos(headJoint)
			pos.z = pos.z + 20
			sys:drawText("State: " .. state, pos, 1, color, 1, 1 );
		end
	end)

	id4.CreateTrack(function()
		while true do
			id4.WaitSeconds(0);

			if prevState == state then
				-- yield if there are no state changes.
				id4.WaitSeconds(0);
			else	
				-- handle a state change.
				prevState = state
	
				-- erase the state's event handlers
				for _,k in ipairs(stateHandlers) do
					RemoveEventHandler(this, k)
				end

				-- allow the event handlers to be GC'd
				stateHandlers = {}
	
				processState(state);
			end
		end
	end)


end