#include "Exception.hpp"

#include <format>
#include <source_location>
#include <string>
#include <utility>

namespace ful
{

Exception::Exception(std::string message, std::source_location loc)
    : m_message{ std::move(message) }
    , m_location{ std::move(loc) }
    , m_formatted{ std::format(
          "[{}:{} in {}]: {}", m_location.file_name(), m_location.line(), m_location.function_name(), m_message
      ) }
{
    if(const std::string trace{ m_trace.format() }; !trace.empty())
        m_formatted.append(std::format("\n\nStacktrace:\n{}", trace));
}

} // namespace ful
