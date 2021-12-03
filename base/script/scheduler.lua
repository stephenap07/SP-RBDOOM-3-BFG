-- Scheduler

local eventHandlers = {}

local function getName(classInst)
	if classInst.getName then
		return classInst:getName()
	else
		return 'sys'
	end
end

function RegisterEventHandler(sender, eventName, func)
	local senderId = getName(sender)
	
	if not eventHandlers[senderId] then
		eventHandlers[senderId] = {}
	end

	local entry = eventHandlers[senderId][eventName]

	if not entry then
		-- This event isn't registered in the scheduler. Add it.
		entry = {}
		entry.count = 0
		entry.freeIndex = {}
		entry.handlers = {}
		eventHandlers[senderId][eventName] = entry
	end

	entry.count = entry.count + 1

	local eventKey = entry.count

	if #entry.freeIndex > 0 then
		-- Pop from the table
		eventKey = table.remove(entry.freeIndex)
	end

	entry.handlers[eventKey] = func

	eventHandlers[senderId][eventName] = entry

	for k,v in pairs(eventHandlers[senderId]) do
		sys:print("Event: " .. k .. "\n")
	end

	return {
		key = eventKey,
		name = eventName
	}
end

function RemoveEventHandler(sender, eventData)
	local senderId = getName(sender)
	if not eventData.key and not eventData.name then
		print("Error: Invalid event data passed to RemoveEventHandler")
	end

	-- Unset
	local entry = eventHandlers[senderId][eventData.name]
	entry.handlers[eventData.key] = nil
	table.insert(entry.freeIndex, eventData.key)
	entry.count = entry.count - 1
end

local function onNilHandler()
	error("Found a nil handler!\n")
end

function ReceiveEvent(sender, eventName, ...)
	local senderId = getName(sender)

	if eventHandlers[senderId] == nil then
		sys:print("registered event handlers:\n")

		for classPtr, eventData in pairs(eventHandlers) do
			sys:print("\t"..tostring(classPtr) .. ": " .. tostring(eventData) .. '\n')
		end
		
		error("No event handler registered for event " .. eventName .. '\n')
	end

	for eventName, eventHandler in pairs(eventHandlers[senderId]) do
		if eventName == eventName then
			--sys:print("Found event handler for " .. eventName .. '\n')
			for _, handler in ipairs(eventHandler.handlers) do
				(handler or onNilHandler)()
			end
		end
	end
end