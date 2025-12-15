#pragma once

#include <string>
#include <map>

namespace HMS {
namespace Model {

/**
 * @struct Statistics
 * @brief Data structure for system statistics
 *
 * Used by AdminService to aggregate and report
 * system-wide metrics.
 */
struct Statistics {
    // ==================== Counts ====================
    int totalPatients = 0;
    int totalDoctors = 0;
    int totalAppointments = 0;

    // ==================== Appointment Status Breakdown ====================
    int scheduledAppointments = 0;
    int completedAppointments = 0;
    int cancelledAppointments = 0;
    int noShowAppointments = 0;

    // ==================== Financial Metrics ====================
    double totalRevenue = 0.0;          // All appointments
    double paidRevenue = 0.0;           // Paid appointments only
    double unpaidRevenue = 0.0;         // Unpaid appointments
    double averageConsultationFee = 0.0;

    // ==================== Time-based Metrics ====================
    int appointmentsToday = 0;
    int appointmentsThisWeek = 0;
    int appointmentsThisMonth = 0;

    // ==================== Specialization Distribution ====================
    std::map<std::string, int> doctorsBySpecialization;
    std::map<std::string, int> appointmentsBySpecialization;

    // ==================== Methods ====================

    /**
     * @brief Display statistics to console
     */
    void display() const;

    /**
     * @brief Reset all statistics to zero
     */
    void reset();

    /**
     * @brief Calculate derived metrics
     *
     * Call after setting raw counts to compute
     * averages and percentages.
     */
    void calculate();

    /**
     * @brief Get completion rate as percentage
     * @return Completion rate (0-100)
     */
    double getCompletionRate() const;

    /**
     * @brief Get cancellation rate as percentage
     * @return Cancellation rate (0-100)
     */
    double getCancellationRate() const;

    /**
     * @brief Get payment rate as percentage
     * @return Payment rate (0-100)
     */
    double getPaymentRate() const;

    /**
     * @brief Export statistics to formatted string
     * @return Formatted statistics report
     */
    std::string toReport() const;
};

} // namespace Model
} // namespace HMS
