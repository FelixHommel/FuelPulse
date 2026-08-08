option(FUL_ENABLE_CLANG_TIDY "Enable clang-tidy during compilation" ON)
option(
    FUL_ENABLE_ADDRESS_SANITIZER
    "Prepare the build to compile with address sanitizer"
    ON
)
option(FUL_ENABLE_VALGRIND "Prepare the build to be used with valgrind" OFF)
option(FUL_DEBUG "Enable debug statements and asserts" ON)
option(FUL_ENABLE_ASSERTIONS "Enable assertions" ON)
option(FUL_ENABLE_CODE_COVERAGE "Enable code coverage analysis" OFF)
option(FUL_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)
option(FUL_ENABLE_STACKTRACE "Enable stacktrace capture in exceptions" ON)
