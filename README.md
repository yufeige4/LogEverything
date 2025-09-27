# LogEverything

LogEverything is an Unreal Engine plugin that embeds Tencent's high-performance [BqLog](https://github.com/Tencent/BqLog) runtime while preserving the familiar UE-style logging surface. It delivers hierarchical category management, asynchronous persistence, and zero-copy emission without forcing teams to abandon the ergonomics of `UE_LOG`.

> 中文说明请参阅 [README_CHN.md](README_CHN.md)。

## Core Capabilities
- **BqLog integration** – Logging macros forward directly to BqLog's template interface, so `{}` formatting, precision specifiers, and `UnrealEngine` string types (`FString`, `FName`, `FText`) function without manual conversion.
- **UE-style surface** – `DECLARE_LE_CATEGORY_EXTERN`, `DEFINE_LE_CATEGORY`, `LE_LOG`, and helper macros mirror Unreal’s native workflow, minimizing the learning curve.
- **Hierarchical category tree** – Categories such as `Game.Combat.Skill` are sourced from `Config/LogEverythingCategories.txt` and mirrored inside a lightweight `ULECategoryTree`, enabling inheritance-aware gating.
- **Runtime subsystem** – `ULELogSubsystem` (`UGameInstanceSubsystem`) tracks global verbosity, per-branch overrides, enable/disable flags, statistics, and debug exports, ensuring `C++` and `Blueprint` share a unified state.
- **Async-first defaults** – `FLELogSettings` ships with asynchronous flushing, sensible buffers, on-disk persistence, and baseline category configuration already enabled.
- **Bundled tooling** – `Windows`/`Linux`/`macOS` category generators and sync scripts keep the vendored **BqLog** sources aligned with upstream releases.

## Getting Started
1. **Install** – Copy `Plugins/LogEverything` into your project or engine `Plugins/` folder and enable the plugin via `LogEverything.uplugin` or the `Unreal Editor` plugin browser.
2. **Configure (optional)** – Edit `Config/LogEverythingCategories.txt` to match your hierarchy and run `Tools/BqLogTools/GenerateLogEverythingCategories.bat` (or the platform-specific generator) to refresh `Source/Generated/LogEverythingLogger.*`.
3. **Build** – Regenerate project files if required and compile from the editor or your preferred build pipeline.
4. **Run** – When the game instance boots, `ULELogSubsystem` initializes **BqLog** with default settings, executes a smoke test, and writes logs under `Saved/LogEverything/LE_<ProcessId>_*.log`.

### Minimal Usage Example
```cpp

// 如何声明一个日志分类
DECLARE_LE_CATEGORY_EXTERN(LELogGameCombatSkill, Game.Combat.Skill);
DEFINE_LE_CATEGORY(LELogGameCombatSkill);

// 任意玩法逻辑
float PlayerHealth = 15.0f;
int32 AmmoCount = 0;
bool bIsEnemyNear = true;
// 使用常规打印宏
// 支持多种日志级别包含：Verbose, Debug, Info, Warning, Error, Fatal  
// 可以限制不同分类具备不同日志等级权限
LE_LOG(LELogTestLogSystem, Debug, TEXT("Current gameplay state:"));
// 使用快捷打印宏
LE_LOG_DEBUG(LELogTestLogSystem, TEXT("- Player health: {:.1f}"), PlayerHealth);
LE_LOG_DEBUG(LELogTestLogSystem, TEXT("- Ammo count: {}"), AmmoCount);
LE_LOG_DEBUG(LELogTestLogSystem, TEXT("- Enemy nearby: {}"), bIsEnemyNear ? TEXT("Yes") : TEXT("No"));
LE_LOG_DEBUG(LELogTestLogSystem, TEXT(""));

LE_CLOG_DEBUG(PlayerHealth < 20.0f, LELogGameCombatDamage, TEXT("Player health critically low: {:.1f}"), PlayerHealth);
LE_CLOG_DEBUG(AmmoCount == 0, LELogGameCombatSkill, TEXT("Ammunition depleted, ranged skills unavailable"));
LE_CLOG_DEBUG(bIsEnemyNear && PlayerHealth < 50.0f, LELogGameAI, TEXT("Danger: enemy closing in while health is low"));

// 最终输出日志如下
UTC+08 2025-09-27 10:51:36.942[tid-177304 GameThread]	[D]	[Test.LogSystem]	Current gameplay state:
UTC+08 2025-09-27 10:51:36.942[tid-177304 GameThread]	[D]	[Test.LogSystem]	- Player health: 15.0
UTC+08 2025-09-27 10:51:36.942[tid-177304 GameThread]	[D]	[Test.LogSystem]	- Ammo count: 0
UTC+08 2025-09-27 10:51:36.942[tid-177304 GameThread]	[D]	[Test.LogSystem]	- Enemy nearby: Yes
UTC+08 2025-09-27 10:51:36.942[tid-177304 GameThread]	[D]	[Test.LogSystem]	
UTC+08 2025-09-27 10:51:36.942[tid-177304 GameThread]	[D]	[Game.Combat.Damage]	Player health critically low: 15.0
UTC+08 2025-09-27 10:51:36.942[tid-177304 GameThread]	[D]	[Game.Combat.Skill]	Ammunition depleted, ranged skills unavailable
UTC+08 2025-09-27 10:51:36.942[tid-177304 GameThread]	[D]	[Game.AI]	Danger: enemy closing in while health is low

```

## Runtime Category Management
`ULELogSubsystem` and the Blueprint-friendly `ULogEverythingUtils` expose the entire category tree:
- Set or query levels with `ULogEverythingUtils::SetLogCategoryLevel`, `GetEffectiveLogLevel`, and `ShouldLogCategory`.
- Enable or disable branches via `SetCategoryEnabled` / `IsCategoryEnabled`, with convenience helpers for `game`/`engine`/`editor` domains.
- Reload or rebuild definitions using `ReloadLogSettings`, `ReinitializeCategoryTree`, and `ExportTreeDebugString`.
- Inspect filtering decisions through the console variable `LogEverything.Debug.LogCategory`.

All logging macros pass through `ULogEverythingUtils::InternalLogImp`, so verbosity checks run **before** any formatting work is performed.

### Console Commands & Debugging
- `LE.Test.ConditionalLogging` – Exercises conditional macros (`LE_CLOG`, `LE_CHECK`, etc.) against a sample gameplay state.
- `LE.Test.DynamicLevelFilter` – Demonstrates live category level adjustments and the `LogEverything.Debug.LogCategory` `CVar` workflow.
- `LE.Debug.PrintCategoryTree` – Emits the full category hierarchy, effective levels, and enablement flags to the log for inspection
- `LE.Debug.QueryCategoryLevel <Category>` – Reports the effective level for a specific category path.

Toggle verbose filtering traces with the `LogEverything.Debug.LogCategory` console variable.

## Tooling & Maintenance
- **Category generators** – `Tools/BqLogTools/` bundles prebuilt generators for `Windows`, `Linux`, and `macOS` plus convenience scripts (e.g. `GenerateLogEverythingCategories.bat`).

## Repository Layout
```
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

## What’s New in 0.7.0
- Added `ULELogSubsystem` as the runtime governance layer for levels, enablement, propagation, and statistics.
- Introduced `ULECategoryTree` to mirror **BqLog** categories, enabling inheritance-aware checks, batch toggles, and debug exports.
- Centralized logging flows through `ULogEverythingUtils::InternalLogImp`, ensuring filters run before formatting while maintaining zero-copy emission into **BqLog**.
- Expanded `Blueprint`/`C++` helpers for runtime adjustments and added `LogEverything.Debug.LogCategory` for diagnostic traces.
- Detailed release notes: [v0.7.0 Changelog (EN)](ChangeLogs/CHANGELOG_v0.7.0_EN.md).

## Licensing
- `LogEverything`: `Apache License 2.0` (`LICENSE`).
- Embedded **BqLog**: `Apache License 2.0` (`Plugins/LogEverything/Source/BQLog/LICENSE.txt`).
- Distributors who decouple **BqLog** should continue providing it under the same terms.

## Support & Roadmap
- **Issues / feature requests** – Open a `GitHub` issue with reproduction steps or desired behaviour.
- **Pull requests** – Mention tested `UnrealEngine` versions/platforms and attach relevant logs for significant changes.
- **Upcoming roadmap** – Environment-specific category and verbosity policies (`development`, `QA`, `shipping`, `dedicated server`) plus configurable settings assets.
- **Changelogs** – Review [v0.7.0 Changelog (EN)](ChangeLogs/CHANGELOG_v0.7.0_EN.md) and earlier files for version history.
