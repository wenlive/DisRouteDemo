cmake_minimum_required(VERSION 3.10)
project(router_demo)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # 生成 compile_commands.json

# 添加编译选项
add_compile_options(-Wall -Wextra)

# 指定头文件目录
include_directories(${PROJECT_SOURCE_DIR}/include)

# 获取源文件
file(GLOB SOURCES "src/*.cpp")

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES}) 