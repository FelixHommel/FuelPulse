#ifndef FUL_SRC_LIB_REPORT_I_REPORT_HPP
#define FUL_SRC_LIB_REPORT_I_REPORT_HPP

namespace ful
{

class IReport
{
public:
    IReport() = default;
    virtual ~IReport() = default;

    IReport(const IReport&) = default;
    IReport& operator=(const IReport&) = default;
    IReport(IReport&&) = default;
    IReport& operator=(IReport&&) = default;

    virtual void report() = 0;
};

} // namespace ful

#endif // !FUL_SRC_LIB_REPORT_I_REPORT_HPP
