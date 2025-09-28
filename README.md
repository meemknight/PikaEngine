# PikaEngine

A lightweight C++ game engine designed to provide maximum flexibility without imposing a rigid workflow. PikaEngine empowers developers to create games in the way that works best for them, offering a modular architecture with hot-reloading capabilities and comprehensive tooling.

## Table of Contents

- [Overview](#overview)
- [Architecture](#architecture)
- [Features](#features)
- [Getting Started](#getting-started)
- [Project Structure](#project-structure)
- [Core Systems](#core-systems)
- [Container System](#container-system)
- [Plugin System](#plugin-system)
- [Third-Party Libraries](#third-party-libraries)
- [Development Workflow](#development-workflow)
- [Building](#building)
- [Examples](#examples)
- [Contributing](#contributing)
- [License](#license)

## Overview

PikaEngine is a modern C++ game engine built with flexibility and developer productivity in mind. It features a unique container-based architecture that allows for hot-reloading of game logic, making development iterations fast and efficient. The engine supports both 2D and 3D game development with comprehensive tooling and editor integration.

### Key Philosophy

- **Non-intrusive**: The engine doesn't force a specific workflow or paradigm
- **Modular**: Components can be used independently or together
- **Hot-reloadable**: Game logic can be reloaded without restarting the engine
- **Cross-platform**: Supports Windows and Linux
- **Performance-oriented**: Built with C++17 and optimized for real-time applications

## Architecture

PikaEngine follows a modular architecture with clear separation of concerns:

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   pikaCore      │    │  pikaGameplay   │    │    Plugins      │
│   (Runtime)     │◄──►│   (DLL)         │◄──►│   (Optional)    │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                       │                       │
         ▼                       ▼                       ▼
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│  Core Systems   │    │   Containers    │    │  Asset Viewers  │
│  - Memory Mgmt  │    │  - Game Logic   │    │  - Text Editor  │
│  - Window Sys   │    │  - Physics      │    │  - 3D Editor    │
│  - Input        │    │  - Rendering    │    │  - Image Viewer │
│  - Logging      │    │  - Audio        │    │  - Sushi Viewer │
└─────────────────┘    └─────────────────┘    └─────────────────┘
```

## Features

### Core Features
- **Hot-reloading**: Modify game logic without restarting
- **Memory Management**: Custom allocators with leak detection
- **Cross-platform**: Windows and Linux support
- **Modern C++**: C++17 standard with RAII principles
- **Modular Design**: Use only what you need

### Rendering
- **2D Rendering**: Hardware-accelerated 2D graphics with GL2D
- **3D Rendering**: PBR-based 3D rendering with GL3D
- **OpenGL**: Modern OpenGL 3.3+ support
- **Shaders**: GLSL shader support with hot-reloading
- **Textures**: Multiple format support (PNG, JPG, TGA, HDR, etc.)

### Physics
- **Box2D Integration**: 2D physics simulation
- **Custom Physics**: 3D physics system (ph2d)
- **Collision Detection**: Broad and narrow phase collision detection
- **Joints and Constraints**: Full Box2D joint system support

### Editor Integration
- **ImGui-based Editor**: Full-featured development environment
- **Asset Management**: Built-in asset browser and management
- **Scene Editor**: Visual scene editing capabilities
- **Debug Tools**: Comprehensive debugging and profiling tools
- **Console**: Integrated console for logging and commands

### Input System
- **Multi-input Support**: Keyboard, mouse, and gamepad
- **Input Recording**: Record and playback input sequences
- **Shortcut System**: Configurable keyboard shortcuts
- **Focus Management**: Proper input focus handling

## Getting Started

### Prerequisites

- **C++17 Compatible Compiler**: MSVC 2019+, GCC 8+, or Clang 8+
- **CMake 3.16+**: For building the project
- **OpenGL 3.3+**: For rendering support
- **Git**: For cloning the repository

### Quick Start

1. **Clone the repository**:
   ```bash
   git clone https://github.com/meemknight/PikaEngine.git
   cd PikaEngine
   ```

2. **Build the project**:
   ```bash
   mkdir build
   cd build
   cmake ..
   cmake --build . --config Release
   ```

3. **Run the engine**:
   ```bash
   ./pikaCore  # Linux
   pikaCore.exe  # Windows
   ```

## Project Structure

```
PikaEngine/
├── Pika/                          # Main engine directory
│   ├── core/                      # Core engine systems
│   │   ├── coreConfig/           # Configuration and build settings
│   │   ├── pikaEditor/           # Editor-specific code
│   │   ├── pikaRuntime/          # Runtime engine code
│   │   ├── pikaSTD/              # Standard library extensions
│   │   └── sharedRuntime/        # Shared runtime components
│   ├── gameplay/                  # Game logic and containers
│   │   ├── containers/           # Game containers (scenes, levels)
│   │   └── dll/                  # DLL interface for hot-reloading
│   ├── pluggins/                 # Engine plugins
│   ├── resources/                # Game assets and resources
│   └── thirdparty/               # Third-party libraries
├── art/                          # Engine artwork and assets
└── README.md                     # This file
```

## Core Systems

### Memory Management (`pikaSTD/`)

PikaEngine features a sophisticated memory management system:

- **Custom Allocators**: Free-list allocators for efficient memory usage
- **Memory Arenas**: Stack-based memory allocation for temporary data
- **Leak Detection**: Built-in memory leak detection and reporting
- **Global Allocator**: Centralized memory management with custom allocator support

```cpp
// Example: Using custom allocators
pika::memory::FreeListAllocator allocator(pika::MB(10));
pika::memory::setGlobalAllocator(&allocator);
```

### Window System (`sharedRuntime/windowSystemm/`)

Cross-platform window management with OpenGL context:

- **GLFW Integration**: Modern window creation and management
- **OpenGL Context**: Automatic context creation and management
- **Input Handling**: Unified input system across platforms
- **Multi-window Support**: Multiple window support for editor

### Logging System (`pikaSTD/logs/`)

Comprehensive logging and debugging:

- **Multiple Outputs**: Console, file, and notification logging
- **Log Levels**: Debug, info, warning, and error levels
- **Thread-safe**: Safe for multi-threaded applications
- **Performance**: Minimal performance impact

```cpp
// Example: Using the logging system
pika::LogManager logs;
logs.init("game.log");
logs.log("Game started", pika::logInfo);
```

### Input System (`sharedRuntime/`)

Unified input handling across all platforms:

- **Keyboard Input**: Full keyboard support with key states
- **Mouse Input**: Mouse position, buttons, and wheel
- **Gamepad Support**: XInput-compatible gamepad support
- **Input Recording**: Record and playback input sequences

```cpp
// Example: Handling input
if (input.buttons[pika::Button::Space].pressed()) {
    // Jump action
}
```

## Container System

The container system is the heart of PikaEngine's modular architecture. Containers are self-contained game logic modules that can be hot-reloaded during development.

### Container Interface

All containers must implement the `Container` interface:

```cpp
struct MyContainer : public Container {
    // Static information about the container
    static ContainerStaticInfo containerInfo() {
        ContainerStaticInfo info = {};
        info.defaultHeapMemorySize = pika::MB(10);
        info.requestImguiFbo = true;
        return info;
    }
    
    // Initialize the container
    bool create(RequestedContainerInfo &requestedInfo, 
                pika::StaticString<256> commandLineArgument) override;
    
    // Update the container each frame
    bool update(pika::Input input, 
                pika::WindowState windowState,
                RequestedContainerInfo &requestedInfo) override;
    
    // Cleanup when container is destroyed
    void destruct(RequestedContainerInfo &requestedInfo) override;
};
```

### Container Features

- **Hot-reloading**: Containers can be reloaded without restarting the engine
- **Memory Management**: Each container has its own memory space
- **ImGui Integration**: Containers can request ImGui IDs for UI
- **File I/O**: Built-in file reading and writing capabilities
- **Console Access**: Containers can write to the engine console

### Example Containers

The engine includes several example containers:

- **Gameplay**: Basic 2D gameplay example with rendering and input
- **HollowKnight**: 2D platformer with physics and level editing
- **Milk**: 3D first-person example with physics simulation
- **SilkSong**: Advanced 2D game with complex systems

## Plugin System

PikaEngine supports a plugin system for extending functionality:

### Built-in Plugins

- **Image Viewer**: View and edit images with UV mapping
- **Text Editor**: Syntax-highlighted text editor
- **3D Editor**: 3D model viewer and editor
- **Sushi Viewer**: UI layout viewer and editor

### Creating Plugins

Plugins follow the same container interface but are designed for tooling:

```cpp
struct MyPlugin : public Container {
    static ContainerStaticInfo containerInfo() {
        ContainerStaticInfo info = {};
        info.extensionsSuported = {".myformat"};
        info.requestImguiIds = 1;
        return info;
    }
    
    // Plugin implementation...
};
```

## Third-Party Libraries

PikaEngine integrates several high-quality third-party libraries:

### Graphics and Rendering
- **GLFW 3.3.2**: Window management and OpenGL context
- **GLAD**: OpenGL function loading
- **GLM**: Mathematics library for graphics
- **GL2D**: 2D rendering library
- **GL3D**: 3D rendering library with PBR support

### Physics
- **Box2D 2.4.1**: 2D physics simulation
- **ph2d**: Custom 3D physics system

### UI and Tools
- **ImGui (Docking)**: Immediate mode GUI
- **stb_image**: Image loading
- **stb_truetype**: Font rendering

### Utilities
- **SafeSave**: Safe file saving with atomic operations
- **Sushi**: UI layout system
- **ProfilerLib**: Performance profiling

## Development Workflow

### Hot-Reloading Development

1. **Start the engine** with the development build
2. **Modify game logic** in container files
3. **Rebuild the DLL** (automatic in some IDEs)
4. **Reload containers** using the editor interface
5. **Test changes** without restarting the engine

### Debugging

- **Memory Leak Detection**: Built-in leak detection
- **Performance Profiling**: Integrated profiling tools
- **Console Logging**: Comprehensive logging system
- **ImGui Debug Windows**: Real-time debugging interfaces

### Asset Pipeline

- **Hot-reloading Assets**: Many assets can be reloaded without restart
- **Asset Browser**: Built-in asset management
- **Format Support**: Multiple image, model, and audio formats
- **Compression**: Automatic asset optimization

## Building

### CMake Configuration

The engine uses CMake for building with the following targets:

- **pikaCore**: Main engine executable (development)
- **pikaGameplay**: Game logic DLL
- **pikaProduction**: Production build (optional)

### Build Options

```cmake
# Development build (default)
cmake -DPIKA_DEVELOPMENT=ON ..

# Production build
cmake -DPIKA_PRODUCTION=ON ..

# Custom resource path
cmake -DPIKA_RESOURCES_PATH="/path/to/resources" ..
```

### Platform-Specific Notes

#### Windows
- Uses MSVC runtime linking
- Supports both Debug and Release configurations
- Automatic DLL loading and unloading

#### Linux
- Requires OpenGL development libraries
- Uses system package manager for dependencies
- Supports both GCC and Clang

## Examples

### Basic 2D Game

```cpp
struct MyGame : public Container {
    gl2d::Renderer2D renderer;
    
    static ContainerStaticInfo containerInfo() {
        ContainerStaticInfo info = {};
        info.defaultHeapMemorySize = pika::MB(10);
        info.requestImguiFbo = true;
        return info;
    }
    
    bool create(RequestedContainerInfo &requestedInfo, 
                pika::StaticString<256> commandLineArgument) override {
        renderer.create(requestedInfo.requestedFBO.fbo);
        return true;
    }
    
    bool update(pika::Input input, pika::WindowState windowState,
                RequestedContainerInfo &requestedInfo) override {
        glClear(GL_COLOR_BUFFER_BIT);
        
        renderer.updateWindowMetrics(windowState.windowW, windowState.windowH);
        renderer.renderRectangle({100, 100, 50, 50}, Colors_Red);
        renderer.flush();
        
        return true;
    }
};
```

### 3D Scene with Physics

```cpp
struct My3DGame : public Container {
    gl3d::Renderer3D renderer;
    Simulator physics;
    
    bool create(RequestedContainerInfo &requestedInfo, 
                pika::StaticString<256> commandLineArgument) override {
        renderer.init(1, 1, PIKA_RESOURCES_PATH "BRDFintegrationMap.png", 
                      requestedInfo.requestedFBO.fbo);
        
        // Load 3D model
        auto model = renderer.loadModel("model.glb");
        auto entity = renderer.createEntity(model, {{0, 0, 0}});
        
        return true;
    }
    
    bool update(pika::Input input, pika::WindowState windowState,
                RequestedContainerInfo &requestedInfo) override {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Update physics
        physics.update(input.deltaTime);
        
        // Render scene
        renderer.render(input.deltaTime);
        
        return true;
    }
};
```

## Contributing

We welcome contributions to PikaEngine! Here's how you can help:

### Development Setup

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

### Code Style

- Follow C++17 best practices
- Use RAII principles
- Prefer composition over inheritance
- Document public APIs
- Write tests for new features

### Areas for Contribution

- **New Containers**: Create example games and demos
- **Plugin Development**: Build useful plugins
- **Documentation**: Improve guides and examples
- **Performance**: Optimize rendering and physics
- **Platform Support**: Add support for new platforms

## License

PikaEngine is released under the MIT License. See the LICENSE file for details.

## Acknowledgments

- **Box2D**: Physics simulation library
- **GLFW**: Window management
- **ImGui**: Immediate mode GUI
- **GLM**: Mathematics library
- **stb libraries**: Image and font loading
- **All contributors**: Thank you for making PikaEngine better!

---

For more information, visit our [GitHub repository](https://github.com/meemknight/PikaEngine) or join our community discussions.
