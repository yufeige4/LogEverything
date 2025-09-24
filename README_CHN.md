# LogEverything（中文说明）

LogEverything 是一个将腾讯高性能日志库 [BqLog](https://github.com/Tencent/BqLog) 深度集成到 Unreal Engine 的插件，通过熟悉的 UE 宏接口即可使用 BqLog 的分层分类和高吞吐能力。插件内置分类管理、生成工具以及默认配置，方便在现有项目中快速落地。

> 英文版说明请参阅 [README.md](README.md)。

## 核心特性
- **UE 风格宏**：DECLARE_LE_CATEGORY_EXTERN、DEFINE_LE_CATEGORY、LE_LOG 等与 UE_LOG 调用保持一致，底层由 BqLog 接管。
- **层级化分类**：在 Config/LogEverythingCategories.txt 中定义 Game.Combat.Skill 等路径，生成的访问代码具备类型安全与智能提示。
- **内置 BqLog 运行时**：仓库直接附带 BqLog 源码，可在 Windows / Linux / macOS 环境下与 UE 模块一起编译。
- **跨语言生成器**：分类生成器可导出 C++、C#、Java 代码，便于扩展编辑器工具或外围系统。
- **异步默认配置**：FLELogSettings 默认开启异步写入，支持分类级别覆盖并落盘保存。
- **示例与自检**：示例代码和自动化测试涵盖玩法、编辑器、测试等常见场景。

## 快速开始
1. **安装插件**
   - 将 Plugins/LogEverything 复制到项目或引擎的 Plugins/ 目录。
   - 在 UE 插件管理器中启用，或直接在 LogEverything.uplugin 中启用。
2. **（可选）调整分类**
   - 修改 Config/LogEverythingCategories.txt 以匹配项目层级结构。
   - 运行 Tools/BqLogTools/GenerateLogEverythingCategories.bat（或对应平台生成器）刷新 Source/Generated/LogEverythingLogger.*。
3. **编译工程**
   - 根据需求重新生成项目文件，并在编辑器或 CI 流程中编译。
4. **运行验证**
   - 模块启动时会加载默认配置并执行一次自检，日志写入 Saved/LogEverything/LE_<进程ID>_*.log。

### 最简使用示例
```cpp
// 头文件
#include "LELogMacros.h"
DECLARE_LE_CATEGORY_EXTERN(LogGameCombatSkill, Game.Combat.Skill);

// 源文件
#include "LETestCategories.h"
DEFINE_LE_CATEGORY(LogGameCombatSkill);

// 任意玩法逻辑
void UMyGameplaySystem::TickSystem()
{
    LE_LOG(LogGameCombatSkill, Warning,
           TEXT("Player {} health: {}, multiplier: {.2f}"),
           PlayerName, PlayerHealth, DamageMultiplier);

    LE_CLOG(PlayerHealth < 20, LogGameCombatSkill, Error,
            TEXT("Critical health for {}"), PlayerName);
}
```

## 分类生成流程
1. 在 Config/LogEverythingCategories.txt 中按行维护分类路径，使用点号表示层级。
2. 运行生成脚本或二进制（GenerateLogEverythingCategories.bat、linux64/BqLog_CategoryLogGenerator、mac/BqLog_CategoryLogGenerator）。
3. 生成结果位于 Source/Generated/：
   - LogEverythingLogger.h
   - LogEverythingLogger.cs
   - LogEverythingLogger.java
   插件仅依赖 C++ 头文件，其余输出可用于外部工具。

## 配置与调试说明
- **默认配置**：异步写入开启、缓冲区 1 MB，并为常见玩法/引擎/编辑器/测试分类设置默认级别。
- **日志位置**：默认输出到 Saved/LogEverything/，可通过修改 FLELogSettings::LogFilePath 或扩展启动流程进行重定向。
- **运行时控制**：LE_SET_GLOBAL_LEVEL、LE_ENABLE_CATEGORY、LE_DISABLE_CATEGORY 等宏可用，后续版本将支持按环境定制分类和级别策略。
- **自动自检**：RunLogEverythingSystemTests 在启动后短暂延迟执行，用于验证桥接是否正常。

## 工具与维护
- **同步脚本**：Tools/SyncBqLog.bat 可以通过环境变量 BQLOG_SOURCE_PATH 同步外部 BqLog 仓库。
- **生成器**：Tools/BqLogTools/ 提供三大桌面平台的预编译生成器及脚本。
- **配置**：Config/LogEverythingCategories.txt 为日志分类定义文件，修改该文件后需要重新执行代码生成以支持新配置。

## 0.6.0 新特性概览
- 宏层直接调用 BqLog 模板接口，移除 FString::Printf 预格式化，实现 UE 类型的零拷贝日志。
- 原生支持 {} 格式化（含 {.2f} 等精度控制），FString、FName、FText、基础类型均可直接传参。
- 模板在编译期校验占位符与参数类型，保持向后兼容。
- 详情参见 [ChangeLogs/CHANGELOG_v0.6.0.md](ChangeLogs/CHANGELOG_v0.6.0.md)。

## 目录结构
```ext
Plugins/LogEverything/
├─ Config/                     # 分类配置文件
├─ Content/                    # 可选 UE 资源
├─ Resources/                  # 插件图标与元数据
├─ Source/
│  ├─ BQLog/                   # 内嵌 BqLog 源码（Apache 2.0）
│  ├─ Generated/               # 生成的分类访问代码
│  └─ LogEverything/           # Unreal 模块实现
└─ Tools/                      # 生成器与辅助脚本
```

## 许可证
- 插件整体遵循 Apache License 2.0（见根目录 LICENSE）。
- 内嵌 BqLog 亦遵循 Apache 2.0（Plugins/LogEverything/Source/BQLog/LICENSE.txt）。
- 若在发布中拆分 BqLog，请继续以相同许可方式向用户提供。

## 支持、规划与贡献
- **问题 / 功能需求**：欢迎在 GitHub 提交 Issue，并附上复现步骤。
- **代码贡献**：提交 Pull Request 时注明测试过的 UE 版本及平台，并附上关键日志。
- **后续计划**：下一版本将支持针对不同环境（开发 / 测试 / 上线 / 专用服务器）定制分类及级别权限。
- **更新记录**：最新变更详见 [ChangeLogs/CHANGELOG_v0.6.0.md](ChangeLogs/CHANGELOG_v0.6.0.md)，历史版本将陆续补充在该目录中。
