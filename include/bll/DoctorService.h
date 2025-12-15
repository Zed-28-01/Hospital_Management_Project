#pragma once

#include "../dal/DoctorRepository.h"
#include "../dal/AppointmentRepository.h"
#include "../model/Doctor.h"
#include "../model/Appointment.h"
#include "../common/Types.h"
#include <string>
#include <vector>
#include <optional>
#include <mutex>

namespace HMS {
namespace BLL {

/**
 * @class DoctorService
 * @brief Service for doctor-related business logic
 *
 * Implements Singleton pattern. Handles doctor management,
 * schedule management, and activity tracking.
 */
class DoctorService {
private:
    // ==================== Singleton ====================
    static DoctorService* s_instance;
    static std::mutex s_mutex;

    // ==================== Dependencies ====================
    DAL::DoctorRepository* m_doctorRepo;
    DAL::AppointmentRepository* m_appointmentRepo;

    // ==================== Private Constructor ====================
    DoctorService();

public:
    // ==================== Singleton Access ====================

    /**
     * @brief Get the singleton instance
     * @return Pointer to the singleton instance
     */
    static DoctorService* getInstance();

    /**
     * @brief Delete copy constructor
     */
    DoctorService(const DoctorService&) = delete;

    /**
     * @brief Delete assignment operator
     */
    DoctorService& operator=(const DoctorService&) = delete;

    /**
     * @brief Destructor
     */
    ~DoctorService();

    // ==================== CRUD Operations ====================

    /**
     * @brief Create a new doctor record
     * @param doctor The doctor data
     * @return True if successful
     */
    bool createDoctor(const Model::Doctor& doctor);

    /**
     * @brief Create doctor with individual parameters
     * @param username Account username
     * @param name Doctor's name
     * @param phone Phone number
     * @param gender Gender
     * @param dateOfBirth Date of birth
     * @param specialization Medical specialization
     * @param schedule Working schedule
     * @param consultationFee Consultation fee
     * @return Created doctor or nullopt if failed
     */
    std::optional<Model::Doctor> createDoctor(
        const std::string& username,
        const std::string& name,
        const std::string& phone,
        Gender gender,
        const std::string& dateOfBirth,
        const std::string& specialization,
        const std::string& schedule,
        double consultationFee);

    /**
     * @brief Update an existing doctor record
     * @param doctor The doctor with updated data
     * @return True if successful
     */
    bool updateDoctor(const Model::Doctor& doctor);

    /**
     * @brief Delete a doctor record
     * @param doctorID The doctor's ID
     * @return True if successful
     */
    bool deleteDoctor(const std::string& doctorID);

    // ==================== Query Operations ====================

    /**
     * @brief Get doctor by ID
     * @param doctorID The doctor's ID
     * @return Doctor if found, nullopt otherwise
     */
    std::optional<Model::Doctor> getDoctorByID(const std::string& doctorID);

    /**
     * @brief Get doctor by username
     * @param username The account username
     * @return Doctor if found, nullopt otherwise
     */
    std::optional<Model::Doctor> getDoctorByUsername(const std::string& username);

    /**
     * @brief Get all doctors
     * @return Vector of all doctors
     */
    std::vector<Model::Doctor> getAllDoctors();

    /**
     * @brief Search doctors by keyword
     * @param keyword Search keyword
     * @return Vector of matching doctors
     */
    std::vector<Model::Doctor> searchDoctors(const std::string& keyword);

    /**
     * @brief Get doctors by specialization
     * @param specialization The medical specialization
     * @return Vector of matching doctors
     */
    std::vector<Model::Doctor> getDoctorsBySpecialization(const std::string& specialization);

    /**
     * @brief Get all available specializations
     * @return Vector of specialization names
     */
    std::vector<std::string> getAllSpecializations();

    /**
     * @brief Get total doctor count
     * @return Number of doctors
     */
    size_t getDoctorCount() const;

    // ==================== Schedule Management ====================

    /**
     * @brief Get doctor's schedule for a specific date
     * @param doctorID The doctor's ID
     * @param date The date (YYYY-MM-DD)
     * @return Vector of appointments on that date
     */
    std::vector<Model::Appointment> getDoctorSchedule(const std::string& doctorID,
                                                       const std::string& date);

    /**
     * @brief Get doctor's upcoming appointments
     * @param doctorID The doctor's ID
     * @return Vector of upcoming appointments
     */
    std::vector<Model::Appointment> getUpcomingAppointments(const std::string& doctorID);

    /**
     * @brief Get doctor's available time slots for a date
     * @param doctorID The doctor's ID
     * @param date The date (YYYY-MM-DD)
     * @return Vector of available time slots (HH:MM format)
     */
    std::vector<std::string> getAvailableSlots(const std::string& doctorID,
                                                const std::string& date);

    /**
     * @brief Check if a time slot is available
     * @param doctorID The doctor's ID
     * @param date The date (YYYY-MM-DD)
     * @param time The time (HH:MM)
     * @return True if slot is available
     */
    bool isSlotAvailable(const std::string& doctorID,
                         const std::string& date,
                         const std::string& time);

    // ==================== Activity Tracking ====================

    /**
     * @brief Get doctor's activity (all appointments)
     * @param doctorID The doctor's ID
     * @return Vector of all doctor's appointments
     */
    std::vector<Model::Appointment> getDoctorActivity(const std::string& doctorID);

    /**
     * @brief Get doctor's completed appointments
     * @param doctorID The doctor's ID
     * @return Vector of completed appointments
     */
    std::vector<Model::Appointment> getCompletedAppointments(const std::string& doctorID);

    /**
     * @brief Get doctor's appointments in date range
     * @param doctorID The doctor's ID
     * @param startDate Start date
     * @param endDate End date
     * @return Vector of appointments in range
     */
    std::vector<Model::Appointment> getAppointmentsInRange(const std::string& doctorID,
                                                            const std::string& startDate,
                                                            const std::string& endDate);

    /**
     * @brief Get today's appointments for a doctor
     * @param doctorID The doctor's ID
     * @return Vector of today's appointments
     */
    std::vector<Model::Appointment> getTodayAppointments(const std::string& doctorID);

    // ==================== Statistics ====================

    /**
     * @brief Get doctor's total revenue
     * @param doctorID The doctor's ID
     * @return Total revenue from completed appointments
     */
    double getDoctorRevenue(const std::string& doctorID);

    /**
     * @brief Get doctor's appointment count
     * @param doctorID The doctor's ID
     * @return Total number of appointments
     */
    size_t getDoctorAppointmentCount(const std::string& doctorID);

    /**
     * @brief Get doctor's patient count (unique patients)
     * @param doctorID The doctor's ID
     * @return Number of unique patients
     */
    size_t getDoctorPatientCount(const std::string& doctorID);

    // ==================== Validation ====================

    /**
     * @brief Validate doctor data
     * @param doctor The doctor to validate
     * @return True if valid
     */
    bool validateDoctor(const Model::Doctor& doctor);

    /**
     * @brief Check if doctor ID exists
     * @param doctorID The doctor ID to check
     * @return True if exists
     */
    bool doctorExists(const std::string& doctorID);

    // ==================== Data Persistence ====================

    /**
     * @brief Save doctor data
     * @return True if successful
     */
    bool saveData();

    /**
     * @brief Load doctor data
     * @return True if successful
     */
    bool loadData();
};

} // namespace BLL
} // namespace HMS
