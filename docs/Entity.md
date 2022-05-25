This page explains each attribute definition available in each C++ Entity.
Each attribute is case insensitive.

| Attribute | Default Value | Description                                       |
|-----------|---------|---------------------------------------------------|
| classname | `NULL` | The entity class to use for an entity definition |
| noGrab    | `0` | If 1, don't allow the grabber to grab this entity |
| skin_xray | `""` | Specifies the idDeclSkin this entity uses |
| cameraTarget | `""` | Specifies the entity to look at for security cams and cinematics |
| solidForTeam | `0` | Specifies behavior of physics objects in 'teams' |
| neverDormant | `0` | TODO |
| hide | `0` | If true this entity is not visible |
| cinematic | `0` | TODO |
| networksync | `1` | TODO |
| name | `<baseclassname>_<classname>_<entityNumber>` | Unique name for the entity |
| target |  `""` | Target entities (call Activate on them) |
| guiTarget | `""` | Target gui |
| health | `0` | Health of the entity means it can take damage |
| model | `""` | Model of the entity |
| bind | `""` | Calls EV_SpawnBind after the entity is spawned |
| scriptobject | `""` | The original doom 3 script |
| luaObject | `""` | Lua state script |
| slowmo | `1` | Which time group the entity goes in. If this value is 0, the entity goes into time group 2 unless it's multiplayer. |