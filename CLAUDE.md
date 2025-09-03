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
- **Dependencies**: Core, CoreUObject, Engine, InputCore, EnhancedInput, AIModule

### Cooperative Platformer Character System
The project implements a modular dual-cat cooperative system designed for platformer gameplay:

1. **ACatBase**: Foundation class for all cat characters
   - Third-person camera system optimized for platforming (SpringArm + Camera)
   - Blueprint-configurable platformer settings (jump height, movement speed, air control)
   - Blueprint-configurable camera settings for optimal platforming view angles
   - Enhanced Input system with responsive platformer controls
   - Virtual special ability system (`OnSpecialAction()` Blueprint event) for cooperative mechanics
   - Movement-responsive rotation with camera-independent controls for precise platforming
   - Base framework for character-specific abilities and cooperative interactions

2. **Specialized Cat Characters**: Each cat type brings unique abilities to cooperative gameplay
   - **AAttackCat**: Combat and obstacle-clearing specialist
     - Melee attacks to break barriers or defeat enemies
     - Can clear paths for the other cat to follow
     - Heavy-hitting abilities for puzzle switches or destructible elements
   
   - **ABiteCat**: Agility and support specialist  
     - Enhanced jumping or climbing abilities
     - Can reach high places to activate switches for the other cat
     - Bite mechanics for grabbing/carrying objects or helping teammate

3. **Cooperative Gameplay Controller (AMultiPlayerController)**
   - Platformer-optimized camera controls with smooth tracking
   - Responsive input for precise platforming (tight jump timing, wall interactions)
   - Camera zoom for both close-up precision and wide area awareness
   - Special action inputs designed for cooperative timing and coordination
   - Input Mapping Context: IMC_Multi optimized for dual-character coordination

4. **Cooperative Game Mode (AMultiGameMode)**
   - Manages two independent cat characters simultaneously
   - Blueprint-configurable cat pairings and ability combinations
   - Supports both local co-op and potential networked cooperative play
   - Level progression and checkpoint systems for cooperative platforming

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

## File Organization

```
Source/FCJ/
├── FCJ.cpp/h                    # Main module files
├── GameMode/                    # Game mode implementations
│   ├── MultiGameMode.cpp/h      # Main multiplayer game mode
│   └── MainMenuGameMode.cpp/h   # Menu mode
├── PlayerCharacter/             # Character implementations
│   ├── CatBase.cpp/h           # Base character class
│   ├── AttackCat.cpp/h         # Combat-focused variant
│   └── BiteCat.cpp/h           # Bite-focused variant
├── PlayerController/            # Controller implementations
│   ├── MultiPlayerController.cpp/h # Enhanced Input controller
│   └── MainMenuController.cpp/h
└── Widdget/                     # UI widgets (note: typo in folder name)
    └── MainMenuWidget.cpp/h
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
- **AttackCat Abilities**: Barrier destruction, enemy combat, heavy switch activation
- **BiteCat Abilities**: Enhanced jumping, climbing, object manipulation, teammate assistance
- Base class framework allows easy extension with new cat types and abilities
- Blueprint events enable complex cooperative interactions and puzzle mechanics

### Cooperative Game Flow
- Level design supports dual-character progression (one cat opens path for other)
- Checkpoint system accounts for both cats' positions and states
- Game modes handle cooperative respawning and progress tracking
- UI system provides feedback for both players' actions and cooperative opportunities

## Technical Implementation Notes

### Platformer-Optimized Camera System
- Free camera rotation for optimal platforming angles and cooperative awareness
- Character auto-rotation to movement direction for precise platforming control
- Dynamic zoom system for close-up precision work and wide-area cooperative planning
- Camera positioning designed to show both cats and cooperative interaction opportunities

### Cooperative Input Configuration
- **Input Mapping Context**: IMC_Multi designed for dual-character coordination
- **Core Actions**: 
  - Move (Vector2D): Precise platformer movement with responsive controls
  - Look (Vector2D): Camera control for cooperative awareness  
  - Jump (Digital): Tight platformer jumping with air control
  - SpecialAction (Digital): Cat-specific abilities (Attack/Bite mechanics)
  - ZoomAction (Axis1D): Dynamic camera zoom for precision and overview
- Configurable sensitivity settings for both precision platforming and cooperative coordination
- Input system supports both local co-op (shared input device) and networked play

### Cooperative Mechanics Framework
- Virtual special action system enables unique cooperative abilities per cat type
- Blueprint event system for complex cat-to-cat interactions and puzzle solutions
- Character movement optimized for platformer physics with cooperative considerations
- Base architecture supports extension to additional cat types with new cooperative abilities

# important-instruction-reminders
Do what has been asked; nothing more, nothing less.
NEVER create files unless they're absolutely necessary for achieving your goal.
ALWAYS prefer editing an existing file to creating a new one.
NEVER proactively create documentation files (*.md) or README files. Only create documentation files if explicitly requested by the User.