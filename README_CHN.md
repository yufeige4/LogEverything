# LogEverything（中文说明）

LogEverything 是一个将 [BqLog](https://github.com/Tencent/BqLog) 深度集成到 Unreal Engine 的日志插件，提供与 UE 原生日志相同的宏体验。当前版本为 **0.5 初始发布**，目标是在 UE 工程中通过熟悉的语法输出不同分类与级别的日志。

> 英文版说明请参阅项目根目录的 [README.md](README.md)。

## 功能特性
- **UE 风格宏**：`DECLARE_LE_CATEGORY_EXTERN`、`DEFINE_LE_CATEGORY`、`LE_LOG` 等宏保持与 `UE_LOG` 一致的调用方式。
- **层级化分类**：使用 `Config/LogEverythingCategories.txt` 定义 `Game.Combat.Skill` 等层级结构，工具会生成静态句柄。
- **内置 BqLog 核心**：仓库直接附带 BqLog 源码与类别生成器，可在 Windows/Linux/macOS 下使用。
- **丰富示例**：`LEUsageExample.cpp`、`LELogSystemTest.cpp` 等文件展示了玩法、编辑器等多种场景的调用方式。
- **同步脚本**：通过 `Tools/SyncBqLog.bat` 可将本地 BqLog 仓库的最新变更同步到插件内。

## 快速开始
1. **安装插件**：把 `Plugins/LogEverything` 复制到项目或引擎的 `Plugins/` 目录，并在插件管理器中启用。
2. **（可选）自定义分类**：修改 `Config/LogEverythingCategories.txt` 后运行 `Tools/BqLogTools/GenerateLogEverythingCategories.bat` 生成新的 `Source/Generated/LogEverythingLogger.*`。
3. **编译项目**：根据需要重新生成项目文件并在 UE 编辑器或命令行完成编译。
4. **运行**：插件启动时会加载默认配置并执行一次自检，日志文件默认保存到 `Saved/LogEverything/LE_<进程ID>_*.log`。

## 关键配置
- **默认设置**：`FLELogSettings` 默认启用异步写入，缓冲区 1 MB，全局级别为 `Log`。
- **日志位置**：可通过修改 `FLELogSettings::LogFilePath` 或自定义初始化逻辑更改输出路径。
- **运行时宏**：`LE_SET_GLOBAL_LEVEL`、`LE_ENABLE_CATEGORY` 等宏已经暴露，但当前版本尚未真正影响 BqLog 输出，后续会接入 `ShouldLogCategory` 流程。
- **系统自检**：`RunLogEverythingSystemTests` 会在启动后稍作延时执行一次基础验证，可能产生少量诊断日志。

## 目录速览
```text
Plugins/LogEverything/
├─ Config/                     # 分类配置文件
├─ Content/                    # 可选 UE 资源（示例素材等）
├─ Resources/                  # 插件图标与资源
├─ Source/
│  ├─ BQLog/                   # 内嵌 BqLog 源码（Apache 2.0）
│  ├─ Generated/               # 自动生成的类别访问代码
│  └─ LogEverything/           # 插件模块实现
└─ Tools/                      # 生成器与辅助脚本
```

## 许可证信息
- 插件整体遵循 Apache License 2.0（见仓库根目录 `LICENSE`）。
- 内嵌的 BqLog 亦遵循 Apache 2.0，许可证文件位于 `Plugins/LogEverything/Source/BQLog/LICENSE.txt`。

## 后续计划
- 打通分类级别过滤，使运行时宏真正影响输出。
- 提供可选的示例项目或模板工程。
- 为系统自检提供开关或条件编译支持。

如需反馈或贡献代码，可在 GitHub 提交 Issue 或 Pull Request。
