// BqLog 实现文件
// 此文件包含所有 BqLog 源码实现

// 定义 BQ_SRC 以启用 BqLog 实现
#define BQ_SRC

// Windows 平台特定定义
#ifdef _WIN32
#define BQ_WIN 1
#define BQ_WIN64 1

// 禁用一些 MSVC 警告，以便 BqLog 源码可以正常编译
#pragma warning(push)
#pragma warning(disable: 4996) // 'function': This function or variable may be unsafe
#pragma warning(disable: 4777) // format string warnings
#pragma warning(disable: 4800) // implicit conversion from 'type' to bool
#pragma warning(disable: 4005) // macro redefinition
#pragma warning(disable: 4819) // The file contains a character that cannot be represented
#pragma warning(disable: 4267) // 'var' : conversion from 'size_t' to 'type', possible loss of data
#pragma warning(disable: 4244) // 'argument' : conversion from 'type1' to 'type2', possible loss of data

// UE 使用 Unicode，但 BqLog 使用 ANSI，需要一些兼容处理
#undef UNICODE
#undef _UNICODE
#define MBCS
#endif

// 包含必要的 BqLog API 实现源文件
// 由于 UE 构建系统限制，我们需要在单个 cpp 文件中包含所有源文件

// API 层实现
#include "../BQLog/bq_log/api/bq_log_api.cpp"

// 日志核心实现
#include "../BQLog/bq_log/log/log_imp.cpp"
#include "../BQLog/bq_log/log/log_manager.cpp"
#include "../BQLog/bq_log/log/log_types.cpp"
#include "../BQLog/bq_log/log/log_level_bitmap.cpp"
#include "../BQLog/bq_log/log/log_snapshot.cpp"
#include "../BQLog/bq_log/log/log_worker.cpp"
#include "../BQLog/bq_log/log/layout.cpp"

// Appender 实现
#include "../BQLog/bq_log/log/appender/appender_base.cpp"
#include "../BQLog/bq_log/log/appender/appender_console.cpp"
#include "../BQLog/bq_log/log/appender/appender_file_base.cpp"
#include "../BQLog/bq_log/log/appender/appender_file_compressed.cpp"
#include "../BQLog/bq_log/log/appender/appender_file_raw.cpp"
#include "../BQLog/bq_log/log/appender/appender_file_text.cpp"
#include "../BQLog/bq_log/log/appender/appender_file_binary.cpp"

// Decoder 实现
#include "../BQLog/bq_log/log/decoder/appender_decoder_base.cpp"
#include "../BQLog/bq_log/log/decoder/appender_decoder_compressed.cpp"
#include "../BQLog/bq_log/log/decoder/appender_decoder_helper.cpp"
#include "../BQLog/bq_log/log/decoder/appender_decoder_manager.cpp"
#include "../BQLog/bq_log/log/decoder/appender_decoder_raw.cpp"

// 类型和工具
#include "../BQLog/bq_log/types/ring_buffer.cpp"
#include "../BQLog/bq_log/utils/log_utils.cpp"

// bq_common 通用实现
#include "../BQLog/bq_common/utils/file_manager.cpp"
#include "../BQLog/bq_common/utils/property.cpp"
#include "../BQLog/bq_common/utils/property_ex.cpp"
#include "../BQLog/bq_common/utils/util.cpp"
#include "../BQLog/bq_common/platform/no_lib_cpp_impl.cpp"

// bq_common 线程支持
#include "../BQLog/bq_common/platform/thread/spin_lock.cpp"
#include "../BQLog/bq_common/platform/thread/thread.cpp"

// Windows 平台特定文件
#ifdef _WIN32
#include "../BQLog/bq_common/platform/win64_misc.cpp"
#include "../BQLog/bq_common/platform/io/memory_map_win.cpp"
#include "../BQLog/bq_common/platform/thread/condition_variable_win64.cpp"
#include "../BQLog/bq_common/platform/thread/mutex_win64.cpp"
#include "../BQLog/bq_common/platform/thread/thread_win64.cpp"

#pragma warning(pop) // 恢复警告设置
#endif