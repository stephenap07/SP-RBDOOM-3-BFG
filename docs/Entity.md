This page explains each attribute definition available in each C++ Entity.
Each attribute is case insensitive.

| Attribute | Default Value | Description                                       |
|-----------|---------|---------------------------------------------------|
| spawnclass | ""     | The c++ spawn class of the entity. Must be specified. |
| classname | `NULL` | name of the entityDef |
| noGrab    | `0` | If true, don't allow the grabber to grab this entity |
| skin_xray | `""` | Specifies the idDeclSkin this entity uses |
| cameraTarget | `""` | Specifies the entity to look at for security cams and objective screenshots |
| solidForTeam | `0` | Specifies behavior of physics objects in 'teams' |
| neverDormant | `0` | If true, the entity never goes dormant |
| hide | `0` | If true this entity is not visible |
| cinematic | `0` | If true, during cinematics, entity will only think if cinematic is set |
| networksync | `1` | If true the entity is synchronized over the network |
| name | `<baseclassname>_<classname>_<entityNumber>` | Unique name for the entity |
| target |  `""` | Target entities (call Activate on them) |
| guiTarget | `""` | Target gui |
| health | `0` | Health of the entity means it can take damage |
| model | `""` | Model of the entity |
| bind | `""` | Name of the parent entity to bind to |
| bindOrientated | `""` | If true, the parent's orientation is used for binding |
| bindanim | `""` | Binds this to a specific animation in the parent |
| bindToJoint | `""` | Name of the joint to bind this entity to |
| bindToBody | `0` | If 1, binds this to parent's physics object. If 0 and this entity is an articulated figure, it will be bound to the parent with a constraint. |
| scriptobject | `""` | Doom 3 script object name of the entity |
| luaObject | `""` | Lua state script |
| slowmo | `1` | Which time group the entity goes in. If this value is 0, the entity goes into time group 2 unless it's multiplayer. |