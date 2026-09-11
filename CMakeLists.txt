cmake_minimum_required(VERSION 3.30)
project(shecell LANGUAGES C)

set(CMAKE_C_STANDARD_23 ON)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(Readline REQUIRED IMPORTED_TARGET readline)

file(GLOB SOURCES CONFIGURE_DEPENDS "src/*.c")

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        include
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        PkgConfig::Readline
)

target_compile_options(${PROJECT_NAME}
    PRIVATE
        -Wall
        -Wextra
)

target_compile_definitions(${PROJECT_NAME} 
    PRIVATE 
        PROJECT_ROOT="${CMAKE_SOURCE_DIR}"
)