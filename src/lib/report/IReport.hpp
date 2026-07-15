#ifndef GAS_SRC_LIB_REPORT_I_REPORT_HPP
#define GAS_SRC_LIB_REPORT_I_REPORT_HPP

namespace gas
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

} // namespace gas

#endif // !GAS_SRC_LIB_REPORT_I_REPORT_HPP
