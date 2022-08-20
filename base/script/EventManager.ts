export interface Event {
    name: string
}

export type EventData = any
export type ListenerFunc = (data?: EventData) => void

interface ListenerArray {
    [index: string]: Set<ListenerFunc> | null
}

/**
 * Interface to manage event handling with entities
 */
export class EventManager {
    private listeners: ListenerArray = {}

    public subscribe(event: Event, func: ListenerFunc) {
        if (!this.listeners[event.name]) {
            // sys.println("added subscription set for event " + event.name)
            this.listeners[event.name] = new Set<ListenerFunc>()
        }
        this.listeners[event.name]?.add(func)
    }

    public unsubscribe(event: Event, listener: ListenerFunc) {
        this.listeners[event.name] = null
    }

    public sendEvent(sender: idEntity, event: Event, data?: EventData) {
        let listenersForEvent = this.listeners[event.name]
        if (listenersForEvent) {
            for (let listener of listenersForEvent) {
                listener(data)
            }
        }
    }

    public clear() {
        this.listeners = {}
    }
}