# LogEverything

LogEverything is an Unreal Engine plugin that embeds Tencent's [BqLog](https://github.com/Tencent/BqLog) and exposes a UE-style macro layer for hierarchical logging. Version **0.5 (initial preview)** focuses on the minimal viable feature set: emitting log entries across categories and verbosities while mirroring the familiar `UE_LOG` workflow.

> Looking for the Chinese documentation? Check out [README_CHN.md](README_CHN.md).

## Highlights
- **UE-first ergonomics**: `DECLARE_LE_CATEGORY_EXTERN`, `DEFINE_LE_CATEGORY`, and `LE_LOG` replicate Unreal's logging style.
- **Hierarchical categories**: `Config/LogEverythingCategories.txt` describes paths such as `Game.Combat.Skill`; the toolchain generates strongly typed handles.
- **Bundled BqLog core**: the repository vendors BqLog sources and the category generator tooling for Windows, Linux, and macOS.
- **Sample coverage**: `LEUsageExample.cpp` and `LELogSystemTest.cpp` walk through common gameplay/editor scenarios.
- **Sync scripts**: `Tools/SyncBqLog.bat` lets you refresh BqLog from a local clone when upstream changes land.

## Getting Started
1. **Install the plugin**
   - Copy `Plugins/LogEverything` into your project's `Plugins/` folder or the engine's plugin directory.
   - Enable it via `LogEverything.uplugin` or the UE plugin browser.
2. **(Optional) Customize categories**
   - Edit `Config/LogEverythingCategories.txt` to match your hierarchy.
   - Run `Tools/BqLogTools/GenerateLogEverythingCategories.bat` (or call the platform-specific generator) to rebuild `Source/Generated/LogEverythingLogger.*`.
3. **Build**
   - Regenerate project files if needed and compile the project from the editor or your build pipeline.
4. **Run**
   - On startup the module bootstraps BqLog with default settings and triggers a one-off smoke test. Logs are emitted under `Saved/LogEverything/LE_<ProcessId>_*.log`.

### Minimal Usage Example
```cpp
// In a header
#include "LELogMacros.h"
DECLARE_LE_CATEGORY_EXTERN(LogGameCombatSkill, Game.Combat.Skill);

// In the matching source file
#include "LETestCategories.h"
DEFINE_LE_CATEGORY(LogGameCombatSkill);

// Anywhere in your gameplay code
void UMyGameplaySystem::TickSystem()
{
    LE_LOG(LogGameCombatSkill, Log, TEXT("Ability queue size: %d"), AbilityQueue.Num());
    LE_CLOG(IsInErrorState(), LogGameCombatSkill, Warning, TEXT("State machine invalid"));
}
```

Explore `Source/LogEverything/Private/LEUsageExample.cpp` for extended patterns including assertions, convenience macros, runtime level tweaks, and perf instrumentation.

## Category Generation Workflow
1. Maintain category paths (one per line) inside `Config/LogEverythingCategories.txt`. Nested hierarchies use dot notation.
2. Invoke the generator script or binaries (`GenerateLogEverythingCategories.bat`, `linux64/BqLog_CategoryLogGenerator`, etc.).
3. The generated artifacts live under `Source/Generated/`:
   - `LogEverythingLogger.h`
   - `LogEverythingLogger.cs`
   - `LogEverythingLogger.java`
   Only the C++ header is required by the plugin today; the other outputs are provided for potential cross-language tooling.

## Configuration Notes
- **Default settings**: the module loads `FLELogSettings` with global level `Log`, asynchronous flushing enabled, and a 1 MB buffer.
- **Log destination**: files land in `Saved/LogEverything/`. Adjust `FLELogSettings::LogFilePath` or extend the initialization flow to redirect output.
- **Runtime macros**: `LE_SET_GLOBAL_LEVEL`, `LE_ENABLE_CATEGORY`, etc., are already exposed but currently **do not gate the BqLog output**. Wiring the `ShouldLogCategory` checks is on the roadmap for a future release.
- **Smoke tests**: `RunLogEverythingSystemTests` schedules a short delay timer to validate the bridge. Expect a small burst of diagnostic messages after startup.

## Roadmap & Limitations
- Category-level filtering is not yet enforced; macros that mutate levels will be hooked up in an upcoming drop.
- The built-in smoke test cannot be disabled in 0.5. Feedback on configurability is welcome.
- Additional samples (e.g., packaged projects or editor tools) are planned as the feature surface grows.

## Repository Layout
```text
Plugins/LogEverything/
├─ Config/                     # Category list consumed by the generator
├─ Content/                    # Optional UE assets (e.g., icons, examples)
├─ Resources/                  # Plugin icon and metadata
├─ Source/
│  ├─ BQLog/                   # Vendored BqLog sources (Apache 2.0)
│  ├─ Generated/               # Generated category accessors
│  └─ LogEverything/           # Plugin module implementation
└─ Tools/                      # Generators and helper scripts
```

## Licensing
- LogEverything is distributed under the Apache License 2.0 (see `LICENSE`).
- BqLog remains Apache 2.0; the upstream license is mirrored at `Plugins/LogEverything/Source/BQLog/LICENSE.txt`.
- If you decouple the bundled BqLog sources, ensure downstream consumers still obtain them under the correct terms.

## Support & Contributions
- Issues and feature requests: open a ticket on GitHub.
- Pull requests: please describe tested platforms and include relevant logs where possible.
- Upstream sync: use `Tools/SyncBqLog.bat` with the `BQLOG_SOURCE_PATH` environment variable pointing at your local BqLog checkout.

---

## Release Notes
- **0.5 (Initial Preview)**
  - UE-style macro surface covering declaration, definition, and log emission.
  - Category generator script and prebuilt binaries bundled for all platforms.
  - Embedded BqLog sources with license attribution.
  - Sample usage and automated smoke test on module startup.
