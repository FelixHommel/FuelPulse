include(CheckCXXSourceCompiles)

check_cxx_source_compiles(
    "
    #include <stacktrace>

    int main()
    {
        const auto trace{ std::stacktrace::current() };
        return trace.size();
    }
    "
    FUL_HAS_STACKTRACE
)

if(
    FUL_HAS_STACKTRACE
    AND FUL_ENABLE_STACKTRTACE
    AND CMAKE_BUILD_TYPE STREQUAL "Debug"
)
    set(FUL_USE_STACKTRACE ON CACHE BOOL "" FORCE)
else()
    set(FUL_USE_STACKTRACE OFF CACHE BOOL "" FORCE)
endif()

add_compile_definitions(FUL_USE_STACKTRACE=$<BOOL:${FUL_USE_STACKTRACE}>)
