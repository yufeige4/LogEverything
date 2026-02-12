# LogEverything v1.0.0 Changelog

*中文版本请参阅 [CHANGELOG_v1.0.0_CHN.md](CHANGELOG_v1.0.0_CHN.md)*

## Highlights

- **ShowDebug Visualization** - Real-time HUD display of complete category tree with effective levels and enable states via `ShowDebug LogEverything` console command
- **Comprehensive Test Suite** - 19 automated tests (11 functional + 8 performance) ensuring full coverage and reliability
- **Performance Benchmark Framework** - Multi-round testing with statistical analysis, visualization charts, and professional Markdown reports
- **Performance Validated** - LE_LOG is **52-73% faster** than UE_LOG while including full category/level filtering

## Performance Results

> Tested on Intel Core i7-14700 (28 cores), 127.6GB RAM, Windows 11, Development build
> 10-round average with coefficient of variation < 2.2%

| Scenario | UE_LOG | LE_LOG (with filtering) | Improvement |
|----------|--------|------------------------|-------------|
| Baseline (1M logs) | 1.66M/s | 2.54M/s | **+52%** |
| Formatted logging | 1.46M/s | 2.53M/s | **+73%** |
| Multi-threaded (4 threads) | 4.93M/s | 5.37M/s | **+9%** |

**Filter overhead**: ~122 nanoseconds per call when logs are filtered out

**Key insight**: LE_LOG performs category/level filtering on every call, yet still outperforms UE_LOG which has no such capability.

## New Features

### ShowDebug Visualization System
- Real-time HUD display via `ShowDebug LogEverything` console command
- Color-coded log levels: Verbose (gray), Debug (cyan), Info (white), Warning (yellow), Error (red), Fatal (purple)
- Explicit settings marked with `*` suffix, disabled nodes shown as `[OFF]`
- Module-level delegate registration ensures PIE multi-instance safety
- Displays total node count, explicit override count, and current global level

### Automated Testing Suite
- **11 functional tests**: Category tree initialization, level inheritance, enable propagation, runtime modification, JSON loading, multi-threading safety, format parameter coverage
- **8 performance tests**: UE_LOG vs LE_LOG comparison across baseline, formatting, filtering, and multi-threading scenarios
- CSV export with detailed metrics including P50/P95/P99 percentile data

### Performance Benchmark Tooling
- `RunPerfTestMultiRound.bat` - Orchestrates N-round testing (default: 10 rounds)
- `GeneratePerfReportMultiRound.py` - Aggregates data, computes statistics (mean, std, min/max), generates professional reports
- Three visualization charts: throughput comparison, round-by-round trends, filter overhead analysis
- System information collection (CPU, memory, OS) for reproducible benchmarks

### Dual Global Level Management
- Separated `DefaultGlobalLevel` (from JSON config, immutable at runtime) and `CurrentGlobalLevel` (modifiable via API)
- `ShouldLogCategory()` now uses the stricter of the two levels
- Enables runtime adjustments while preserving ability to reset to initial configuration

## Breaking Changes

- `FLEBqLogConfig.GlobalLogLevel` removed - use JSON config `defaultLevel` field instead
- `ULELogEverythingSettings::GetEffectiveLogConfigAsset()` removed - functionality consolidated in `LELogSubsystem::LoadConfigAssetForCurrentEnvironment()`

## Upgrade Notes

1. **Source compatibility**: All existing `LE_LOG` / `LE_CLOG` calls remain fully compatible
2. **Config migration**: Run `Tools/BqLogTools/GenerateLogEverythingCategories.bat` to regenerate JSON config
3. **ShowDebug usage**: Enter `ShowDebug LogEverything` in game console to visualize category tree
4. **Performance testing**: Use UE Session Frontend -> LogEverything.Performance to run benchmarks
5. **Global level**: If you were using `FLEBqLogConfig.GlobalLogLevel`, migrate to JSON `defaultLevel` field

## Documentation

- Full performance report: [docs/benchmark/PERFORMANCE_REPORT.md](../docs/benchmark/PERFORMANCE_REPORT.md)
- Performance testing methodology: [Plugins/LogEverything/docs/solutions/performance-testing/](../Plugins/LogEverything/docs/solutions/performance-testing/)
