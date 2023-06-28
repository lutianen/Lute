#!/bin/bash

# 格式化
clang-format -i `find include/Base -type f -name *.h`
clang-format -i `find src/ -type f -name *.cc`

# 统计代码行数
cloc --git `git branch --show-current`
