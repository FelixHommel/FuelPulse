#ifndef FUL_SRC_LIB_UTILITY_THREADING_SEMAPHORE_RELEASE_GUARD_HPP
#define FUL_SRC_LIB_UTILITY_THREADING_SEMAPHORE_RELEASE_GUARD_HPP

#include <concepts>

namespace ful
{

/// \brief Concept that specifies what can be used as a semaphore in \ref SemaphoreReleaseGuard.
///
/// \tparam The semaphore type
///
/// \author Felix Hommel
/// \date 8/14/2026
template<typename T>
concept Semaphore = requires(T& t) {
    { t.release() } -> std::same_as<void>;
};

/// \brief A RAII-style wrapper for semaphores of the standard library (currently \ref std::counting_semaphore and
///     \ref std::binary_semaphore) that releases the semaphore exactly once when the guard goes out of scope unless
///     disarmed prior.
///
/// \author Felix Hommel
/// \date 8/14/2026
template<Semaphore T>
class SemaphoreReleaseGuard
{
public:
    explicit SemaphoreReleaseGuard(T& sem) : m_sem{ &sem } {}
    ~SemaphoreReleaseGuard()
    {
        if(m_sem != nullptr)
            m_sem->release();
    }

    void disarm() { m_sem = nullptr; }

    SemaphoreReleaseGuard(const SemaphoreReleaseGuard&) = delete;
    SemaphoreReleaseGuard& operator=(const SemaphoreReleaseGuard&) = delete;
    SemaphoreReleaseGuard(SemaphoreReleaseGuard&&) = delete;
    SemaphoreReleaseGuard& operator=(SemaphoreReleaseGuard&&) = delete;

private:
    T* m_sem;
};

} // namespace ful

#endif // !FUL_SRC_LIB_UTILITY_THREADING_SEMAPHORE_RELEASE_GUARD_HPP
