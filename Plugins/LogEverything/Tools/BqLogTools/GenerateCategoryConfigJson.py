#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
LogEverything Category Config JSON Generator

从 Config/LogEverythingCategories.txt 生成树状 JSON 配置模板。
如果 JSON 文件已存在，执行合并模式：保留现有 level/enabled 配置，
仅添加新分类、移除已删除分类。

用法：
    python GenerateCategoryConfigJson.py [--config CONFIG_FILE] [--output OUTPUT_FILE]

默认路径：
    CONFIG_FILE: ../../Config/LogEverythingCategories.txt
    OUTPUT_FILE: ../../Config/LogEverythingCategoryConfig.json
"""

import json
import os
import sys
import argparse


def parse_categories_txt(filepath):
    """解析 LogEverythingCategories.txt，返回路径列表"""
    paths = []
    with open(filepath, 'r', encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            # 忽略空行和注释
            if not line or line.startswith('#'):
                continue
            paths.append(line)
    return paths


def build_tree_from_paths(paths):
    """从扁平路径列表构建树状结构"""
    # 收集所有需要存在的路径（包括中间节点）
    all_paths = set()
    for path in paths:
        parts = path.split('.')
        for i in range(1, len(parts) + 1):
            all_paths.add('.'.join(parts[:i]))

    # 按路径排序确保父节点先处理
    sorted_paths = sorted(all_paths)

    # 构建树
    root_nodes = []
    node_map = {}  # path -> node dict

    for path in sorted_paths:
        parts = path.split('.')
        name = parts[-1]
        node = {"name": name, "level": "NotSet", "enabled": "NotSet", "children": []}

        if len(parts) == 1:
            # 根节点
            root_nodes.append(node)
        else:
            # 子节点，添加到父节点
            parent_path = '.'.join(parts[:-1])
            if parent_path in node_map:
                node_map[parent_path]["children"].append(node)

        node_map[path] = node

    return root_nodes


def merge_config(existing_node, new_node):
    """
    合并已有配置到新节点。
    保留已有节点的 level 和 enabled 字段，递归合并 children。
    """
    # 保留已有的 level 和 enabled 配置
    if "level" in existing_node:
        new_node["level"] = existing_node["level"]
    if "enabled" in existing_node:
        new_node["enabled"] = existing_node["enabled"]

    # 递归合并子节点
    existing_children = {child["name"]: child for child in existing_node.get("children", [])}
    for new_child in new_node.get("children", []):
        if new_child["name"] in existing_children:
            merge_config(existing_children[new_child["name"]], new_child)


def clean_tree(nodes):
    """清理树结构，移除空的 children 数组以保持 JSON 简洁"""
    for node in nodes:
        if node.get("children"):
            clean_tree(node["children"])
        else:
            # 移除空 children 数组
            node.pop("children", None)


def main():
    parser = argparse.ArgumentParser(description='Generate LogEverything Category Config JSON')
    parser.add_argument('--config', default=None, help='Path to LogEverythingCategories.txt')
    parser.add_argument('--output', default=None, help='Path to output JSON file')
    args = parser.parse_args()

    # 确定路径
    script_dir = os.path.dirname(os.path.abspath(__file__))
    plugin_root = os.path.normpath(os.path.join(script_dir, '..', '..'))

    config_file = args.config or os.path.join(plugin_root, 'Config', 'LogEverythingCategories.txt')
    output_file = args.output or os.path.join(plugin_root, 'Config', 'LogEverythingCategoryConfig.json')

    # 检查配置文件
    if not os.path.exists(config_file):
        print(f"ERROR: Config file not found: {config_file}")
        sys.exit(1)

    print(f"Reading categories from: {config_file}")

    # 解析分类路径
    paths = parse_categories_txt(config_file)
    print(f"Found {len(paths)} category paths")

    # 构建树
    new_tree = build_tree_from_paths(paths)

    # 如果输出文件已存在，执行合并
    if os.path.exists(output_file):
        print(f"Existing JSON found, performing merge: {output_file}")
        try:
            with open(output_file, 'r', encoding='utf-8') as f:
                existing_data = json.load(f)

            existing_categories = existing_data.get("categories", [])
            existing_map = {cat["name"]: cat for cat in existing_categories}

            for new_node in new_tree:
                if new_node["name"] in existing_map:
                    merge_config(existing_map[new_node["name"]], new_node)

            # 保留已有的 defaultLevel
            default_level = existing_data.get("defaultLevel", "Info")
        except (json.JSONDecodeError, KeyError) as e:
            print(f"WARNING: Failed to parse existing JSON ({e}), generating fresh file")
            default_level = "Info"
    else:
        default_level = "Info"

    # 清理空 children
    clean_tree(new_tree)

    # 构建输出
    output_data = {
        "version": "1.0",
        "description": "LogEverything Category Configuration",
        "defaultLevel": default_level,
        "categories": new_tree
    }

    # 确保输出目录存在
    os.makedirs(os.path.dirname(output_file), exist_ok=True)

    # 写入 JSON
    with open(output_file, 'w', encoding='utf-8') as f:
        json.dump(output_data, f, indent=2, ensure_ascii=False)

    print(f"SUCCESS: JSON config generated at: {output_file}")
    print(f"  Categories: {len(paths)}")
    print(f"  Default Level: {default_level}")


if __name__ == '__main__':
    main()
