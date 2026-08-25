set(NAME "FuelPulseLib")
include(${PROJECT_SOURCE_DIR}/cmake/StaticAnalyzers.cmake)

add_library(
    ${NAME}
    ./core/CommandInterpreter.cpp
    ./core/CommandRegistry.cpp
    ./core/EventBus.cpp
    ./fuel/ConfigLoader.cpp
    ./fuel/DailyAnalyzer.cpp
    ./fuel/FuelPulseConfig.cpp
    ./fuel/TankerKoenigAPIParser.cpp
    ./fuel/SQLiteFuelRepository.cpp
    ./utility/env/EnvironmentVariableHelper.cpp
    ./utility/exception/ConfigValidationException.cpp
    ./utility/exception/Exception.cpp
    ./utility/exception/FileIOException.cpp
    ./utility/exception/SQLiteAccessException.cpp
    ./utility/exception/SQLiteConnectionException.cpp
    ./utility/exception/WebRequestException.cpp
    ./utility/file/FileIO.cpp
    ./utility/logging/LoggerFactory.cpp
    ./utility/sqlite/SQLiteConnection.cpp
    ./utility/validation/Validator.cpp
)

target_sources(
    ${NAME}
    PRIVATE
        FILE_SET HEADERS
            FILES
                ./analyzer/IAnalyzer.hpp
                ./collection/ICollector.hpp
                ./core/CommandInterpreter.hpp
                ./core/CommandRegistry.hpp
                ./core/EventBus.hpp
                ./core/Scheduler.hpp
                ./fuel/ConfigLoader.hpp
                ./fuel/CprTankerKoenigGateway.hpp
                ./fuel/DailyAnalyzer.hpp
                ./fuel/Domain.hpp
                ./fuel/FuelPulseConfig.hpp
                ./fuel/FuelType.hpp
                ./fuel/IFuelRepository.hpp
                ./fuel/SQLiteFuelRepository.hpp
                ./fuel/TankerKoenigAPIParser.hpp
                ./fuel/TankerKoenigCollector.hpp
                ./report/IReport.hpp
                ./utility/Assert.hpp
                ./utility/StringOperations.hpp
                ./utility/HttpErrorCodes.hpp
                ./utility/env/EnvironmentVariableHelper.hpp
                ./utility/exception/ConfigValidationException.hpp
                ./utility/exception/Exception.hpp
                ./utility/exception/FileIOException.hpp
                ./utility/exception/SQLiteAccessException.hpp
                ./utility/exception/SQLiteConnectionException.hpp
                ./utility/exception/Stacktrace.hpp
                ./utility/exception/WebRequestException.hpp
                ./utility/file/FileIO.hpp
                ./utility/hash/HashCombine.hpp
                ./utility/logging/LoggerFactory.hpp
                ./utility/sqlite/SQLiteConnection.hpp
                ./utility/threading/SemaphoreReleaseGuard.hpp
                ./utility/threading/Threading.hpp
                ./utility/threading/ThreadingDetail.hpp
                ./utility/validation/Validator.hpp
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
    PRIVATE
        cpr::cpr
        nlohmann_json::nlohmann_json
        nlohmann_json_schema_validator::validator
)
target_include_directories(${NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
