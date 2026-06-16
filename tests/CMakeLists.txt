cmake_minimum_required(VERSION 3.16)
project(SmartCommunityTests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Sql Test Widgets)

# 将 schema.sql 复制到构建目录，确保测试能找到
file(COPY ${CMAKE_SOURCE_DIR}/../sql/schema.sql DESTINATION ${CMAKE_BINARY_DIR}/sql)

add_executable(test_core_logic
    test_core_logic.cpp
    ${CMAKE_SOURCE_DIR}/../src/database/DatabaseManager.h
    ${CMAKE_SOURCE_DIR}/../src/services/AuthService.h
)
target_include_directories(test_core_logic PRIVATE ${CMAKE_SOURCE_DIR}/../src)
target_link_libraries(test_core_logic PRIVATE Qt6::Core Qt6::Sql Qt6::Test Qt6::Widgets)

enable_testing()
add_test(NAME CoreLogicTest COMMAND test_core_logic WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
