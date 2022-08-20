import { EventManager } from "./EventManager";
import { Task } from "./Task";

export interface IState {
    onEnter?: (task: Task) => void;
    onExit?: (task: Task) => void;
}

interface IStateWithName extends IState {
    name: string;
}

interface StateMachineInput<TState extends IState> {
    initial: string;
    states?: { [key: string]: TState; };
};

export class StateMachine<TState extends IState> extends EventManager {
    private currentState?: IStateWithName;
    private stateMap = new Map<string, IState>();
    private currentTask = new Task();

    constructor(input: StateMachineInput<TState>) {
        super();
        if(input.states) {
            Object.entries(input.states).forEach(([name, state]) => this.addState(name, state));
        }
        this.setState(input.initial);
        sys.println("Initialized state machine");
    }

    public addState(name: string, state: TState): StateMachine<TState> {
        if (!this.stateMap.has(name)) {
            this.stateMap.set(name, state)
        }
        return this;
    }

    public setState(state: string) {
        if (!this.stateMap.has(state)) {
            sys.warning("State " + state + " does not exist")
            return
        }

        // TODO(Stephen): How do I combine types so that I don't have to use the `any` keyword?
        let newState: any = this.stateMap.get(state);
        newState.name = state; 

        if (this.currentState == newState) {
            sys.warning("Assigning the same state " + state)
            return;
        }

        this.currentState?.onExit?.(this.currentTask);
        this.clear();
        this.currentState = newState;
        if (this.currentState) {
            this.registerState(this.currentState);
        }
        this.currentState?.onEnter?.(this.currentTask);
    }

    public execute() {
        this.currentTask.execute()
    }

    public currentStateName(): string | null {
        if (this.currentState) {
            return this.currentState.name;
        }
        return null
    }

    public clear() {
        super.clear()
        this.currentTask.clear()
    }
    
    private registerState(state: IState) {
        for (let name of Object.keys(state)) {
            if (name == 'onEnter' || name == 'onExit' || name == 'name') {
                continue;
            }
            let evName = name.replace('on', '');
            // TODO: Maybe make this a bit more typesafe. Is there a way to extract
            // just the event functions?
            let func = (state as any)[name];
            this.subscribe({name: evName}, func);
        }
    }
}