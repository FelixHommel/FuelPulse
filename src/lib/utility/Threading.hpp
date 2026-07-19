#ifndef FUL_SRC_LIB_UTILITY_THREADING_HPP
#define FUL_SRC_LIB_UTILITY_THREADING_HPP

#include <version>

#if defined(__cpp_lib_jthread) && __cpp_lib_jthread >= 201911L
#    define HAS_JTHREAD
#endif

#if defined(HAS_JTHREAD)
#    include <stop_token>
#    include <thread>
#else
#    include "utility/ThreadingDetail.hpp"
#endif

namespace ful::threading
{

#if defined(HAS_JTHREAD)
using thread_t = std::jthread;
using stop_token_t = std::stop_token;
using stop_source_t = std::stop_source;
#else
using thread_t = detail::thread_t;
using stop_token_t = detail::stop_token_t;
using stop_source_t = detail::stop_source_t;
#endif

} // namespace ful::threading

#endif // FUL_SRC_LIB_UTILITY_THREADING_HPP

