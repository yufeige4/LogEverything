# LogEverything 插件 v1.0.0 更新日志（中文）

*English version available in [CHANGELOG_v1.0.0_EN.md](CHANGELOG_v1.0.0_EN.md)*

## 核心亮点

- **ShowDebug 可视化系统** - 通过 `ShowDebug LogEverything` 控制台命令实时显示完整分类树，包含有效级别和启用状态
- **完整自动化测试套件** - 19 个自动化测试（11 个功能测试 + 8 个性能测试），确保全面覆盖和可靠性
- **性能基准测试框架** - 多轮测试支持统计分析、可视化图表和专业 Markdown 报告
- **性能验证** - LE_LOG 比 UE_LOG **快 52-73%**，同时包含完整的分类/级别过滤功能

## 性能测试结果

> 测试环境：Intel Core i7-14700 (28核), 127.6GB 内存, Windows 11, Development 构建
> 10 轮测试平均值，变异系数 < 2.2%

| 测试场景 | UE_LOG | LE_LOG（含过滤） | 性能提升 |
|---------|--------|-----------------|---------|
| 基准测试（100万条日志） | 1.66M/秒 | 2.54M/秒 | **+52%** |
| 格式化测试（带参数） | 1.46M/秒 | 2.53M/秒 | **+73%** |
| 多线程测试（4线程） | 4.93M/秒 | 5.37M/秒 | **+9%** |

**过滤开销**：当日志被过滤时，每次调用仅消耗约 122 纳秒

**关键发现**：LE_LOG 在每次调用时都执行分类/级别过滤检查，但仍然比没有此功能的 UE_LOG 更快。

## 新增功能

### ShowDebug 可视化系统
- 通过 `ShowDebug LogEverything` 控制台命令实时 HUD 显示
- 颜色编码的日志级别：Verbose（灰色）、Debug（青色）、Info（白色）、Warning（黄色）、Error（红色）、Fatal（紫色）
- 显式设置的级别使用 `*` 后缀标记，禁用节点显示为 `[OFF]`
- Module 级别的委托注册确保 PIE 多实例安全
- 显示总节点数、显式覆盖数和当前全局级别

### 自动化测试套件
- **11 个功能测试**：分类树初始化、级别继承、启用传播、运行时修改、JSON 加载、多线程安全、格式参数覆盖
- **8 个性能测试**：UE_LOG vs LE_LOG 对比，覆盖基准、格式化、过滤和多线程场景
- CSV 导出详细指标，包含 P50/P95/P99 百分位数据

### 性能基准测试工具
- `RunPerfTestMultiRound.bat` - 编排 N 轮测试（默认 10 轮）
- `GeneratePerfReportMultiRound.py` - 数据聚合、统计计算（平均值、标准差、最小/最大值）、生成专业报告
- 三张可视化图表：吞吐量对比、轮次趋势、过滤开销分析
- 系统信息采集（CPU、内存、操作系统）确保基准可重现

### 双全局级别管理
- 分离 `DefaultGlobalLevel`（来自 JSON 配置，运行时不可修改）和 `CurrentGlobalLevel`（可通过 API 修改）
- `ShouldLogCategory()` 现在使用两者中更严格的级别
- 支持运行时调整，同时保留重置到初始配置的能力

## 破坏性变更

- `FLEBqLogConfig.GlobalLogLevel` 已移除 - 请改用 JSON 配置的 `defaultLevel` 字段
- `ULELogEverythingSettings::GetEffectiveLogConfigAsset()` 已移除 - 功能已整合到 `LELogSubsystem::LoadConfigAssetForCurrentEnvironment()`

## 升级指南

1. **源码兼容性**：所有现有的 `LE_LOG` / `LE_CLOG` 调用保持完全兼容
2. **配置迁移**：运行 `Tools/BqLogTools/GenerateLogEverythingCategories.bat` 重新生成 JSON 配置
3. **ShowDebug 使用**：在游戏控制台输入 `ShowDebug LogEverything` 可视化分类树
4. **性能测试**：使用 UE Session Frontend -> LogEverything.Performance 运行基准测试
5. **全局级别**：如果之前使用 `FLEBqLogConfig.GlobalLogLevel`，请迁移到 JSON `defaultLevel` 字段

## 文档

- 完整性能报告：[docs/benchmark/PERFORMANCE_REPORT_CHN.md](../docs/benchmark/PERFORMANCE_REPORT_CHN.md)
