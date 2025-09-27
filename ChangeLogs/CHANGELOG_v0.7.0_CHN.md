# LogEverything 插件 v0.7.0 更新日志（中文）

*English version available in [CHANGELOG_v0.7.0_EN.md](CHANGELOG_v0.7.0_EN.md).* 

## 核心亮点
- 新增 **ULELogSubsystem**（GameInstance 子系统）作为运行时控制中枢，负责分类状态、继承规则与全局级别管理。
- 引入轻量级 **ULECategoryTree**，镜像 BqLog 分类层级，实现继承级别判断、分支启停、统计与调试导出。
- 通过 ULogEverythingUtils::InternalLogImp 统一日志流程，在格式化前完成级别校验，通过的调用直接进入 BqLog 模板接口实现零拷贝输出。
- 拓展 **蓝图/C++ 工具函数**，可运行时设置或查询分类级别、启停分支、重载配置、获取树统计，并对游戏/引擎/编辑器分类快速套用预设。
- 日志宏保留 UE 语法，同时经过子系统与桥接层，将通过校验的消息直接投递到 BqLog 模板接口。

## 细节优化
- 新增 UE 控制台命令：
  - LE.Test.ConditionalLogging 演示条件日志宏（模拟游戏状态）。
  - LE.Test.DynamicLevelFilter 演示运行时日志级别调整与调试 CVar 流程。
  - LE.Debug.PrintCategoryTree 将完整分类树（节点层级、级别、启用状态）打印到日志。
  - LE.Debug.QueryCategoryLevel <Category> 查询指定路径的有效级别。
- 新增控制台变量 LogEverything.Debug.LogCategory，用于开关过滤过程中的调试输出。
- 重构默认分类结构（Game / Engine / Editor / Test 根节点及常用子类），并自动解析 BqLog 生成的元数据。
- FLEBqLogBridge 持有子系统弱引用，使蓝图工具与日志宏共享统一的运行时状态。
- 调整源码目录结构（Bridge/、Category/、System/、Utils/），职责更清晰，便于后续扩展。
- 更新 ELELogVerbosity 枚举，使其与 BqLog 级别完全对应，并可无缝映射回 UE 日志级别，方便编辑器工具使用。

## 升级提示
- 现有 LE_LOG / LE_CLOG 调用保持兼容；合并新文件后即可自动获得前置过滤与子系统治理能力。
- 请确保插件运行在包含 UGameInstance 的上下文中，以便子系统成功初始化并接管分类。
- 如新增分类，请重新生成 BqLog 类别文件，子系统将基于该头文件构建分类树。

## 相关资源
- 相关文档与示例已更新到仓库的 Source/LogEverything 与 Tools/ 目录。

