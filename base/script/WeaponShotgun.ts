import { RegisterStateEntity } from "./EntitySystem"
import { AnimChannel, SoundChannel } from "./Global"
import { StateMachine } from "./StateMachine"
import { Task } from "./Task"
import { DefaultWeaponState, WeaponStateEntity } from "./Weapon"
import "./imgui"

const SHOTGUN_FIRERATE = 1.333
const SHOTGUN_LOWAMMO = 2
const SHOTGUN_RELOADRATE = 2
const SHOTGUN_NUMPROJECTILES = 13

// blend times
const SHOTGUN_IDLE_TO_LOWER = 4
const SHOTGUN_IDLE_TO_FIRE = 1
const SHOTGUN_IDLE_TO_RELOAD = 4
const SHOTGUN_IDLE_TO_NOAMMO = 4
const SHOTGUN_NOAMMO_TO_RELOAD = 4
const SHOTGUN_NOAMMO_TO_IDLE = 4
const SHOTGUN_RAISE_TO_IDLE = 1
const SHOTGUN_FIRE_TO_IDLE = 4
const SHOTGUN_RELOAD_TO_IDLE = 4
const SHOTGUN_RELOAD_TO_FIRE = 4
const SHOTGUN_RELOAD_TO_LOWER = 2

class WeaponShotgun extends WeaponStateEntity {

    spread: number = 0;
    nextAttack: number = 0;
    endReload: boolean = false;
    isAttacking: boolean = false;
    isNetReload: boolean = false;
    isRaiseWeapon: boolean = false;

    constructor(entity: idWeapon) {
        super(entity);
        this.spread = this.entity.getFloatKey("spread");
        sys.println(`Constructed shotgun for ${this.entity.getName()}, id = ${this.entity.getEntityNum()}`);
    }

    stateMachine = new StateMachine<DefaultWeaponState>({
        initial: "Initial",
        states: {
            Initial: {
                onEnter: (): void => {
                    sys.println("Initial state")
                    this.nextAttack = sys.getTime()
                    this.entity.weaponState("Raise", 0)
                }
            },

            Raise: {
                onEnter: (task: Task): void => {
                    sys.println("Raise state");
                    this.entity.weaponRising();
                    this.entity.playAnim(AnimChannel.All, "raise");
                    task.create((): void => {
                        while (this.entity.animDone(AnimChannel.All, SHOTGUN_RAISE_TO_IDLE) != 1) {
                            task.waitFrame();
                        }
                        this.entity.weaponState("Idle", 1);
                    });
                }
            },

            Lower: {
                onEnter: (task: Task): void => {
                    sys.println("lowering")
                    this.entity.weaponLowering();
                    this.entity.playAnim(AnimChannel.All, "putaway");
                    task.create((): void => {
                        while (this.entity.animDone(AnimChannel.All, SHOTGUN_RAISE_TO_IDLE) != 1) {
                            task.waitFrame();
                        }
                        this.entity.weaponHolstered();
                    })
                },
                onRaiseWeapon: (): void => {
                    return this.entity.setWeaponState("Raise");
                }
            },

            Idle: {
                onEnter: (task: Task): void => {
                    sys.println("Idle state");
        
                    if (this.entity.ammoInClip() == 0) { 
                        sys.println("weapon out of ammo");
                        this.entity.weaponOutOfAmmo();
                        return;
                    } else {
                        sys.println("weapon ready");
                        this.entity.weaponReady();
                    }

                    task.create((): void => {
                        sys.println("Before loop");
                        while (true) {
                            if (imgui.isReadyToRender()) {
                                imgui.beginWindow("Weapon state", imgui.WindowFlags.NoNavFocus);
                                imgui.text("Idle state");
                                imgui.endWindow();
                            }
                            task.waitFrame();
                        }
                    });
                
                    this.entity.playCycle(AnimChannel.All, "idle");
                },
                onLowerWeapon: (): void => {
                    this.entity.weaponState("Lower", SHOTGUN_IDLE_TO_LOWER);
                    this.isRaiseWeapon = false;
                },
                onHolster: (): void => {
                    this.isRaiseWeapon = false;
                },
                onBeginAttack: (): void => {
                    sys.println("Attack");
        
                    const currentTime = sys.getTime();
                    const ammoClip = this.entity.ammoInClip();
                    const ammoAvailable = this.entity.ammoAvailable();
        
                    if (currentTime >= this.nextAttack) {
                        if (ammoClip > 0) {
                            this.nextAttack = sys.getTime() + SHOTGUN_FIRERATE;
                            this.entity.weaponState("Fire", SHOTGUN_IDLE_TO_FIRE);
                        }
                        else if (ammoAvailable > 0) {
                            if (this.entity.autoReload()) {
                                this.entity.netReload();
                                this.entity.weaponState("Reload", SHOTGUN_IDLE_TO_RELOAD);
                            }
                            else {
                                this.entity.weaponState("NoAmmo", SHOTGUN_IDLE_TO_NOAMMO);
                            }
                        }
                        else {
                            this.entity.weaponState("NoAmmo", SHOTGUN_IDLE_TO_NOAMMO);
                        }
                    }
                },
                onReload: (): void => {
                    const ammoClip = this.entity.ammoInClip();
                    const clipSize = this.entity.clipSize();
                    const ammoAvailable = this.entity.ammoAvailable();
                    if (ammoAvailable > ammoClip && ammoClip < clipSize) {
                        this.entity.netReload();
                        this.entity.weaponState("Reload", SHOTGUN_IDLE_TO_RELOAD);
                    }
                },
                onNetReload: (): void => {
                    this.isNetReload = true;
                    this.entity.weaponState("Reload", SHOTGUN_IDLE_TO_RELOAD);
                }
            },
            Fire: {
                onEnter: (task: Task): void => {
                    sys.println("Start fire");
        
                    if (this.isNetReload) {
                        this.isNetReload = false;
                        this.entity.weaponState("Reload", SHOTGUN_IDLE_TO_RELOAD);
                        return;
                    }
            
                    const ammoClip = this.entity.ammoInClip();
                    if (ammoClip == SHOTGUN_LOWAMMO) {
                        this.entity.startSound("snd_lowammo", SoundChannel.Item, 1);
                    }
        
                    this.entity.launchProjectiles(SHOTGUN_NUMPROJECTILES, this.spread, 0, 1.0, 1.0);
                    this.entity.playAnim(AnimChannel.All, "fire");

                    task.create((): void => {
                        while (this.entity.animDone(AnimChannel.All, SHOTGUN_FIRE_TO_IDLE) != 1) {
                            task.waitFrame();
                        }
                        this.entity.weaponState("Idle", SHOTGUN_FIRE_TO_IDLE);
                    })

                    task.create((): void => {
                        while (true) {
                            if (imgui.isReadyToRender()) {
                                imgui.beginWindow("Poop", imgui.WindowFlags.None);
                                imgui.text("Fire state");
                                imgui.endWindow();
                            }
                            task.waitFrame();
                        }
                    });
                },
                onWeaponNetReload: (): void => {
                    this.entity.weaponState("Reload", SHOTGUN_IDLE_TO_RELOAD);
                }
            },
            Reload: {
                onEnter: (task: Task): void => {
                    this.isAttacking = false;
                    this.endReload = false;
        
                    sys.println("Start reload");
        
                    let clipSize = this.entity.clipSize();
                    let ammoAvail = this.entity.ammoAvailable();
                    let ammoClip = this.entity.ammoInClip();
        
                    this.entity.weaponReloading();
                    this.entity.playAnim(AnimChannel.All, "reload_start");
        
                    task.create((): void => {
                        while (this.entity.animDone(AnimChannel.All, 0) != 1) {
                            task.waitFrame();
                        }
                        
                        while (ammoClip < clipSize && ammoClip < ammoAvail) {
                            this.entity.weaponReloading();
                            this.entity.playAnim(AnimChannel.All, "reload_loop");
                            this.entity.waitFrame();
        
                            // allow the player to shoot or switch weapons since shotgun is so slow reloading
                            if (this.isAttacking && ammoClip > 0) {
                                break;
                            }
                        
                            if (!this.isRaiseWeapon || this.endReload) {
                                break;
                            }
                        
                            while (this.entity.animDone(AnimChannel.All, 0) != 1) {
                                task.waitFrame();
                            }
                        
                            this.entity.addToClip(SHOTGUN_RELOADRATE);
                            this.entity.weaponReady();
                            task.waitFrame();
                        
                            if (this.isAttacking || !this.isRaiseWeapon || this.endReload) {
                                // allow the player to shoot or switch weapons since shotgun is so slow reloading
                                break;
                            }
                        
                            ammoAvail = this.entity.ammoAvailable();
                            ammoClip = this.entity.ammoInClip();
                        }
                        
                        this.entity.netEndReload();
                        this.entity.playAnim(AnimChannel.All, "reload_end");
                        task.waitFrame();
                        
                        while (this.entity.animDone(AnimChannel.All, SHOTGUN_RELOAD_TO_IDLE) == 0) {
                            if (!this.isRaiseWeapon) {
                                this.entity.weaponState("Lower", SHOTGUN_RELOAD_TO_LOWER);
                                return;
                            }
                            else if (this.isAttacking) {
                                this.entity.weaponState("Fire", SHOTGUN_RELOAD_TO_FIRE);
                                return;
                            }
                            task.waitFrame();
                        }
                        
                        this.entity.weaponState("Idle", SHOTGUN_RELOAD_TO_IDLE);
                    })
                },
                onBeginAttack: (): void => {
                    this.isAttacking = true;
                },
                onEndAttack: (): void => {
                    this.isAttacking = false;
                },
                onLowerWeapon: (): void => {
                    this.isRaiseWeapon = false;
                },
                onHolster: (): void => {
                    this.isRaiseWeapon = false;
                },
                onEndReload: (): void => {
                    this.endReload = true;
                }
            },
            NoAmmo: {
                onEnter: (task: Task): void => {
                    if (this.isNetReload) {
                        this.isNetReload = false;
                        this.entity.weaponState( "Reload", SHOTGUN_NOAMMO_TO_RELOAD );
                        return;
                    }
                
                    this.entity.playAnim( AnimChannel.All, "noammo" );
                    task.create((): void => {
                        while (this.entity.animDone(AnimChannel.All, SHOTGUN_NOAMMO_TO_IDLE) != 1) {
                            task.waitFrame()
                        }
                        this.entity.weaponState("Lower", SHOTGUN_NOAMMO_TO_IDLE)
                    })
                }
            }
        }
    });
}

RegisterStateEntity(WeaponShotgun);
