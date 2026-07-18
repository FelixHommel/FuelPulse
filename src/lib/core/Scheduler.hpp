#ifndef FUL_SRC_LIB_CORE_SCHEDULER_HPP
#define FUL_SRC_LIB_CORE_SCHEDULER_HPP

#include "utility/LoggerFactory.hpp"
#include "utility/Threading.hpp"

#include <spdlog/logger.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <concepts>
#include <condition_variable>
#include <cstdint>
#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <utility>
#include <vector>

namespace ful
{

/// \brief Requirements a clock must fulfill to be used with the \ref Scheduler.
template<typename T>
concept SchedulerClock = requires {
    typename T::time_point;
    typename T::duration;
    { T::now() } -> std::same_as<typename T::time_point>;
};

/// \brief Provide capability for users to schedule arbitrary tasks at points in time and then executes these tasks
///     when they are due.
///
/// \tparam ClockT (optional) The clock that is used to determine the scheduling times.
///
/// \author Felix Hommel
/// \date 7/18/2026
template<SchedulerClock ClockT = std::chrono::steady_clock>
class Scheduler
{
public:
    using TimePoint = typename ClockT::time_point;
    using Duration = typename ClockT::duration;
    using TaskId = std::uint64_t;
    using Task = std::function<void()>;

    Scheduler()
    {
        LoggerFactory factory{};
        m_logger = factory.create("Scheduler", LoggerProfile::Console);
    }

    ~Scheduler() { requestStop(true); }

    Scheduler(const Scheduler&) = delete;
    Scheduler(Scheduler&&) = delete;
    Scheduler& operator=(const Scheduler&) = delete;
    Scheduler& operator=(Scheduler&&) = delete;

    /// \brief Schedule \p task at \p when.
    ///
    /// \param when The \ref TimePoint when the task should be executed
    /// \param task The task that is executed
    ///
    /// \returns the \ref TaskId of the scheduled task
    TaskId scheduleAt(TimePoint when, Task task) { return scheduleImpl(when, std::nullopt, std::move(task)); }

    /// \brief Schedule \p task that is executed after \p delay has elapsed.
    /// \returns the \ref TaskId of the scheduled task
    ///
    /// \param delay The delay after which the task should be executed
    /// \param task The task that is executed
    ///
    /// \returns the \ref TaskId of the scheduled task
    template<typename Rep, typename Period>
    TaskId scheduleIn(std::chrono::duration<Rep, Period> delay, Task task)
    {
        return scheduleImpl(ClockT::now() + std::chrono::duration_cast<Duration>(delay), std::nullopt, std::move(task));
    }

    /// \brief Schedule \p task to be repeating after \p interval.
    ///
    /// \param interval The interval in which the task is executed
    /// \param task The task that is executed
    /// \param firstRun (optional) specify when the task is executed for the first time
    ///
    /// \returns the \ref TaskId of the scheduled task
    template<typename Rep, typename Period>
    TaskId scheduleEvery(std::chrono::duration<Rep, Period> interval, Task task, TimePoint firstRun = ClockT::now())
    {
        return scheduleImpl(
            firstRun, std::make_optional(std::chrono::duration_cast<Duration>(interval)), std::move(task)
        );
    }

    /// \brief Cancel a scheduled task.
    ///
    /// \param id The \ref TaskId of the scheduled task
    ///
    /// \returns true if a pending task was canceled, false otherwise
    bool cancel(TaskId id)
    {
        std::scoped_lock lock(m_mutex);

        const auto it{ std::ranges::find(m_heap, id, &ScheduledTask::id) };
        if(it == m_heap.cend() || it->canceled)
            return false;

        it->canceled = true;
        m_wakeUp.notify_all();

        return true;
    }

    /// \brief Run every task that is due at \p now and reschedule recurring ones.
    ///
    /// \param now (optional) Current time
    ///
    /// \returns The \ref TimePoint of the next pending task if there are any remaining tasks
    std::optional<TimePoint> runDue(TimePoint now = ClockT::now())
    {
        const auto due{ filterDueTasks(now) };

        for(const auto& task : due)
        {
            try
            {
                task();
            }
            catch(const std::exception& e)
            {
                m_logger->warn("executed task threw std::exception: {}", e.what());
            }
            catch(...)
            {
                m_logger->warn("executed task threw non-std::exception");
            }
        }

        std::scoped_lock lock(m_mutex);

        if(m_heap.empty())
            return std::nullopt;

        return m_heap.front().nextRun;
    }

    /// \brief Start the background thread that executes the scheduled tasks.
    void start()
    {
        if(m_thread.joinable())
        {
            m_logger->warn("start() called even though interpreter is already running");
            return;
        }

        // NOTE: From: https://stackoverflow.com/a/65701431
        m_thread = threading::thread_t(std::bind_front(&Scheduler::run, this));
        m_logger->info("Starting background thread");
    }

    /// \brief Request to stop the background task executor thread.
    ///
    /// \param waitForStop (optional) Block until the background executor has been stopped
    void requestStop(bool waitForStop = false)
    {
        m_thread.request_stop();
        m_wakeUp.notify_all();
        m_logger->info("Requested to stop the background thread");

        if(waitForStop)
            waitUnitlStopped();
    }

    /// \brief Block until the background executor thread has stopped.
    void waitUnitlStopped()
    {
        if(m_thread.joinable())
            m_thread.join();
    }

private:
    /// \brief Data for a scheduled task.
    ///
    /// \author Felix Hommel
    /// \date 7/18/2026
    struct ScheduledTask
    {
        TaskId id{};
        TimePoint nextRun;
        std::optional<Duration> interval;
        Task task;
        bool canceled{ false };
    };

    static constexpr auto HEAP_COMPARE{ [](const ScheduledTask& a, const ScheduledTask& b) {
        return a.nextRun > b.nextRun;
    } };

    std::unique_ptr<spdlog::logger> m_logger;

    std::mutex m_mutex;
    std::condition_variable m_wakeUp;
    std::vector<ScheduledTask> m_heap;
    std::atomic<TaskId> m_nextId{ 0 };

    threading::thread_t m_thread;

    /// \brief Schedule a new task.
    ///
    /// \param when The \ref TimePoint when the task is executed
    /// \param interval A \ref std::optional repetition time that is used for repeating tasks
    /// \param task The task that is executed
    ///
    /// \returns the \ref TaskId of the scheduled task
    TaskId scheduleImpl(TimePoint when, std::optional<Duration> interval, Task task)
    {
        std::scoped_lock lock(m_mutex);

        const auto id{ m_nextId++ };

        m_heap.emplace_back(id, when, std::move(interval), std::move(task));
        std::ranges::push_heap(m_heap, HEAP_COMPARE);

        m_wakeUp.notify_all();

        return id;
    }

    /// \brief The background scheduling loop.
    ///
    /// \param token A stop token indicating that a stop was requested
    void run(threading::stop_token_t token)
    {
        std::unique_lock lock(m_mutex);

        while(!token.stop_requested())
        {
            if(m_heap.empty())
            {
                m_wakeUp.wait(lock, [&token, this]() { return token.stop_requested() || !m_heap.empty(); });
                continue;
            }

            const auto deadline{ m_heap.front().nextRun };
            lock.unlock();

            {
                std::unique_lock waitLock(m_mutex);
                m_wakeUp.wait_until(waitLock, deadline, [&token, this, deadline]() {
                    return token.stop_requested() || m_heap.empty() || m_heap.front().nextRun < deadline
                        || ClockT::now() >= deadline;
                });
            }

            if(!token.stop_requested())
                runDue();

            lock.lock();
        }
    }

    /// \brief Filter out the tasks that re due at \p now.
    ///
    /// \param now The point of time at filtering
    ///
    /// \returns \ref std::vector containing the due \ref Task
    [[nodiscard]] std::vector<Task> filterDueTasks(TimePoint now)
    {
        std::scoped_lock lock(m_mutex);

        std::vector<Task> result;

        while(!m_heap.empty() && m_heap.front().nextRun <= now)
        {
            std::ranges::pop_heap(m_heap, HEAP_COMPARE);
            ScheduledTask task{ std::move(m_heap.back()) };
            m_heap.pop_back();

            if(task.canceled)
                continue;

            if(task.interval.has_value())
            {
                task.nextRun = now + *task.interval;
                result.push_back(task.task);
                m_heap.push_back(std::move(task));
                std::ranges::push_heap(m_heap, HEAP_COMPARE);
            }
            else
                result.push_back(std::move(task.task));
        }

        return result;
    }
};

} // namespace ful

#endif // !FUL_SRC_LIB_CORE_SCHEDULER_HPP
