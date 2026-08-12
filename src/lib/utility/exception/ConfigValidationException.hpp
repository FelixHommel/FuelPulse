#ifndef FUL_SRC_LIB_UTILITY_EXCEPTION_CONFIG_VALIDATION_EXCEPTION_HPP
#define FUL_SRC_LIB_UTILITY_EXCEPTION_CONFIG_VALIDATION_EXCEPTION_HPP

#include "utility/exception/Exception.hpp"
#include "utility/validation/Validator.hpp"

#include <source_location>

namespace ful
{

/// \brief Exception used to indicate that the validation of a JSON config file failed.
///
/// \author Felix Hommel
/// \date 8/8/2026
class ConfigValidationException final : public Exception
{
public:
    ~ConfigValidationException() override = default;

    ConfigValidationException(const ConfigValidationException&) = default;
    ConfigValidationException& operator=(const ConfigValidationException&) = default;
    ConfigValidationException(ConfigValidationException&&) = default;
    ConfigValidationException& operator=(ConfigValidationException&&) = default;

    /// \brief Factory function to create a new \ref ConfigValidationException.
    ///
    /// \param errors The \ref ValidationErrors the \ref Validator found
    /// \param loc (optional) The \ref std::source_location where the exception was caused
    ///
    /// \returns The new \ref ConfigValidationException
    [[nodiscard]] static ConfigValidationException create(
        Validator::ValidationErrors errors, std::source_location loc = std::source_location::current()
    );

    /// \brief Access the errors that caused the exception.
    [[nodiscard]] const Validator::ValidationErrors& errors() const noexcept { return m_errors; }

private:
    Validator::ValidationErrors m_errors;

    ConfigValidationException(Exception exception, Validator::ValidationErrors errors);
};

} // namespace ful

#endif // !FUL_SRC_LIB_UTILITY_EXCEPTION_CONFIG_VALIDATION_EXCEPTION_HPP
