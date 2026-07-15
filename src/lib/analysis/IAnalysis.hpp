#ifndef FUL_SRC_LIB_ANALYSIS_I_ANALYSIS_HPP
#define FUL_SRC_LIB_ANALYSIS_I_ANALYSIS_HPP

namespace ful
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

} // namespace ful

#endif // !FUL_SRC_LIB_ANALYSIS_I_ANALYSIS_HPP
