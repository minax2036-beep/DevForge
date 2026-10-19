cmake_minimum_required(VERSION 3.20)

project(DevForge
    VERSION 0.1.0
    DESCRIPTION "Linux Developer Toolbox"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Widgets)

add_executable(DevForge
    main.cpp
)

target_link_libraries(DevForge PRIVATE Qt6::Widgets)

target_compile_options(DevForge PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

install(TARGETS DevForge
    RUNTIME DESTINATION bin
)
