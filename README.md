# LogEverything

LogEverything 是一个将 [BqLog](https://github.com/Tencent/BqLog) 深度集成到 Unreal Engine 的日志插件，使用 UE 风格的宏即可记录分层分类日志。当前版本为 **0.5 初始发布**，目标是提供最小可用闭环：在 UE 项目中以熟悉的方式输出不同 Category 与 Verbosity 的日志。

> English readers: a short summary is available at the end of this document.

## 功能特性
- **UE 风格宏**：`DECLARE_LE_CATEGORY_EXTERN`、`DEFINE_LE_CATEGORY`、`LE_LOG` 等宏完全贴合 UE 使用习惯。
- **层级化分类**：通过 `Config/LogEverythingCategories.txt` 描述形如 `Game.Combat.Skill` 的层级，并自动生成静态句柄。
- **跨平台 BqLog 核心**：仓库直接捆绑 BqLog 源码与工具，默认启用异步写入和文件输出。
- **示例与测试**：`LEUsageExample.cpp` 和 `LELogSystemTest.cpp` 演示常见场景，便于快速验证功能。
- **工具链支持**：包含 Windows/Linux/Mac 版本的类别生成器，可一键同步外部 BqLog 更新。

## 快速上手
1. **获取插件**：
   - 将仓库的 `Plugins/LogEverything` 目录复制到你的 UE 工程或引擎的 `Plugins/` 下。
   - 确保 `LogEverything.uplugin` 处于启用状态。
2. **（可选）更新分类**：编辑 `Config/LogEverythingCategories.txt`，然后运行 `Tools/BqLogTools/GenerateLogEverythingCategories.bat` 生成新的 `Source/Generated/LogEverythingLogger.*`。
3. **编译工程**：在 UE 编辑器或命令行重新生成项目文件并编译。
4. **初始化日志**：插件加载时会自动以默认配置启动 BqLog 并执行一次基础自测，日志文件默认写入 `Saved/LogEverything/`（文件名前缀为 `LE_<进程ID>`）。

### 最简单的使用示例
```cpp
// 头文件中声明分类
#include "LELogMacros.h"
DECLARE_LE_CATEGORY_EXTERN(LogGameCombatSkill, Game.Combat.Skill);

// 源文件中定义分类
#include "LETestCategories.h"
DEFINE_LE_CATEGORY(LogGameCombatSkill);

// 在任意位置输出日志
void UMyGameplaySystem::TickSystem()
{
    LE_LOG(LogGameCombatSkill, Log, TEXT("Ability queue size: %d"), AbilityQueue.Num());
    LE_CLOG(IsInErrorState(), LogGameCombatSkill, Warning, TEXT("State machine invalid"));
}
```

更多示例见 `Plugins/LogEverything/Source/LogEverything/Private/LEUsageExample.cpp`，涵盖条件日志、便利宏、性能度量、断言等场景。

## 分类生成流程
1. 在 `Config/LogEverythingCategories.txt` 中按行维护分类路径，支持任意深度的点分层级。
2. 运行 `Tools/BqLogTools/GenerateLogEverythingCategories.bat`（Linux/Mac 可直接调用对应平台的 `BqLog_CategoryLogGenerator`）。
3. 生成结果位于 `Source/Generated/`，包括：
   - `LogEverythingLogger.h`
   - `LogEverythingLogger.cs`
   - `LogEverythingLogger.java`
   目前 UE 插件仅依赖 C++ 头文件；其余语言输出用于潜在的跨语言工具链，可视情况提交或忽略。

## 配置与调试
- **默认设置**：模块在启动时加载 `FLELogSettings` 默认配置（全局级别 `Log`，异步写入开启，缓冲区 1 MB）。
- **日志位置**：`Saved/LogEverything/LE_<PID>_*.log`。可通过修改 `FLELogSettings::LogFilePath` 或自定义初始化流程调整输出路径。
- **运行时宏**：`LE_SET_GLOBAL_LEVEL`、`LE_ENABLE_CATEGORY` 等宏已提供接口，但 **当前版本尚未实现对 BqLog 输出的过滤**（计划在后续版本打通 `ShouldLogCategory` 逻辑）。
- **系统自测**：`RunLogEverythingSystemTests` 会在插件启动后延时执行一次基础验证，可用于确认环境配置是否正确。

## 版本规划与限制
- 目前只聚焦最小功能闭环：使用 UE 风格宏驱动 BqLog 输出。
- Category 级别过滤尚未完成，调用相关宏不会改变实际输出；后续版本会补齐。
- 插件默认启用自测日志，若不需要可在未来版本中增加开关或条件编译。

## 目录结构概览
```text
Plugins/LogEverything/
├─ Config/                     # 分类配置
├─ Content/                    # UE 资源（示例资产）
├─ Resources/                  # 插件图标
├─ Source/
│  ├─ BQLog/                   # 内嵌 BqLog 源码（Apache 2.0）
│  ├─ Generated/               # 自动生成的类别访问代码
│  └─ LogEverything/           # 插件模块源码
└─ Tools/                      # 分类生成器及同步脚本
```

## 许可信息
- 插件整体采用 Apache License 2.0（见仓库根目录 `LICENSE`）。
- 内嵌的 BqLog 依旧遵循 Apache License 2.0，完整版可在 `Plugins/LogEverything/Source/BQLog/LICENSE.txt` 查看。
- 若在发行版中剔除 `Source/BQLog/`，请确保外部依赖以合法方式提供并附带许可说明。

## 支持与交流
- 代码问题：提交 GitHub Issue 或直接发起 Pull Request。
- BqLog 更新：使用 `Tools/SyncBqLog.bat` 指向你的本地 BqLog 仓库即可同步最新源码与头文件。
- 了解更多：阅读 `Plugins/LogEverything/AGENTS.md`、`Plugins/LogEverything/CLAUDE.md` 获取内部工作流说明。

---

## English Summary
LogEverything is an Unreal Engine plugin that embeds Tencent's BqLog while exposing UE-style macros (`LE_LOG`, `DECLARE_LE_CATEGORY_EXTERN`, etc.) for hierarchical logging. Version 0.5 focuses on the minimal viable feature set—printing log entries across categories/verbosities. Category-level filtering macros are already exposed but not wired yet. Refer to `Config/LogEverythingCategories.txt` plus the generator under `Tools/BqLogTools/` to extend category trees. BqLog sources are bundled under `Source/BQLog/` and keep their Apache 2.0 license (see `Plugins/LogEverything/Source/BQLog/LICENSE.txt`).
