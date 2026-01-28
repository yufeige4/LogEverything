# LogEverything 插件 v0.9.0 更新日志（中文）

*English version available in [CHANGELOG_v0.9.0_EN.md](CHANGELOG_v0.9.0_EN.md).*

## 核心亮点
- **JSON 驱动的分类配置** -- 用 `Config/LogEverythingCategoryConfig.json` 替代 `ULECategoryConfigNode` UObject 树。每个分类节点支持可选的 `level` 和 `enabled` 字段，省略则继承父节点。
- **Python 生成器（支持合并模式）** -- `GenerateCategoryConfigJson.py` 从 `LogEverythingCategories.txt` 构建 JSON，重新生成时保留已有的配置覆盖。
- **环境感知的 DataAsset 加载** -- 新增 `LoadConfigAssetForCurrentEnvironment()`，根据构建配置（Development/Debug/Test/Shipping）和服务器类型自动选择对应的 `ULELogConfigAsset`。
- **BqLog 桥接重构** -- 提取 `InitializeBqLogBridge(FLEBqLogConfig)` 方法，从配置参数显式初始化 BqLog 系统。
- **自动打印分类树** -- 启用控制台变量 `LogEverything.Debug.LogCategory` 时，初始化完成后通过 `UE_LOG` 输出完整分类树。

## 新增文件
| 文件 | 用途 |
|------|------|
| `Tools/BqLogTools/GenerateCategoryConfigJson.py` | 从 `LogEverythingCategories.txt` 生成并合并分类 JSON |
| `Tools/BqLogTools/README_CategoryConfig.md` | JSON 格式文档，包含字段说明与使用示例 |
| `Config/LogEverythingCategoryConfig.json` | 生成的分类配置文件（生成后可手动编辑） |
| `Source/LogEverything/Public/Config/LELogConfigAsset.h` | 配置资产，包含 JSON 路径引用和解析方法 |
| `Source/LogEverything/Private/Config/LELogConfigAsset.cpp` | 基于 `FJsonSerializer` 的 JSON 解析实现 |
| `docs/solutions/architecture/json-category-config-replaces-uobject-tree.md` | 架构决策记录 |

## 修改文件
| 文件 | 变更摘要 |
|------|----------|
| `Source/LogEverything/Public/System/LELogSubsystem.h` | 新增 `InitializeBqLogBridge()`、`LoadConfigAssetForCurrentEnvironment()`；移除 `ApplyDefaultCategoryConfigurations()` |
| `Source/LogEverything/Private/System/LELogSubsystem.cpp` | 初始化流程通过 ConfigAsset 加载 JSON；移除硬编码默认配置；新增分类树调试打印 |
| `Source/LogEverything/LogEverything.Build.cs` | 公共依赖新增 `DeveloperSettings`；私有依赖新增 `Projects`、`Json`、`JsonUtilities` |
| `Tools/BqLogTools/GenerateLogEverythingCategories.bat` | 在 BqLog 代码生成后集成 Python 脚本调用 |

## 移除内容
- `ApplyDefaultCategoryConfigurations()` 硬编码默认配置方法

## JSON 配置格式
每个分类节点支持以下字段（完整文档参阅 [README_CategoryConfig.md](../Plugins/LogEverything/Tools/BqLogTools/README_CategoryConfig.md)）：

| 字段 | 类型 | 必填 | 可选值 |
|------|------|------|--------|
| `name` | string | 是 | 分类子名称（如 `"Combat"`） |
| `level` | string | 否 | `"NotSet"` / `"Verbose"` / `"Debug"` / `"Info"` / `"Warning"` / `"Error"` / `"Fatal"` |
| `enabled` | string | 否 | `"NotSet"` / `"Enabled"` / `"Disabled"` |
| `children` | array | 否 | 子分类节点数组 |

## 升级提示
- 现有 `LE_LOG` / `LE_CLOG` 调用保持完全源代码兼容。
- 运行 `Tools/BqLogTools/GenerateLogEverythingCategories.bat` 生成初始 JSON 配置。
- 将之前的硬编码分类配置迁移到 `Config/LogEverythingCategoryConfig.json`。
