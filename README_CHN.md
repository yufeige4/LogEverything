# LogEverything（中文说明）

LogEverything 是一个将腾讯高性能日志库 [BqLog](https://github.com/Tencent/BqLog) 深度集成到 `Unreal Engine` 的插件。它通过提供熟悉的UE风格的日志宏接口，支持层级化分类、异步持久化与零拷贝输出能力，让项目在保留 UE_LOG 体验的同时享受 BqLog 的性能与现代格式化方案。

> Looking for English documentation? See [README.md](README.md)。

## 核心特性
- **BqLog 支撑**：`UnealEngine` 日志调用直接进入 `BqLog` 模板接口，原生支持 `{}` 占位符、精度控制以及 `FString` / `FName` / `FText` 等 `UE` 类型的零拷贝传递。
- **UE 风格接口**：`DECLARE_LE_CATEGORY_EXTERN`、`DEFINE_LE_CATEGORY`、`LE_LOG` 等写法与 `UE_LOG` 完全一致，迁移成本极低。
- **层级化分类树**：`Config/LogEverythingCategories.txt` 定义的 `Game.Combat.Skill` 等路径会生成轻量级 `ULECategoryTree`，支持继承级别判断与分支管理。
- **运行时子系统**：`ULELogSubsystem`（`UGameInstanceSubsystem`）负责全局级别、分支启停、统计与调试导出，`C++` 与蓝图共享同一运行时状态。
- **异步默认配置**：`FLELogSettings` 默认开启异步写入、合理缓冲与落盘路径，无需额外配置即可直接使用。
- **配套工具链**：随插件提供 `Windows` / `Linux` / `macOS` 的分类生成器与同步脚本，方便与上游 `BqLog` 保持一致。

## 快速开始
1. **安装插件** – 将 `Plugins/LogEverything` 复制到项目或引擎的 `Plugins/` 目录，并在 `UnealEngine` 插件管理器中启用。
2. **调整分类（可选）** – 修改 `Config/LogEverythingCategories.txt` 后，运行 `Tools/BqLogTools/GenerateLogEverythingCategories.bat`（或对应平台生成器）刷新 `Source/Generated/LogEverythingLogger.*`。
3. **编译工程** – 视情况重新生成项目文件，并在编辑器或 CI 流程中完成编译。
4. **运行验证** – 游戏实例启动时 `ULELogSubsystem` 会加载默认配置、执行一次自检，并将日志写入 `Saved/LogEverything/LE_<进程ID>_*.log`。

### 最简使用示例
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
// 可以使用{}占位符来进行数据打印, 同时支持{:.2f}或{.2f}风格
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
- **后续路线** – 计划在后续版本支持按不同环境（`开发`、`QA`、`正式`、`专用服务器`）定制分类/级别策略，并补充配置资产。
- **更新记录** – 历史版本详见 [v0.7.0 更新日志（中文）](ChangeLogs/CHANGELOG_v0.7.0_CHN.md) 及更早条目。
