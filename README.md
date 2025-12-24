# Spiro - Multi-Game Breathing Platform

A modular, OTA-updatable breathing game platform for ESP32 with shared core infrastructure.

## Overview

Spiro is a breath-controlled gaming platform that transforms the monolithic Inhale application into a multi-game architecture where each game is independently buildable and deployable. Built on ESP32 with pressure sensor input (BMP280/BME280), games respond to breath patterns to create interactive experiences.

## Architecture

### Core Philosophy
- **Shared Infrastructure**: Common hardware abstraction (BreathData, Display, Sensor, Storage)
- **Independent Games**: Each game is a separate build target with its own entry point
- **Flexible Deployment**: Support for multiple sensors (BMP280/BME280) via build flags
- **Future OTA Ready**: Architecture designed for over-the-air game updates (Phase 2+)

### Directory Structure

```
spiro/
├── src/
│   ├── core/                      # Shared infrastructure
│   │   ├── hardware/              # Hardware abstraction layer
│   │   │   ├── BreathData.cpp/h   # Breath detection & normalization
│   │   │   ├── Display.cpp/h      # TFT display (LovyanGFX)
│   │   │   ├── Sensor.cpp/h       # Pressure sensor interface
│   │   │   └── Storage.cpp/h      # NVS storage
│   │   ├── scenes/                # Base scene class
│   │   │   └── SceneBase.h
│   │   └── ui/                    # Shared UI components
│   │       └── diagnostic/        # Diagnostic scene
│   │
│   ├── games/                     # Game-specific code
│   │   ├── balloon/               # Balloon game
│   │   │   ├── main.cpp           # Game entry point
│   │   │   └── scenes/
│   │   │       └── BalloonScene.cpp/h
│   │   │
│   │   └── live_breath/           # Live breath visualization
│   │       ├── main.cpp           # Game entry point
│   │       └── scenes/
│   │           └── LiveScene.cpp/h
│   │
│   ├── config.h                   # Global configuration
│   └── LGFX_Config.hpp            # Display configuration
│
├── simulator/                     # Simulator-specific implementations
│   ├── Sensor.cpp                 # Mouse-based breath simulation
│   └── Storage.cpp                # In-memory storage
│
└── platformio.ini                 # Build configuration
```

## Current Games

### 🎈 Balloon Game
Control a balloon's altitude with your breath. Exhale to rise, inhale to descend. Navigate through a scrolling sky environment with dynamic physics.

**Features:**
- Smooth breath-based movement with physics
- Spring-based string simulation
- Parallax scrolling background
- Real-time score tracking

### 🌊 Live Breath Visualization
Real-time waveform visualization of breath patterns. Watch your breathing create flowing, organic visuals.

**Features:**
- Sine wave breath visualization
- Smooth animations
- Color-coded breath states

## Building & Running

### Prerequisites
- PlatformIO CLI or IDE
- For simulator: SDL2 (`brew install sdl2` on macOS)
- For ESP32: BMP280 or BME280 sensor

### Build Environments

**Simulators** (for development without hardware):
```bash
pio run -e balloon_simulator        # Default
pio run -e live_breath_simulator
```

**ESP32 with BME280 sensor**:
```bash
pio run -e balloon_esp32_bme280 --target upload
pio run -e live_breath_esp32_bme280 --target upload
```

**ESP32 with BMP280 sensor**:
```bash
pio run -e balloon_esp32_bmp280 --target upload
pio run -e live_breath_esp32_bmp280 --target upload
```

### Running the Simulator

```bash
# Build and run balloon game
pio run -e balloon_simulator
./.pio/build/balloon_simulator/program

# Controls:
# - Mouse Y position: Simulates breath (up=exhale, down=inhale)
# - ESC/Q: Quit
```

## Hardware Setup

### Components
- ESP32 development board
- BMP280 or BME280 pressure sensor (I2C)
- TFT display (configured in LGFX_Config.hpp)

### Sensor Wiring
```
BMP280/BME280:
  VCC  -> 3.3V
  GND  -> GND
  SCL  -> GPIO 22
  SDA  -> GPIO 21
```

The sensor is auto-detected at I2C addresses 0x76 or 0x77.

## Development Roadmap

### ✅ Phase 1: Code Reorganization (COMPLETE)
- [x] Core/Games directory structure
- [x] Separate game entry points
- [x] Independent build environments
- [x] Simulator support
- [x] Multi-sensor support (BMP280/BME280)

### 🚧 Phase 2: OTA Infrastructure (Planned)
- [ ] ESP32 partition scheme for dual-boot OTA
- [ ] OTA update manager with WiFi
- [ ] Game selection menu/launcher
- [ ] Rollback safety on failed updates

### 📋 Phase 3: Metadata & Versioning (Planned)
- [ ] Firmware metadata embedding
- [ ] Version tracking in NVS
- [ ] Game catalog management

### 🌐 Phase 4: Server Infrastructure (Future)
- [ ] Game distribution server
- [ ] Build automation pipeline
- [ ] Usage analytics (optional)

## Adding a New Game

1. **Create game directory**:
   ```bash
   mkdir -p src/games/my_game/scenes
   ```

2. **Create main.cpp**:
   - Copy from `games/balloon/main.cpp`
   - Update game name and scene references

3. **Create your scene**:
   - Extend `SceneBase`
   - Implement `init()`, `update(dt)`, `draw(canvas)`

4. **Add build environments** in `platformio.ini`:
   ```ini
   [env:my_game_simulator]
   extends = ...
   build_flags = -DSIMULATOR -DGAME_MY_GAME
   build_src_filter = +<core/> +<games/my_game/> -<games/balloon/> -<games/live_breath/>
   ```

5. **Build and test**:
   ```bash
   pio run -e my_game_simulator
   ```

## Core APIs

### BreathData
```cpp
extern BreathData breathData;

breathData.detect(pressureDelta);      // Update breath state
float normalized = breathData.getNormalizedBreath();  // -1 to +1
BreathState state = breathData.getState();  // INHALE/EXHALE/IDLE/HOLD
```

### Display
```cpp
extern Display display;

Canvas& canvas = display.getCanvas();
canvas.fillRect(x, y, w, h, color);
display.blit();  // Push to screen
```

### Sensor
```cpp
extern Sensor pressureSensor;

pressureSensor.update();
float delta = pressureSensor.getDelta();  // Pascals from baseline
```

## License

MIT License (Non-Commercial Use)

Copyright (c) 2025 Keren Software LLC

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to use,
copy, modify, and merge copies of the Software for **non-commercial purposes only**,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

**Commercial use of this Software is strictly prohibited without prior written
permission from the copyright holders.**

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

## Credits

- Platform and interactions developed by Itay Keren
- LovyanGFX graphics library
- Adafruit sensor libraries
