#pragma once

#include "HMSFacade.h"
#include "InputValidator.h"
#include "DisplayHelper.h"
#include "../common/Types.h"
#include <string>
#include <vector>

namespace HMS {
namespace Model {
    class Appointment;
    class Prescription;
}
}

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

    // ==================== Medicine Management (Admin) ====================

    /**
     * @brief View all medicines
     */
    void viewMedicines();

    /**
     * @brief Add a new medicine
     */
    void addMedicine();

    /**
     * @brief Edit medicine information
     */
    void editMedicine();

    /**
     * @brief Delete a medicine
     */
    void deleteMedicine();

    /**
     * @brief Search for medicines
     */
    void searchMedicine();

    /**
     * @brief Update medicine stock
     */
    void updateMedicineStock();

    /**
     * @brief View low stock alerts
     */
    void viewLowStockAlerts();

    /**
     * @brief View expiry alerts
     */
    void viewExpiryAlerts();

    // ==================== Department Management (Admin) ====================

    /**
     * @brief View all departments
     */
    void viewDepartments();

    /**
     * @brief Add a new department
     */
    void addDepartment();

    /**
     * @brief Edit department information
     */
    void editDepartment();

    /**
     * @brief Delete a department
     */
    void deleteDepartment();

    /**
     * @brief Assign doctor to department
     */
    void assignDoctorToDepartment();

    /**
     * @brief View department statistics
     */
    void viewDepartmentStats();

    // ==================== Prescription Management (Doctor) ====================

    /**
     * @brief Create a new prescription
     */
    void createPrescription();

    /**
     * @brief View prescriptions
     */
    void viewPrescriptions();

    /**
     * @brief Dispense a prescription
     */
    void dispensePrescription();

    /**
     * @brief Print prescription details
     */
    void printPrescription();

    // ==================== Report Generation (Admin) ====================

    /**
     * @brief Generate daily report
     */
    void generateDailyReport();

    /**
     * @brief Generate weekly report
     */
    void generateWeeklyReport();

    /**
     * @brief Generate monthly report
     */
    void generateMonthlyReport();

    /**
     * @brief Generate revenue report
     */
    void generateRevenueReport();

    /**
     * @brief Export report to file
     */
    void exportReport();

    // ==================== Sub-menus ====================

    /**
     * @brief Show medicine management sub-menu (Admin)
     */
    void showMedicineMenu();

    /**
     * @brief Show department management sub-menu (Admin)
     */
    void showDepartmentMenu();

    /**
     * @brief Show report generation sub-menu (Admin)
     */
    void showReportMenu();

    /**
     * @brief Show prescription management sub-menu (Doctor)
     */
    void showPrescriptionMenu();

    // ==================== Patient View-Only Features ====================

    /**
     * @brief View patient's prescriptions (Patient)
     */
    void viewMyPrescriptions();

    /**
     * @brief Search medicines by patient (view-only)
     */
    void searchMedicinePatient();

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
     * @brief Display date selection (future dates only)
     * @return Selected date in YYYY-MM-DD format
     */
    std::string selectDate();

    /**
     * @brief Display date selection (past dates only, for reports)
     * @return Selected date in YYYY-MM-DD format
     */
    std::string selectPastDate();

    /**
     * @brief Display time slot selection
     * @param slots Available time slots
     * @return Selected time or empty string if cancelled
     */
    std::string selectTimeSlot(const std::vector<std::string>& slots);

    /**
     * @brief Display medicine selection list
     * @return Selected medicine ID or empty string if cancelled
     */
    std::string selectMedicine();

    /**
     * @brief Display department selection list
     * @return Selected department ID or empty string if cancelled
     */
    std::string selectDepartment();

    /**
     * @brief Display prescription selection list
     * @return Selected prescription ID or empty string if cancelled
     */
    std::string selectPrescription();

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
