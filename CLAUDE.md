# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

FCJ is an Unreal Engine 5.6 project implementing a dual-character control system with "FrontCat" and "BackCat" characters managed by a single player character.

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

### Character System Architecture
The project implements a unique dual-character control pattern with physical connections:

1. **ALocalPlayerCharacter**: Main controller character managing connected cats
   - Spawns and manages FrontCat and BackCat instances at runtime
   - Implements physics-based spring connection system between cats
   - Features dynamic connection meshes that stretch/scale based on distance
   - Uses SpringArm and Camera components for third-person view
   - Physics constraints with configurable spring force and damping
   - Connection distance limits (min/max) to prevent over-stretching

2. **AFrontCat/ABackCat**: Individual character classes extending ACharacter
   - Independent movement and physics simulation
   - Connected through physics constraints to central body
   - Separate input handling for dual-character control

3. **Connection System**: Physical link between characters
   - CentralBodyMesh: Shared body component between cats  
   - FrontConnectionMesh/BackConnectionMesh: Dynamic connection visuals
   - Physics constraints with spring forces for realistic connection
   - Real-time mesh scaling based on distance between cats

4. **Input System**: Enhanced Input system with dual controls
   - Separate input actions for front/back cat movement
   - Input Mapping Context: IMC_Local
   - Input routing from LocalPlayerCharacter to individual cats

### Game Mode Structure
- **ALocalGameMode**: Main game mode for gameplay
- **AMainMenuGameMode**: Separate game mode for main menu
- **ALocalPlayerController**: Custom player controller
- **AMainMenuController**: Menu-specific controller

### UI System
- **UMainMenuWidget**: Main menu interface (UMG-based)
- Note: There's a typo in the directory structure ("Widdget" instead of "Widget")

### Level Organization
- **MainMenu.umap**: Main menu level
- **Test.umap**: Gameplay testing level

## File Organization

```
Source/FCJ/
├── FCJ.cpp/h                    # Main module files
├── GameMode/                    # Game mode implementations
│   ├── LocalGameMode.cpp/h      # Main gameplay mode
│   └── MainMenuGameMode.cpp/h   # Menu mode
├── PlayerCharacter/             # Character implementations
│   ├── LocalPlayerCharacter.cpp/h # Main player character
│   ├── FrontCat.cpp/h          # Front character
│   └── BackCat.cpp/h           # Back character
├── PlayerController/            # Controller implementations
│   ├── LocalPlayerController.cpp/h
│   └── MainMenuController.cpp/h
└── Widdget/                     # UI widgets (note: typo in folder name)
    └── MainMenuWidget.cpp/h
```

## Development Notes

### Content Structure
- **Assets/Mesh/**: 3D model assets
- **Input/**: Enhanced Input configurations
- **PlayerCharacter/**: Blueprint versions of characters
- **Widget/**: UI Blueprint widgets

### Plugin Dependencies
- ModelingToolsEditorMode (Editor-only)
- RiderLink plugin for JetBrains Rider integration

### Key Design Patterns
- **Physics-Based Connections**: Spring constraint system for realistic dual-character physics
- **Dynamic Mesh Scaling**: Connection visuals that scale and position based on character distance
- **Composition over Inheritance**: Character management through references rather than inheritance
- **Separation of Concerns**: Distinct game modes for different contexts (gameplay vs menu)
- **Enhanced Input Architecture**: Modern input system with action-based routing
- **Runtime Character Spawning**: Characters created dynamically rather than placed in levels

### Technical Implementation Details
- **Connection Physics**: Configurable spring force (default 15.0f) and damping (default 2.0f)
- **Distance Constraints**: Min distance 80.0f, max distance 200.0f units
- **Mesh Stretching**: Base connection length 50.0f with max stretch factor 3.0x
- **Update Frequency**: Connection system updates every frame via Tick()
# important-instruction-reminders
Do what has been asked; nothing more, nothing less.
NEVER create files unless they're absolutely necessary for achieving your goal.
ALWAYS prefer editing an existing file to creating a new one.
NEVER proactively create documentation files (*.md) or README files. Only create documentation files if explicitly requested by the User.