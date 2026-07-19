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

include(GoogleTest)
gtest_discover_tests(${NAME})
