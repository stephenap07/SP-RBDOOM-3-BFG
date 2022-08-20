import { StateMachine } from "./StateMachine"
import { Task } from "./Task"
import { Event } from "./EventManager"

interface ThinkableSendable {
    think(): void;
    sendEvent(entity: idEntity, event: Event): void;
}

export interface StateEntity<TEntity extends idEntity> extends ThinkableSendable {
    entity: TEntity;
    mainTask?: Task;
    stateMachine?: StateMachine<any>;
}

interface EntityMap {
    [key: string]: { new (ent: idEntity): any };
}

let entityTypes: EntityMap = {};
let entities: idEntity[] = [];

/**
 * @noSelf
 * @param theType
 */
export function RegisterStateEntity(theType: any): void {
    entityTypes[theType.name] = theType;
}

/**
 * @noSelf
 * @param ent
 * @returns 
 */
 export function Register(ent: idEntity): void {
    const stateObj = ent.getKey("stateScript");
    if( stateObj in entityTypes ) {
        entities.push(new entityTypes[stateObj](ent));
    }
    else {
        sys.warning("Could not find type for " + stateObj);
    }
}

/**
 * @noSelf
 * @param ent
 */
export function Think(ent: idEntity): void {
    ent.stateEntity.think();
}

/**
 * @noSelf
 * @param ent
 * @param event 
 */
export function SendEvent(ent: idEntity, event: Event) {
    ent.stateEntity.sendEvent(ent, event);
}