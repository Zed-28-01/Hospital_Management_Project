/**
 * @file main.cpp
 * @brief Main entry point for the Hospital Management System
 *
 * This file initializes the system, loads data, and starts
 * the console user interface.
 */

#include "ui/HMSFacade.h"
#include "ui/ConsoleUI.h"

#include <iostream>
#include <exception>

int main()
{
    try
    {
        // Get the facade instance
        HMS::UI::HMSFacade *facade = HMS::UI::HMSFacade::getInstance();

        // Initialize the system (loads all data)
        if (!facade->initialize())
        {
            std::cerr << "Error: Failed to initialize the system." << std::endl;
            return 1;
        }

        // Create and run the console UI
        HMS::UI::ConsoleUI ui;
        ui.run();

        // Shutdown the system (saves all data)
        facade->shutdown();

        std::cout << "\nThank you for using Hospital Management System!" << std::endl;
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "Unknown fatal error occurred." << std::endl;
        return 1;
    }
}
