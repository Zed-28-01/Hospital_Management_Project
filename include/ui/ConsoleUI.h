#pragma once

#include "HMSFacade.h"
#include "InputValidator.h"
#include "DisplayHelper.h"
#include "../common/Types.h"
#include <string>

namespace HMS {
namespace UI {

/**
 * @class ConsoleUI
 * @brief Main console user interface for the HMS
 *
 * Handles all user interactions through the console.
 * Uses HMSFacade for all business operations, InputValidator
 * for input validation, and DisplayHelper for output formatting.
 */
class ConsoleUI {
private:
    HMSFacade* m_facade;
    bool m_isRunning;

public:
    // ==================== Constructor/Destructor ====================

    /**
     * @brief Constructor
     */
    ConsoleUI();

    /**
     * @brief Destructor
     */
    ~ConsoleUI();

    // ==================== Main Loop ====================

    /**
     * @brief Run the main application loop
     *
     * Entry point for the console application.
     * Continues until user exits.
     */
    void run();

    /**
     * @brief Stop the application
     */
    void stop();

private:
    // ==================== Menu Displays ====================

    /**
     * @brief Show the main menu (login/register/exit)
     */
    void showMainMenu();

    /**
     * @brief Show the login screen
     */
    void showLoginScreen();

    /**
     * @brief Show the registration screen
     */
    void showRegisterScreen();

    /**
     * @brief Show the patient menu
     */
    void showPatientMenu();

    /**
     * @brief Show the doctor menu
     */
    void showDoctorMenu();

    /**
     * @brief Show the admin menu
     */
    void showAdminMenu();

    // ==================== Menu Handlers ====================

    /**
     * @brief Handle main menu choice
     * @param choice User's choice
     */
    void handleMainMenuChoice(int choice);

    /**
     * @brief Handle patient menu choice
     * @param choice User's choice
     */
    void handlePatientChoice(int choice);

    /**
     * @brief Handle doctor menu choice
     * @param choice User's choice
     */
    void handleDoctorChoice(int choice);

    /**
     * @brief Handle admin menu choice
     * @param choice User's choice
     */
    void handleAdminChoice(int choice);

    // ==================== Patient Operations ====================

    /**
     * @brief Book a new appointment
     */
    void bookAppointment();

    /**
     * @brief View patient's appointments
     */
    void viewAppointments();

    /**
     * @brief View patient's upcoming appointments
     */
    void viewUpcomingAppointments();

    /**
     * @brief Cancel an appointment
     */
    void cancelAppointment();

    /**
     * @brief View patient's total bill
     */
    void viewTotalBill();

    /**
     * @brief View patient profile
     */
    void viewMyProfile();

    // ==================== Doctor Operations ====================

    /**
     * @brief Add a new patient
     */
    void addPatient();

    /**
     * @brief Search for a patient
     */
    void searchPatient();

    /**
     * @brief Edit patient information
     */
    void editPatient();

    /**
     * @brief Delete a patient
     */
    void deletePatient();

    /**
     * @brief List all patients
     */
    void listAllPatients();

    /**
     * @brief View doctor's schedule
     */
    void viewDoctorSchedule();

    /**
     * @brief Mark appointment as completed
     */
    void markAppointmentComplete();

    // ==================== Admin Operations ====================

    /**
     * @brief Add a new doctor
     */
    void addDoctor();

    /**
     * @brief Search for a doctor
     */
    void searchDoctor();

    /**
     * @brief Edit doctor information
     */
    void editDoctor();

    /**
     * @brief Delete a doctor
     */
    void deleteDoctor();

    /**
     * @brief List all doctors
     */
    void listAllDoctors();

    /**
     * @brief View system statistics
     */
    void viewStatistics();

    /**
     * @brief Generate and view reports
     */
    void viewReports();

    // ==================== Helper Methods ====================

    /**
     * @brief Display doctor selection list
     * @return Selected doctor ID or empty string if cancelled
     */
    std::string selectDoctor();

    /**
     * @brief Display patient selection list
     * @return Selected patient ID or empty string if cancelled
     */
    std::string selectPatient();

    /**
     * @brief Display appointment selection list
     * @param appointments List of appointments to choose from
     * @return Selected appointment ID or empty string if cancelled
     */
    std::string selectAppointment(const std::vector<Model::Appointment>& appointments);

    /**
     * @brief Display date selection
     * @return Selected date in YYYY-MM-DD format
     */
    std::string selectDate();

    /**
     * @brief Display time slot selection
     * @param slots Available time slots
     * @return Selected time or empty string if cancelled
     */
    std::string selectTimeSlot(const std::vector<std::string>& slots);

    /**
     * @brief Route user to appropriate menu based on role
     */
    void routeToRoleMenu();

    /**
     * @brief Perform logout
     */
    void performLogout();
};

} // namespace UI
} // namespace HMS
