---
title: "JSON 分类配置替代 UObject 树形配置"
version: "0.9.0"
category: architecture
severity: enhancement
tags:
  - json-config
  - category-tree
  - code-generation
  - uobject-removal
  - configuration-system
components:
  - LELogConfigAsset
  - LELogSubsystem
  - LECategoryTree
  - GenerateCategoryConfigJson.py
date_solved: 2026-01-28
related_files:
  - Source/LogEverything/Public/Config/LELogConfigAsset.h
  - Source/LogEverything/Private/Config/LELogConfigAsset.cpp
  - Source/LogEverything/Private/System/LELogSubsystem.cpp
  - Source/LogEverything/Public/System/LELogSubsystem.h
  - Tools/BqLogTools/GenerateCategoryConfigJson.py
  - Tools/BqLogTools/GenerateLogEverythingCategories.bat
  - Config/LogEverythingCategoryConfig.json
  - LogEverything.Build.cs
  - CLAUDE.md
---

# JSON 分类配置替代 UObject 树形配置

## 问题描述

LogEverything v0.7.0 使用 `ULECategoryConfigNode`（UObject 子类）构建树形分类配置。该方案存在以下问题：

1. **UObject 开销**: 每个分类节点都是堆分配的 UObject，参与垃圾回收和反射，对静态配置来说开销过重
2. **二进制 DataAsset**: 配置存储在 `.uasset` 二进制文件中，无法在版本控制中 diff 和合并
3. **递归 UObject 限制**: UE 的 USTRUCT 不支持自引用递归，被迫使用 UObject 包装树节点
4. **编辑器代码耦合**: 大量 `WITH_EDITOR` 下的 `PostEditChangeProperty` / `PostEditChangeChainProperty` 逻辑仅为维护 UObject 树一致性
5. **硬编码默认配置**: `ApplyDefaultCategoryConfigurations()` 在 C++ 中硬编码分类级别，修改需要重新编译

## 解决方案

用 JSON 文件替代 UObject 树，通过 Python 脚本自动生成，运行时由 UE 内置 JSON 解析器读取。

### 架构变更

```
变更前:
  Categories.txt -> BqLog生成器 -> LogEverythingLogger.h
  ULECategoryConfigNode (UObject树) -> DataAsset -> 运行时应用

变更后:
  Categories.txt -> BqLog生成器 -> LogEverythingLogger.h
                 -> Python脚本 -> LogEverythingCategoryConfig.json
  ULELogConfigAsset (引用JSON路径) -> 运行时读取JSON -> 应用到CategoryTree
```

### 实现细节

#### 1. Python 脚本 (GenerateCategoryConfigJson.py)

- 从 `Config/LogEverythingCategories.txt` 读取扁平分类路径
- 构建树状 JSON 结构，每个节点包含 `name`、`level`、`enabled`、`children` 字段
- 支持合并模式：已有 JSON 中的 level/enabled 配置在重新生成时被保留
- 集成到 `GenerateLogEverythingCategories.bat`，代码生成后自动调用

#### 2. LELogConfigAsset 改造

移除内容：
- 整个 `ULECategoryConfigNode` UCLASS（约 228 行）
- `TArray<TObjectPtr<ULECategoryConfigNode>> CategoryTree` 属性
- 所有 `WITH_EDITOR` 下的 `PostEditChangeProperty` / `PostEditChangeChainProperty` / `UpdateEntireCategoryTree`

新增内容：
- `FString CategoryConfigJsonPath` 属性（默认 `"LogEverythingCategoryConfig.json"`）
- `GetJsonConfigFullPath()`: 通过 `IPluginManager` 定位插件目录拼接完整路径
- `LoadAndApplyCategoryConfigFromJson()`: 读取 JSON、解析 defaultLevel、递归应用分类配置、传播 EffectiveLevel 和 EnabledState
- `ParseAndApplyJsonNodeRecursive()`: 递归处理每个 JSON 节点
- `StringToLogVerbosity()` / `StringToEnabledState()`: 字符串到枚举的转换

#### 3. LELogSubsystem 调整

- 移除 `ApplyDefaultCategoryConfigurations()` 硬编码方法
- `Initialize()` 中通过 `ConfigAsset->ApplyToCategoryTree()` 加载 JSON 配置
- 初始化完成后通过 CVar `LogEverything.Debug.LogCategory` 控制打印完整分类树
- `ReinitializeCategoryTree()` 改为重新加载 ConfigAsset 并应用

#### 4. 构建依赖

`LogEverything.Build.cs` 新增模块依赖：
- `Projects`: `IPluginManager` 接口
- `Json`: `FJsonSerializer` / `FJsonObject`
- `JsonUtilities`: JSON 工具函数

### JSON 配置格式

```json
{
  "version": "1.0",
  "defaultLevel": "Info",
  "categories": [
    {
      "name": "Game",
      "level": "Debug",
      "enabled": "NotSet",
      "children": [
        {
          "name": "Combat",
          "level": "NotSet",
          "enabled": "NotSet",
          "children": [
            { "name": "Damage", "level": "Warning", "enabled": "NotSet" }
          ]
        }
      ]
    }
  ]
}
```

字段说明：
- `level`: `"NotSet"` / `"Verbose"` / `"Debug"` / `"Info"` / `"Warning"` / `"Error"` / `"Fatal"`
- `enabled`: `"NotSet"` (继承父节点) / `"Enabled"` / `"Disabled"`
- 省略 level/enabled 等同于 `"NotSet"`，继承父节点配置

## 验证结果

1. Python 脚本生成 JSON 结构正确（16 个分类路径）
2. 合并模式验证通过（手动设置的 `"level": "Debug"` 在重新生成后被保留）
3. UE 项目编译通过（22 个编译单元，0 错误）
4. 模块依赖正确（Projects、Json、JsonUtilities）

## 关键决策记录

### 为什么选择树状 JSON 而非扁平格式

审查建议使用扁平 key-value 格式（如 `"Game.Combat.Damage": {"level": "Warning"}`）。选择树状格式的原因：
- 与 `LogEverythingCategories.txt` 的层级结构保持视觉一致
- 用户编辑时可直观看到父子关系
- children 数组天然表达树结构

### 为什么保留 ULELogConfigAsset

审查建议移除 DataAsset，直接在 Subsystem 中读取 JSON。保留的原因：
- DataAsset 仍承载 `FLEBqLogConfig`（BqLog 全局配置），通过编辑器可视化配置
- `ULELogEverythingSettings` 的多环境路由机制依赖 DataAsset 引用
- JSON 路径作为 DataAsset 属性可在编辑器中方便调整

### 为什么保留 Python 合并模式

审查认为合并模式过度设计。保留的原因：
- 分类经常变动（新增/删除），每次重新生成后手动恢复配置不实际
- 实现简单（约 15 行合并逻辑），维护成本低

## 后续改进方向

以下为审查中提出但本次未实施的建议，供后续参考：

1. **friend class 访问**: `ULELogConfigAsset` 通过 friend 直接修改 `ULECategoryTree::Nodes`。可考虑提供公共 `ApplyConfig(DTO)` 方法
2. **缺失 JSON 回退**: 当前 JSON 缺失时仅打印日志，无硬编码回退。可考虑添加最小默认配置
3. **线程安全文档**: `ReloadLogSettings()` 应明确要求在 GameThread 调用
4. **构建时一致性检查**: 验证生成的 JSON 与 C++ 头文件的分类列表一致

## 文件变更清单

| 操作 | 文件 | 说明 |
|------|------|------|
| 新增 | `Tools/BqLogTools/GenerateCategoryConfigJson.py` | JSON 生成脚本 |
| 新增 | `Config/LogEverythingCategoryConfig.json` | 生成的 JSON 配置 |
| 新增 | `Tools/README_CategoryConfig.md` | JSON 格式文档 |
| 修改 | `Tools/BqLogTools/GenerateLogEverythingCategories.bat` | 集成 Python 调用 |
| 重写 | `Source/LogEverything/Public/Config/LELogConfigAsset.h` | 移除 UObject 树，添加 JSON 支持 |
| 重写 | `Source/LogEverything/Private/Config/LELogConfigAsset.cpp` | JSON 解析实现 |
| 修改 | `Source/LogEverything/Public/System/LELogSubsystem.h` | 移除 ApplyDefaultCategoryConfigurations 声明 |
| 修改 | `Source/LogEverything/Private/System/LELogSubsystem.cpp` | 初始化流程调整，添加树打印 |
| 修改 | `Source/LogEverything/LogEverything.Build.cs` | 添加 Projects/Json/JsonUtilities 依赖 |
| 修改 | `CLAUDE.md` | 版本更新至 0.9.0，文档更新 |
