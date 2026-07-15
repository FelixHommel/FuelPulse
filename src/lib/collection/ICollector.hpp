#ifndef FUL_SRC_LIB_COLLECTOR_I_COLLECTOR_HPP
#define FUL_SRC_LIB_COLLECTOR_I_COLLECTOR_HPP

namespace ful
{

class ICollector
{
public:
    ICollector() = default;
    virtual ~ICollector() = default;

    ICollector(const ICollector&) = default;
    ICollector& operator=(const ICollector&) = default;
    ICollector(ICollector&&) = default;
    ICollector& operator=(ICollector&&) = default;

    virtual void collect() = 0;
};

} // namespace ful

#endif // !FUL_SRC_LIB_COLLECTOR_I_COLLECTOR_HPP
