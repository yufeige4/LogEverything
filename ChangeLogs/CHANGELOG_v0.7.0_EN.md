# LogEverything Plugin v0.7.0 Changelog (English)

*中文版请参见 [CHANGELOG_v0.7.0_CHN.md](CHANGELOG_v0.7.0_CHN.md)。*

## Highlights
- Introduced **`ULELogSubsystem`** (`GameInstance` subsystem) as the runtime control hub governing category state, propagation rules, and global verbosity.
- Added a lightweight **`ULECategoryTree`** that mirrors the `BqLog` hierarchy, enabling inheritance-aware level checks, branch toggles, statistics, and debug exports.
- Centralised logging flow through `ULogEverythingUtils::InternalLogImp`, so verbosity filters run before any formatting; approved messages still hit `BqLog`'s template interface for zero-copy emission.
- Expanded **`Blueprint`/`C++` utilities** to manage categories at runtime (set/query levels, enable/disable branches, reload or rebuild definitions, collect statistics).
- Retained `UE`-style macros while routing validated calls through the subsystem + bridge directly into `BqLog` templates.

## Improvements
- Added `UE` console commands:
  - `LE.Test.ConditionalLogging` exercises conditional logging macros against a sample gameplay state.
  - `LE.Test.DynamicLevelFilter` demonstrates live category level adjustments and the debug `CVar` workflow.
  - `LE.Debug.PrintCategoryTree` emits the full category tree (hierarchy, effective levels, enablement flags) to the log for visual inspection, for example:
    ```
    === Category Tree Debug Info ===
    Total Nodes: 18, Root Index: 0
    
    [0] Node[LogRoot]: Explicit=Yes(ELELogVerbosity::Info), Effective=ELELogVerbosity::Info, Enabled=Yes, Parent=-1, Children=4
      [1] Node[Engine]: Explicit=Yes(ELELogVerbosity::Info), Effective=ELELogVerbosity::Info, Enabled=Yes, Parent=0, Children=0
      [2] Node[Game]: Explicit=Yes(ELELogVerbosity::Verbose), Effective=ELELogVerbosity::Verbose, Enabled=Yes, Parent=0, Children=4
        [3] Node[Game.Combat]: Explicit=No(None), Effective=ELELogVerbosity::Verbose, Enabled=Yes, Parent=2, Children=3
          [4] Node[Game.Combat.Damage]: Explicit=No(None), Effective=ELELogVerbosity::Verbose, Enabled=Yes, Parent=3, Children=0
          [5] Node[Game.Combat.Skill]: Explicit=No(None), Effective=ELELogVerbosity::Verbose, Enabled=Yes, Parent=3, Children=0
          [6] Node[Game.Combat.Input]: Explicit=No(None), Effective=ELELogVerbosity::Verbose, Enabled=Yes, Parent=3, Children=0
        [7] Node[Game.Animation]: Explicit=No(None), Effective=ELELogVerbosity::Verbose, Enabled=Yes, Parent=2, Children=0
        [8] Node[Game.AI]: Explicit=No(None), Effective=ELELogVerbosity::Verbose, Enabled=Yes, Parent=2, Children=2
          [9] Node[Game.AI.BehaviorTree]: Explicit=No(None), Effective=ELELogVerbosity::Verbose, Enabled=Yes, Parent=8, Children=0
          [10] Node[Game.AI.Pathfinding]: Explicit=No(None), Effective=ELELogVerbosity::Verbose, Enabled=Yes, Parent=8, Children=0
        [11] Node[Game.Input]: Explicit=No(None), Effective=ELELogVerbosity::Verbose, Enabled=Yes, Parent=2, Children=3
          [12] Node[Game.Input.Ability]: Explicit=No(None), Effective=ELELogVerbosity::Verbose, Enabled=Yes, Parent=11, Children=0
          [13] Node[Game.Input.Movement]: Explicit=No(None), Effective=ELELogVerbosity::Verbose, Enabled=Yes, Parent=11, Children=0
          [14] Node[Game.Input.Interaction]: Explicit=No(None), Effective=ELELogVerbosity::Verbose, Enabled=Yes, Parent=11, Children=0
      [15] Node[Editor]: Explicit=Yes(ELELogVerbosity::Info), Effective=ELELogVerbosity::Info, Enabled=Yes, Parent=0, Children=0
      [16] Node[Test]: Explicit=Yes(ELELogVerbosity::Verbose), Effective=ELELogVerbosity::Verbose, Enabled=Yes, Parent=0, Children=1
        [17] Node[Test.LogSystem]: Explicit=Yes(ELELogVerbosity::Debug), Effective=ELELogVerbosity::Debug, Enabled=Yes, Parent=16, Children=0
    ```
  - `LE.Debug.QueryCategoryLevel <Category>` reports the effective level for a specific category path.
- Added console variable `LogEverything.Debug.LogCategory` to toggle verbose decision output during filtering.
- Reorganised default categories (`Game` / `Engine` / `Editor` / `Test` roots plus common subtrees) and auto-populated them from generated `BqLog` metadata.
- Refined `FLEBqLogBridge` to maintain a weak pointer to the subsystem, letting `Blueprint` utilities and macros share runtime state.
- Restructured source layout (`Bridge/`, `Category/`, `System/`, `Utils/`) to clarify responsibilities and simplify future extensions.
- Updated `ELELogVerbosity` to mirror `BqLog` levels directly while mapping cleanly back to `UE` verbosity for editor tooling.

## Upgrade Notes
- Existing `LE_LOG` / `LE_CLOG` calls remain source-compatible; projects automatically gain early-out filtering and subsystem governance after merging the new files.
- Ensure the plugin runs where a `UGameInstance` exists so the subsystem can initialise and manage categories.
- Regenerate `BqLog` categories if new paths were added; the subsystem derives its tree from the generated header.

## Assets
- Documentation and samples refreshed under `Source/LogEverything/` and `Tools/` inside the repository.
