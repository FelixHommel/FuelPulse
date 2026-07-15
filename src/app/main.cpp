#include "Application.hpp"

#include <exception>
#include <iostream>
#include <memory>

int main()
{
    using namespace gas::app;

    std::unique_ptr<Application> app;

    try
    {
        app = std::make_unique<Application>();
    }
    catch(const std::exception& e)
    {
        std::cout << "Failed to create the Application: " << e.what();
        return 0;
    }

    return 0;
}
