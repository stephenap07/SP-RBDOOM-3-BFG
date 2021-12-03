-- This file implements waitSeconds, waitSignal, signal, and their supporting stuff.

local wait_support = {}

-- This table is indexed by coroutine and simply contains the time at which the coroutine
-- should be woken up.
local WAITING_ON_TIME = {}

-- This table is indexed by signal and contains list of coroutines that are waiting
-- on a given signal
local WAITING_ON_SIGNAL = {}

local SIGNAL_STATES = {}

-- Keep track of how long the game has been running.
local CURRENT_TIME = 0

function wait_support.Init(currentSeconds)
    CURRENT_TIME = currentSeconds
end

function wait_support.WaitSeconds(seconds)
    -- Grab a reference to the current running coroutine.
    local co = coroutine.running()

    -- If co is nil, that means we're on the main process, which isn't a coroutine and can't yield
    assert(co ~= nil, "The main thread cannot wait!")

    -- Store the coroutine and its wakeup time in the WAITING_ON_TIME table
    local wakeupTime = CURRENT_TIME + seconds
    WAITING_ON_TIME[co] = wakeupTime

    --sys:print("Thread(" .. tostring(co) .. ") is waiting until " .. wakeupTime .. "\n")

    -- And suspend the process
    return coroutine.yield(co)
end

local function hasValue (tab, val)
    for index, value in ipairs(tab) do
        if value == val then
            return true
        end
    end

    return false
end

local function addThreadsToWake()
    local threadsToWake = {}
    local threadsToKeepAsleep = {}
    for co, wakeupTime in pairs(WAITING_ON_TIME) do
        --sys:print("Waiting for " .. tostring(co) .. " to wake up until " .. wakeupTime .. ", time " .. CURRENT_TIME .. "\n")
        if wakeupTime <= CURRENT_TIME then
            --sys:print("Waking up thread(" .. tostring(co) .. ") after time " .. wakeupTime .. "\n")
            table.insert(threadsToWake, co)
        else
            table.insert(threadsToKeepAsleep, co)
        end
    end

    return threadsToWake, threadsToKeepAsleep
end

function wait_support.WakeUpWaitingThreads(deltaTime)
    -- This function should be called once per game logic update with the amount of time
    -- that has passed since it was last called
    CURRENT_TIME = CURRENT_TIME + deltaTime

    local threadsToWake, threadsToKeepAsleep  = addThreadsToWake()

    -- First, grab a list of the threads that need to be woken up. They'll need to be removed
    -- from the WAITING_ON_TIME table which we don't want to try and do while we're iterating
    -- through that table, hence the list.

    for signalName, signalState in pairs(SIGNAL_STATES) do
        if signalState == 1 then
            local threads = WAITING_ON_SIGNAL[signalName]
            local toRemove = {}

            if threads ~= nil then
                for k, co in ipairs(threads) do
                    if not hasValue(threadsToKeepAsleep, co) then
                        table.insert(threadsToWake, co)
                        table.insert(toRemove, k)
                    end
                end
            end

            for _, k in ipairs(toRemove) do
                table.remove(WAITING_ON_SIGNAL[signalName], co)
            end
        end
    end

    -- Now wake them all up.
    for _, co in ipairs(threadsToWake) do
        WAITING_ON_TIME[co] = nil

        coroutine.resume(co)
    end
end

function wait_support.WaitSignal(signalName)
    -- Same check as in waitSeconds; the main thread cannot wait
    local co, isMain = coroutine.running()

    assert(co ~= nil, "The main thread cannot wait!")

    if WAITING_ON_SIGNAL[signalName] == nil then
        WAITING_ON_SIGNAL[signalName] = {}
    end

    table.insert(WAITING_ON_SIGNAL[signalName], co)

    return coroutine.yield(co)
end

function wait_support.Signal(signalName)
	SIGNAL_STATES[signalName] = 1
end

function wait_support.DestroySignal(signalName)
	SIGNAL_STATES[signalName] = nil
end

function wait_support.CreateTrack(func)
    -- This function is just a quick wrapper to start a coroutine.
    local co = coroutine.create(func)
    return coroutine.resume(co)
end

return wait_support