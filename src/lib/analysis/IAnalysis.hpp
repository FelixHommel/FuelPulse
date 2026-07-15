#ifndef GAS_SRC_LIB_ANALYSIS_I_ANALYSIS_HPP
#define GAS_SRC_LIB_ANALYSIS_I_ANALYSIS_HPP

namespace gas
{

class IAnalysis
{
public:
    IAnalysis() = default;
    virtual ~IAnalysis() = default;

    IAnalysis(const IAnalysis&) = default;
    IAnalysis& operator=(const IAnalysis&) = default;
    IAnalysis(IAnalysis&&) = default;
    IAnalysis& operator=(IAnalysis&&) = default;

    virtual void analyze() = 0;
};

} // namespace gas

#endif // !GAS_SRC_LIB_ANALYSIS_I_ANALYSIS_HPP
