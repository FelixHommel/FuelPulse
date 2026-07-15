#ifndef FUL_SRC_APP_APPLICATION_HPP
#define FUL_SRC_APP_APPLICATION_HPP

#include "analysis/IAnalysis.hpp"
#include "collection/ICollector.hpp"

#include <memory>

namespace ful::app
{

class Application
{
public:
    Application() = default;
    ~Application() = default;

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) = default;
    Application& operator=(Application&&) = default;

private:
    std::unique_ptr<ICollector> m_collector;
    std::unique_ptr<IAnalysis> m_analyzer;
};

} // namespace ful::app

#endif // !FUL_SRC_APP_APPLICATION_HPP
