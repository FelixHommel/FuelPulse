#include "EventBus.hpp"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <exception>
#include <functional>
#include <mutex>
#include <typeindex>
#include <utility>
#include <vector>

namespace ful
{

/// \brief Implement the subscription behavior.
///
/// \param type \ref std::type_index of the event that is subscribed to
/// \param handler The function that deals with new incoming events of \p type
///
/// \returns The ID of the new subscriber
EventBus::SubscriptionId EventBus::subscribeImpl(std::type_index type, std::function<void(const void*)> handler)
{
    std::scoped_lock lock(m_mutex);

    const auto id{ m_nextId++ };

    // NOTE: Needs to use operator[] to create empty pair if \p type is not in the map yet.
    m_subscribers[type].emplace_back(std::make_pair(id, std::move(handler)));

    return id;
}

/// \brief Implement the unsubscription behavior.
///
/// \param type \ref std::type_index of the event that is unsubscribed from
/// \param id The ID of the subscriber that is unsubscribing from the bus
void EventBus::unsubscribeImpl(std::type_index type, SubscriptionId id)
{
    std::scoped_lock lock(m_mutex);

    const auto mapIt{ m_subscribers.find(type) };
    if(mapIt == m_subscribers.cend())
        return;

    auto& subs{ mapIt->second };
    if(const auto it{ std::ranges::find(subs, id, &Subscriber::first) }; it != subs.cend())
        subs.erase(it);
}

/// \brief Implement the publishing behavior.
///
/// \param type \ref std::type_index of the event that is published
/// \param pEvent type-erased pointer to the event
void EventBus::publishImpl(std::type_index type, const void* pEvent)
{
    // NOTE: By operating the publish operation on a copy of the subscribers, it is ensured that recursive publishing
    //  calls do not lead to a deadlock or other invalid states.
    SubscriberList copyOfSubs;

    {
        std::scoped_lock lock(m_mutex);

        if(const auto it{ m_subscribers.find(type) }; it != m_subscribers.cend())
            copyOfSubs = it->second;
    }

    std::ranges::for_each(copyOfSubs, [event = pEvent](const auto& sub) {
        try
        {
            sub.second(event);
        }
        catch(const std::exception& e)
        {
            spdlog::error("EventBus: handler threw: {}", e.what());
        }
        catch(...)
        {
            spdlog::error("EventBus: handler threw a non-std::exception");
        }
    });
}

} // namespace ful
