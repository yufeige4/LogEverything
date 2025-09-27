# LogEverything Plugin v0.7.0 Changelog (English)

*中文版请参见 [CHANGELOG_v0.7.0_CHN.md](CHANGELOG_v0.7.0_CHN.md).*

## Highlights
- Introduced **ULELogSubsystem** (GameInstance subsystem) as the runtime control hub that manages category state, propagation rules, and global verbosity.
- Added a lightweight **ULECategoryTree** mirroring the BqLog hierarchy, enabling inheritance-aware checks, branch toggles, statistics, and debug export.
- Centralized logging flow via ULogEverythingUtils::InternalLogImp, evaluating verbosity before formatting; approved messages still hit BqLog's template interface for zero-copy emission.
- Expanded **Blueprint/C++ utilities** for runtime management: set/query category levels, enable/disable branches, reload settings, collect stats, and apply presets for game/engine/editor domains.
- Logging macros retain UE syntax but now pass through the subsystem + bridge so that validated calls route straight into BqLog templates.

## Improvements
- Added UE console commands:
  - LE.Test.ConditionalLogging exercises conditional logging macros with a sample gameplay state.
  - LE.Test.DynamicLevelFilter demonstrates live category level adjustments and the debug CVar workflow.
  - LE.Debug.PrintCategoryTree emits the full category tree (node hierarchy, levels, enablement flags) to the log for visualization.
  - LE.Debug.QueryCategoryLevel <Category> reports the effective level for a specific path.
- Added console variable LogEverything.Debug.LogCategory to toggle verbose decision output during filtering.
- Reorganized default categories (Game / Engine / Editor / Test roots plus common subtrees) and auto-populated them from generated BqLog metadata.
- Refined FLEBqLogBridge to maintain a weak pointer to the subsystem, letting Blueprint utilities and macros share runtime state.
- Restructured source layout (Bridge/, Category/, System/, Utils/) to clarify responsibilities and simplify future extensions.
- Updated ELELogVerbosity so it mirrors BqLog levels directly while mapping cleanly back to UE verbosity for editor tooling.

## Upgrade Notes
- Existing LE_LOG / LE_CLOG calls remain source-compatible; projects automatically gain early-out filtering and subsystem governance after merging the new files.
- Ensure the plugin runs where a UGameInstance exists so the subsystem can initialize and manage categories.
- Regenerate BqLog categories if new paths were added; the subsystem derives its tree from the generated header.

## Assets
- Documentation and samples refreshed under Source/LogEverything and Tools/ inside the repository.

