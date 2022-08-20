import { Task } from "./Task"
import "./WeaponShotgun"

let mainTask = new Task();

let color: Vector = {x: 1, y: 1, z: 1};
let start: Vector = {x: 0, y: 0, z: 0};
let end: Vector = {x: 0, y: 0, z: 64};

mainTask.create(() => {
    // while (true) {
    //     if (imgui.isReadyToRender()) {
    //         imgui.beginWindow("Main window", imgui.WindowFlags.AlwaysAutoResize);
    //         imgui.text("Here's something from the main task");
    //         //imgui.text(`Tics per second ${sys.getTicsPerSecond()}`);
    //         imgui.endWindow();
    //         sys.debugArrow(color, start, end, 10, 0);
    //     }
    //     mainTask.waitFrame();
    // }
});

/**
 * @noSelf
 */
export function main(): void {
}

/// Called every game frame
/**
 * @noSelf
 */
export function mainThink(): void {
    //mainTask.execute();
}