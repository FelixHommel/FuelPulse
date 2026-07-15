#ifndef FUL_SRC_APP_APPLICATION_HPP
#define FUL_SRC_APP_APPLICATION_HPP

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
};

} // namespace ful::app

#endif // !FUL_SRC_APP_APPLICATION_HPP
