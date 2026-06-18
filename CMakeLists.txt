cmake_minimum_required(VERSION 3.16)
project(SmartCommunity VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Sql
    Charts
    Network
)

file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/*.h"
)

# Set application icon for Windows
if(WIN32 AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/resources/app.ico")
    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/resources/app.rc")
else()
    set(APP_ICON_RESOURCE_WINDOWS "")
endif()

add_executable(${PROJECT_NAME}
    ${SOURCES}
    resources/resources.qrc
    ${APP_ICON_RESOURCE_WINDOWS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
    Qt6::Charts
    Qt6::Network
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets
)
