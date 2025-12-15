#pragma once

#include "PatientService.h"
#include "DoctorService.h"
#include "AppointmentService.h"
#include "../model/Statistics.h"
#include "../common/Types.h"
#include <string>
#include <vector>
#include <mutex>

namespace HMS {
namespace BLL {

/**
 * @class AdminService
 * @brief Service for admin-related business logic
 *
 * Implements Singleton pattern. Handles administrative operations,
 * statistics generation, and system-wide queries.
 */
class AdminService {
private:
    // ==================== Singleton ====================
    static AdminService* s_instance;
    static std::mutex s_mutex;

    // ==================== Dependencies ====================
    PatientService* m_patientService;
    DoctorService* m_doctorService;
    AppointmentService* m_appointmentService;

    // ==================== Private Constructor ====================
    AdminService();

public:
    // ==================== Singleton Access ====================

    /**
     * @brief Get the singleton instance
     * @return Pointer to the singleton instance
     */
    static AdminService* getInstance();

    /**
     * @brief Delete copy constructor
     */
    AdminService(const AdminService&) = delete;

    /**
     * @brief Delete assignment operator
     */
    AdminService& operator=(const AdminService&) = delete;

    /**
     * @brief Destructor
     */
    ~AdminService();

    // ==================== Statistics ====================

    /**
     * @brief Get comprehensive system statistics
     * @return Statistics object with all metrics
     */
    Model::Statistics getStatistics();

    /**
     * @brief Get total number of patients
     * @return Patient count
     */
    int getTotalPatients();

    /**
     * @brief Get total number of doctors
     * @return Doctor count
     */
    int getTotalDoctors();

    /**
     * @brief Get total number of appointments
     * @return Appointment count
     */
    int getTotalAppointments();

    /**
     * @brief Get total revenue
     * @return Total revenue amount
     */
    double getTotalRevenue();

    /**
     * @brief Get paid revenue
     * @return Paid revenue amount
     */
    double getPaidRevenue();

    /**
     * @brief Get unpaid revenue
     * @return Unpaid revenue amount
     */
    double getUnpaidRevenue();

    // ==================== Status-Based Statistics ====================

    /**
     * @brief Get count of scheduled appointments
     * @return Scheduled appointment count
     */
    int getScheduledAppointmentsCount();

    /**
     * @brief Get count of completed appointments
     * @return Completed appointment count
     */
    int getCompletedAppointmentsCount();

    /**
     * @brief Get count of cancelled appointments
     * @return Cancelled appointment count
     */
    int getCancelledAppointmentsCount();

    /**
     * @brief Get count of no-show appointments
     * @return No-show appointment count
     */
    int getNoShowAppointmentsCount();

    // ==================== Date-Based Queries ====================

    /**
     * @brief Get appointments for today
     * @return Vector of today's appointments
     */
    std::vector<Model::Appointment> getAppointmentsToday();

    /**
     * @brief Get appointments for this week
     * @return Vector of this week's appointments
     */
    std::vector<Model::Appointment> getAppointmentsThisWeek();

    /**
     * @brief Get appointments for this month
     * @return Vector of this month's appointments
     */
    std::vector<Model::Appointment> getAppointmentsThisMonth();

    /**
     * @brief Get appointments in date range
     * @param startDate Start date (YYYY-MM-DD)
     * @param endDate End date (YYYY-MM-DD)
     * @return Vector of appointments in range
     */
    std::vector<Model::Appointment> getAppointmentsByDateRange(
        const std::string& startDate,
        const std::string& endDate);

    // ==================== Doctor Statistics ====================

    /**
     * @brief Get doctors by appointment count (sorted)
     * @return Vector of doctors sorted by activity
     */
    std::vector<Model::Doctor> getDoctorsByActivity();

    /**
     * @brief Get doctors by revenue (sorted)
     * @return Vector of doctors sorted by revenue
     */
    std::vector<Model::Doctor> getDoctorsByRevenue();

    /**
     * @brief Get doctor count by specialization
     * @return Map of specialization to count
     */
    std::map<std::string, int> getDoctorCountBySpecialization();

    /**
     * @brief Get appointment count by specialization
     * @return Map of specialization to appointment count
     */
    std::map<std::string, int> getAppointmentCountBySpecialization();

    // ==================== Reports ====================

    /**
     * @brief Generate daily report
     * @param date The date (YYYY-MM-DD)
     * @return Report as formatted string
     */
    std::string generateDailyReport(const std::string& date);

    /**
     * @brief Generate weekly report
     * @param startDate Start date of the week
     * @return Report as formatted string
     */
    std::string generateWeeklyReport(const std::string& startDate);

    /**
     * @brief Generate monthly report
     * @param month Month (1-12)
     * @param year Year
     * @return Report as formatted string
     */
    std::string generateMonthlyReport(int month, int year);

    /**
     * @brief Generate summary report
     * @return Summary report as formatted string
     */
    std::string generateSummaryReport();

    // ==================== Data Management ====================

    /**
     * @brief Save all data to files
     * @return True if successful
     */
    bool saveAllData();

    /**
     * @brief Load all data from files
     * @return True if successful
     */
    bool loadAllData();

    /**
     * @brief Create backup of all data files
     * @return True if successful
     */
    bool createBackup();

    /**
     * @brief Restore from backup
     * @return True if successful
     */
    bool restoreFromBackup();

    // ==================== System Health ====================

    /**
     * @brief Check system health
     * @return True if all systems operational
     */
    bool checkSystemHealth();

    /**
     * @brief Get system status report
     * @return Status report string
     */
    std::string getSystemStatus();
};

} // namespace BLL
} // namespace HMS
