set(NAME "FuelPulseLib")
include(${PROJECT_SOURCE_DIR}/cmake/StaticAnalyzers.cmake)

add_library(
    ${NAME}
    ./core/CommandInterpreter.cpp
    ./core/EventBus.cpp
    ./core/Scheduler.cpp
)

target_sources(
    ${NAME}
    PRIVATE
        FILE_SET HEADERS
            FILES
                ./analysis/IAnalysis.hpp
                ./collection/ICollector.hpp
                ./core/CommandInterpreter.hpp
                ./core/EventBus.hpp
                ./core/Scheduler.hpp
                ./report/IReport.hpp
                ./storage/IStorage.hpp
                ./utility/Assert.hpp
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

target_link_libraries(${NAME} PRIVATE spdlog::spdlog)
target_include_directories(${NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
