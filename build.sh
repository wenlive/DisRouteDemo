#!/bin/bash

# 创建构建目录
mkdir -p build

# 进入构建目录
cd build

# 运行 cmake，使用 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON 生成 compile_commands.json
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..

# 构建项目
cmake --build .

# 复制 compile_commands.json 到项目根目录（为了 clangd）
cp compile_commands.json ..

# 返回项目根目录
cd .. 