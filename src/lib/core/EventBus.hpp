#ifndef FUL_SRC_ENGINE_LIB_CORE_EVENT_BUS_HPP
#define FUL_SRC_ENGINE_LIB_CORE_EVENT_BUS_HPP

#include <spdlog/logger.h>

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ful
{

/// \brief The event bus is a system that is used for inter-system communication and coordination.
///
/// The \ref EventBus operations are implemented thread-safe using a mutex.
///
/// \author Felix Hommel
/// \date 7/16/2026
class EventBus
{
public:
    using SubscriptionId = std::uint64_t;

    EventBus();
    ~EventBus() = default;

    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;
    EventBus(EventBus&&) = delete;
    EventBus& operator=(EventBus&&) = delete;

    /// \brief Subscribe a listener for \p EventT to the bus.
    ///
    /// \tparam EventT The type of the event that is being subscribed to
    /// \param handler The handler that is reacting when a \p EventT is published on the bus
    ///
    /// \returns the id that the listener has been assigned
    template<typename EventT>
    SubscriptionId subscribe(std::function<void(const EventT&)> handler)
    {
        return subscribeImpl(std::type_index(typeid(EventT)), [handler = std::move(handler)](const void* pEvent) {
            handler(*static_cast<const EventT*>(pEvent));
        });
    }

    /// \brief Unsubscribe a listener from \p EventT.
    ///
    /// \tparam EventT The type of the event that is being unsubscribed from
    /// \param id The subscriber that is being unsubscribed
    template<typename EventT>
    void unsubscribe(SubscriptionId id)
    {
        unsubscribeImpl(std::type_index(typeid(EventT)), id);
    }

    /// \brief Publish a new \p EventT.
    ///
    /// \tparam EventT The type of the event that is being published
    /// \param event (optional) The \p EventT that is published
    template<typename EventT>
    void publish(const EventT& event = {})
    {
        publishImpl(std::type_index(typeid(EventT)), &event);
    }

private:
    /// \brief The handler function is a simple function that takes a void* (event object) and returns nothing.
    using HandlerFn = std::function<void(const void*)>;
    /// \brief Store handlers as std::shared_ptr for easier copying during the publish operation.
    using Subscriber = std::pair<SubscriptionId, std::shared_ptr<HandlerFn>>;
    /// \brief The subscriber list is simply a vector of \ref Subscriber
    using SubscriberList = std::vector<Subscriber>;

    std::unique_ptr<spdlog::logger> m_logger;

    std::mutex m_mutex;
    std::unordered_map<std::type_index, SubscriberList> m_subscribers;
    std::atomic<SubscriptionId> m_nextId{ 0 };

    SubscriptionId subscribeImpl(std::type_index type, HandlerFn handler);
    void unsubscribeImpl(std::type_index type, SubscriptionId id);
    void publishImpl(std::type_index type, const void* pEvent);
};

} // namespace ful

#endif // !FUL_SRC_ENGINE_LIB_CORE_EVENT_BUS_HPP
