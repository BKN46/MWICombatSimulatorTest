cmake_minimum_required(VERSION 3.10)
project(CombatSimulator)

set(CMAKE_CXX_STANDARD 17)

# 添加头文件路径
include_directories(include include/events lib)

# 添加所有源文件
file(GLOB SOURCES "src/*.cc" "src/events/*.cc")

# 添加可执行文件
add_executable(combatsimulator ${SOURCES} test/test.cc)

# 复制数据文件到可执行文件目录
add_custom_command(TARGET combatsimulator POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:combatsimulator>/data)

# 添加测试可执行文件
add_executable(test_minimal test/test.cc src/player.cc src/combat_unit.cc src/ability.cc
    src/consumable.cc src/equipment.cc src/house_room.cc src/trigger.cc src/buff.cc)

# 为测试程序也复制数据文件
add_custom_command(TARGET test_minimal POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:test_minimal>/data)

