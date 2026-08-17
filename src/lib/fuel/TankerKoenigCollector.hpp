#ifndef FUL_SRC_LIB_FUEL_TANKER_KOENIG_COLLECTOR_HPP
#define FUL_SRC_LIB_FUEL_TANKER_KOENIG_COLLECTOR_HPP

#include "collection/ICollector.hpp"
#include "core/EventBus.hpp"
#include "fuel/CprTankerKoenigGateway.hpp"
#include "fuel/IFuelRepository.hpp"
#include "fuel/TankerKoenigAPIParser.hpp"
#include "utility/logging/LoggerFactory.hpp"
#include "utility/threading/SemaphoreReleaseGuard.hpp"
#include "utility/threading/Threading.hpp"

#include <nlohmann/json.hpp>
#include <spdlog/logger.h>

#include <atomic>
#include <concepts>
#include <exception>
#include <memory>
#include <mutex>
#include <semaphore>
#include <string>
#include <utility>
#include <vector>

namespace ful::fuel
{

/// \brief A concept that describes a class that can be used to gather fuel prices.
///
/// \tparam T The possible type.
template<typename T>
concept TankerKoenigGateway
    = std::movable<T> && requires(const T& t, const std::string& apiKey, unsigned int postalCode) {
          { t.fetchPrices(apiKey, postalCode) } -> std::same_as<std::string>;
      };

/// \brief Collect fuel price data from the TankerKoenig API and save the data to a \ref IFuelRepository.
///
/// \tparam GatewayT The \ref TankerKoenigGateway that is used to make the request
///
/// \author Felix Hommel
/// \date 8/14/2026
///
/// \see https://creativecommons.tankerkoenig.de/swagger/
template<TankerKoenigGateway GatewayT = CprTankerKoenigGateway>
class TankerKoenigCollector : public ICollector
{
public:
    /// \brief Create a new \ref TankerKoenigCollector.
    ///
    /// This constructor type uses a default-constructed \p GatewayT object.
    ///
    /// \param apiKey The API key for the TankerKoenig API
    /// \param postalCode The postalCode around which to gather the fuel prices
    /// \param repo The \ref IFuelRepository where the fuel prices are saved to
    /// \param bus The \ref EventBus where finished events can be published on
    TankerKoenigCollector(std::string apiKey, unsigned int postalCode, IFuelRepository& repo, EventBus& bus)
        requires std::default_initializable<GatewayT>
        : TankerKoenigCollector(std::move(apiKey), postalCode, repo, bus, GatewayT{})
    {}

    /// \brief Create a new \ref TankerKoenigCollector.
    ///
    /// \param apiKey The API key for the TankerKoenig API
    /// \param postalCode The postalCode around which to gather the fuel prices
    /// \param repo The \ref IFuelRepository where the fuel prices are saved to
    /// \param bus The \ref EventBus where finished events can be published on
    /// \param gateway The \ref GatewayT that is responsible for doing the requests from the API
    TankerKoenigCollector(
        std::string apiKey, unsigned int postalCode, IFuelRepository& repo, EventBus& bus, GatewayT gateway
    )
        : m_apiKey{ std::move(apiKey) }
        , m_postalCode{ postalCode }
        , m_repo{ repo }
        , m_bus{ bus }
        , m_gateway{ std::move(gateway) }
    {
        LoggerFactory factory{};
        m_logger = factory.create("TankerKoenigCollector", LoggerProfile::Console);
    }
    ~TankerKoenigCollector() override = default;

    TankerKoenigCollector(const TankerKoenigCollector&) = delete;
    TankerKoenigCollector& operator=(const TankerKoenigCollector&) = delete;
    TankerKoenigCollector(TankerKoenigCollector&&) noexcept = delete;
    TankerKoenigCollector& operator=(TankerKoenigCollector&&) noexcept = delete;

    /// \brief Start a new collecting task if the previous task is already finished with the web request.
    void collect() override
    {
        if(!m_fetchGate.try_acquire())
        {
            m_logger->warn("Skipping collect(): previous request still awaiting network response");
            return;
        }

        joinFinishedWorkers();

        auto finished{ std::make_shared<std::atomic<bool>>(false) };

        std::scoped_lock lock{ m_workersMutex };
        m_workers.emplace_back(threading::thread_t([this, finished] { runCollection(finished); }), finished);
    }

private:
    /// \brief Simple struct defining a single collection workload.
    ///
    /// \author Felix Hommel
    /// \date 8/14/2026
    struct WorkerSlot
    {
        threading::thread_t thread;
        std::shared_ptr<std::atomic<bool>> finished;
    };

    std::string m_apiKey;
    unsigned int m_postalCode;

    IFuelRepository& m_repo;
    EventBus& m_bus;

    GatewayT m_gateway;
    std::binary_semaphore m_fetchGate{ 1 };
    std::mutex m_workersMutex;
    std::mutex m_storeMutex;
    std::vector<WorkerSlot> m_workers;
    std::unique_ptr<spdlog::logger> m_logger;

    /// \brief Execute a collection request and save the resulting data to the \ref IFuelRepository.
    ///
    /// \param finished A flag to indicate if the collection and saving is finished.
    void runCollection(const std::shared_ptr<std::atomic<bool>>& finished)
    {
        SemaphoreReleaseGuard<std::binary_semaphore> guard{ m_fetchGate };

        try
        {
            const auto json{ m_gateway.fetchPrices(m_apiKey, m_postalCode) };
            guard.disarm();
            m_fetchGate.release();

            {
                std::scoped_lock lock{ m_storeMutex };

                for(const auto& m : parseStationPrices(json))
                    m_repo.store(m);
            }

            // NOTE: Possibly publish a CollectionFinished event here, if needed by anything else
        }
        catch(const std::exception& e)
        {
            m_logger->error("Collection failed: {}", e.what());
        }
        catch(...)
        {
            m_logger->error("Collection failed for unknown reason");
        }

        finished->store(true, std::memory_order_release);
    }

    /// \brief Join the workers that have finished their work.
    void joinFinishedWorkers()
    {
        std::scoped_lock lock{ m_workersMutex };

        std::erase_if(m_workers, [](WorkerSlot& slot) {
            if(!slot.finished->load(std::memory_order_acquire))
                return false;

            slot.thread.join();
            return true;
        });
    }
};

} // namespace ful::fuel

#endif // !FUL_SRC_LIB_FUEL_TANKER_KOENIG_COLLECTOR_HPP
