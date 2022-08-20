import { IState, StateMachine } from "./StateMachine"
import { Task } from "./Task"
import { StateEntity } from "./EntitySystem"
import { Event } from "./EventManager"

type StateFunction = (t?: Task) => void;

interface BasicWeaponState {
    onBeginAttack?: StateFunction;
    onEndAttack?: StateFunction;
    onReload?: StateFunction;
    onEndReload?: StateFunction;
    onRaiseWeapon?: StateFunction;
    onLowerWeapon?: StateFunction;
    onHolster?: StateFunction;
    onNetReload?: StateFunction;
    onWeaponNetReload?: StateFunction;
}

export type DefaultWeaponState = IState & BasicWeaponState;

export class WeaponStateEntity implements StateEntity<idWeapon> {
    constructor(entity: idWeapon)
    {
        this.entity = entity;
        this.entity.stateEntity = this;
        this.mainTask = new Task();
        this.mainTask.create((): void => { this.runStateTask(); });
    }

    think(): void {
        this.mainTask.execute();
    }

    sendEvent(entity: idEntity, event: Event): void {
        this.stateMachine?.sendEvent(entity, event);
    }

    runStateTask() : void {
        while (true) {
            this.stateMachine?.execute();
            let weaponState = this.entity.getWeaponState();
            if (this.stateMachine?.currentStateName() != weaponState) {
                this.stateMachine?.setState(weaponState)
            }
            this.mainTask.waitFrame();
        }
    }

    entity: idWeapon;
    mainTask: Task;
    stateMachine?: StateMachine<DefaultWeaponState> = undefined;
}