# LogEverything

LogEverything is an Unreal Engine plugin that embeds Tencent's high-performance [BqLog](https://github.com/Tencent/BqLog) runtime and exposes it through UE-style logging macros. The integration delivers hierarchical category management, asynchronous persistence, and zero-copy emissions while retaining the developer experience of `UE_LOG`.

> Looking for Chinese documentation? See [README_CHN.md](README_CHN.md).

## Core Capabilities
- **BqLog under the hood**: UE calls are forwarded straight into BqLog's template interface, so `{}` formatting, precision specifiers, and zero-copy arguments (including `FString`, `FName`, `FText`) all work out of the box.
- **UE-style surface**: `DECLARE_LE_CATEGORY_EXTERN`, `DEFINE_LE_CATEGORY`, `LE_LOG`, and helpers behave like their `UE_LOG` counterparts, making adoption frictionless.
- **Hierarchical category tree**: Categories such as `Game.Combat.Skill` are generated from `Config/LogEverythingCategories.txt` and mirrored inside a lightweight `ULECategoryTree` for inheritance-aware gating.
- **Runtime subsystem**: `ULELogSubsystem` (a `UGameInstanceSubsystem`) tracks global verbosity, per-branch overrides, enable/disable flags, statistics, and debug export so C++ and Blueprint code share the same state.
- **Async-first defaults**: `FLELogSettings` enables asynchronous flushing, per-category defaults, and on-disk persistence without extra setup.
- **Tooling bundle**: Windows/Linux/macOS category generators and a sync script keep BqLog sources and metadata aligned with upstream releases.

## Getting Started
1. **Install the plugin**
   - Copy `Plugins/LogEverything` into your project's `Plugins/` directory (or into the engine-wide plugins folder).
   - Enable the plugin via `LogEverything.uplugin` or through the Unreal Editor plugin browser.
2. **(Optional) Adjust categories**
   - Edit `Config/LogEverythingCategories.txt` to reflect your logging hierarchy.
   - Run `Tools/BqLogTools/GenerateLogEverythingCategories.bat` (or the platform-specific generator) to refresh `Source/Generated/LogEverythingLogger.*`.
3. **Build**
   - Regenerate project files if necessary and compile from the editor or your pipeline.
4. **Run**
   - When the game instance boots, `ULELogSubsystem` initializes BqLog with default `FLELogSettings`, runs an automated smoke test, and starts writing logs under `Saved/LogEverything/LE_<ProcessId>_*.log`.

### Minimal Usage Example
```cpp
// Header
#include "LELogMacros.h"
DECLARE_LE_CATEGORY_EXTERN(LogGameCombatSkill, Game.Combat.Skill);

// Source
#include "LECategoryDefine.h"
DEFINE_LE_CATEGORY(LogGameCombatSkill);

// Gameplay code
void UMyGameplaySystem::TickSystem()
{
    LE_LOG(LogGameCombatSkill, Warning,
           TEXT("Player {} health: {}, multiplier: {.2f}"),
           PlayerName, PlayerHealth, DamageMultiplier);

    LE_CLOG(PlayerHealth < 20, LogGameCombatSkill, Error,
            TEXT("Critical health for {}"), PlayerName);
}
```

## Runtime Category Management
`ULELogSubsystem` and the Blueprint-friendly `ULogEverythingUtils` expose the full category tree:
- Set or query category levels: `ULogEverythingUtils::SetLogCategoryLevel`, `GetEffectiveLogLevel`, `ShouldLogCategory`.
- Toggle branches: `SetCategoryEnabled`, `IsCategoryEnabled`, and convenience helpers to adjust game/engine/editor groups.
- Reload or rebuild the tree: `ReloadLogSettings`, `ReinitializeCategoryTree`, `ExportTreeDebugString`.
- Inspect decisions with the console variable `LogEverything.Debug.LogCategory` (prints category evaluation details when enabled).

All logging macros pass through `ULogEverythingUtils::InternalLogImp`, so the subsystem decides whether a message should emit **before** any formatting cost is incurred.

### Console Commands & Debugging
- `LE.Test.ConditionalLogging` exercises conditional logging macros with a sample gameplay state.
- `LE.Test.DynamicLevelFilter` demonstrates live category level adjustments and showcases the debug CVar workflow.
- `LE.Debug.PrintCategoryTree` emits the full category tree (hierarchy, effective levels, enablement) to the log for visualization.
- `LE.Debug.QueryCategoryLevel <Category>` reports the effective level for a specific path.

Toggle verbosity decision traces with the console variable `LogEverything.Debug.LogCategory`.

## Tooling & Maintenance
- **Category generators**: `Tools/BqLogTools/` bundles prebuilt generators for Windows, Linux, and macOS plus scripts such as `GenerateLogEverythingCategories.bat`.
- **BqLog sync**: `Tools/SyncBqLog.bat` copies sources from a local BqLog checkout (`BQLOG_SOURCE_PATH`) into `Source/BQLog/` so the plugin tracks upstream changes.
- **Samples & tests**: `Source/LogEverything/Private/LEUsageExample.cpp` and `LELogSystemTest.cpp` illustrate gameplay/editor/test scenarios and validate the integration pipeline.

## Repository Layout
```text
Plugins/LogEverything/
├─ Config/                     # Category definitions consumed by the generator
├─ Content/                    # Optional Unreal assets
├─ Resources/                  # Plugin icon and metadata
├─ Source/
│  ├─ BQLog/                   # Vendored BqLog sources (Apache 2.0)
│  ├─ Generated/               # Generated category accessors
│  └─ LogEverything/
│     ├─ Bridge/               # BqLog bridge layer
│     ├─ Category/             # Category declarations and tree implementation
│     ├─ System/               # GameInstance subsystem & type definitions
│     ├─ Utils/                # Blueprint utilities & internal helpers
│     └─ Macros/               # UE-style logging macros
└─ Tools/                      # Generators and helper scripts
```

## What's New in 0.7.0
- Added `ULELogSubsystem` (GameInstance subsystem) as the runtime governance layer for levels, enablement, propagation, and stats.
- Introduced `ULECategoryTree` to mirror BqLog categories, enabling inheritance-aware checks, bulk toggles, and debug exports.
- Centralized logging flow through `ULogEverythingUtils::InternalLogImp` so filters run before formatting; approved messages still stream into BqLog's template interface for zero-copy emission.
- Expanded Blueprint/C++ helpers for adjusting categories at runtime and added `LogEverything.Debug.LogCategory` for diagnostic dumps.
- Detailed changelog: [v0.7.0 Changelog (EN)](ChangeLogs/CHANGELOG_v0.7.0_EN.md).

## Licensing
- LogEverything: Apache License 2.0 (`LICENSE`).
- Embedded BqLog: Apache License 2.0 (`Plugins/LogEverything/Source/BQLog/LICENSE.txt`).
- If you distribute BqLog separately, continue providing it under the same terms.

## Support & Roadmap
- **Issues / feature requests**: Open a GitHub issue with reproduction steps or desired behavior.
- **Pull requests**: Mention tested UE versions and target platforms; attach logs for significant changes.
- **Upcoming roadmap**: Environment-specific category policies (e.g., development vs. QA vs. dedicated server profiles) and configurable settings assets are planned for the next release.
- **Changelogs**: Review [v0.7.0 Changelog (EN)](ChangeLogs/CHANGELOG_v0.7.0_EN.md) and earlier files for version history.
