if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(
        STATUS
        "Setting build type to 'Debug' because none other was specified."
    )
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)

    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

find_program(CCACHE ccache)
if(CCACHE)
    message(STATUS "Using ccache")
    set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
else()
    message(STATUS "Ccache not found")
endif()

if(MSVC)
    add_compile_options(/utf-8)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

option(GAS_DEBUG "Enable debug statements and asserts" ON)
option(GAS_ENABLE_ASSERTIONS "Enable assertions" ON)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(GAS_DEBUG OFF CACHE BOOL "" FORCE)
    set(GAS_ENABLE_ASSERTIONS OFF CACHE BOOL "" FORCE)
endif()

add_compile_definitions(GAS_DEBUG=$<BOOL:${GAS_DEBUG}>)
add_compile_definitions(GAS_ENABLE_ASSERTIONS=$<BOOL:${GAS_ENABLE_ASSERTIONS}>)
