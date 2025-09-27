# LogEverything 插件 v0.7.0 更新日志（中文）

*English version available in [CHANGELOG_v0.7.0_EN.md](CHANGELOG_v0.7.0_EN.md)。*

## 核心亮点
- 新增 **ULELogSubsystem**（`GameInstance` 子系统）作为运行时控制中枢，负责分类状态、继承规则与全局级别管理。
- 引入轻量级 **ULECategoryTree**，镜像 **BqLog** 分类层级，实现继承级别判断、分支启停、统计与调试导出。
- 通过 `ULogEverythingUtils::InternalLogImp` 统一日志流程，在格式化前完成级别校验；通过的记录仍以零拷贝方式进入 **BqLog** 模板接口。
- 拓展 **蓝图/C++ 工具函数**，支持运行时设置/查询分类级别、启停分支、重载配置与统计查询。
- 日志宏保持 `UnrealEngine` 语法，同时通过子系统与桥接层将验证后的调用直接送入 **BqLog** 模板接口。

## 细节优化
- 新增 `UnrealEngine` 控制台命令：
  - `LE.Test.ConditionalLogging` 演示条件日志宏（`LE_CLOG` 等）并模拟游戏状态。
  - `LE.Test.DynamicLevelFilter` 演示运行时日志级别调整与 `LogEverything.Debug.LogCategory` 调试 `CVar` 工作流。
  - `LE.Debug.PrintCategoryTree` 将完整分类树（层级、有效级别、启用状态）打印到日志，便于可视化，例如：
    ```
    === Category Tree Debug Info ===
    Total Nodes: 18, Root Index: 0
    
    [0] Node[LogRoot]: Explicit=Yes(ELELogVerbosity::Info), Effective=ELELogVerbosity::Info, Enabled=Yes, Parent=-1, Children=4
      [1] Node[Engine]: Explicit=Yes(ELELogVerbosity::Info), Effective=ELELogVerbosity::Info, Enabled=Yes, Parent=0, Children=0
      [2] Node[Game]: Explicit=Yes(ELELogVerbosity::Verbose), Effective=ELELogVerbosity::Verbose, Enabled=Yes, Parent=0, Children=4
        [3] Node[Game.Combat]: Explicit=No(None), Effective=ELELogVerbosity::Verbose, Enabled=Yes, Parent=2, Children=3
          [4] Node[Game.Combat.Damage]: Explicit=No(None), Effective=ELELogVerbosity::Verbose, Enabled=Yes, Parent=3, Children=0
          [5] Node[Game.Combat.Skill]: Explicit=No(None), Effective=ELELogVerbosity::Verbose, Enabled=Yes, Parent=3, Children=0
          [6] Node[Game.Combat.Input]: Explicit=No(None), Effective=ELELogVerbosity::Verbose, Enabled=Yes, Parent=3, Children=0
        [7] Node[Game.Animation]: Explicit=No(None), Effective=ELELogVerbosity::Verbose, Enabled=Yes, Parent=2, Children=0
        [8] Node[Game.AI]: Explicit=No(None), Effective=ELELogVerbosity::Verbose, Enabled=Yes, Parent=2, Children=2
          [9] Node[Game.AI.BehaviorTree]: Explicit=No(None), Effective=ELELogVerbosity::Verbose, Enabled=Yes, Parent=8, Children=0
          [10] Node[Game.AI.Pathfinding]: Explicit=No(None), Effective=ELELogVerbosity::Verbose, Enabled=Yes, Parent=8, Children=0
        [11] Node[Game.Input]: Explicit=No(None), Effective=ELELogVerbosity::Verbose, Enabled=Yes, Parent=2, Children=3
          [12] Node[Game.Input.Ability]: Explicit=No(None), Effective=ELELogVerbosity::Verbose, Enabled=Yes, Parent=11, Children=0
          [13] Node[Game.Input.Movement]: Explicit=No(None), Effective=ELELogVerbosity::Verbose, Enabled=Yes, Parent=11, Children=0
          [14] Node[Game.Input.Interaction]: Explicit=No(None), Effective=ELELogVerbosity::Verbose, Enabled=Yes, Parent=11, Children=0
      [15] Node[Editor]: Explicit=Yes(ELELogVerbosity::Info), Effective=ELELogVerbosity::Info, Enabled=Yes, Parent=0, Children=0
      [16] Node[Test]: Explicit=Yes(ELELogVerbosity::Verbose), Effective=ELELogVerbosity::Verbose, Enabled=Yes, Parent=0, Children=1
        [17] Node[Test.LogSystem]: Explicit=Yes(ELELogVerbosity::Debug), Effective=ELELogVerbosity::Debug, Enabled=Yes, Parent=16, Children=0
    ```
  - `LE.Debug.QueryCategoryLevel <Category>` 查询指定分类路径的有效级别。
- 新增控制台变量 `LogEverything.Debug.LogCategory`，用于开关过滤过程中的调试输出。
- 重构默认分类结构（`Game` / `Engine` / `Editor` / `Test` 根节点及常용子类），并自动解析 **BqLog** 生成的元数据。
- `FLEBqLogBridge` 现持有子系统的弱引用，使蓝图工具与日志宏共享统一运行时状态。
- 调整源码目录结构（`Bridge/`、`Category/`、`System/`、`Utils/`），职责更加清晰，便于后续扩展。
- 更新 `ELELogVerbosity` 枚举，使其与 **BqLog** 级别完全对应，同时可映射回 `UnrealEngine` 日志级别。

## 升级提示
- 现有 `LE_LOG` / `LE_CLOG` 调用保持源代码兼容；合并新文件后即可自动获得前置过滤与子系统治理能力。
- 请确保插件运行在包含 `UGameInstance` 的上下文中，以便子系统成功初始化并接管分类。
- 如新增分类，请重新生成 `BqLog` 类别文件，子系统会基于生成的头文件构建分类树。

## 相关资源
- 更新后的文档与示例位于仓库的 `Source/LogEverything/` 与 `Tools/` 目录。
