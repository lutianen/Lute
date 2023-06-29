#!/bin/bash

# 格式化
clang-format -i `find Base/include/Base -type f -name *.h`
clang-format -i `find Base/src/ -type f -name *.cc`

clang-format -i `find polaris/include/polaris -type f -name *.h`
clang-format -i `find polaris/src/ -type f -name *.cc`

# 统计代码行数
cloc --git `git branch --show-current`
