#pragma once

#include "../model/Patient.h"
#include "../model/Doctor.h"
#include "../model/Appointment.h"
#include "../model/Statistics.h"
#include "../advance/Medicine.h"
#include "../advance/Department.h"
#include "../advance/Prescription.h"
#include "../bll/MedicineService.h"
#include "../common/Types.h"
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>

namespace HMS {
namespace UI {

/**
 * @class DisplayHelper
 * @brief Utility class for console output formatting
 *
 * Provides static methods for displaying formatted output
 * including headers, menus, tables, and entity information.
 */
class DisplayHelper {
public:
    // ==================== Screen Control ====================

    /**
     * @brief Clear the console screen
     */
    static void clearScreen();

    /**
     * @brief Pause and wait for user to press Enter
     */
    static void pause();

    /**
     * @brief Pause with custom message
     * @param message Message to display before waiting
     */
    static void pause(const std::string& message);

    // ==================== Headers & Titles ====================

    /**
     * @brief Print application header
     */
    static void printAppHeader();

    /**
     * @brief Print a section header
     * @param title Section title
     */
    static void printHeader(const std::string& title);

    /**
     * @brief Print a sub-header
     * @param title Sub-header title
     */
    static void printSubHeader(const std::string& title);

    /**
     * @brief Print a separator line
     * @param width Line width (default 60)
     * @param ch Character to use (default '=')
     */
    static void printSeparator(int width = 60, char ch = '=');

    /**
     * @brief Print a thin separator line
     * @param width Line width (default 60)
     */
    static void printThinSeparator(int width = 60);

    // ==================== Menu Display ====================

    /**
     * @brief Print a menu with numbered options
     * @param title Menu title
     * @param options Vector of option strings
     */
    static void printMenu(const std::string& title,
                          const std::vector<std::string>& options);

    /**
     * @brief Print main menu
     */
    static void printMainMenu();

    /**
     * @brief Print patient menu
     * @param username Current user's name
     */
    static void printPatientMenu(const std::string& username);

    /**
     * @brief Print doctor menu
     * @param username Current user's name
     */
    static void printDoctorMenu(const std::string& username);

    /**
     * @brief Print admin menu
     * @param username Current user's name
     */
    static void printAdminMenu(const std::string& username);

    // ==================== Input Prompts ====================

    /**
     * @brief Get string input from user
     * @param prompt The prompt message
     * @return User's input
     */
    static std::string getInput(const std::string& prompt);

    /**
     * @brief Get integer input from user
     * @param prompt The prompt message
     * @param min Minimum valid value
     * @param max Maximum valid value
     * @return User's choice or -1 if invalid
     */
    static int getIntInput(const std::string& prompt, int min, int max);

    /**
     * @brief Get double input from user
     * @param prompt The prompt message
     * @return User's input or -1 if invalid
     */
    static double getDoubleInput(const std::string& prompt);

    /**
     * @brief Get yes/no confirmation
     * @param message Confirmation message
     * @return True if user confirms
     */
    static bool confirm(const std::string& message);

    /**
     * @brief Get password input (masked)
     * @param prompt The prompt message
     * @return User's password
     */
    static std::string getPasswordInput(const std::string& prompt);

    // ==================== Entity Display ====================

    /**
     * @brief Print patient information
     * @param patient The patient to display
     */
    static void printPatientInfo(const Model::Patient& patient);

    /**
     * @brief Print doctor information
     * @param doctor The doctor to display
     */
    static void printDoctorInfo(const Model::Doctor& doctor);

    /**
     * @brief Print appointment information
     * @param appointment The appointment to display
     */
    static void printAppointmentInfo(const Model::Appointment& appointment);

    /**
     * @brief Print statistics information
     * @param stats The statistics to display
     */
    static void printStatistics(const Model::Statistics& stats);

    /**
     * @brief Print medicine information
     * @param medicine The medicine to display
     */
    static void printMedicineInfo(const Model::Medicine& medicine);

    /**
     * @brief Print department information
     * @param department The department to display
     */
    static void printDepartmentInfo(const Model::Department& department);

    /**
     * @brief Print prescription information
     * @param prescription The prescription to display
     */
    static void printPrescriptionInfo(const Model::Prescription& prescription);

    /**
     * @brief Print stock alert
     * @param alert The stock alert to display
     */
    static void printStockAlert(const BLL::StockAlert& alert);

    /**
     * @brief Print expiry alert
     * @param alert The expiry alert to display
     */
    static void printExpiryAlert(const BLL::ExpiryAlert& alert);

    // ==================== Table Display ====================

    /**
     * @brief Print a table with headers and rows
     * @param headers Column headers
     * @param rows Table rows (each row is a vector of strings)
     * @param colWidths Column widths (empty for auto-width)
     */
    static void printTable(const std::vector<std::string>& headers,
                           const std::vector<std::vector<std::string>>& rows,
                           const std::vector<int>& colWidths = {});

    /**
     * @brief Print patient list as table
     * @param patients Vector of patients
     */
    static void printPatientTable(const std::vector<Model::Patient>& patients);

    /**
     * @brief Print doctor list as table
     * @param doctors Vector of doctors
     */
    static void printDoctorTable(const std::vector<Model::Doctor>& doctors);

    /**
     * @brief Print appointment list as table
     * @param appointments Vector of appointments
     */
    static void printAppointmentTable(const std::vector<Model::Appointment>& appointments);

    /**
     * @brief Print medicine list as table
     * @param medicines Vector of medicines
     */
    static void printMedicineTable(const std::vector<Model::Medicine>& medicines);

    /**
     * @brief Print department list as table
     * @param departments Vector of departments
     */
    static void printDepartmentTable(const std::vector<Model::Department>& departments);

    /**
     * @brief Print prescription list as table
     * @param prescriptions Vector of prescriptions
     */
    static void printPrescriptionTable(const std::vector<Model::Prescription>& prescriptions);

    /**
     * @brief Print stock alerts as table
     * @param alerts Vector of stock alerts
     */
    static void printStockAlertTable(const std::vector<BLL::StockAlert>& alerts);

    /**
     * @brief Print expiry alerts as table
     * @param alerts Vector of expiry alerts
     */
    static void printExpiryAlertTable(const std::vector<BLL::ExpiryAlert>& alerts);

    // ==================== List Display (with numbering) ====================

    /**
     * @brief Print numbered patient list for selection
     * @param patients Vector of patients
     */
    static void printPatientList(const std::vector<Model::Patient>& patients);

    /**
     * @brief Print numbered doctor list for selection
     * @param doctors Vector of doctors
     */
    static void printDoctorList(const std::vector<Model::Doctor>& doctors);

    /**
     * @brief Print numbered appointment list for selection
     * @param appointments Vector of appointments
     */
    static void printAppointmentList(const std::vector<Model::Appointment>& appointments);

    /**
     * @brief Print numbered time slot list for selection
     * @param slots Vector of time slots
     */
    static void printTimeSlotList(const std::vector<std::string>& slots);

    /**
     * @brief Print numbered medicine list for selection
     * @param medicines Vector of medicines
     */
    static void printMedicineList(const std::vector<Model::Medicine>& medicines);

    /**
     * @brief Print numbered department list for selection
     * @param departments Vector of departments
     */
    static void printDepartmentList(const std::vector<Model::Department>& departments);

    /**
     * @brief Print numbered prescription list for selection
     * @param prescriptions Vector of prescriptions
     */
    static void printPrescriptionList(const std::vector<Model::Prescription>& prescriptions);

    // ==================== Menu Display (Advanced Features) ====================

    /**
     * @brief Print medicine management menu
     */
    static void printMedicineMenu();

    /**
     * @brief Print department management menu
     */
    static void printDepartmentMenu();

    /**
     * @brief Print prescription menu
     */
    static void printPrescriptionMenu();

    /**
     * @brief Print report generation menu
     */
    static void printReportMenu();

    // ==================== Messages ====================

    /**
     * @brief Print success message
     * @param message The message to display
     */
    static void printSuccess(const std::string& message);

    /**
     * @brief Print error message
     * @param message The message to display
     */
    static void printError(const std::string& message);

    /**
     * @brief Print warning message
     * @param message The message to display
     */
    static void printWarning(const std::string& message);

    /**
     * @brief Print info message
     * @param message The message to display
     */
    static void printInfo(const std::string& message);

    /**
     * @brief Print "No data found" message
     * @param entityType Type of entity (e.g., "patients", "appointments")
     */
    static void printNoData(const std::string& entityType);

    // ==================== Formatting Utilities ====================

    /**
     * @brief Format money value for display
     * @param amount The amount
     * @return Formatted string (e.g., "500,000 VND")
     */
    static std::string formatMoney(double amount);

    /**
     * @brief Format date for display
     * @param date Date in YYYY-MM-DD format
     * @return Formatted date (e.g., "15 Mar 2024")
     */
    static std::string formatDate(const std::string& date);

    /**
     * @brief Format status for display with color
     * @param status Appointment status
     * @return Formatted status string
     */
    static std::string formatStatus(AppointmentStatus status);

    /**
     * @brief Truncate string to max length
     * @param str The string to truncate
     * @param maxLength Maximum length
     * @return Truncated string with "..." if needed
     */
    static std::string truncate(const std::string& str, size_t maxLength);

    /**
     * @brief Pad string to fixed width
     * @param str The string to pad
     * @param width Target width
     * @param leftAlign Left align if true
     * @return Padded string
     */
    static std::string pad(const std::string& str, size_t width, bool leftAlign = true);

private:
    /**
     * @brief Private constructor to prevent instantiation
     */
    DisplayHelper() = default;

    // ==================== ANSI Color Codes ====================
    static constexpr const char* RESET = "\033[0m";
    static constexpr const char* RED = "\033[31m";
    static constexpr const char* GREEN = "\033[32m";
    static constexpr const char* YELLOW = "\033[33m";
    static constexpr const char* BLUE = "\033[34m";
    static constexpr const char* CYAN = "\033[36m";
    static constexpr const char* BOLD = "\033[1m";
};

} // namespace UI
} // namespace HMS
