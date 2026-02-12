# Multi-Language Documentation Image Reference Management

---
problem_type: documentation
components:
  - Tools/PerfReport/GeneratePerfReportMultiRound.py
  - docs/benchmark/PERFORMANCE_REPORT.md
  - docs/benchmark/PERFORMANCE_REPORT_CHN.md
  - README.md
  - README_CHN.md
symptoms:
  - Image references in Markdown reports point to non-existent files
  - English report overwritten with Chinese content by Python script
  - Image filenames lack language suffixes (_chn/_en)
keywords:
  - performance report
  - image reference
  - markdown
  - localization
  - bilingual documentation
  - matplotlib chart
  - Python script
severity: medium
resolved: 2026-02-12
version: v1.0.0
---

## Problem Summary

During the LogEverything v1.0.0 release, performance report documentation had multiple image reference issues that caused broken images in GitHub rendering.

## Symptoms

1. GitHub reported missing image files: `docs/benchmark/perf_rounds_trend.png`
2. English performance report (PERFORMANCE_REPORT.md) displayed Chinese content
3. Image links in documentation pointed to old filenames without language suffixes

## Root Cause Analysis

### Problem Chain

```
[Initial State]
  Image files: perf_throughput_comparison.png (no language suffix)
  Both EN/CHN documents reference the same image

      ↓ Added bilingual chart support

[First Modification]
  Chart generation functions added `lang` parameter
  Image files renamed: _chn.png, _en.png
  Document references updated to new filenames

      ↓ Re-generated charts (problem introduced)

[Problem Introduction]
  Ran Python script to regenerate charts
  generate_markdown_report() template still used old filenames
  Script output overwrote PERFORMANCE_REPORT.md (English replaced with Chinese)

      ↓ Problem detected

[Problem Manifestation]
  - English report PERFORMANCE_REPORT.md became Chinese
  - Image references pointed to non-existent old filenames
```

### Core Issue

The `generate_markdown_report()` function in `GeneratePerfReportMultiRound.py` was a hardcoded Chinese report generator with these defects:

1. **Single-language template**: Embedded Markdown template only had Chinese version
2. **Old image references**: Template image references not updated to use language suffixes
3. **Fixed output filename**: Output filename fixed as `PERFORMANCE_REPORT.md`, unable to distinguish language versions

```python
# Problematic code (before fix)
report += """
### 5.1 Throughput Comparison

![Throughput](perf_throughput_comparison.png)  # Old filename, no language suffix
"""

report_path = output_dir / 'PERFORMANCE_REPORT.md'  # Fixed filename
```

## Solution

### Step 1: Update Chart Generation Functions

Added `lang` parameter to all chart generation functions:

```python
def generate_throughput_chart(stats: list, output_dir: Path, rounds_count: int, lang: str = 'chn'):
    """Generate throughput comparison chart

    Args:
        lang: Language, 'chn' for Chinese, 'en' for English
    """
    texts = {
        'chn': {'scenarios': ['Baseline', 'Formatted', 'Multi-threaded'], ...},
        'en': {'scenarios': ['Baseline', 'Formatted', 'Multi-threaded'], ...}
    }

    suffix = '_chn' if lang == 'chn' else '_en'
    chart_path = output_dir / f'perf_throughput_comparison{suffix}.png'
```

### Step 2: Fix Markdown Template Image References

Updated `generate_markdown_report()` to use correct image filenames:

```diff
# Before
-![Throughput](perf_throughput_comparison.png)
-![Rounds Trend](perf_rounds_trend.png)
-![Filter Overhead](perf_filter_overhead.png)

# After
+![Throughput](perf_throughput_comparison_chn.png)
+![Rounds Trend](perf_rounds_trend_chn.png)
+![Filter Overhead](perf_filter_overhead_chn.png)
```

### Step 3: Restore English Document from Git History

```bash
# Restore English version from commit e01ab02
git checkout e01ab02 -- docs/benchmark/PERFORMANCE_REPORT.md
```

### Step 4: Sync All Branches

```bash
git checkout release/v1.0.0
git merge master --no-edit
git push origin release/v1.0.0
```

## Verification

### Check Image Files Exist

```bash
ls -la docs/benchmark/*.png
# Expected:
# perf_filter_overhead_chn.png
# perf_filter_overhead_en.png
# perf_rounds_trend_chn.png
# perf_rounds_trend_en.png
# perf_throughput_comparison_chn.png
# perf_throughput_comparison_en.png
```

### Check Document References Match Files

```bash
# English docs should reference _en.png
grep "\.png" docs/benchmark/PERFORMANCE_REPORT.md
# Expected: perf_throughput_comparison_en.png, etc.

# Chinese docs should reference _chn.png
grep "\.png" docs/benchmark/PERFORMANCE_REPORT_CHN.md
# Expected: perf_throughput_comparison_chn.png, etc.
```

## Prevention Strategies

### 1. Image Naming Convention

Adopt `{function}_{language}.{extension}` format:
- `perf_throughput_comparison_chn.png`
- `perf_throughput_comparison_en.png`

### 2. Script and Document Decoupling

- Manually maintained documents should not be overwritten by automated scripts
- Use distinct output filenames for generated reports (e.g., `GENERATED_REPORT.md`)

### 3. Pre-Release Verification Checklist

- [ ] All image files follow `{name}_{lang}.png` naming convention
- [ ] Chinese version images (`_chn`) all exist
- [ ] English version images (`_en`) all exist
- [ ] Markdown references match actual filenames
- [ ] Image alt text matches document language

### 4. Git Workflow Best Practices

- Check `git diff` immediately after running scripts
- Backup or commit files before running overwrite scripts
- Use `git restore` or `git checkout` to quickly recover files

## Related Commits

| Commit | Description |
|--------|-------------|
| c9f970e | Add bilingual chart support |
| ff998c8 | Restore English version document |
| ec72265 | Fix Python script image references |

## Related Files

- `Plugins/LogEverything/Tools/PerfReport/GeneratePerfReportMultiRound.py`
- `docs/benchmark/PERFORMANCE_REPORT.md`
- `docs/benchmark/PERFORMANCE_REPORT_CHN.md`
- `README.md`
- `README_CHN.md`

## Lessons Learned

1. **Bilingual reports should use separate functions or language parameters**
2. **Output filenames should include language identifiers**
3. **Automated scripts should not overwrite manually maintained content**
4. **Always verify git diff after running generation scripts**

---

*Documented on 2026-02-12 during LogEverything v1.0.0 release*
