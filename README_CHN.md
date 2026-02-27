# LogEverything（中文说明）

LogEverything 是一个将腾讯高性能日志库 [BqLog](https://github.com/Tencent/BqLog) 深度集成到 `Unreal Engine` 的插件。它通过提供熟悉的UE风格的日志宏接口，支持层级化分类、异步持久化与零拷贝输出能力，让项目在保留类似 `UE_LOG` 体验的同时享受 **BqLog** 的性能与现代格式化方案。

> Looking for English documentation? See [README.md](README.md)。

## 核心特性
- **BqLog 支撑**：`UnealEngine` 日志调用直接进入 `BqLog` 模板接口，原生支持 `{}` 占位符、精度控制以及 `FString` / `FName` / `FText` 等 `UE` 类型的零拷贝传递。
- **UE 风格接口**：`DECLARE_LE_CATEGORY_EXTERN`、`DEFINE_LE_CATEGORY`、`LE_LOG` 等写法与 `UE_LOG` 完全一致，迁移成本极低。
- **层级化分类树**：`Config/LogEverythingCategories.txt` 定义的 `Game.Combat.Skill` 等路径会生成轻量级 `ULECategoryTree`，支持继承级别判断与分支管理。
- **运行时子系统**：`ULELogSubsystem`（`UGameInstanceSubsystem`）负责全局级别、分支启停、统计与调试导出，`C++` 与蓝图共享同一运行时状态。
- **异步默认配置**：`FLELogSettings` 默认开启异步写入、合理缓冲与落盘路径，无需额外配置即可直接使用。
- **配套工具链**：随插件提供 `Windows` / `Linux` / `macOS` 的分类生成器与同步脚本，方便与上游 `BqLog` 保持一致。

## 快速开始

### 安装

1. 将 `Plugins/LogEverything` 复制到项目的 `Plugins/` 目录
2. 在 Unreal Editor 插件管理器中启用插件
3. 编译项目

即可使用！LogEverything 提供开箱即用的默认配置。

### 基本用法

```cpp
// 第一步：声明日志分类（头文件中）
DECLARE_LE_CATEGORY_EXTERN(LogCombat, Game.Combat);

// 第二步：定义分类（源文件中）
DEFINE_LE_CATEGORY(LogCombat);

// 第三步：使用现代 {} 格式化语法记录日志
LE_LOG(LogCombat, Info, TEXT("玩家 {} 造成了 {} 点伤害"), PlayerName, DamageAmount);
```

### 为什么选择 LogEverything

| 特性 | LE_LOG | UE_LOG |
|------|--------|--------|
| 吞吐量 | 254万条/秒 | 166万条/秒 |
| 格式化语法 | 现代 `{}` 占位符 | printf 风格 `%s %d` |
| 分类层级 | 支持 `Game.Combat.Skill` | 扁平分类 |
| 运行时过滤 | 按分类级别过滤 | 有限支持 |
| 异步持久化 | 支持 | 同步写入 |

### 代码示例

**基础日志**
```cpp
LE_LOG(LogCombat, Info, TEXT("战斗开始"));
LE_LOG(LogCombat, Warning, TEXT("弹药不足：剩余 {} 发"), AmmoCount);
LE_LOG(LogCombat, Error, TEXT("投射物生成失败"));
```

**条件日志**
```cpp
LE_CLOG(Health < 20.0f, LogCombat, Warning, TEXT("血量危急：{:.1f}"), Health);
LE_CHECK(IsValid(Target), LogCombat, Error, TEXT("目标无效"));
```

**便捷宏**
```cpp
LE_LOG_INFO(LogCombat, TEXT("比赛开始，共 {} 名玩家"), PlayerCount);
LE_LOG_WARNING(LogCombat, TEXT("服务器延迟过高：{}ms"), Latency);
LE_LOG_ERROR(LogCombat, TEXT("连接断开"));
```

**运行时配置**
```cpp
// 运行时调整日志级别
LE_SET_CATEGORY_LEVEL(LogCombat, Warning);  // 仅输出 Warning 及以上
LE_SET_GLOBAL_LEVEL(Info);                  // 设置全局阈值
LE_DISABLE_CATEGORY(LogCombat);             // 临时禁用分类
```

### 输出格式

```
UTC+08 2025-09-27 10:51:36.942[tid-177304 GameThread] [I] [Game.Combat] 玩家 John 造成了 150 点伤害
UTC+08 2025-09-27 10:51:36.943[tid-177304 GameThread] [W] [Game.Combat] 弹药不足：剩余 5 发
```

### 高级配置（可选）

如需自定义分类层级，编辑 `Config/LogEverythingCategories.txt`：

```text
Game
Game.Combat
Game.Combat.Damage
Game.Combat.Skill
Game.AI
Game.AI.Pathfinding
```

然后运行 `Tools/BqLogTools/GenerateLogEverythingCategories.bat` 重新生成分类代码。

如需按分类设置日志级别，编辑 `Config/LogEverythingCategoryConfig.json`：

```json
{
  "defaultLevel": "Info",
  "categories": [
    { "name": "Game", "children": [
      { "name": "Combat", "level": "Debug" },
      { "name": "AI", "level": "Warning" }
    ]}
  ]
}
```

## 运行时分类管理
借助 `ULELogSubsystem` 与蓝图友好的 `ULogEverythingUtils` 可以：
- 设置或查询分类级别：`ULogEverythingUtils::SetLogCategoryLevel`、`GetEffectiveLogLevel`、`ShouldLogCategory`。
- 启用或禁用分支：`SetCategoryEnabled`、`IsCategoryEnabled` 并提供针对游戏/引擎/编辑器分类的便捷函数。
- 重新加载或重建分类树：`ReloadLogSettings`、`ReinitializeCategoryTree`、`ExportTreeDebugString`。
- 通过控制台变量 `LogEverything.Debug.LogCategory` 查看过滤决策的调试信息。

所有日志宏都会经过 `ULogEverythingUtils::InternalLogImp`，在格式化之前完成级别判断，通过后才交由 `BqLog` 模板接口零拷贝输出。

### 控制台命令与调试
- `LE.Test.ConditionalLogging` – 演示条件日志宏（`LE_CLOG` 等）并模拟游戏状态。
- `LE.Test.DynamicLevelFilter` – 演示运行时日志级别调整与 `LogEverything.Debug.LogCategory` 调试 `CVar` 工作流。
- `LE.Debug.PrintCategoryTree` – 将完整分类树（层级、有效级别、启用状态）打印到日志，便于可视化
- `LE.Debug.QueryCategoryLevel <Category>` – 查询指定分类路径的有效级别。

使用控制台变量 `LogEverything.Debug.LogCategory` 可以开关过滤过程中的调试输出。

## 工具与维护
- **分类生成器** – `Tools/BqLogTools/` 提供三大桌面平台的预编译生成器与脚本（如 `GenerateLogEverythingCategories.bat`）。

## 目录结构
```
Plugins/LogEverything/
├─ Config/                     # 分类配置文件
├─ Content/                    # 可选 UE 资源
├─ Resources/                  # 插件图标与元数据
├─ Source/
│  ├─ BQLog/                   # 内嵌 BqLog 源码（Apache 2.0）
│  ├  Generated/               # 生成的分类访问代码
│  └─ LogEverything/
│     ├─ Bridge/               # BqLog 桥接层
│     ├─ Category/             # 分类声明与树实现
│     ├─ System/               # GameInstance 子系统与类型定义
│     ├─ Utils/                # 蓝图工具与内部辅助函数
│     └─ Macros/               # UE 风格日志宏
└─ Tools/                      # 生成器与辅助脚本
```

## 性能基准测试

> 测试环境：Intel Core i7-14700 (28核), 127.6GB 内存, Windows 11, Development 构建
> 10 轮测试平均值，变异系数 < 2.2%

### LE_LOG vs UE_LOG 性能对比

**吞吐量**（越高越好）：

| 测试场景 | UE_LOG | LE_LOG（含过滤） | 性能提升 |
|---------|--------|-----------------|---------|
| 基准测试（100万条） | 1.66M/秒 | 2.54M/秒 | **+52%** |
| 格式化测试（带参数） | 1.46M/秒 | 2.53M/秒 | **+73%** |
| 多线程测试（4线程） | 4.93M/秒 | 5.37M/秒 | **+9%** |

**单条日志耗时**（越低越好）：

| 测试场景 | UE_LOG | LE_LOG（含过滤） | 耗时降低 |
|---------|--------|-----------------|---------|
| 基准测试 | 601 纳秒 | 394 纳秒 | **-34%** |
| 格式化测试 | 683 纳秒 | 396 纳秒 | **-42%** |
| 多线程测试 | 203 纳秒 | 187 纳秒 | **-8%** |

**说明**：LE_LOG 耗时已包含每次调用的完整分类/级别过滤开销，而 UE_LOG 不具备此功能。

### 过滤开销

| 过滤类型 | 每次调用开销 |
|---------|------------|
| 级别过滤 | **122 纳秒** |
| 分类禁用 | **122 纳秒** |

即使在高频调用被过滤的日志时，每次调用仅消耗约 120 纳秒，对性能影响极小。

### 性能优势来源

1. **无锁环形缓冲** - BqLog 内核减少线程竞争
2. **异步持久化** - 日志先缓冲在内存，后台线程异步落盘
3. **高效格式化** - `{}` 占位符比 printf 风格更高效
4. **轻量级过滤** - 分类检查仅增加约 120 纳秒开销

![性能对比](https://raw.githubusercontent.com/yufeige4/LogEverything/master/docs/benchmark/perf_throughput_comparison_chn.png)

详细测试方法和完整结果请查看 [性能测试报告](docs/benchmark/PERFORMANCE_REPORT_CHN.md)。

## 1.0.0 新特性
- **ShowDebug 可视化** - 通过 `ShowDebug LogEverything` 控制台命令实时显示分类树
- **性能基准测试** - LE_LOG 比 UE_LOG **快 52-73%**，同时包含完整过滤功能
- **自动化测试套件** - 19 个测试（11 功能 + 8 性能）确保可靠性
- **双全局级别** - 分离初始化级别和运行时级别，控制更灵活
- **过滤开销** - 测量约 122 纳秒/次
- 详细变更请查看 [v1.0.0 更新日志（中文）](ChangeLogs/CHANGELOG_v1.0.0_CHN.md)

## 0.9.0 新特性
- 用人类可读的 **JSON 配置文件** (`Config/LogEverythingCategoryConfig.json`) 替代 `ULECategoryConfigNode` UObject 树，作为分类级别与启用状态的配置来源。
- 新增 `Tools/BqLogTools/GenerateCategoryConfigJson.py`，从 `LogEverythingCategories.txt` 自动生成 JSON，支持**合并模式**（重新生成时保留已有配置）。
- 精简 `ULELogConfigAsset`，移除所有编辑器 UObject 树回调，DataAsset 现在引用 JSON 路径与 `FLEBqLogConfig`。
- 初始化完成后，启用 `LogEverything.Debug.LogCategory` 时自动打印完整分类树到日志。
- JSON 格式详细说明请查看 [JSON 配置文档](Plugins/LogEverything/Tools/BqLogTools/README_CategoryConfig.md)。
- 详细变更请查看 [v0.9.0 更新日志（中文）](ChangeLogs/CHANGELOG_v0.9.0_CHN.md)。

## 0.7.0 新特性
- 新增 `ULELogSubsystem` 作为运行时控制层，负责级别管理、启停传播与统计输出。
- 引入 `ULECategoryTree` 同步 `BqLog` 分类，实现继承校验、批量操作与调试导出。
- 日志流程集中到 `ULogEverythingUtils::InternalLogImp`，在格式化前完成过滤，同时保持进入 `BqLog` 的零拷贝特性。
- 拓展蓝图/`C++` 工具函数，提供运行时分类调整，并加入 `LogEverything.Debug.LogCategory` 调试输出。
- 详细变更请查看 [v0.7.0 更新日志（中文）](ChangeLogs/CHANGELOG_v0.7.0_CHN.md)。

## 许可证
- 插件整体遵循 `Apache License 2.0`（根目录 `LICENSE`）。
- 内嵌 `BqLog` 亦遵循 `Apache 2.0`（`Plugins/LogEverything/Source/BQLog/LICENSE.txt`）。
- 如果单独分发 `BqLog`，请继续以相同许可方式向使用者提供。

## 支持、规划与贡献
- **问题 / 功能需求** – 欢迎在 `GitHub` 提交 `Issue`，并附上复现步骤。
- **代码贡献** – 提交 `Pull Request` 时请注明测试过的 `UnealEngine` 版本与平台，并附上关键日志。
- **后续路线** -- 支持按不同环境（`开发`、`QA`、`正式`、`专用服务器`）定制分类/级别策略；CI/CD 性能回归测试。
- **更新记录** -- 历史版本详见 [v1.0.0 更新日志（中文）](ChangeLogs/CHANGELOG_v1.0.0_CHN.md)、[v0.9.0 更新日志（中文）](ChangeLogs/CHANGELOG_v0.9.0_CHN.md)、[v0.7.0 更新日志（中文）](ChangeLogs/CHANGELOG_v0.7.0_CHN.md) 及更早条目。
