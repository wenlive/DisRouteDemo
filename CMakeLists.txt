cmake_minimum_required(VERSION 3.10)
project(router_demo)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加编译选项
add_compile_options(-Wall -Wextra)

# 指定头文件目录
include_directories(${PROJECT_SOURCE_DIR}/include)

# 获取所有源文件
file(GLOB SOURCES 
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES}) 