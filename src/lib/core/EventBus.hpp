#ifndef FUL_SRC_ENGINE_LIB_CORE_EVENT_BUS_HPP
#define FUL_SRC_ENGINE_LIB_CORE_EVENT_BUS_HPP

#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ful
{

/// \brief The event bus is a system that is used for inter-system communication and coordination.
///
/// \author Felix Hommel
/// \date 7/16/2026
class EventBus
{
public:
    using SubscriptionId = std::uint64_t;

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
    using Subscriber = std::pair<SubscriptionId, std::function<void(const void*)>>;
    using SubscriberList = std::vector<Subscriber>;

    SubscriptionId subscribeImpl(std::type_index type, std::function<void(const void*)> handler);
    void unsubscribeImpl(std::type_index type, SubscriptionId id);
    void publishImpl(std::type_index type, const void* pEvent);

    std::mutex m_mutex;
    std::unordered_map<std::type_index, SubscriberList> m_subscribers;
    std::atomic<SubscriptionId> m_nextId{ 0 };
};

} // namespace ful

#endif // !FUL_SRC_ENGINE_LIB_CORE_EVENT_BUS_HPP
