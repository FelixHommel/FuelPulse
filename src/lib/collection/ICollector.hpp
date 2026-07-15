#ifndef GAS_SRC_LIB_COLLECTOR_I_COLLECTOR_HPP
#define GAS_SRC_LIB_COLLECTOR_I_COLLECTOR_HPP

namespace gas
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

} // namespace gas

#endif // !GAS_SRC_LIB_COLLECTOR_I_COLLECTOR_HPP
