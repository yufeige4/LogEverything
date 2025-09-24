# BqLog 代码生成工具

## 工具来源
本目录包含的 BqLog_CategoryLogGenerator 工具来自 BqLog 项目：
- **项目地址**: https://github.com/Tencent/BqLog
- **许可证**: Apache License 2.0
- **版本**: 1.5.0

## 平台支持
- **Windows**: BqLog_CategoryLogGenerator.exe
- **Linux 64位**: linux64/BqLog_CategoryLogGenerator
- **Mac**: mac/BqLog_CategoryLogGenerator

## 使用方法

### Windows
```cmd
GenerateLogEverythingCategories.bat
```

## 工具功能
BqLog_CategoryLogGenerator 用于根据类别配置文件生成静态的类别访问代码，支持：
- 层级化类别定义 (如 Game.Combat.Skill)
- 静态类型安全的类别访问
- 完整的IDE代码提示支持

## 配置文件格式
类别配置文件 (Config/LogEverythingCategories.txt) 格式：
```
# 注释行以 # 开始
Game.Combat.Skill
Game.Combat.Input
Game.AI.BehaviorTree
Editor.Tools.Blueprint
```

## 生成输出
工具会在 Generated/ 目录生成 LogEverythingLogger.h 文件，包含：
- LogEverythingLogger 类定义
- 层级化的类别结构
- 静态 constexpr 类别句柄

## 许可证声明
BqLog_CategoryLogGenerator 工具遵循 Apache License 2.0 许可证。
完整许可证文本请参考 BqLog 项目的 LICENSE 文件。

Copyright (C) 2024 Tencent.
BQLOG is licensed under the Apache License, Version 2.0.