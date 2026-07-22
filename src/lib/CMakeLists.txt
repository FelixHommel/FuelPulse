set(NAME "FuelPulseLib")
include(${PROJECT_SOURCE_DIR}/cmake/StaticAnalyzers.cmake)

add_library(
    ${NAME}
    ./core/CommandInterpreter.cpp
    ./core/CommandRegistry.cpp
    ./core/EventBus.cpp
    ./utility/LoggerFactory.cpp
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
                ./fuel/Domain.hpp
                ./report/IReport.hpp
                ./storage/IRepository.hpp
                ./utility/Assert.hpp
                ./utility/LoggerFactory.hpp
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
    PUBLIC spdlog::spdlog
    PRIVATE nlohmann_json::nlohmann_json
)
target_include_directories(${NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
