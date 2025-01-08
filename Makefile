# 编译器和编译选项
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra

# 项目结构
INC_DIR = include
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# 包含路径
INCLUDES = -I$(INC_DIR)

# 最终可执行文件
TARGET = $(BIN_DIR)/router_demo

# 源文件和目标文件
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# 默认目标
.PHONY: all clean

all: $(TARGET)

# 确保输出目录存在
$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CXX) $(OBJS) -o $@

# 编译规则
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# 创建必要的目录
$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

# 清理
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# 运行程序
.PHONY: run
run: $(TARGET)
	./$(TARGET) 