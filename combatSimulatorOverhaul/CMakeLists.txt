cmake_minimum_required(VERSION 3.10)
project(CombatSimulator)

set(CMAKE_CXX_STANDARD 17)

# 查找第三方库
find_package(nlohmann_json 3.9.1 REQUIRED PATHS "lib")

# 添加头文件路径
include_directories(include)

# 添加所有源文件
file(GLOB SOURCES "src/*.cc")

# 添加可执行文件
add_executable(combatsimulator ${SOURCES} test/test.cc)

# 链接库
target_link_libraries(combatsimulator PRIVATE nlohmann_json::nlohmann_json)

# 复制数据文件到构建目录
file(COPY data DESTINATION ${CMAKE_BINARY_DIR})

# 添加测试可执行文件
add_executable(event_queue_test test/test.cc src/event_queue.cc)
target_link_libraries(event_queue_test PRIVATE nlohmann_json::nlohmann_json)

