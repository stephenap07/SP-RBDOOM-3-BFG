/**
 * Wait support
 */

export class Task {
    /// Holds a mapping of lua threads to the time it needs to wake up.
    private waitingThreads = new Map<LuaThread, number>();

    /// Creates a coroutine.
    public create(func: (this: void, ...args: any[]) => any): void {
        let co = coroutine.create(func);
        coroutine.resume(co)
    }

    /// Executes the task by waking up threads and running until they all yield.
    public execute(): void {
        if (this.waitingThreads.size == 0) {
            return
        }

        let threadsToWakeUp: LuaThread[] = []

        const currTime = sys.getTime()

        for (let [co, wakeUpTime] of this.waitingThreads) {
            if (wakeUpTime <= currTime) {
                threadsToWakeUp.push(co)
            }
        }

        // TODO(Stephen): I don't really know if it's necessary to have a separate
        // loop to remove the tasks or I can just do it in
        // the iterator loop.
        for (let i = 0; i < threadsToWakeUp.length; i++) {
            let co = threadsToWakeUp[i]
            this.deleteWaitingTask(co)
            if (co) {
                coroutine.resume(co);
            }
        }
    }

    /// Put the current thread on a waiting list to be woken up in the future.
    public wait(seconds: number): void {
        let co = coroutine.running();

        // This must be a coroutine and not the main thread.
        if (!co) {
            sys.warning("Attempting to suspend the main thread.");
            return;
        }

        this.waitingThreads.set(co, sys.getTime() + seconds);

        coroutine.yield();
    }

    /// Waits one frame.
    public waitFrame(): void {
        this.wait(0);
    }

    public deleteWaitingTask(co: LuaThread) {
        this.waitingThreads.delete(co)
    }

    /// Clear out the waiting tasks.
    public clear(): void { 
        this.waitingThreads.clear();
    }
}