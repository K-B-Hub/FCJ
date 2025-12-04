# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

FCJ is an Unreal Engine 5.6 cooperative platformer game featuring two independent cat characters working together. The project focuses on a modular character system where each cat has specialized abilities and players must coordinate their actions to overcome platforming challenges. The architecture emphasizes Blueprint-configurable base classes that support different cat types with unique cooperative mechanics.

## Build System

This project uses Unreal Engine 5.6's standard build system:

**IMPORTANT: Do NOT attempt to build or compile the project automatically. The user will handle all build and compilation tasks manually.**

### Development Commands
- **Generate Project Files**: Run `UnrealBuildTool -projectfiles -project="FCJ.uproject" -game -rocket -progress`
- **Build Game (Development)**: `UnrealBuildTool FCJ Win64 Development -Project="FCJ.uproject" -WaitMutex -FromMsBuild`
- **Build Editor**: `UnrealBuildTool FCJEditor Win64 Development -Project="FCJ.uproject" -WaitMutex -FromMsBuild`
- **Hot Reload**: Use Unreal Editor's Live Coding (Ctrl+Alt+F11) for C++ code changes
- **Open in Visual Studio**: Use the generated `FCJ.sln` file
- **Launch Editor**: Run `UE5Editor.exe "FCJ.uproject"` from UE5 installation directory

### Build Configuration
- **Build Settings**: V5 with Unreal 5.6 include order
- **PCH Usage**: Explicit or shared precompiled headers
- **Target Configurations**: Game and Editor targets with separate .Target.cs files

## Core Architecture

### Module Structure
- **Main Module**: `FCJ` (Runtime module with Engine and UMG dependencies)
- **Build Configuration**: Uses PCH (Precompiled Headers) with explicit/shared usage mode
- **Dependencies**: Core, CoreUObject, Engine, InputCore, EnhancedInput, AIModule, Slate, SlateCore, OnlineSubsystem, OnlineSubsystemSteam, ApplicationCore
- **Plugins**: ModelingToolsEditorMode (Editor only), MotionWarping, OnlineSubsystemSteam
- **Note**: MotionWarping plugin is enabled but the parkour system uses pure root motion with Flying movement mode without MotionWarping code dependencies

### Cooperative Platformer Character System
The project implements a modular dual-cat cooperative system designed for platformer gameplay:

1. **ACatBase**: Foundation class for all cat characters
   - Third-person camera system optimized for platforming (SpringArm + Camera)
   - Blueprint-configurable platformer settings (jump height, movement speed, air control)
   - Blueprint-configurable camera settings for optimal platforming view angles
   - Advanced wall jumping system with cooldown and air jump limits
   - **Parkour System**: Box collision-based root motion climbing system
     - Dual box collision detection (ParkourLowerBox and ParkourUpperBox)
     - Lower box must overlap with StaticMesh, upper box must not overlap
     - Uses Flying movement mode during parkour for proper Z-axis root motion
     - Pure root motion animation without Motion Warping dependency
   - Special action detection box for character-specific interactions
   - Enhanced Input system with responsive platformer controls
   - Virtual special ability system (`OnSpecialAction()` Blueprint event) for cooperative mechanics
   - Movement-responsive rotation with camera-independent controls for precise platforming
   - Base framework for character-specific abilities and cooperative interactions

2. **Specialized Cat Characters**: Each cat type brings unique abilities to cooperative gameplay
   - **AAttackCat**: Combat and parrying specialist
     - Parrying system with animation-driven mechanics via ParryingNotifyState
     - Projectile reflection during parry windows (reverses projectile velocity)
     - Object pushing system: parry pushes nearby HoldingObjects in forward direction
     - Server-authoritative physics with configurable PushForce (default: 1500.0f)
     - SpecialAction triggers parry animation montage

   - **ABiteCat**: Object manipulation and throwing specialist
     - Advanced object interaction through holding/grabbing mechanics
     - Charge-based throwing system with hold-to-charge input
     - Configurable force range (MinThrowForce: 50.0f, MaxThrowForce: 1200.0f)
     - Auto-release at max charge time (default: 1.0s)
     - Server-replicated charge state for networked gameplay
     - Detects and interacts with `AHoldingObject` instances in the world

3. **Cooperative Gameplay Controller (AMultiPlayerController)**
   - Individual directional input actions (MoveForward, MoveBackward, MoveLeft, MoveRight)
   - Combined movement system for precise platformer control with parkour lock
   - Configurable camera controls with mouse sensitivity and Y-axis inversion
   - Dynamic zoom system with configurable speed and distance limits
   - Key remapping and settings persistence system
   - Input Mapping Context with Enhanced Input system integration
   - ESC menu system with proper input mode management for multiplayer
   - Display settings persistence across level transitions
   - **SpecialAction Input Handling**:
     - ETriggerEvent::Started → PerformSpecialAction() (grab object or start charge/parry)
     - ETriggerEvent::Completed → OnSpecialActionReleased() (BiteCat charge release)

4. **Cooperative Game Mode (AMultiGameMode)**
   - Blueprint-configurable player controller and pawn classes
   - Display settings management and persistence
   - Supports cooperative gameplay initialization
   - Framework for level progression and cooperative mechanics

5. **Interactive Actor System**
   - **AWallJumpObject**: Wall surfaces that enable advanced parkour movement
     - Configurable wall jump forces (horizontal and vertical)
     - Detection distance for wall proximity
     - Wall normal calculation for realistic jump directions

   - **AHoldingObject**: Interactive objects for BiteCat manipulation
     - Configurable weight system with collision and physics properties
     - Hold offset positioning for realistic carrying animations
     - State tracking (held/released) with proper physics damping
     - Projectile damage transfer: When held by BiteCat and hit by projectile, damage/knockback transfers to BiteCat

   - **AMovingPlatform**: Linear moving platforms for dynamic level elements
     - Blueprint-configurable movement distance (FVector for X/Y/Z axes)
     - Configurable movement speed (units per second)
     - Ping-pong movement between start and target location
     - Characters attach during parkour to maintain relative position

   - **ARotationPlatform**: Rotating platforms for orientation-based puzzles
     - Configurable rotation speed per axis (Roll, Pitch, Yaw in degrees/sec)
     - Continuous rotation on specified axes
     - Characters attach during parkour to maintain relative position

6. **Projectile Combat System**
   - **AProjectile**: Advanced projectile actors with multiple behavior types
     - Straight, homing, and guided projectile variants
     - Physics-based knockback with character state awareness
     - Configurable damage, speed, and lifetime parameters
     - Distance-based auto-destruction from spawn volume
     - Damage transfer: When hitting held HoldingObject, applies damage/knockback to holding BiteCat

   - **AProjectileVolume**: Area-based projectile spawning system
     - Multiple launch modes (targeted, random, mixed)
     - Configurable spawn rates and simultaneous projectile limits
     - Player detection and targeting system
     - Blueprint-exposed Korean tooltips for designer-friendly configuration

7. **Event-Driven Trigger System**
   - **ABaseTrigger**: Abstract base class for all trigger types
     - Event-based delegate architecture (`FOnTriggerStateChanged`)
     - Replicated state with RepNotify callbacks for client synchronization
     - Server-authoritative state management
     - Invert trigger option for flexible logic
     - Virtual `GetInternalTriggerState()` for derived class implementation

   - **Trigger Variants**: Specialized trigger implementations
     - **HoldingObjectOverlapTrigger**: Activates when HoldingObject overlaps trigger volume
     - **ProjectilePassTrigger**: One-time trigger activated by projectile passing through
     - **PlayerOverlapTrigger**: Activates when player overlaps trigger volume
     - **PlayerVisitedTrigger**: One-time trigger activated when player visits location

   - **ATriggeredTurret**: Composite actor with ChildActorComponents
     - Contains ATurret and ABaseTrigger as child actors
     - Delegate-based activation (no Tick polling)
     - Blueprint-adjustable component transforms
     - Server-authoritative turret firing

   - **AMovingDoor**: Animated door that opens upward
     - Opens permanently when activated (never closes)
     - Replicated door state with RepNotify
     - Blueprint-configurable movement distance and speed
     - Timeline-based smooth animation

   - **ATriggeredDoor**: Two-trigger AND logic door system
     - Contains AMovingDoor and 2 ABaseTrigger as child actors
     - Opens only when both triggers are active simultaneously
     - One-time activation (door stays open permanently)
     - Automatic delegate unbinding after door opens

   - **AOneTriggeredDoor**: Single-trigger door system
     - Contains AMovingDoor and 1 ABaseTrigger as child actors
     - Opens when single trigger is activated
     - One-time activation (door stays open permanently)
     - Automatic delegate unbinding after door opens

8. **Zone-Based Progression System**
   - **AZoneVolume**: Puzzle zone tracking and completion system
     - `bIsPuzzleZone`: Enables ClearTrigger tracking within zone bounds
     - `ZoneNumber`: Priority for teleportation system (lower = earlier zone)
     - Automatically collects all ClearTriggers overlapping with VolumeBox
     - Broadcasts `OnZoneCleared` delegate when all triggers cleared
     - Replicated state for multiplayer synchronization

   - **AClearTrigger**: Puzzle completion wrapper
     - Wraps any BaseTrigger as ChildActorComponent
     - Detects trigger activation as puzzle completion
     - `bIsCleared` state changes dynamically with trigger state
     - Used by ZoneVolume for multi-puzzle zone tracking
     - BoxComponent for ZoneVolume overlap detection

   - **AClearDoor**: Zone-completion door system
     - Opens when associated ZoneVolume is cleared
     - Contains ZoneVolume + MovingDoor as ChildActorComponents
     - Blueprint-configurable component positioning
     - Propagates ZoneNumber to child ZoneVolume
     - One-time activation (door stays open permanently)

9. **Player Management Systems**
   - **Distance-Based Teleportation** (AMultiGameMode)
     - `MaxAllowedDistance`: 3000.0f prevents player separation
     - `DistanceCheckInterval`: 1.0s timer-based distance checks
     - Teleports player in lower ZoneNumber to higher zone location
     - Prevents level progression softlocks where one player advances too far
     - Uses ZoneVolume.ZoneNumber to determine teleport direction

   - **Respawn System** (ARespawnVolume)
     - Kill volumes with visual spawn point markers
     - BoxComponent for death trigger (overlaps with Pawn channel)
     - CapsuleComponent for editor-only spawn point visualization
     - Auto-respawns characters at SpawnPointVisualizer location
     - Resets character velocity to prevent falling momentum
     - Server-authoritative teleportation

   - **Speed Modifier System** (ACatBase)
     - Server-replicated `CurrentSpeedModifier` property
     - `ApplySpeedModifier()` with RPC pattern for client calls
     - `OnRep_SpeedModifier()` syncs speed changes to all clients
     - Used by DebuffVolume (0.5x speed in hazard zones)

10. **Level Transition & Loading System**
    - **UFCJGameInstance**: Persistent game instance across level changes
      - Manages LoadingWidget lifecycle (survives level transitions)
      - `ShowLevelLoadingWidget()` / `HideLevelLoadingWidget()` for UI control
      - `StartCheckingStreamingCompletion()` for async resource loading
      - Timer-based streaming completion checks

    - **ULoadingWidget**: Multi-state loading UI
      - SessionLoadWidget: Steam session creation/join progress
      - SessionFailContainer: Connection failure with retry button
      - LevelLoadWidget: Level streaming progress display
      - Persists across level transitions via GameInstance

### Game Mode Structure
- **AMultiGameMode**: Cooperative platformer game mode
  - Manages dual-cat cooperative gameplay with role-based character spawning
  - Distance-based teleportation system (MaxAllowedDistance: 3000.0f)
  - Handles level progression and cooperative puzzle states
  - Checkpoint system for cooperative respawning
  - Display settings management and persistence
- **AMainMenuGameMode**: Main menu and level selection
- **AMainMenuController**: UI navigation for game mode selection
- **UFCJGameInstance**: Persistent game instance for loading UI and cross-level state

### UI System
- **UMainMenuWidget**: Main menu with cooperative game options
  - Local Co-op Play: Two players on one machine
  - Multi Play: Networked cooperative play via Steam
  - Settings: Configure controls for both cats
  - Level selection and cooperative challenge modes

- **UMultiSessionWidget**: Session creation and browser for online multiplayer
  - Create server functionality with session ID generation
  - Find and join sessions by session ID
  - Player role selection (AttackCat/BiteCat) with visual indicators

- **ULobbyWidget**: Pre-game lobby for networked sessions
  - Player readiness management
  - Role assignment and swapping (0 = AttackCat, 1 = BiteCat)
  - Session information display with replicated player roles
  - Start game when all players ready

- **USettingsWidget**: Configuration interface for game settings
  - Input sensitivity and control customization
  - Key remapping interface with controller integration
  - Display and graphics settings management with immediate application
  - Resolution and window mode changes applied instantly without save button

- **UESCWidget**: In-game pause menu system
  - Non-pausing multiplayer-friendly design
  - Resume game, return to main menu, and exit options
  - Created once at controller BeginPlay and toggled via visibility
  - ESCAction binding for keyboard accessibility

- **ULoadingWidget**: Multi-state loading screen
  - SessionLoadWidget: Steam session operations
  - SessionFailContainer: Connection failure handling
  - LevelLoadWidget: Level streaming progress
  - Managed by UFCJGameInstance for persistence

## File Organization

```
Source/FCJ/
├── FCJ.cpp/h                    # Main module files
├── FCJ.Build.cs                 # Module build configuration
├── GameInstance/                # Game instance for persistent state
│   └── FCJGameInstance.cpp/h    # Loading widget & level transition management
├── GameMode/                    # Game mode implementations
│   ├── MultiGameMode.cpp/h      # Main multiplayer mode with distance teleportation
│   ├── MainMenuGameMode.cpp/h   # Menu mode
│   └── LobbyGameState.cpp/h     # Replicated lobby state with player role management
├── PlayerCharacter/             # Character implementations
│   ├── CatBase.cpp/h           # Base character class with wall jump & special actions
│   ├── AttackCat.cpp/h         # Combat-focused variant (minimal implementation)
│   └── BiteCat.cpp/h           # Object holding/grabbing specialist
├── PlayerController/            # Controller implementations
│   ├── MultiPlayerController.cpp/h # Enhanced Input with individual direction controls
│   └── MainMenuController.cpp/h
├── Subsystem/                   # Game subsystems
│   └── MultiSessionSubsystem.cpp/h # Steam Online Subsystem integration for sessions
├── Actor/                       # Game objects and actors (organized in subdirectories)
│   ├── Triggers/               # Event-driven trigger implementations
│   │   ├── BaseTrigger.cpp/h                    # Abstract base trigger with delegate system
│   │   ├── ClearTrigger.cpp/h                   # Puzzle completion wrapper for zone system
│   │   ├── HoldingObjectOverlapTrigger.cpp/h    # Trigger for HoldingObject overlap detection
│   │   ├── ProjectilePassTrigger.cpp/h          # One-time projectile pass trigger
│   │   ├── PlayerOverlapTrigger.cpp/h           # Trigger for player overlap detection
│   │   └── PlayerVisitedTrigger.cpp/h           # One-time player visit trigger
│   ├── TriggeredActors/        # Actors controlled by triggers
│   │   ├── Turret.cpp/h                         # Basic turret with firing logic
│   │   ├── TriggeredTurret.cpp/h                # Trigger-controlled turret system
│   │   ├── MovingDoor.cpp/h                     # Animated upward-opening door
│   │   ├── TriggeredDoor.cpp/h                  # Two-trigger AND logic door system
│   │   ├── OneTriggeredDoor.cpp/h               # Single-trigger door system
│   │   └── ClearDoor.cpp/h                      # Zone-completion door system
│   ├── Volumes/                # Volume-based game mechanics
│   │   ├── ProjectileVolume.cpp/h               # Spawns and manages projectiles
│   │   ├── DebuffVolume.cpp/h                   # Applies debuffs to players in volume
│   │   ├── RespawnVolume.cpp/h                  # Kill volumes with visual spawn points
│   │   └── ZoneVolume.cpp/h                     # Puzzle zone tracking and completion
│   └── Objects/                # Interactive game objects
│       ├── WallJumpObject.cpp/h                 # Wall surfaces for wall jumping
│       ├── HoldingObject.cpp/h                  # Objects grabbable by BiteCat
│       ├── Projectile.cpp/h                     # Advanced projectile system
│       ├── MovingPlatform.cpp/h                 # Linear moving platforms
│       └── RotationPlatform.cpp/h               # Rotating platforms
├── Animation/                   # Animation notify states
│   └── ParryingNotifyState.cpp/h # Animation notify for parrying mechanics
└── Widdget/                     # UI widgets (note: typo in folder name)
    ├── MainMenuWidget.cpp/h    # Main menu interface
    ├── MultiSessionWidget.cpp/h # Session creation and browser
    ├── LobbyWidget.cpp/h       # Pre-game lobby with role selection
    ├── SettingsWidget.cpp/h    # Settings configuration UI
    ├── ESCWidget.cpp/h         # In-game pause menu with resume/main menu/exit options
    └── LoadingWidget.cpp/h     # Multi-state loading screen
```

### Content Structure
- **Input/**: Enhanced Input configurations optimized for cooperative platforming
  - IMC_Multi: Dual-character input mapping context
  - Responsive platformer input actions (Move, Jump, SpecialAbility, Look, Zoom)
- **GameMode/**: Cooperative game mode configurations
  - BP_MultiGameMode: Main cooperative platformer mode
- **PlayerCharacter/**: Specialized cat character Blueprints
  - BP_AttackCat: Combat specialist with parrying abilities
  - BP_BiteCat: Object manipulation specialist with charge-throw mechanics
  - **CRITICAL**: Both must have ParkourMontage set in Blueprint or parkour will fail with "ParkourMontage is NULL" error
- **PlayerController/**: Platformer-optimized controller setups
- **Widget/**: Cooperative game UI elements
- **Levels/**: Cooperative platformer levels and test environments

## Key Design Patterns

### Cooperative Platformer Architecture
- Blueprint-configurable cat abilities designed for cooperative puzzle-solving
- C++ base classes provide platforming framework, Blueprints define cooperative mechanics
- Virtual special action system enables unique cat abilities (combat, agility, support)
- Modular design allows easy addition of new cat types with different cooperative roles

### Dual-Character Input System
- Enhanced Input System optimized for precise platformer controls
- Independent character control with coordinated cooperative actions
- Camera-relative movement for intuitive platforming navigation
- Special action inputs designed for timing-critical cooperative maneuvers

### Specialized Ability System
- **AttackCat Parrying System**: Animation-driven combat mechanics
  - ParryingNotifyState triggers both projectile reflection and object pushing
  - Timer-based projectile detection (0.01s intervals) during parry window
  - SpecialActionBox overlap detection for both projectiles and HoldingObjects
  - Server RPC pattern: `PushNearbyObjects()` → `ServerPushNearbyObjects_Implementation()`
  - Disables homing on parried projectiles and reverses velocity
  - Physics-based object pushing with mass consideration

- **BiteCat Charge Throwing System**: Hold-to-charge mechanic
  - Client initiates: `StartCharging()` → `ServerStartCharging_Implementation()`
  - Server updates charge time in Tick() with `HasAuthority()` check
  - Replicated state (bIsCharging, CurrentChargeTime) syncs to all clients
  - Release: `ReleaseThrow()` → `ServerReleaseThrow_Implementation(float ChargeTime)`
  - Linear interpolation: `FMath::Lerp(MinThrowForce, MaxThrowForce, ChargeRatio)`
  - Auto-release triggers at MaxChargeTime for maximum force throw

- **Base Class Extensibility**: Virtual special action system supports new cat types
- **Blueprint Integration**: OnSpecialAction() event for complex cooperative mechanics
- **AnimNotify Pattern**: ParryingNotifyState demonstrates animation-driven gameplay events

### Event-Driven Trigger Architecture
The project uses a delegate-based trigger system to eliminate unnecessary Tick polling:

- **Delegate Pattern**: `DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTriggerStateChanged, bool, bNewState)`
- **Event Propagation**: Triggers broadcast state changes via `OnTriggerStateChanged.Broadcast(bNewState)`
- **Subscriber Pattern**: Triggered actors bind callbacks via `AddDynamic(this, &AClass::Callback)`
- **Network Synchronization**:
  - Server authority: Only server can call `SetTriggerActive()`
  - RepNotify: `OnRep_IsActive()` broadcasts delegate on clients for UI/effects
  - Server-side delegates trigger gameplay logic, client-side delegates trigger visual feedback

**ChildActorComponent Networking Pattern**:
- ChildActorComponent-spawned actors (like Turret in TriggeredTurret) lack owning connections
- NEVER use Server RPCs for child actors - causes "No owning connection" warnings
- Instead: Use direct function calls with `HasAuthority()` checks
- Example: `void SetActive(bool bNewActive) { if (!HasAuthority()) return; ... }`

**Trigger System Benefits**:
- Eliminates constant Tick polling overhead
- Immediate response to state changes (no frame delay)
- Clean separation between trigger logic and triggered behavior
- Easy to add new trigger types by extending `ABaseTrigger`
- Delegate unbinding after one-time events (e.g., door opens permanently)

### Cooperative Game Flow
- Level design supports dual-character progression (one cat opens path for other)
- Checkpoint system accounts for both cats' positions and states
- Game modes handle cooperative respawning and progress tracking
- UI system provides feedback for both players' actions and cooperative opportunities
- Trigger-based puzzles require coordination between cats (e.g., both standing on pressure plates)

## Networking & Multiplayer Architecture

### Replication Strategy
The project implements comprehensive multiplayer networking designed for both listen server and dedicated server configurations:

- **Property Replication**: Uses `DOREPLIFETIME` macro for key gameplay state synchronization
- **RPC Patterns**:
  - Server RPCs (`UFUNCTION(Server, Reliable)`) for authoritative game logic
  - Multicast RPCs (`UFUNCTION(NetMulticast, Reliable)`) for visual effects and client synchronization
  - Client RPCs for player-specific feedback
- **Component Replication**: Mesh and collision components marked as replicated for proper visual sync

### Key Networked Systems

#### BiteCat Charge Throwing System
```cpp
// Server-authoritative charge throwing
UFUNCTION(Server, Reliable, Category = "Charge")
void ServerStartCharging();

UFUNCTION(Server, Reliable, Category = "Charge")
void ServerReleaseThrow(float ChargeTime);

// Replicated state for all clients
UPROPERTY(Replicated) bool bIsCharging;
UPROPERTY(Replicated) float CurrentChargeTime;
UPROPERTY(Replicated) AHoldingObject* CurrentHeldObject;
```

**Implementation Pattern**:
- Tick() updates CurrentChargeTime only on server (`HasAuthority()`)
- Client sends current charge time to server on release
- Server calculates force and applies physics impulse
- Physics replication handles visual sync across clients

#### Character Movement Networking Patterns
- **Wall Jump System**: Server-authoritative physics with client visual feedback
  ```cpp
  // Client calls PerformWallJump() -> ServerPerformWallJump RPC -> MulticastPerformWallJump for effects
  UFUNCTION(Server, Reliable, Category = "Wall Jump")
  void ServerPerformWallJump(FVector JumpDirection);

  UFUNCTION(NetMulticast, Reliable, Category = "Wall Jump")
  void MulticastPerformWallJump(FVector JumpDirection);
  ```

- **Parkour System**: Root motion animation synchronized across all clients
  ```cpp
  // Server manages state, multicast handles animation synchronization
  UFUNCTION(Server, Reliable, Category = "Parkour")
  void ServerPerformParkour(AActor* ParkourTarget);

  UFUNCTION(NetMulticast, Reliable, Category = "Parkour")
  void MulticastPerformParkour(AActor* ParkourTarget);

  // Critical state replication for parkour
  UPROPERTY(Replicated) bool bIsPerformingParkour;
  UPROPERTY(Replicated) AActor* CurrentParkourActor;
  UPROPERTY(Replicated) bool bIsMontageePlaying;
  ```

#### Projectile Networking Pattern
- **Simplified RPC Flow**: InitializeProjectile now directly calls ServerInitializeProjectile without HasAuthority checks
- **Server Authority**: All projectile logic processed on server, replicated to clients
- **Multicast Synchronization**: Visual effects and state changes broadcast to all clients
- **Optimized for Listen Server**: Architecture supports both dedicated and listen server configurations

#### Projectile Volume System
- Server-controlled activation and projectile spawning with replicated state
- Player detection and targeting replicated across all clients
- Configurable spawn patterns with networked synchronization

#### AttackCat Parrying System
```cpp
// Server-authoritative object pushing
UFUNCTION(Server, Reliable, Category = "Attack")
void ServerPushNearbyObjects();
```

**Implementation Pattern**:
- ParryingNotifyState::NotifyBegin() triggers PushNearbyObjects()
- Client calls → Server RPC applies physics impulses
- SpecialActionBox finds all overlapping HoldingObjects
- Server applies directional impulse with configurable PushForce
- Physics simulation replicates results to all clients

### Online Session Management (Steam)

The project uses **UMultiSessionSubsystem** (GameInstanceSubsystem) for Steam Online Subsystem integration with **ALobbyGameState** for replicated player role management:

#### Session Management (UMultiSessionSubsystem)
- **Session Lifecycle**: Handles creation, destruction, finding, and joining sessions via OnlineSubsystemSteam
- **Session Discovery**: Finds sessions by exact session ID match
- **bInServer Flag**: Tracks whether client is in an active session (persists across level changes)
- **Client Disconnection Handling**:
  - `DestroyServer()` kicks all remote clients using `IsLocalController()` to distinguish host from clients
  - Uses `ClientReturnToMainMenu` RPC to notify clients of disconnection
  - Clients clear `bInServer = false` before returning to main menu to prevent lobby widget from appearing

**Key Functions**:
```cpp
void CreateServer();                          // Creates Steam session
void FindServers(FString SessionId);          // Searches by session ID
void DestroyServer();                         // Kicks clients and destroys session
void LeaveSession();                          // Client leaves session
FString GetCurrentSessionId() const;          // Gets active session ID
```

#### Player Role Management (ALobbyGameState)
- **Centralized Role Management**: All player role logic lives in GameState (not Subsystem)
- **Replicated State**: Uses `DOREPLIFETIME` for PlayerRoles array and SessionId
- **RepNotify Pattern**: `OnRep_PlayerRoles()` and `OnRep_SessionId()` trigger UI updates on clients
- **FPlayerRoleInfo Structure**:
  ```cpp
  struct FPlayerRoleInfo {
    FString PlayerNetId;  // UniqueNetId for player identification
    FString PlayerName;   // Display name
    int32 Role;           // 0 = AttackCat, 1 = BiteCat
  };
  ```

**Key Functions**:
```cpp
void AddPlayer(const FString& PlayerNetId, const FString& PlayerName);  // Called in PostLogin
void RemovePlayer(const FString& PlayerNetId);                          // Called in Logout
void SwapPlayerRoles();                                                  // Swaps roles between two players
int32 GetPlayerRole(const FString& PlayerNetId) const;                   // Query player role by UniqueNetId
```

#### Integration Flow
1. **Host Creates Session**: `CreateServer()` → `OnCreateSessionComplete()` → `ServerTravel("/Game/Levels/MainMenu?listen")`
2. **Client Joins Session**: `FindServers(SessionId)` → `OnJoinSessionComplete()` → `ClientTravel(ConnectInfo)`
3. **PostLogin Flow**: `MainMenuGameMode::PostLogin()` → `LobbyGameState::AddPlayer()` → Roles replicated to all clients
4. **Lobby Widget**: Binds to `LobbyGameState::OnPlayerRolesChanged` delegate for automatic UI updates
5. **Client Disconnection**:
   - Voluntary: `LobbyWidget::OnBackClicked()` → `LeaveSession()` → `OpenLevel("MainMenu")`
   - Kicked: Server calls `ClientReturnToMainMenu()` RPC → Client sets `bInServer = false` → `LeaveSession()` → `OpenLevel("MainMenu")`

### Multiplayer Design Patterns
- **Listen Server Optimized**: Primary design for host-player scenarios with dedicated server support
- **Steam Integration**: Uses OnlineSubsystemSteam for session management and matchmaking
- **Authority-First**: All gameplay logic validated on server before client updates
- **Visual Separation**: Effects and animations handled via multicast, logic via server RPCs
- **State Synchronization**: Critical gameplay state replicated automatically via property replication
- **Root Motion Replication**: Character animations with root motion properly synchronized across clients
  - Server controls state transitions (Flying/Walking movement modes)
  - Multicast ensures animation plays on all clients simultaneously
  - Timer-based completion callbacks only execute on server authority

## Critical Implementation Details

### Parkour System Debugging
When parkour is not working, check the following in order:
1. **ParkourMontage Assignment**: Verify ParkourMontage is set in Blueprint (BP_AttackCat/BP_BiteCat)
2. **Collision Settings**: Static meshes must have:
   - Collision Enabled: QueryOnly or QueryAndPhysics
   - Object Type: WorldStatic OR WorldDynamic
   - Generate Overlap Events: Enabled (if not using direct queries)
3. **Box Positioning**: In editor, enable visibility for ParkourLowerBox and ParkourUpperBox to verify placement
4. **Logs**: Search for `[PARKOUR]` in output log to trace detection failures

### ChildActorComponent Networking Anti-Pattern
**NEVER use Server RPCs with ChildActorComponent-spawned actors** - they lack owning connections and cause "No owning connection" warnings.
```cpp
// WRONG - Will fail with ChildActorComponent
UFUNCTION(Server, Reliable)
void ServerActivate();

// CORRECT - Use direct function calls with authority check
void SetActive(bool bActive) {
    if (!HasAuthority()) return;
    // Server logic here
}
```
Applies to: ATurret in TriggeredTurret, ABaseTrigger in composite actors, AMovingDoor in door systems.

## Technical Implementation Notes

### Platformer-Optimized Camera System
- Free camera rotation for optimal platforming angles and cooperative awareness
- Character auto-rotation to movement direction for precise platforming control
- Dynamic zoom system for close-up precision work and wide-area cooperative planning
- Camera positioning designed to show both cats and cooperative interaction opportunities

### Enhanced Input System Configuration
- **Individual Directional Controls**: Separate actions for each movement direction
  - MoveForwardAction, MoveBackwardAction, MoveLeftAction, MoveRightAction
  - Combined movement calculation for precise platformer control
  - Allows for complex movement combinations and fine-tuned platforming
- **Core Input Actions**:
  - LookAction (Vector2D): Camera control with configurable sensitivity
  - JumpAction (Digital): Standard and wall jump capabilities
  - SpecialAction (Digital): Character-specific abilities (holding, combat framework)
  - ZoomAction (Axis1D): Dynamic camera zoom with speed and limit controls
  - ESCAction (Digital): In-game menu toggle (note: ESC key may be reserved in editor)
- **Settings System**: Persistent configuration for mouse sensitivity, Y-axis inversion, and key remapping
- **Enhanced Input Integration**: Full UE5 Enhanced Input system with mapping context support

### Advanced Mechanics Implementation
- **Wall Jump System**: Sophisticated platforming with detection radius, cooldown timers, and air jump limits
  - Integration with AWallJumpObject actors for level design flexibility
  - Configurable wall jump forces and detection parameters
  - Cooldown system prevents infinite wall jumping exploits
  - **Networking**: Server-authoritative physics with RPC pattern for multiplayer support

- **Parkour System**: Box collision-based root motion climbing for waist-level obstacles
  - **Dual Box Detection**: Uses two UBoxComponent instances for precise parkour detection
    - ParkourLowerBox: Must overlap with objects (something to climb)
    - ParkourUpperBox: Must NOT overlap with objects (clear space above)
  - **Collision Configuration**:
    - Responds to both ECC_WorldStatic AND ECC_WorldDynamic channels
    - ECC_Pawn set to ECR_Ignore to prevent character self-overlap
    - QueryOnly collision enabled, no physics interaction
  - **Hybrid Detection Logic** (CatBase.cpp:363-535):
    - Primary: `GetOverlappingActors()` with `UpdateOverlaps()` for overlap-enabled objects
    - Fallback: Direct `OverlapMultiByChannel()` queries for both WorldStatic and WorldDynamic channels
    - Merges results from both methods to catch all valid parkour targets
    - **Critical**: Always runs direct queries even when GetOverlappingActors succeeds (some static meshes have overlap events disabled)
  - **StaticMesh Validation**: Only accepts actors with UStaticMeshComponent (filters out volumes/triggers)
  - **Movement Mode Management**: Switches to Flying mode during parkour for Z-axis root motion
  - **Root Motion Priority**: Pure animation-driven movement without Motion Warping dependency
  - **Jump Priority**: Parkour → Wall Jump → Normal Jump execution order
  - **Actor Attachment**: Character attaches to parkour target during animation (KeepWorld rules)
    - Maintains relative position when parkour target is moving (MovingPlatform, RotationPlatform)
    - Detaches on completion to restore independent movement
  - **Networking**: Server manages state transitions, multicast synchronizes animations with proper root motion replication
  - **Common Issue**: World-placed static meshes may need "Generate Overlap Events" enabled or use collision preset other than "Default"

- **Object Interaction Framework**: Complete holding/carrying system for puzzle mechanics
  - Weight-based interaction limits and physics integration
  - State management for held objects with proper collision handling
  - Configurable hold offsets for realistic character animations

- **Virtual Special Action System**: Blueprint-extensible framework for character abilities
- **Modular Architecture**: Easy addition of new cat types with unique cooperative abilities
- **Enhanced Input Integration**: Responsive controls optimized for precise platforming gameplay

- **Projectile Combat System**: Multi-type projectile system with physics integration
  - Three projectile types: Straight, Homing, and Slight Guided behaviors
  - Knockback system with unified horizontal + vertical force application
  - Volume-based spawning with configurable targeting and spawn patterns
  - Distance-based cleanup to prevent performance issues
  - Korean localized Blueprint tooltips for level designers

- **UI State Management**: Robust widget lifecycle and input mode handling
  - Settings applied immediately without save dependency for better UX
  - ESC menu created once and toggled via visibility for performance
  - Proper input mode transitions (GameOnly ↔ GameAndUI) for multiplayer compatibility
  - Display setting persistence across level changes through GameUserSettings