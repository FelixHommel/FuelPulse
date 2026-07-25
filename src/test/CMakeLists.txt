set(NAME "FuelPulseTests")

include(${PROJECT_SOURCE_DIR}/cmake/StaticAnalyzers.cmake)

add_executable(
    ${NAME}
    ./testMain.cpp
    ./core/CommandRegistryTest.cpp
    ./core/CommandInterpreterTest.cpp
    ./core/EventBusTest.cpp
    ./core/SchedulerTest.cpp
    ./integration/CoreIntegrationTest.cpp
    ./utility/ThreadingTest.cpp
    ./utility/SQLiteConnectionTest.cpp
    ./utility/StringOperationsTest.cpp
    ./testUtility/ConcurrencyTest.cpp
)

target_sources(
    ${NAME}
    PRIVATE
        FILE_SET HEADERS
            FILES
                ./testUtility/RandomNumberGenerator.hpp
                ./testUtility/ConcurrencyTest.hpp
)

target_compile_features(${NAME} PRIVATE cxx_std_23)
target_link_libraries(
    ${NAME}
    PRIVATE project_warnings GTest::gtest GTest::gmock FuelPulseLib
)
target_compile_definitions(
    ${NAME}
    PRIVATE TEST_RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/testResources"
)

include(GoogleTest)
gtest_discover_tests(${NAME})
