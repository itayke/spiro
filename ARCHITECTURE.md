# Multi-Game OTA Firmware Architecture

## Status: Phase 1 Complete ✅

This document outlines the architecture transformation of Spiro from a monolithic application into a multi-game OTA-updatable platform.

## Overview
Transform the original single-mode Inhale application into a multi-game platform where each game is independently buildable and (eventually) OTA-updatable, while sharing common core functionality.

## Evolution

**Original State (Inhale):**
- Single monolithic firmware with 3 modes (Live, Diagnostic, Balloon)
- Scene-based architecture with shared core components
- All code compiled into one binary

**Current State (Spiro - Phase 1):**
- ✅ Modular directory structure (core/ + games/)
- ✅ Independent game builds with separate entry points
- ✅ Multi-sensor support (BMP280/BME280) via build flags
- ✅ Simulator and ESP32 build environments
- 🚧 OTA infrastructure (planned for Phase 2)

## Goals
- ✅ Support multiple independent games as separate builds
- ✅ Share core infrastructure (BreathData, Display, Sensor, Storage)
- 🚧 Enable OTA updates to download/install new games (Phase 2)
- ✅ Maintain code quality and avoid excessive duplication

---

## Phase 1: Code Reorganization ✅ COMPLETE

### 1.1 Create Core Library Structure ✅
**Objective:** Extract shared functionality into a core library
**Status:** Complete

**Completed Steps:**
1. Create new directory structure:
   ```
   src/
   ├── core/                    # Shared infrastructure
   │   ├── hardware/
   │   │   ├── BreathData.cpp/h
   │   │   ├── Display.cpp/h
   │   │   ├── Sensor.cpp/h
   │   │   └── Storage.cpp/h
   │   ├── ui/
   │   │   ├── MenuSystem.cpp/h      # NEW: Game selection menu
   │   │   ├── DiagnosticScene.cpp/h
   │   │   └── SettingsScene.cpp/h   # NEW: Settings UI
   │   ├── auth/
   │   │   └── Login.cpp/h            # NEW: Authentication (future)
   │   └── scenes/
   │       └── SceneBase.h
   │
   ├── games/                   # Game-specific code
   │   ├── balloon/
   │   │   ├── main.cpp
   │   │   └── scenes/
   │   │       └── BalloonScene.cpp/h
   │   └── live_breath/
   │       ├── main.cpp
   │       └── scenes/
   │           └── LiveScene.cpp/h
   │
   └── config.h                 # Global config (shared)
   ```

2. Move existing files:
   - `BreathData.*` → `core/hardware/`
   - `Display.*` → `core/hardware/`
   - `Sensor.*` → `core/hardware/`
   - `Storage.*` → `core/hardware/`
   - `scenes/SceneBase.h` → `core/scenes/`
   - `scenes/diagnostic/*` → `core/ui/DiagnosticScene.*`
   - `scenes/balloon/*` → `games/balloon/scenes/`
   - `scenes/live/*` → `games/live_breath/scenes/`

3. Update `#include` paths throughout codebase

**Files affected:** All source files
**Testing:** ✅ Simulator and ESP32 builds verified working

---

### 1.2 Create Game Entry Points ✅
**Objective:** Each game has its own main.cpp
**Status:** Complete

**Completed Steps:**
1. Create `games/balloon/main.cpp`:
   ```cpp
   #include "core/hardware/BreathData.h"
   #include "core/hardware/Display.h"
   #include "core/hardware/Sensor.h"
   #include "core/hardware/Storage.h"
   #include "scenes/BalloonScene.h"

   void setup() {
     // Initialize hardware
     // Start balloon game
   }

   void loop() {
     // Game loop
   }
   ```

2. Create `games/live_breath/main.cpp` (similar structure)

3. Keep original `src/main.cpp` as template

**Files created:** ✅
- `games/balloon/main.cpp`
- `games/live_breath/main.cpp`

**Testing:** Each game builds independently

---

### 1.3 Update Build Configuration ✅
**Objective:** Create separate build environments for each game
**Status:** Complete

**Completed Steps:**
1. Update `platformio.ini`:
   ```ini
   [common]
   lib_deps =
     lovyan03/LovyanGFX@^1.2.7
     adafruit/Adafruit BMP280 Library

   [env:balloon_game_esp32]
   platform = espressif32
   board = esp32dev
   framework = arduino
   build_src_filter =
     +<core/>
     +<games/balloon/>
     -<games/live_breath/>
   build_flags = -DGAME_BALLOON

   [env:live_breath_esp32]
   platform = espressif32
   board = esp32dev
   framework = arduino
   build_src_filter =
     +<core/>
     +<games/live_breath/>
     -<games/balloon/>
   build_flags = -DGAME_LIVE_BREATH

   [env:balloon_simulator]
   platform = native
   build_src_filter =
     +<core/>
     +<games/balloon/>
     -<games/live_breath/>
   build_flags = -DSIMULATOR -DGAME_BALLOON
   ```

2. Test builds:
   ```bash
   # Simulators
   pio run -e balloon_simulator
   pio run -e live_breath_simulator

   # ESP32 builds with sensor variants
   pio run -e balloon_esp32_bme280
   pio run -e balloon_esp32_bmp280
   pio run -e live_breath_esp32_bme280
   pio run -e live_breath_esp32_bmp280
   ```

**Files affected:** `platformio.ini`
**Testing:** ✅ All environments build successfully
**Bonus:** Added multi-sensor support (BMP280/BME280) via build flags

---

### Phase 1 Summary

**Completion Date:** December 24, 2024

**Achievements:**
- ✅ Successfully refactored monolithic Inhale into modular Spiro architecture
- ✅ Separated code into core infrastructure and game-specific modules
- ✅ Created independent build environments for each game
- ✅ Maintained full simulator and hardware build compatibility
- ✅ Added BMP280/BME280 sensor flexibility via compile flags
- ✅ Both games (Balloon, Live Breath) building and running successfully

**Repository:** https://github.com/itayke/spiro

**Next Steps:** Phase 2 - OTA Infrastructure (when ready)

---

## Phase 2: OTA Infrastructure (PLANNED)

### 2.1 Partition Scheme Setup
**Objective:** Configure ESP32 for OTA updates

**Steps:**
1. Create custom partition table `partitions.csv`:
   ```csv
   # Name,   Type, SubType, Offset,  Size
   nvs,      data, nvs,     0x9000,  0x5000
   otadata,  data, ota,     0xe000,  0x2000
   app0,     app,  ota_0,   0x10000, 0x1E0000
   app1,     app,  ota_1,   0x1F0000,0x1E0000
   spiffs,   data, spiffs,  0x3D0000,0x30000
   ```

2. Update `platformio.ini`:
   ```ini
   board_build.partitions = partitions.csv
   ```

**Files created:** `partitions.csv`
**Testing:** Flash and verify partition table with `esptool.py`

---

### 2.2 OTA Update Module
**Objective:** Create reusable OTA update system

**Steps:**
1. Create `core/ota/OTAManager.cpp/h`:
   ```cpp
   class OTAManager {
   public:
     void checkForUpdates();
     void downloadFirmware(const char* url);
     bool installUpdate();
     void rollback();
   };
   ```

2. Implement basic OTA flow:
   - Connect to WiFi (credentials from NVS)
   - Check server for available games
   - Download firmware binary
   - Verify checksum
   - Write to inactive partition
   - Reboot to new firmware

3. Add rollback safety:
   - Track boot count in NVS
   - If boot fails 3 times, rollback to previous partition

**Files created:**
- `core/ota/OTAManager.cpp/h`
- `core/ota/WiFiConfig.cpp/h`

**Testing:**
- Test OTA update between two test firmwares
- Test rollback on bad firmware

---

### 2.3 Game Selection Menu
**Objective:** Base firmware that lists/launches games

**Steps:**
1. Create minimal "launcher" firmware:
   - Shows list of installed games
   - Shows available games from server
   - Allows game selection/download

2. Create `core/ui/MenuSystem.cpp/h`:
   ```cpp
   class MenuSystem {
   public:
     void showGameList();
     void showDownloadMenu();
     void launchGame(int gameId);
   };
   ```

3. Store game metadata in NVS:
   ```cpp
   struct GameInfo {
     char name[32];
     char version[16];
     uint32_t partition;  // Which OTA partition
     uint32_t size;
   };
   ```

**Files created:**
- `games/launcher/main.cpp`
- `core/ui/MenuSystem.cpp/h`

**Testing:** Menu navigates correctly, can switch between games

---

## Phase 3: Game Metadata & Versioning

### 3.1 Firmware Metadata
**Objective:** Embed metadata in each firmware

**Steps:**
1. Create `core/metadata.h`:
   ```cpp
   #define FIRMWARE_VERSION "1.0.0"
   #define GAME_NAME "Balloon"
   #define GAME_ID 0x01

   struct FirmwareMetadata {
     uint32_t magic;        // 0x494E4841 ("INHA")
     uint32_t gameId;
     char gameName[32];
     char version[16];
     uint32_t buildDate;
     uint32_t checksum;
   };
   ```

2. Store metadata at fixed offset in each firmware

3. Server API returns available games with metadata

**Files created:** `core/metadata.h`
**Testing:** Read metadata from flashed firmware

---

### 3.2 Version Management
**Objective:** Track installed games and versions

**Steps:**
1. Extend `Storage` class:
   ```cpp
   void saveGameMetadata(uint8_t slot, FirmwareMetadata* meta);
   FirmwareMetadata* getGameMetadata(uint8_t slot);
   ```

2. On boot, validate installed game metadata

3. Show version info in diagnostic mode

**Files affected:** `core/hardware/Storage.cpp/h`
**Testing:** Install multiple games, verify metadata persists

---

## Phase 4: Server Infrastructure (Future)

### 4.1 Game Server API
**Objective:** Server to host game firmware files

**Endpoints:**
```
GET  /api/games              # List available games
GET  /api/games/{id}         # Game metadata
GET  /api/games/{id}/download # Download firmware binary
POST /api/analytics          # Usage telemetry (optional)
```

### 4.2 Build Pipeline
**Objective:** Automated builds for each game

**Steps:**
1. GitHub Actions workflow:
   - Build all game variants
   - Generate checksums
   - Upload to server
   - Update game catalog

**Files created:** `.github/workflows/build-games.yml`

---

## Testing Strategy

### Unit Tests
- [ ] Core modules build independently
- [ ] Each game builds without core changes
- [ ] Simulator builds for each game

### Integration Tests
- [ ] OTA update from game A to game B
- [ ] OTA rollback on failed boot
- [ ] Game metadata persists across updates
- [ ] WiFi configuration survives updates

### Hardware Tests
- [ ] Flash game via USB
- [ ] OTA update over WiFi
- [ ] Boot with corrupted firmware (rollback test)
- [ ] Multiple game installs (if multi-slot)

---

## Migration Path

### Recommended Order:
1. ✅ **Phase 1.1-1.3** - Reorganize code (1-2 hours)
   - Can be done incrementally
   - Low risk, improves structure regardless

2. **Phase 2.1** - Partition setup (30 min)
   - One-time configuration
   - Enables OTA testing

3. **Phase 1.2** - Game entry points (1 hour)
   - Create separate main.cpp per game
   - Test independent builds

4. **Phase 2.2** - Basic OTA (2-3 hours)
   - Implement WiFi + OTA download
   - Test firmware updates

5. **Phase 3** - Metadata & versioning (1-2 hours)
   - Add game identification
   - Track installed games

6. **Phase 2.3** - Game selection menu (2-3 hours)
   - Build launcher firmware
   - Integrate game switching

7. **Phase 4** - Server infrastructure (Future)
   - When ready to distribute games

### Rollback Plan:
- Keep original monolithic structure in git branch
- Each phase is reversible
- Dual partition allows testing without breaking current firmware

---

## Open Questions

1. **WiFi Credentials Storage:**
   - Store in NVS (persists across game updates)?
   - How to configure for first-time setup?

2. **Game Selection UX:**
   - Dedicated launcher firmware?
   - Long-press button to enter game menu?
   - Combo button sequence?

3. **Shared Assets:**
   - Fonts, images used by multiple games?
   - Bundle with each game vs shared partition?

4. **Multi-Tenancy:**
   - User profiles (if login implemented)?
   - Per-game or per-device calibration?

5. **Fallback Behavior:**
   - What if no WiFi available?
   - What if OTA server down?
   - Should launcher be in read-only partition?

---

## Benefits of This Architecture

✅ **Full C++ flexibility** - Each game is native code
✅ **Independent updates** - Update games without affecting others
✅ **Safe rollback** - Dual partition enables recovery
✅ **Scalable** - Easy to add new games
✅ **Testable** - Each game builds/tests independently
✅ **Maintainable** - Core changes benefit all games

---

## Estimated Effort

| Phase | Time | Status | Actual |
|-------|------|--------|--------|
| 1.1 Code reorganization | 2h | ✅ Complete | ~2h |
| 1.2 Game entry points | 1h | ✅ Complete | ~1h |
| 1.3 Build config | 1h | ✅ Complete | ~1.5h |
| 2.1 Partition setup | 0.5h | 🚧 Planned | - |
| 2.2 OTA implementation | 3h | 🚧 Planned | - |
| 2.3 Game menu | 2h | 🚧 Planned | - |
| 3.1-3.2 Metadata | 2h | 📋 Future | - |
| 4.1-4.2 Server | TBD | 📋 Future | - |

**Phase 1 (Complete):** ~4.5 hours actual
**Remaining OTA system:** ~5.5 hours estimated
**Full OTA system:** ~10 hours estimated

---

## Phase 1 Completion Checklist

- [x] Review and finalize architecture plan
- [x] Create core/ and games/ directory structure
- [x] Extract both games (balloon & live_breath)
- [x] Create independent build environments
- [x] Add multi-sensor support (BMP280/BME280)
- [x] Test simulator builds
- [x] Verify ESP32 build compatibility
- [x] Document new architecture in README
- [x] Push to GitHub repository

## Phase 2 Planning Checklist

- [ ] Review OTA partition requirements
- [ ] Decide on WiFi/OTA UX approach
- [ ] Design game selection menu flow
- [ ] Plan rollback/recovery mechanism
- [ ] Start Phase 2.1 when ready

---

*Document created: 2024-12-22*
*Last updated: 2024-12-24 (Phase 1 Complete)*
