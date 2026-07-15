set(NAME "GasWatchApp")

include(${PROJECT_SOURCE_DIR}/cmake/StaticAnalyzers.cmake)

add_executable(${NAME} ./main.cpp ./Application.cpp)
target_sources(${NAME} PRIVATE FILE_SET HEADERS FILES ./Application.hpp)

target_compile_features(${NAME} PRIVATE cxx_std_23)
target_link_libraries(${NAME} PRIVATE project_warnings GasWatchLib)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
