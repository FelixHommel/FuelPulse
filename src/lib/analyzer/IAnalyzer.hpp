#ifndef FUL_SRC_LIB_ANALYSIS_I_ANALYSIS_HPP
#define FUL_SRC_LIB_ANALYSIS_I_ANALYSIS_HPP

namespace ful
{

class IAnalyzer
{
public:
    IAnalyzer() = default;
    virtual ~IAnalyzer() = default;

    IAnalyzer(const IAnalyzer&) = default;
    IAnalyzer& operator=(const IAnalyzer&) = default;
    IAnalyzer(IAnalyzer&&) = default;
    IAnalyzer& operator=(IAnalyzer&&) = default;

    virtual void analyze() = 0;
};

} // namespace ful

#endif // !FUL_SRC_LIB_ANALYSIS_I_ANALYSIS_HPP
