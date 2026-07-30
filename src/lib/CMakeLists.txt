set(NAME "FuelPulseLib")
include(${PROJECT_SOURCE_DIR}/cmake/StaticAnalyzers.cmake)

add_library(
    ${NAME}
    ./core/CommandInterpreter.cpp
    ./core/CommandRegistry.cpp
    ./core/EventBus.cpp
    ./fuel/ConfigLoader.cpp
    ./fuel/FuelPulseConfig.cpp
    ./fuel/SQLiteFuelRepository.cpp
    ./utility/LoggerFactory.cpp
    ./utility/SQLiteConnection.cpp
)

target_sources(
    ${NAME}
    PRIVATE
        FILE_SET HEADERS
            FILES
                ./analysis/IAnalysis.hpp
                ./collection/ICollector.hpp
                ./core/CommandInterpreter.hpp
                ./core/CommandRegistry.hpp
                ./core/EventBus.hpp
                ./core/Scheduler.hpp
                ./fuel/ConfigLoader.hpp
                ./fuel/Domain.hpp
                ./fuel/FuelPulseConfig.hpp
                ./fuel/IFuelRepository.hpp
                ./fuel/SQLiteFuelRepository.hpp
                ./report/IReport.hpp
                ./utility/Assert.hpp
                ./utility/HashCombine.hpp
                ./utility/LoggerFactory.hpp
                ./utility/SQLiteConnection.hpp
                ./utility/StringOperations.hpp
                ./utility/Threading.hpp
                ./utility/ThreadingDetail.hpp
)

target_compile_features(${NAME} PRIVATE cxx_std_23)

if(MSVC)
    target_compile_options(
        ${NAME}
        PRIVATE /external:W0 /external:anglebrackets /external:templates-
    )
endif()

target_compile_options(
    ${NAME}
    INTERFACE $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_WARNINGS}>
)

target_link_libraries(
    ${NAME}
    PUBLIC spdlog::spdlog SQLiteCpp
    PRIVATE nlohmann_json::nlohmann_json
)
target_include_directories(${NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
