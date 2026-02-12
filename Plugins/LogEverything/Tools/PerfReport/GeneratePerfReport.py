#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
LogEverything 性能报告生成器
从 perf_results.csv 读取测试数据，生成图表和 Markdown 报告
"""

import csv
import sys
from pathlib import Path
from datetime import datetime
import argparse

# 尝试导入可视化库（可选）
try:
    import matplotlib.pyplot as plt
    import matplotlib
    HAS_MATPLOTLIB = True
except ImportError:
    HAS_MATPLOTLIB = False
    print("[警告] matplotlib 未安装，将跳过图表生成")


def setup_chinese_font():
    """配置中文字体支持"""
    if not HAS_MATPLOTLIB:
        return
    plt.rcParams['font.sans-serif'] = ['SimHei', 'Microsoft YaHei', 'Arial Unicode MS', 'DejaVu Sans']
    plt.rcParams['axes.unicode_minus'] = False


def load_csv(csv_path: Path) -> list:
    """加载 CSV 数据"""
    if not csv_path.exists():
        print(f"[错误] CSV 文件不存在: {csv_path}")
        sys.exit(1)

    results = []
    with open(csv_path, 'r', encoding='utf-8') as f:
        reader = csv.DictReader(f)
        for row in reader:
            results.append({
                'TestCase': row['TestCase'],
                'LogCount': int(row['LogCount']),
                'TotalTimeMs': float(row['TotalTimeMs']),
                'AvgTimeNs': float(row['AvgTimeNs']),
                'Throughput': float(row['Throughput']),
                'Platform': row.get('Platform', 'Unknown'),
                'BuildConfig': row.get('BuildConfig', 'Unknown')
            })

    print(f"[信息] 从 {csv_path} 加载了 {len(results)} 条测试结果")
    return results


def format_number(num):
    """格式化数字"""
    if num >= 1000000:
        return f"{num/1000000:.2f}M"
    elif num >= 1000:
        return f"{num/1000:.1f}K"
    else:
        return f"{num:.0f}"


def generate_comparison_chart(results: list, output_dir: Path):
    """生成 UE_LOG vs LE_LOG 对比图表"""
    if not HAS_MATPLOTLIB:
        return None, None, None, None

    # 提取对比数据
    pairs = [
        ('PERF-01', 'PERF-02', '基准测试'),
        ('PERF-03', 'PERF-04', '格式化测试'),
        ('PERF-07', 'PERF-08', '多线程测试')
    ]

    scenarios = []
    ue_throughputs = []
    le_throughputs = []
    improvements = []

    for ue_id, le_id, name in pairs:
        ue_row = next((r for r in results if ue_id in r['TestCase']), None)
        le_row = next((r for r in results if le_id in r['TestCase']), None)

        if ue_row and le_row:
            scenarios.append(name)
            ue_throughputs.append(ue_row['Throughput'])
            le_throughputs.append(le_row['Throughput'])
            # 计算性能提升百分比
            imp = ((le_row['Throughput'] - ue_row['Throughput']) / ue_row['Throughput']) * 100
            improvements.append(imp)

    if not scenarios:
        print("[警告] 未找到对比测试数据")
        return None, None, None, None

    # 创建图表
    fig, axes = plt.subplots(1, 2, figsize=(14, 6))

    x = range(len(scenarios))
    width = 0.35

    # 左图: 吞吐量对比
    ax1 = axes[0]
    bars1 = ax1.bar([i - width/2 for i in x], [t/1000000 for t in ue_throughputs], width,
                    label='UE_LOG', color='#4A90D9')
    bars2 = ax1.bar([i + width/2 for i in x], [t/1000000 for t in le_throughputs], width,
                    label='LE_LOG', color='#50C878')

    ax1.set_ylabel('吞吐量 (百万条/秒)')
    ax1.set_xlabel('测试场景')
    ax1.set_title('UE_LOG vs LE_LOG: 吞吐量对比 (100万条日志)')
    ax1.set_xticks(x)
    ax1.set_xticklabels(scenarios)
    ax1.legend()
    ax1.grid(axis='y', linestyle='--', alpha=0.7)

    # 添加数值标签
    for bar, val in zip(bars1, ue_throughputs):
        ax1.text(bar.get_x() + bar.get_width()/2, bar.get_height(),
                 f'{val/1000000:.2f}M', ha='center', va='bottom', fontsize=9)
    for bar, val in zip(bars2, le_throughputs):
        ax1.text(bar.get_x() + bar.get_width()/2, bar.get_height(),
                 f'{val/1000000:.2f}M', ha='center', va='bottom', fontsize=9)

    # 右图: 性能提升百分比
    ax2 = axes[1]
    colors = ['#FF6B6B', '#4ECDC4', '#45B7D1']
    bars3 = ax2.bar(scenarios, improvements, color=colors[:len(scenarios)])

    ax2.set_ylabel('性能提升 (%)')
    ax2.set_xlabel('测试场景')
    ax2.set_title('LE_LOG 相对于 UE_LOG 的性能提升')
    ax2.grid(axis='y', linestyle='--', alpha=0.7)

    # 添加数值标签
    for bar, val in zip(bars3, improvements):
        ax2.text(bar.get_x() + bar.get_width()/2, bar.get_height(),
                 f'+{val:.0f}%', ha='center', va='bottom', fontsize=12, fontweight='bold')

    plt.tight_layout()

    chart_path = output_dir / 'perf_comparison.png'
    fig.savefig(chart_path, dpi=150, bbox_inches='tight')
    plt.close(fig)
    print(f"[信息] 图表已保存: {chart_path}")

    return scenarios, ue_throughputs, le_throughputs, improvements


def generate_filter_chart(results: list, output_dir: Path):
    """生成过滤开销图表"""
    if not HAS_MATPLOTLIB:
        return

    filter_row = next((r for r in results if 'PERF-05' in r['TestCase']), None)
    disable_row = next((r for r in results if 'PERF-06' in r['TestCase']), None)

    if not filter_row or not disable_row:
        print("[警告] 未找到过滤测试数据，跳过过滤开销图表")
        return

    fig, ax = plt.subplots(figsize=(8, 5))

    names = ['级别过滤', '分类禁用']
    throughputs = [filter_row['Throughput']/1000000, disable_row['Throughput']/1000000]
    latencies = [filter_row['AvgTimeNs'], disable_row['AvgTimeNs']]

    bars = ax.bar(names, throughputs, color=['#9B59B6', '#E74C3C'])
    ax.set_ylabel('吞吐量 (百万次/秒)')
    ax.set_title('LE_LOG 过滤开销测试 (100万次检查)')
    ax.grid(axis='y', linestyle='--', alpha=0.7)

    for bar, tp, lat in zip(bars, throughputs, latencies):
        ax.text(bar.get_x() + bar.get_width()/2, bar.get_height(),
                f'{tp:.1f}M/秒\n({lat:.0f} 纳秒/次)',
                ha='center', va='bottom', fontsize=10)

    plt.tight_layout()

    chart_path = output_dir / 'perf_filter_overhead.png'
    fig.savefig(chart_path, dpi=150, bbox_inches='tight')
    plt.close(fig)
    print(f"[信息] 图表已保存: {chart_path}")


def generate_bar(value, max_value, width=40):
    """生成文本进度条"""
    if max_value == 0:
        return " " * width
    filled = int((value / max_value) * width)
    return "#" * filled + "-" * (width - filled)


def generate_markdown_report(results: list, output_dir: Path,
                             scenarios=None, ue_throughputs=None,
                             le_throughputs=None, improvements=None):
    """生成 Markdown 性能报告"""

    timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    platform = results[0]['Platform'] if results else 'Unknown'
    build_config = results[0]['BuildConfig'] if results else 'Unknown'
    log_count = results[0]['LogCount'] if results else 0

    # 提取测试数据
    ue_baseline = next((r for r in results if 'PERF-01' in r['TestCase']), None)
    le_baseline = next((r for r in results if 'PERF-02' in r['TestCase']), None)
    ue_format = next((r for r in results if 'PERF-03' in r['TestCase']), None)
    le_format = next((r for r in results if 'PERF-04' in r['TestCase']), None)
    le_filter = next((r for r in results if 'PERF-05' in r['TestCase']), None)
    le_disabled = next((r for r in results if 'PERF-06' in r['TestCase']), None)
    ue_mt = next((r for r in results if 'PERF-07' in r['TestCase']), None)
    le_mt = next((r for r in results if 'PERF-08' in r['TestCase']), None)
    le_latency = next((r for r in results if 'PERF-10' in r['TestCase']), None)

    max_throughput = max(r['Throughput'] for r in results)

    report = f"""# LogEverything 性能测试报告

**生成时间**: {timestamp}
**测试平台**: {platform} / {build_config}
**日志数量**: 每个测试 {log_count:,} 条

---

## 性能对比: LE_LOG vs UE_LOG

| 测试场景 | UE_LOG | LE_LOG | 性能提升 |
|----------|--------|--------|----------|
"""

    if ue_baseline and le_baseline:
        imp = ((le_baseline['Throughput'] - ue_baseline['Throughput']) / ue_baseline['Throughput']) * 100
        report += f"| **基准测试** | {format_number(ue_baseline['Throughput'])}/秒 | {format_number(le_baseline['Throughput'])}/秒 | **+{imp:.0f}%** |\n"

    if ue_format and le_format:
        imp = ((le_format['Throughput'] - ue_format['Throughput']) / ue_format['Throughput']) * 100
        report += f"| **格式化测试** | {format_number(ue_format['Throughput'])}/秒 | {format_number(le_format['Throughput'])}/秒 | **+{imp:.0f}%** |\n"

    if ue_mt and le_mt:
        imp = ((le_mt['Throughput'] - ue_mt['Throughput']) / ue_mt['Throughput']) * 100
        report += f"| **多线程测试** | {format_number(ue_mt['Throughput'])}/秒 | {format_number(le_mt['Throughput'])}/秒 | **+{imp:.0f}%** |\n"

    report += """
---

## 详细测试结果

| 测试用例 | 总耗时 | 平均延迟 | 吞吐量 |
|----------|--------|----------|--------|
"""

    test_name_map = {
        'PERF-01': 'PERF-01 UE_LOG 基准测试',
        'PERF-02': 'PERF-02 LE_LOG 基准测试',
        'PERF-03': 'PERF-03 UE_LOG 格式化测试',
        'PERF-04': 'PERF-04 LE_LOG 格式化测试',
        'PERF-05': 'PERF-05 LE_LOG 级别过滤',
        'PERF-06': 'PERF-06 LE_LOG 分类禁用',
        'PERF-07': 'PERF-07 UE_LOG 多线程测试',
        'PERF-08': 'PERF-08 LE_LOG 多线程测试',
        'PERF-09': 'PERF-09 LE_LOG Trace分析',
        'PERF-10': 'PERF-10 LE_LOG 延迟分布',
    }

    for r in results:
        # 查找中文名称
        test_name = r['TestCase']
        for key, cn_name in test_name_map.items():
            if key in test_name:
                test_name = cn_name
                break
        report += f"| {test_name} | {r['TotalTimeMs']:.2f} 毫秒 | {r['AvgTimeNs']:.0f} 纳秒 | {r['Throughput']:,.0f}/秒 |\n"

    report += """
---

## 吞吐量可视化

```
吞吐量 (条/秒)
"""
    report += "=" * 60 + "\n\n"

    # 基准测试对比
    if ue_baseline and le_baseline:
        imp = ((le_baseline['Throughput'] - ue_baseline['Throughput']) / ue_baseline['Throughput']) * 100
        report += "基准测试:\n"
        report += f"  UE_LOG  {generate_bar(ue_baseline['Throughput'], max_throughput)}  {format_number(ue_baseline['Throughput'])}/秒\n"
        report += f"  LE_LOG  {generate_bar(le_baseline['Throughput'], max_throughput)}  {format_number(le_baseline['Throughput'])}/秒  (+{imp:.0f}%)\n\n"

    # 格式化测试对比
    if ue_format and le_format:
        imp = ((le_format['Throughput'] - ue_format['Throughput']) / ue_format['Throughput']) * 100
        report += "格式化测试:\n"
        report += f"  UE_LOG  {generate_bar(ue_format['Throughput'], max_throughput)}  {format_number(ue_format['Throughput'])}/秒\n"
        report += f"  LE_LOG  {generate_bar(le_format['Throughput'], max_throughput)}  {format_number(le_format['Throughput'])}/秒  (+{imp:.0f}%)\n\n"

    # 多线程测试对比
    if ue_mt and le_mt:
        imp = ((le_mt['Throughput'] - ue_mt['Throughput']) / ue_mt['Throughput']) * 100
        report += "多线程测试 (4线程):\n"
        report += f"  UE_LOG  {generate_bar(ue_mt['Throughput'], max_throughput)}  {format_number(ue_mt['Throughput'])}/秒\n"
        report += f"  LE_LOG  {generate_bar(le_mt['Throughput'], max_throughput)}  {format_number(le_mt['Throughput'])}/秒  (+{imp:.0f}%)\n\n"

    # 过滤测试
    if le_filter or le_disabled:
        report += "LE_LOG 过滤/禁用 (无实际输出):\n"
        if le_filter:
            report += f"  级别过滤  {generate_bar(le_filter['Throughput'], max_throughput)}  {format_number(le_filter['Throughput'])}/秒\n"
        if le_disabled:
            report += f"  分类禁用  {generate_bar(le_disabled['Throughput'], max_throughput)}  {format_number(le_disabled['Throughput'])}/秒\n"

    report += """```

---

## 关键结论

"""

    if ue_baseline and le_baseline:
        imp = ((le_baseline['Throughput'] - ue_baseline['Throughput']) / ue_baseline['Throughput']) * 100
        report += f"1. **LE_LOG 基准性能超越 UE_LOG {imp:.0f}%**\n"

    if ue_format and le_format:
        imp = ((le_format['Throughput'] - ue_format['Throughput']) / ue_format['Throughput']) * 100
        report += f"2. **格式化字符串性能**: LE_LOG 快 {imp:.0f}%\n"

    if le_filter:
        report += f"3. **过滤开销极低**: 每条被过滤的日志仅需 {le_filter['AvgTimeNs']:.0f} 纳秒\n"

    if le_disabled:
        report += f"4. **分类禁用开销**: 每次检查仅需 {le_disabled['AvgTimeNs']:.0f} 纳秒\n"

    if ue_mt and le_mt:
        imp = ((le_mt['Throughput'] - ue_mt['Throughput']) / ue_mt['Throughput']) * 100
        report += f"5. **多线程扩展性**: 4线程场景下 LE_LOG 快 {imp:.0f}%\n"

    report += """
---

## 推荐配置

| 使用场景 | 缓冲区大小 | 可靠性级别 | 说明 |
|----------|------------|------------|------|
| **高性能** | 1 MB | Low | 极端负载下可能丢弃日志 |
| **平衡** | 16 MB | Normal | 不丢日志，大缓冲区避免阻塞 |
| **调试/关键** | 1 MB | High | 实时落盘，较慢但可靠 |

---

## 延迟分布分析 (PERF-10)

运行 PERF-10 测试可获取详细的延迟分布数据:
- P50/P90/P95/P99/P99.9 百分位延迟
- 延迟尖峰检测 (>1微秒, >10微秒, >100微秒, >1毫秒, >10毫秒)
- 阻塞检测，用于验证缓冲区配置是否合理

---

*由 LogEverything 性能报告生成器自动生成*
"""

    report_path = output_dir / 'PERFORMANCE_REPORT.md'
    report_path.write_text(report, encoding='utf-8')
    print(f"[信息] 报告已保存: {report_path}")


def main():
    parser = argparse.ArgumentParser(
        description='LogEverything 性能报告生成器')
    parser.add_argument(
        '--input', '-i',
        default='../../../../../../Saved/LogEverything/PerfReport/perf_results.csv',
        help='输入 CSV 文件路径')
    parser.add_argument(
        '--output', '-o',
        default='../../../../../../Saved/LogEverything/PerfReport',
        help='输出目录')

    args = parser.parse_args()

    # 解析路径
    script_dir = Path(__file__).parent
    csv_path = Path(args.input)
    if not csv_path.is_absolute():
        csv_path = script_dir / csv_path

    output_dir = Path(args.output)
    if not output_dir.is_absolute():
        output_dir = script_dir / output_dir

    output_dir.mkdir(parents=True, exist_ok=True)

    print(f"[信息] LogEverything 性能报告生成器")
    print(f"[信息] 输入 CSV: {csv_path}")
    print(f"[信息] 输出目录: {output_dir}")
    print()

    # 配置中文字体
    if HAS_MATPLOTLIB:
        setup_chinese_font()

    # 加载数据
    results = load_csv(csv_path)

    # 生成图表
    scenarios, ue_tp, le_tp, improvements = generate_comparison_chart(results, output_dir)
    generate_filter_chart(results, output_dir)

    # 生成报告
    generate_markdown_report(results, output_dir, scenarios, ue_tp, le_tp, improvements)

    print()
    print(f"[成功] 报告生成完成!")
    print(f"  - 报告: {output_dir / 'PERFORMANCE_REPORT.md'}")
    if HAS_MATPLOTLIB:
        print(f"  - 图表: {output_dir / 'perf_comparison.png'}")
        print(f"  - 图表: {output_dir / 'perf_filter_overhead.png'}")


if __name__ == '__main__':
    main()
