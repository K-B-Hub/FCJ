# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

FCJ is an Unreal Engine 5.6 cooperative platformer game featuring two independent cat characters working together. The project focuses on a modular character system where each cat has specialized abilities and players must coordinate their actions to overcome platforming challenges. The architecture emphasizes Blueprint-configurable base classes that support different cat types with unique cooperative mechanics.

## Build System

This project uses Unreal Engine 5.6's standard build system:

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
- **Dependencies**: Core, CoreUObject, Engine, InputCore, EnhancedInput, AIModule, Slate, SlateCore
- **Note**: MotionWarping was removed from dependencies - parkour system now uses pure root motion with Flying movement mode

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
   - **AAttackCat**: Combat and obstacle-clearing specialist
     - Currently minimal implementation inheriting base abilities
     - Framework ready for combat-specific features and abilities
     - Special action system available for future combat mechanics
   
   - **ABiteCat**: Object manipulation and holding specialist  
     - Advanced object interaction through holding/grabbing mechanics
     - Can pick up and carry objects with configurable weight limits (`MaxHoldWeight`)
     - Detects and interacts with `AHoldingObject` instances in the world
     - Object positioning with configurable hold offsets for realistic carrying

3. **Cooperative Gameplay Controller (AMultiPlayerController)**
   - Individual directional input actions (MoveForward, MoveBackward, MoveLeft, MoveRight)
   - Combined movement system for precise platformer control
   - Configurable camera controls with mouse sensitivity and Y-axis inversion
   - Dynamic zoom system with configurable speed and distance limits
   - Key remapping and settings persistence system
   - Input Mapping Context with Enhanced Input system integration
   - ESC menu system with proper input mode management for multiplayer
   - Display settings persistence across level transitions

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

6. **Projectile Combat System**
   - **AProjectile**: Advanced projectile actors with multiple behavior types
     - Straight, homing, and guided projectile variants
     - Physics-based knockback with character state awareness
     - Configurable damage, speed, and lifetime parameters
     - Distance-based auto-destruction from spawn volume
   
   - **AProjectileVolume**: Area-based projectile spawning system
     - Multiple launch modes (targeted, random, mixed)
     - Configurable spawn rates and simultaneous projectile limits
     - Player detection and targeting system
     - Blueprint-exposed Korean tooltips for designer-friendly configuration

### Game Mode Structure
- **AMultiGameMode**: Cooperative platformer game mode
  - Manages dual-cat cooperative gameplay
  - Handles level progression and cooperative puzzle states
  - Checkpoint system for cooperative respawning
- **AMainMenuGameMode**: Main menu and level selection
- **AMainMenuController**: UI navigation for game mode selection

### UI System
- **UMainMenuWidget**: Main menu with cooperative game options
  - Local Co-op Play: Two players on one machine
  - Multi Play: Networked cooperative play
  - Settings: Configure controls for both cats
  - Level selection and cooperative challenge modes

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

## File Organization

```
Source/FCJ/
├── FCJ.cpp/h                    # Main module files
├── FCJ.Build.cs                 # Module build configuration
├── GameMode/                    # Game mode implementations
│   ├── MultiGameMode.cpp/h      # Main multiplayer game mode
│   └── MainMenuGameMode.cpp/h   # Menu mode
├── PlayerCharacter/             # Character implementations
│   ├── CatBase.cpp/h           # Base character class with wall jump & special actions
│   ├── AttackCat.cpp/h         # Combat-focused variant (minimal implementation)
│   └── BiteCat.cpp/h           # Object holding/grabbing specialist
├── PlayerController/            # Controller implementations
│   ├── MultiPlayerController.cpp/h # Enhanced Input with individual direction controls
│   └── MainMenuController.cpp/h
├── Actor/                       # Game objects and actors
│   ├── WallJumpObject.cpp/h    # Wall surfaces for wall jumping mechanics
│   ├── HoldingObject.cpp/h     # Objects that can be grabbed by BiteCat
│   ├── Projectile.cpp/h        # Advanced projectile system with homing and knockback
│   └── ProjectileVolume.cpp/h  # Spawns and manages projectiles in designated areas
└── Widdget/                     # UI widgets (note: typo in folder name)
    ├── MainMenuWidget.cpp/h    # Main menu interface
    ├── SettingsWidget.cpp/h    # Settings configuration UI
    └── ESCWidget.cpp/h         # In-game pause menu with resume/main menu/exit options
```

### Content Structure
- **Input/**: Enhanced Input configurations optimized for cooperative platforming
  - IMC_Multi: Dual-character input mapping context
  - Responsive platformer input actions (Move, Jump, SpecialAbility, Look, Zoom)
- **GameMode/**: Cooperative game mode configurations
  - BP_MultiGameMode: Main cooperative platformer mode
- **PlayerCharacter/**: Specialized cat character Blueprints
  - BP_AttackCat: Combat specialist with barrier-breaking abilities
  - BP_BiteCat: Agility specialist with climbing/support abilities
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
- **AttackCat Framework**: Ready for combat implementation with special action system
  - Inherits all base platforming capabilities including wall jumping
  - Special action box configured for future combat/attack mechanics
  - Framework established for barrier destruction and combat features

- **BiteCat Object Interaction**: Advanced holding and manipulation system  
  - Object detection and weight-based interaction system
  - Hold/release mechanics with proper physics integration
  - Configurable object positioning and carrying mechanics
  - Integration with AHoldingObject actors for puzzle elements

- **Base Class Extensibility**: Virtual special action system supports new cat types
- **Blueprint Integration**: OnSpecialAction() event for complex cooperative mechanics

### Cooperative Game Flow
- Level design supports dual-character progression (one cat opens path for other)
- Checkpoint system accounts for both cats' positions and states
- Game modes handle cooperative respawning and progress tracking
- UI system provides feedback for both players' actions and cooperative opportunities

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

#### BiteCat Object Holding System
```cpp
// Server-authoritative object interaction
UFUNCTION(Server, Reliable, Category = "Holding")
void ServerHoldObject(AHoldingObject* Object);

// Replicated state for all clients
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Holding")
AHoldingObject* CurrentHeldObject;
```

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

### Multiplayer Design Patterns
- **Listen Server Optimized**: Primary design for host-player scenarios with dedicated server support
- **Authority-First**: All gameplay logic validated on server before client updates
- **Visual Separation**: Effects and animations handled via multicast, logic via server RPCs
- **State Synchronization**: Critical gameplay state replicated automatically via property replication
- **Root Motion Replication**: Character animations with root motion properly synchronized across clients
  - Server controls state transitions (Flying/Walking movement modes)
  - Multicast ensures animation plays on all clients simultaneously
  - Timer-based completion callbacks only execute on server authority

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
    - ParkourLowerBox: Must overlap with StaticMesh objects (something to climb)
    - ParkourUpperBox: Must NOT overlap with StaticMesh objects (clear space above)
  - **Collision Configuration**:
    - ECollisionChannel::ECC_WorldStatic overlap response only
    - ECC_Pawn set to ECR_Ignore to prevent character self-overlap
    - QueryOnly collision enabled, no physics interaction
  - **Detection Logic**: Uses GetOverlappingActors() with UpdateOverlaps() for reliable detection
  - **Movement Mode Management**: Switches to Flying mode during parkour for Z-axis root motion
  - **Root Motion Priority**: Pure animation-driven movement without Motion Warping dependency
  - **Jump Priority**: Parkour → Wall Jump → Normal Jump execution order
  - **Networking**: Server manages state transitions, multicast synchronizes animations with proper root motion replication

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

# important-instruction-reminders
Do what has been asked; nothing more, nothing less.
NEVER create files unless they're absolutely necessary for achieving your goal.
ALWAYS prefer editing an existing file to creating a new one.
NEVER proactively create documentation files (*.md) or README files. Only create documentation files if explicitly requested by the User.