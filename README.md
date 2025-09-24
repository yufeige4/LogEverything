# LogEverything

LogEverything is an Unreal Engine plugin that embeds Tencent's high-performance [BqLog](https://github.com/Tencent/BqLog) library and exposes its capabilities through familiar UE-style logging macros. The package includes hierarchical category support, bundled tooling, and sane defaults so teams can adopt BqLog without rewriting existing logging code.

> Looking for Chinese documentation? See [README_CHN.md](README_CHN.md).

## Core Features
- **UE-style macros**: DECLARE_LE_CATEGORY_EXTERN, DEFINE_LE_CATEGORY, and LE_LOG match UE_LOG usage while routing messages into BqLog.
- **Hierarchical categories**: configure paths like Game.Combat.Skill in Config/LogEverythingCategories.txt; generated accessors offer type safety and auto-complete.
- **Bundled BqLog runtime**: vendored BqLog sources build on Windows, Linux, and macOS alongside Unreal modules.
- **Cross-language generators**: category generators emit C++, C#, and Java code for editor tooling or external consumers.
- **Async-first defaults**: FLELogSettings enables asynchronous flushing, per-category overrides, and on-disk persistence out of the box.
- **Examples & tests**: shipped samples and smoke tests demonstrate gameplay, editor, and automation scenarios.

## Getting Started
1. **Install the plugin**
   - Copy Plugins/LogEverything into your project's Plugins/ directory (or the engine-wide plugins folder).
   - Enable it via LogEverything.uplugin or through the Unreal Editor plugin browser.
2. **(Optional) Adjust categories**
   - Edit Config/LogEverythingCategories.txt to match your hierarchy.
   - Run Tools/BqLogTools/GenerateLogEverythingCategories.bat (or the platform-specific generator) to refresh Source/Generated/LogEverythingLogger.*.
3. **Build**
   - Regenerate project files if required and compile from the editor or your CI pipeline.
4. **Run**
   - On startup the module initializes BqLog with default settings, triggers an automated smoke test, and writes logs under Saved/LogEverything/LE_<ProcessId>_*.log.

### Minimal Usage Example
```cpp
// Header
#include "LELogMacros.h"
DECLARE_LE_CATEGORY_EXTERN(LogGameCombatSkill, Game.Combat.Skill);

// Source
#include "LETestCategories.h"
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

## Category Generation Workflow
1. Maintain one category per line in Config/LogEverythingCategories.txt using dot notation for hierarchy.
2. Run the generator script or binary (GenerateLogEverythingCategories.bat, linux64/BqLog_CategoryLogGenerator, mac/BqLog_CategoryLogGenerator).
3. Generated files appear under Source/Generated/:
   - LogEverythingLogger.h
   - LogEverythingLogger.cs
   - LogEverythingLogger.java
   The plugin only requires the C++ header; other outputs are optional for tooling.

## Configuration Notes
- **Default profile**: asynchronous logging enabled, 1 MB buffers, and baseline verbosity overrides for gameplay, engine, editor, and test categories.
- **Log destinations**: files land in Saved/LogEverything/. Override FLELogSettings::LogFilePath or extend the startup module to redirect output.
- **Runtime controls**: macros such as LE_SET_GLOBAL_LEVEL, LE_ENABLE_CATEGORY, and LE_DISABLE_CATEGORY are available. Upcoming releases will add environment-aware policies so builds can tailor category/verbosity combinations.
- **Smoke test**: RunLogEverythingSystemTests executes shortly after startup to validate the bridge and emits a short burst of diagnostic messages.

## Tools & Maintenance
- **Sync script**: Tools/SyncBqLog.bat mirrors sources from a local BqLog checkout via the BQLOG_SOURCE_PATH environment variable.
- **Generators**: Tools/BqLogTools/ ships prebuilt generators for Windows, Linux, and macOS along with helper scripts.
- **Config**：Config/LogEverythingCategories.txt contains config for log categories, user can modify this file and execute generator tools to support customized categories.
## What's New in 0.6.0
- Macros now call BqLog template interfaces directly, removing FString::Printf pre-formatting and enabling zero-copy logging for UE types.
- Native support for modern {} formatting (including precision specifiers such as {.2f}) with compile-time validation of argument mismatches.
- Fully backward compatible with existing LE_LOG/LE_CLOG calls.
- Detailed changes: see [ChangeLogs/CHANGELOG_v0.6.0.md](ChangeLogs/CHANGELOG_v0.6.0.md).

## Repository Layout
```ext
Plugins/LogEverything/
├─ Config/                     # Category definitions consumed by the generator
├─ Content/                    # Optional Unreal assets
├─ Resources/                  # Plugin icon and metadata
├─ Source/
│  ├─ BQLog/                   # Vendored BqLog sources (Apache 2.0)
│  ├─ Generated/               # Generated category accessors
│  └─ LogEverything/           # Unreal module implementation
└─ Tools/                      # Generators and helper scripts
```

## Licensing
- LogEverything: Apache License 2.0 (LICENSE).
- Embedded BqLog: Apache License 2.0 (Plugins/LogEverything/Source/BQLog/LICENSE.txt).
- Distributors who externalize BqLog should continue supplying it under the same terms.

## Support, Roadmap & Contributions
- **Issues / feature requests**: open a GitHub issue with repro steps or desired behavior.
- **Pull requests**: describe tested UE versions/platforms and attach logs for significant changes.
- **Roadmap**: next release focuses on environment-specific category/verbosity policies (development vs. QA vs. dedicated server, etc.).
- **Changelogs**: refer to [ChangeLogs/CHANGELOG_v0.6.0.md](ChangeLogs/CHANGELOG_v0.6.0.md) for the latest history.
