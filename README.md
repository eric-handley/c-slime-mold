# C Slime Mold Simulation

A real-time slime mold simulation built in C using OpenGL compute shaders.

## Demo

This demo (quality and FPS reduced) was captured at a total resolution of 5760x3240, with 1,500,000 agents!

![Slime Mold Simulation Demo](img/demo.webp)

## Configuration

The simulation can be configured by editing these parameters in `src/main.c`:

- `NUM_AGENTS` - Number of simulated agents (default: 500,000)
- `RES_SCALE_FACTOR` - Upscaling factor (default: x2)
- `fade_factor` - Default amount to fade pixels by each frame
- `autoswap_time_interval` - Seconds between auto-randomization in auto swap mode
- `autotweak_time_interval` - Seconds between parameter tweaks in auto tweak mode

## Controls

### Basic Controls

|   Key   | Action |
|---------|--------|
| `I`     | Randomize movement parameters |
| `O`     | Randomize colors/number of species |
| `P`     | Randomize both movement and colors/species |
| `M`     | Toggle autoswap mode (auto-randomize on interval - disables autotweak) |
| `N`     | Toggle autotweak mode (smoothly drift parameters - disables autoswap) |
| `K`     | Reset position of all agents (randomize on screen) |
| `L`     | Reset agents to circle formation (facing outward) |
| `SPACE` | Clear screen completely |
| `ESC`   | Exit |

> **Tip:** Try pressing `SPACE` and `L` at the same time

### Mouse Controls

| Input | Action |
|-------|--------|
| `Left Mouse Button` (hold) | All agents avoid the cursor + erases trails |
| `Right Mouse Button` (hold) | Half the species are attracted to cursor |

Pressing both `RMB` and `LMB` will attract half of species and repel the other half

### Parameter Controls

| Decrease  | Increase | Parameter | Description |
|-----------|----------|-----------|-------------|
| `Q` | `E` | Turn randomness | Random variation in agent turning |
| `S` | `W` | Agent speed | How fast agents move |
| `A` | `D` | Turn speed | Speed at which agents turn toward their own species |
| `R` | `T` | Sampling distance | Distance at which agents look for their own species |
| `F` | `G` | Sampling angle | Angle at which agents look for their own species |
| `C` | `V` | Number of species | Max 16 - number of different colors |
| `X` | `Z` | Trail decay | How quickly trails fade |

## Saving/Loading Presets

### Movement Settings
- **`0-9`** - Load movement settings from preset 0-9
- **`Shift + 0-9`** - Save current movement settings to preset 0-9

### Species/Color Settings
- **`Alt + 0-9`** - Load species/color settings from preset 0-9
- **`Shift + Alt + 0-9`** - Save current species/color settings to preset 0-9

Sample presets are included in the `data/presets/` directory

## Building

### Prerequisites
- GCC
- GLFW3
- GLEW
- OpenGL 4.3+ (compute shader support required)

## Project Structure

```
├── src/                 - Source code (modular design)
│   ├── main.c               - Main application and setup logic
│   ├── graphics.c           - OpenGL, GLFW, windowing, shaders, timing
│   ├── utils.c              - Random number generation utilities
│   ├── settings.c           - Settings, save/load, input handling
│   └── mman.c               - Memory mapping utilities
├── include/             - Header files
│   ├── main.h             
│   ├── graphics.h         
│   ├── utils.h            
│   ├── settings.h         
│   └── mman.h             
├── assets/           
│   └── shaders/         - OpenGL compute and rendering shaders
│       ├── compute.comp     - Agent simulation compute shader
│       ├── quad.frag        - Fragment shader for blur/bloom effects rendering
│       └── quad.vert        - Vertex shader for quad rendering
├── data/            
│   └── presets/         - User settings and presets
│       ├── movement/        - Movement parameter presets (0-9)
│       └── species/         - Species/color presets (0-9)
├── build/
├── makefile               
└── README.md              
```
