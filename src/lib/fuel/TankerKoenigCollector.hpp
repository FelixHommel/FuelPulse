#ifndef FUL_SRC_LIB_FUEL_TANKER_KOENIG_COLLECTOR_HPP
#define FUL_SRC_LIB_FUEL_TANKER_KOENIG_COLLECTOR_HPP

#include "collection/ICollector.hpp"
#include "fuel/IFuelRepository.hpp"

namespace ful::fuel
{

class TankerKoenigCollector : public ICollector
{
public:
    TankerKoenigCollector(std::string apiKey, unsigned int postalCode, IFuelRepository& repo);
    ~TankerKoenigCollector() override = default;

    TankerKoenigCollector(const TankerKoenigCollector&) = delete;
    TankerKoenigCollector& operator=(const TankerKoenigCollector&) = delete;
    TankerKoenigCollector(TankerKoenigCollector&&) noexcept = delete;
    TankerKoenigCollector& operator=(TankerKoenigCollector&&) noexcept = delete;

    void collect() override;

private:
    static constexpr auto REQUEST_URL{ "https://creativecommons.tankerkoenig.de/api/v4/stations/postalcode?" };

    std::string m_apiKey;
    unsigned int m_postalCode;
    IFuelRepository& m_repo;
};

} // namespace ful::fuel

#endif //! FUL_SRC_LIB_FUEL_TANKER_KOENIG_COLLECTOR_HPP
