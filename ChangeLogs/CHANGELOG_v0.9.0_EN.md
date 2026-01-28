# LogEverything Plugin v0.9.0 Changelog (English)

*For Chinese version, see [CHANGELOG_v0.9.0_CHN.md](CHANGELOG_v0.9.0_CHN.md).*

## Highlights
- **JSON-driven category configuration** -- Replaced the `ULECategoryConfigNode` UObject tree with `Config/LogEverythingCategoryConfig.json`. Each category node supports optional `level` and `enabled` fields; omitted fields inherit from the parent.
- **Python generator with merge mode** -- `GenerateCategoryConfigJson.py` builds the JSON from `LogEverythingCategories.txt` and preserves existing overrides on regeneration.
- **Environment-aware DataAsset loading** -- New `LoadConfigAssetForCurrentEnvironment()` selects the correct `ULELogConfigAsset` based on build configuration (Development/Debug/Test/Shipping) and server type.
- **BqLog bridge refactor** -- Extracted `InitializeBqLogBridge(FLEBqLogConfig)` for explicit BqLog initialization from configuration parameters.
- **Auto-print category tree** -- Full tree dump via `UE_LOG` when `LogEverything.Debug.LogCategory` console variable is enabled.

## New Files
| File | Purpose |
|------|---------|
| `Tools/BqLogTools/GenerateCategoryConfigJson.py` | Generates and merges category JSON from `LogEverythingCategories.txt` |
| `Tools/BqLogTools/README_CategoryConfig.md` | JSON schema documentation with field reference and examples |
| `Config/LogEverythingCategoryConfig.json` | Generated category configuration (editable after generation) |
| `Source/LogEverything/Public/Config/LELogConfigAsset.h` | New config asset with JSON path reference and parsing methods |
| `Source/LogEverything/Private/Config/LELogConfigAsset.cpp` | JSON parsing implementation using `FJsonSerializer` |
| `docs/solutions/architecture/json-category-config-replaces-uobject-tree.md` | Architecture decision record |

## Modified Files
| File | Change Summary |
|------|----------------|
| `Source/LogEverything/Public/System/LELogSubsystem.h` | Added `InitializeBqLogBridge()`, `LoadConfigAssetForCurrentEnvironment()`; removed `ApplyDefaultCategoryConfigurations()` |
| `Source/LogEverything/Private/System/LELogSubsystem.cpp` | Init flow loads JSON via ConfigAsset; removed hardcoded defaults; added tree debug printing after initialization |
| `Source/LogEverything/LogEverything.Build.cs` | Added `DeveloperSettings` to public dependencies; added `Projects`, `Json`, `JsonUtilities` to private dependencies |
| `Tools/BqLogTools/GenerateLogEverythingCategories.bat` | Integrated Python script invocation after BqLog code generation |

## Removed
- `ApplyDefaultCategoryConfigurations()` hardcoded default configuration method

## JSON Configuration Format
Each category node supports the following fields (see [README_CategoryConfig.md](../Plugins/LogEverything/Tools/BqLogTools/README_CategoryConfig.md) for full documentation):

| Field | Type | Required | Values |
|-------|------|----------|--------|
| `name` | string | Yes | Category sub-name (e.g. `"Combat"`) |
| `level` | string | No | `"NotSet"` / `"Verbose"` / `"Debug"` / `"Info"` / `"Warning"` / `"Error"` / `"Fatal"` |
| `enabled` | string | No | `"NotSet"` / `"Enabled"` / `"Disabled"` |
| `children` | array | No | Child category nodes |

## Upgrade Notes
- Existing `LE_LOG` / `LE_CLOG` calls remain fully source-compatible.
- Run `Tools/BqLogTools/GenerateLogEverythingCategories.bat` to generate the initial JSON config.
- Migrate any previous hardcoded category settings to `Config/LogEverythingCategoryConfig.json`.
