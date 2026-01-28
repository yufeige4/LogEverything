# LogEverything Category Config JSON

本文档说明 `LogEverythingCategoryConfig.json` 的格式和使用方法。

## 文件位置

```
LogEverything/Config/LogEverythingCategoryConfig.json
```

## 生成方式

运行 `Tools/BqLogTools/GenerateLogEverythingCategories.bat`，会自动从 `Config/LogEverythingCategories.txt` 生成此 JSON 文件。

如果 JSON 文件已存在，脚本执行**合并模式**：保留已有的 `level` 和 `enabled` 配置，仅更新分类树结构（添加新分类、移除已删除分类）。

## JSON 结构

```json
{
  "version": "1.0",
  "description": "LogEverything Category Configuration",
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
            {
              "name": "Damage",
              "level": "Warning",
              "enabled": "NotSet"
            }
          ]
        }
      ]
    }
  ]
}
```

## 字段说明

### 根级别字段

| 字段 | 类型 | 必填 | 说明 |
|------|------|------|------|
| `version` | string | 是 | 配置文件版本号，当前为 `"1.0"` |
| `description` | string | 否 | 配置文件描述 |
| `defaultLevel` | string | 否 | 全局默认日志级别，缺省为 `"Info"` |
| `categories` | array | 是 | 顶级分类节点数组 |

### 分类节点字段

每个分类节点是一个 JSON 对象，包含以下字段：

| 字段 | 类型 | 必填 | 说明 |
|------|------|------|------|
| `name` | string | 是 | 分类子名称（不含父路径），如 `"Combat"` |
| `level` | string | 否 | 日志级别，`"NotSet"` 表示继承父节点 |
| `enabled` | string | 否 | 启用状态，`"NotSet"` 表示继承父节点 |
| `children` | array | 否 | 子分类节点数组，叶节点无此字段 |

## level 字段可选值

| 值 | 说明 |
|------|------|
| `"NotSet"` | 未设置，继承父节点的日志级别（默认值） |
| `"Verbose"` | 最详细级别，输出所有日志 |
| `"Debug"` | 调试级别，输出调试及以上日志 |
| `"Info"` | 信息级别，输出一般信息及以上日志 |
| `"Warning"` | 警告级别，仅输出警告及以上日志 |
| `"Error"` | 错误级别，仅输出错误和致命日志 |
| `"Fatal"` | 致命级别，仅输出致命错误日志 |

级别从低到高：`Verbose < Debug < Info < Warning < Error < Fatal`

设置某级别后，只有该级别及以上的日志才会输出。例如设置为 `"Warning"` 后，仅 Warning、Error、Fatal 会输出。

## enabled 字段可选值

| 值 | 说明 |
|------|------|
| `"NotSet"` | 未设置，继承父节点的启用状态（默认值） |
| `"Enabled"` | 显式启用，但父节点禁用时仍会被强制禁用 |
| `"Disabled"` | 显式禁用，即使父节点启用也保持禁用 |

### 启用状态继承规则

1. 父节点禁用时，所有子节点强制禁用（无论子节点如何设置）
2. 父节点启用时，子节点的实际状态取决于自身 `enabled` 值：
   - `"NotSet"`: 继承父节点状态
   - `"Disabled"`: 禁用
   - `"Enabled"`: 启用

## 配置示例

### 只关注战斗日志

将 Game 分类下的 Combat 设为 Debug 级别，其他保持默认：

```json
{
  "name": "Game",
  "level": "NotSet",
  "enabled": "NotSet",
  "children": [
    {
      "name": "Combat",
      "level": "Debug",
      "enabled": "NotSet",
      "children": [
        { "name": "Damage", "level": "NotSet", "enabled": "NotSet" },
        { "name": "Skill", "level": "NotSet", "enabled": "NotSet" },
        { "name": "Input", "level": "NotSet", "enabled": "NotSet" }
      ]
    }
  ]
}
```

### 禁用 AI 日志

```json
{
  "name": "AI",
  "level": "NotSet",
  "enabled": "Disabled",
  "children": [
    { "name": "BehaviorTree", "level": "NotSet", "enabled": "NotSet" },
    { "name": "Pathfinding", "level": "NotSet", "enabled": "NotSet" }
  ]
}
```

AI 及其所有子分类（BehaviorTree、Pathfinding）的日志都不会输出。

### 只输出错误和警告

将 `defaultLevel` 设为 `"Warning"`：

```json
{
  "defaultLevel": "Warning"
}
```

所有未显式设置级别的分类都将只输出 Warning 及以上日志。

## 调试

在 UE 控制台中执行以下命令，可在初始化时打印完整分类树：

```
LogEverything.Debug.LogCategory 1
```

输出包含每个节点的路径、有效级别和启用状态。
