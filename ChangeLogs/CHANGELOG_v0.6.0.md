# LogEverything Plugin v0.6.0 版本更新日志

## 📦 版本信息
- **版本号**: v0.6.0 (从 v0.5 升级)
- **主要更新**: 高性能架构重构与 BqLog 模板接口深度集成

## 🚀 重大架构升级

### 🔥 核心性能革命
本版本实现了 LogEverything 插件的根本性性能重构：

- **消除字符串预格式化瓶颈**: 不再使用 `FString::Printf` 进行字符串预处理
- **直接调用 BqLog 模板接口**: 实现 `LogEverythingLogger::XXX(category, format_string, args...)` 零拷贝调用
- **模板元编程优化**: 利用 C++ 模板在编译时进行类型推导和优化

### ⚡ 与 UE_LOG 的对比优势

#### 🎯 格式化系统对比

| 特性 | UE_LOG | LogEverything v0.6.0 |
|------|---------|---------------------|
| **格式化语法** | `%s`, `%d`, `%f` (printf 风格) | `{}`, `{.2f}` (BqLog 现代风格) |
| **类型安全** | 运行时检查 | 编译时模板检查 |
| **性能开销** | 字符串预格式化 | 零拷贝直接传递 |
| **UE 类型支持** | 需要手动转换 `*FString` | 直接支持 `FString`, `FName`, `FText` |
| **精度控制** | `%.2f` | `{.2f}` (更直观) |

#### 📝 使用对比示例

```cpp
// UE_LOG 传统方式
FString PlayerName = TEXT("TestPlayer");
int32 Health = 75;
float DamageMultiplier = 1.5f;
UE_LOG(LogTemp, Warning, TEXT("Player %s health: %d, multiplier: %.2f"),
       *PlayerName, Health, DamageMultiplier);

// LogEverything v0.6.0 现代方式
FString PlayerName = TEXT("TestPlayer");
int32 Health = 75;
float DamageMultiplier = 1.5f;
LE_LOG(LogGameCombat, Warning, TEXT("Player {} health: {}, multiplier: {.2f}"),
       PlayerName, Health, DamageMultiplier);  // 注意：无需 *PlayerName
```

#### 🔧 核心技术优势

1. **零拷贝参数传递**
   - UE_LOG: 需要 `*FString` 解引用，创建临时 C 字符串
   - LogEverything: 直接传递 FString 对象，BqLog 内置适配器处理

2. **编译时类型安全**
   - UE_LOG: printf 风格，类型不匹配在运行时可能导致崩溃
   - LogEverything: 模板参数在编译时验证，类型错误编译失败

3. **现代化格式控制**
   - UE_LOG: `%.2f` 传统格式控制
   - LogEverything: `{.2f}` 更清晰的 BqLog 格式规范

## 🆕 新功能特性

### 🎯 现代化格式化系统
```cpp
// 基础参数化
LE_LOG(LogGameCombat, Log, TEXT("Player {} dealt {} damage"), PlayerName, Damage);

// 精确浮点数控制
LE_LOG(LogPerformance, Verbose, TEXT("Timings: {.1f}ms, {.3f}ms, {.6f}ms"),
       Time1, Time2, Time3);

// 布尔值显示
bool bCriticalHit = true;
LE_LOG(LogGameCombat, Log, TEXT("Critical hit: {}"), bCriticalHit);  // 输出: "TRUE"

// 混合类型支持
FName SkillName = FName(TEXT("FireBall"));
FText Description = FText::FromString(TEXT("魔法技能"));
LE_LOG(LogGameSkill, Log, TEXT("Skill: {}, Description: {}"), SkillName, Description);
```

### 🛠️ 高性能模板接口
新增核心模板方法，实现零拷贝日志记录：
```cpp
template<typename CategoryType, typename FormatType, typename... Args>
void LogWithTemplate(const CategoryType& Category, ELogVerbosity::Type Level,
                     const FormatType& Format, const Args&... Arguments);
```

### 🔧 Unreal Engine 类型原生支持
- **FString**: 直接支持，无需 `*` 解引用
- **FName**: 自动转换为字符串表示
- **FText**: 完美支持本地化文本，自动处理编码
- **基础类型**: int32, float, double, bool 等完全支持

## 📝 API 增强

### ✅ 完全向后兼容
所有现有的 LE_LOG 宏调用无需修改：
```cpp
// 这些调用方式完全不变，自动获得性能提升
LE_LOG(LogGameAI, Warning, TEXT("AI behavior failed"));
LE_CLOG(Health < 20, LogCombat, Error, TEXT("Critical health!"));
```

### 🎛️ 增强的便利宏
```cpp
LE_LOG_FATAL(Category, Format, ...)    // 致命错误级别
LE_LOG_ERROR(Category, Format, ...)    // 错误级别
LE_LOG_WARNING(Category, Format, ...)  // 警告级别
LE_LOG_INFO(Category, Format, ...)     // 信息级别
LE_LOG_VERBOSE(Category, Format, ...)  // 详细级别
```

### 🔍 改进的条件与断言宏
```cpp
// 条件日志 - 高效的条件检查
LE_CLOG(PlayerHealth < 20, LogGameCombat, Warning,
        TEXT("Player {} health critical: {}"), PlayerName, PlayerHealth);

// 断言日志 - 表达式失败时记录详细信息
LE_CHECK(IsValid(WeaponObject), LogGameWeapon, Error,
         TEXT("Weapon validation failed: {}"), WeaponObject->GetName());

// 确保日志 - 类似断言但继续执行
LE_ENSURE(ConfigFile != nullptr, LogGameConfig, Warning,
          TEXT("Config file missing, using defaults"));
```

## 🏗️ 内部架构优化

### 📊 核心组件重构
1. **LELogMacros.h**: 完全重写宏系统，实现高性能模板化调用
2. **LEBqLogBridge.h/.cpp**: 新增 `LogWithTemplate` 方法，直接桥接 BqLog 模板接口
3. **分类系统保持**: 保留原有的 Category 对象设计，为未来 CheckCanLog 功能预留

### 🔐 类型安全性增强
- **编译时类型检查**: 所有模板参数在编译时验证
- **自动类型推导**: 支持各种 UE 和标准 C++ 类型
- **SFINAE 友好**: 优雅处理不支持的参数类型

### 🎯 分类系统维持
- **保留级别检查**: `ShouldLogCategory` 机制继续工作
- **层级化分类**: 完整支持 `Game.Combat.Skill` 风格的分类
- **运行时配置**: 级别控制和分类开关功能保持不变

## 🧪 测试验证

### ✅ 功能测试通过
测试日志输出示例（来自实际运行结果）：
```
[I][Test.System.Basic] 测试现代化参数化消息: 数值=42, 字符串=测试字符串, 浮点=3.14, 布尔=TRUE
[I][Test.System.Basic] 精度测试: 123.4, 123.456, 123.456789
[W][Test.System.Basic] 玩家 TestPlayer 生命值过低: 15
[I][Test.System.Basic] FName 测试: TestCategory
[I][Game.Combat.Skill] 技能系统日志测试
[I][Engine.Core.Memory] 内存分配跟踪
```

### 📈 验证覆盖
- ✅ **现代化格式化**: `{.2f}` 精度控制正确工作
- ✅ **UE 类型支持**: FString/FName/FText 无需转换
- ✅ **条件日志**: `LE_CLOG` 条件触发机制正常
- ✅ **便利宏**: 所有日志级别正确输出
- ✅ **分类系统**: 层级化分类正确显示
- ✅ **布尔值显示**: 自动转换为 "TRUE"/"FALSE"

## 🔧 技术栈要求

### 📚 依赖项
- **BqLog**: v1.5.0+ (高性能 C++ 日志库)
- **Unreal Engine**: 4.27+ / 5.0+ (完全兼容测试通过)
- **编译器**: MSVC 2019+, Clang 10+, GCC 9+

### 🏛️ 架构特性
- **模板元编程**: 编译时类型处理和优化
- **零拷贝设计**: 避免不必要的内存分配和字符串拷贝
- **RAII 资源管理**: 自动化资源生命周期管理

## 🚀 迁移指南

### 从旧版本升级

#### 1. 推荐的现代化改进
```cpp

// 新格式 (推荐，更高性能)
LE_LOG(LogTest, Log, TEXT("Player: {}, Score: {}, Ratio: {.2f}"), PlayerName, Score, Ratio);
```

#### 3. UE 类型使用简化
```cpp
// 旧方式 - 需要手动转换
FString Message = TEXT("Hello");
FName CategoryName = FName(TEXT("Test"));
LE_LOG(LogTest, Log, TEXT("Message: %s, Category: %s"), *Message, *CategoryName.ToString());

// 新方式 - 直接传递
FString Message = TEXT("Hello");
FName CategoryName = FName(TEXT("Test"));
LE_LOG(LogTest, Log, TEXT("Message: {}, Category: {}"), Message, CategoryName);
```

### 🎯 最佳实践建议
1. **优先使用 `{}` 占位符**: 更安全、更高性能
2. **直接传递 UE 类型**: 无需手动解引用或转换
3. **利用精度控制**: 使用 `{.2f}` 等格式化浮点数
4. **使用便利宏**: `LE_LOG_INFO`, `LE_LOG_WARNING` 等简化常见场景

## 🐛 已修复问题

- ✅ **编译警告 C4840**: 修复可变参数函数的类型传递问题
- ✅ **性能瓶颈**: 消除 FString::Printf 字符串预格式化开销
- ✅ **类型转换复杂性**: 简化 UE 类型的日志记录方式
- ✅ **内存分配开销**: 实现真正的零拷贝参数传递

## 🔮 技术债务清理

- 🔄 **接口设计保留**: Category 对象设计为未来 CheckCanLog 功能保留
- 🎛️ **级别检查机制**: ShouldLogCategory 逻辑完整保持
- 📊 **扩展性**: 模板化设计为未来功能扩展提供基础

## 📊 实际使用效果

### 开发体验提升
1. **编译时错误检查**: 参数类型不匹配在编译时发现
2. **代码简化**: 无需手动处理 UE 类型转换
3. **格式化直观**: `{.2f}` 比 `%.2f` 更清晰易读

### 运行时优势
1. **减少字符串分配**: 直接传递对象给 BqLog
2. **更好的缓存局部性**: 避免临时字符串创建
3. **类型安全**: 编译时验证避免运行时错误

---

**LogEverything v0.6.0** 通过深度的架构重构，在保持完全向后兼容的同时，实现了显著的性能提升和开发体验改进。这次更新为 Unreal Engine 项目提供了现代化、高性能的日志解决方案，同时为未来功能扩展奠定了坚实基础。