# Crist-Project: 3D Ising Model Simulation

The `crist-project` is a sophisticated 3D visualization and simulation tool for the Ising model, a fundamental concept in statistical mechanics used to study magnetic phase transitions. This application combines an interactive graphical interface powered by Raylib and ImGui, a basic authentication system, and support for multiple lattice structures (cubic, hexagonal, FCC, BCC). Users can manipulate simulation parameters, visualize atomic spins in real-time, and explore the system's evolution using Monte Carlo methods.

## Table of Contents

1. [Features](#features)
2. [Dependencies](#dependencies)
3. [Project Structure](#project-structure)
4. [File Descriptions](#file-descriptions)
   - [include/auth.h and src/auth.cpp](#includeauthh-and-srcauthcpp)
   - [include/imgui_style.h](#includeimgui_styleh)
   - [include/simulation.h and src/simulation.cpp](#includesimulationh-and-srcsimulationcpp)
   - [include/simulation_ui.h and src/simulation_ui.cpp](#includesimulation_uih-and-srcsimulation_uicpp)
   - [src/main.cpp](#srcmaincpp)
5. [Building and Running](#building-and-running)
   - [Using CMake](#using-cmake)
   - [Manual Compilation](#manual-compilation)
6. [Usage](#usage)
   - [Authentication](#authentication)
   - [Simulation Interface](#simulation-interface)
7. [Technical Details](#technical-details)
   - [Ising Model Implementation](#ising-model-implementation)
   - [Lattice Structures](#lattice-structures)
   - [Rendering](#rendering)
   - [Monte-Carlo Simulation](#monte-carlo-simulation)
8. [Future Improvements](#future-improvements)
9. [License](#license)

## Features

- **Authentication System**: A login/registration interface using ImGui, storing hashed credentials in `users.txt`.
- **Custom UI Theme**: A dark Rosé Pine-themed ImGui interface with scalable fonts and modern styling.
- **3D Visualization**: Real-time rendering of atomic lattices with Raylib, using spheres for atoms and cylinders for bonds.
- **Lattice Options**: Supports cubic, hexagonal close-packed (HCP), face-centered cubic (FCC), and body-centered cubic (BCC) structures.
- **Interactive Camera**: Free 3D camera movement with mouse and keyboard controls.
- **Simulation Controls**: Adjust temperature, coupling constant (J), magnetic field (B), and Monte Carlo steps per frame.
- **Energy Visualization**: Toggle between spin-based (up/down) and energy-based coloring of atoms.
- **Performance Optimization**: Chunked cylinder rendering for efficient handling of large lattices.

## Dependencies

- **Raylib**: For 3D graphics rendering (install via package manager or compile from source).
- **ImGui**: Immediate mode GUI library (included in `imgui/` directory).
- **rlImGui**: Integration layer for Raylib and ImGui (included in `rlImGui/` directory).
- **CMake**: Build system (version 3.28.3 or later recommended).
- **C++17 Compiler**: e.g., g++ or clang++.
- **Font Files**: `JetBrainsMonoNLNerdFont-Regular.ttf` (in `assets/`) for ImGui rendering.

Ensure Raylib is installed system-wide or linked appropriately in your build environment.

## Project Structure

```plaintext
crist-project/
├── CMakeLists.txt          # CMake configuration file
├── README.md               # Project documentation
├── assets/                 # Font assets
│   ├── JetBrainsMonoNLNerdFont-Bold.ttf
│   ├── JetBrainsMonoNLNerdFont-Italic.ttf
│   └── JetBrainsMonoNLNerdFont-Regular.ttf
├── build/                  # Build output directory
│   ├── CMakeCache.txt
│   ├── CMakeFiles/
│   ├── Makefile
│   ├── cmake_install.cmake
│   ├── compile_commands.json
│   ├── crist-project       # Compiled executable
│   ├── imgui.ini           # ImGui configuration (generated)
│   └── users.txt           # User credentials (generated)
├── imgui/                  # ImGui library source
│   ├── LICENSE.txt
│   ├── backends/
│   ├── docs/
│   ├── examples/
│   ├── imconfig.h
│   ├── imgui.cpp
│   ├── imgui.h
│   └── ... (other ImGui files)
├── include/                # Header files
│   ├── auth.h
│   ├── imgui_style.h
│   ├── simulation.h
│   └── simulation_ui.h
├── rlImGui/                # rlImGui integration source
│   ├── LICENSE
│   ├── README.md
│   ├── examples/
│   ├── rlImGui.cpp
│   ├── rlImGui.h
│   └── ... (other rlImGui files)
└── src/                    # Source files
    ├── auth.cpp
    ├── main.cpp
    ├── simulation.cpp
    ├── simulation_ui.cpp
    └── users.txt           # Optional initial user file
```

## File Descriptions

### include/auth.h and src/auth.cpp

- **Purpose**: Provides a simple authentication system with login and registration.
- **Key Functions**:
  - `simple_hash(const string &input)`: A basic hash function using a seed (131) and bit rotation for password hashing.
  - `runAuthentication()`: Renders an ImGui-based login/registration window, returning `true` on successful login.
- **Details**:
  - Stores credentials in `users.txt` as `username:hashed_password`.
  - Login validates against existing entries; registration appends new users.
  - Uses Raylib for window management and rlImGui for ImGui integration.

### include/imgui_style.h

- **Purpose**: Customizes ImGui with a dark Rosé Pine theme and scalable font.
- **Key Function**:
  - `SetCustomImGuiStyle(float scaling)`: Applies a custom color palette, loads `JetBrainsMonoNLNerdFont-Regular.ttf` from `assets/`, and scales UI elements.
- **Details**:
  - Uses oversampling (2x) for crisp font rendering.
  - Defines colors like `base`, `surface`, `love`, etc., for a cohesive look.
  - Adjusts rounding, padding, and borders for a modern aesthetic.

### include/simulation.h and src/simulation.cpp

- **Purpose**: Implements the core Ising model simulation logic and lattice generation.
- **Key Components**:
  - **Enums**:
    - `Spin`: `UP = 1`, `DOWN = -1`.
    - `StructureType`: `CUBIC`, `HEXAGONAL`, `FCC`, `BCC`.
    - `SimulationState`: `PAUSED`, `RUNNING`, `STEP`.
  - **Structs**:
    - `Atome`: Holds position, spin, neighbors, energy, and radius.
  - **Global Variables**: `simState`, `temperature`, `J`, `B`, `stepsPerFrame`, `showEnergy`, `upColor`, `downColor`.
  - **Structure Generation**:
    - `make_cubic_struc`: Simple cubic lattice (6 neighbors).
    - `make_hexagonal_struc`: HCP lattice (~12 neighbors).
    - `make_fcc_struc`: FCC lattice (12 neighbors).
    - `make_bcc_struc`: BCC lattice (8 neighbors).
  - **Visualization**:
    - `CreateChunkedCylinderLines`: Generates chunked meshes for bonds.
    - `CreateBakedCylinderLines`: Single mesh for all bonds (less efficient).
    - `DrawInstanced`: Renders meshes with transforms (not fully instanced).
  - **Simulation**:
    - `CalculateTotalEnergy`: Sums atomic energies, halved to avoid double-counting.
    - `UpdateEnergies`: Updates energies based on spin interactions and field.
    - `MonteCarloStep`: Flips a random spin using the Metropolis algorithm.
- **Details**:
  - Neighbor detection uses distance thresholds, which may need refinement.

### include/simulation_ui.h and src/simulation_ui.cpp

- **Purpose**: Manages the simulation UI and 3D rendering.
- **Key Function**:
  - `runSimulation()`: Sets up the window, camera, and ImGui controls, running the main loop.
- **Details**:
  - **Camera**: WASD/space/control movement, mouse rotation.
  - **Rendering**: Spheres for atoms, cylinders for bonds, with spin/energy coloring.
  - **UI**: Left-aligned ImGui drawer with controls for lattice, visuals, simulation, and stats.

### src/main.cpp

- **Purpose**: Program entry point, linking authentication and simulation.
- **Details**:
  - Initializes a full-screen Raylib window.
  - Runs `runAuthentication()`, then `runSimulation()` if successful.
  - Handles resource cleanup between phases.

## Building and Running

### Using CMake

1. **Clone the Repository**:

   ```bash
   git clone <repository-url>
   cd crist-project
   ```

2. **Configure CMake**:

   ```bash
   mkdir build
   cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release
   ```

   - Ensure Raylib is installed (e.g., via `sudo apt install libraylib-dev` on Ubuntu).
   - Adjust `CMakeLists.txt` if Raylib or font paths differ.

3. **Build**:

   ```bash
   make
   ```

4. **Run**:

   ```bash
   ./crist-project
   ```

### Manual Compilation

If CMake isn’t preferred:

```bash
g++ -o crist-project src/*.cpp rlImGui/rlImGui.cpp imgui/*.cpp -Iinclude -Iimgui -IrlImGui -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
```

Adjust flags for your platform (Linux assumed).

## Usage

### Authentication

- **Login**: Enter username/password matching `users.txt`.
- **Register**: Click "Register", enter details, and save to `users.txt`.
- **Errors**: Shown in red for invalid input or file issues.

### Simulation Interface

- **Camera**:
  - WASD/QZ: Move horizontally.
  - Space/Control: Move vertically.
  - Left-click + drag: Rotate.
- **UI Controls**:
  - Adjust lattice size, distance, and type.
  - Modify sphere/bond radius and grid visibility.
  - Start/pause/step simulation, tweak parameters.
  - Toggle energy view, pick spin colors.
  - Monitor stats (energy, spins, magnetization, FPS).

## Technical Details

### Ising Model Implementation

Energy per atom:
$$ E_i = -J \cdot s_i \cdot \sum_{j \in \text{neighbors}} s_j - B \cdot s_i $$

- $ s_i = \pm 1 $, $ J $: coupling, $ B $: magnetic field.

### Lattice Structures

- **Cubic**: 6 neighbors, simple grid.
- **HCP**: ~12 neighbors, ABAB stacking.
- **FCC**: 12 neighbors, face-centered.
- **BCC**: 8 neighbors, body-centered.

### Rendering

- **Atoms**: Spheres via `GenMeshSphere`.
- **Bonds**: Cylinders via `CreateChunkedCylinderLines`.
- **Optimization**: Chunking reduces draw calls.

### Monte Carlo Simulation

- Random spin flip, accepted if \( \Delta E < 0 \) or \( e^{-\Delta E / T} > \text{random}(0,1) \).

## Future Improvements

- **Security**: Use SHA-256 with salts for passwords.
- **Instancing**: Optimize `DrawInstanced`.
- **Font Path**: Make configurable.
- **Accuracy**: Improve neighbor detection.
- **Features**: Add save/load functionality.
- **Portability**: Support Windows/macOS.

## License

Unlicensed by default. Consider adding an MIT or GPL license.
