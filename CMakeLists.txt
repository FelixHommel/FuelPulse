cmake_minimum_required(VERSION 3.30)
project(
    GasWatch
    DESCRIPTION "Track gas station prices"
    LANGUAGES CXX
    VERSION 0.1.0
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/vcpkg-dependencies.cmake)
include(cmake/DefaultProjectSettings.cmake)
include(cmake/CodeCoverage.cmake)

add_library(project_warnings INTERFACE)
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

add_compile_definitions(GAS_ROOT="${CMAKE_SOURCE_DIR}/")

add_subdirectory(src)
