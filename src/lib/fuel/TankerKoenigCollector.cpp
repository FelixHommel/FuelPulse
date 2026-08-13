#include "TankerKoenigCollector.hpp"

#include "fuel/IFuelRepository.hpp"

#include <cpr/api.h>
#include <cpr/cprtypes.h>
#include <cpr/parameters.h>

#include <string>
#include <utility>

namespace ful::fuel
{

TankerKoenigCollector::TankerKoenigCollector(std::string apiKey, unsigned int postalCode, IFuelRepository& repo)
    : m_apiKey{ std::move(apiKey) }, m_postalCode{ postalCode }, m_repo{ repo }
{}

void TankerKoenigCollector::collect()
{
    // TODO: Dispatch this in separate thread so that the main thread does not block to wait for response
    const auto curlResponse{
        cpr::Get(
            cpr::Url{ REQUEST_URL },
            cpr::Parameters{ { "apikey", m_apiKey }, { "postalcode", std::to_string(m_postalCode) } }
        )
    };

    //  TODO: Use the response to deal with error cases or save data to the repo
}

} // namespace ful::fuel
