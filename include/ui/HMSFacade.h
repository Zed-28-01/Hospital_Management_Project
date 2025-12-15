#pragma once

#include "../bll/AuthService.h"
#include "../bll/PatientService.h"
#include "../bll/DoctorService.h"
#include "../bll/AppointmentService.h"
#include "../bll/AdminService.h"
#include "../model/Patient.h"
#include "../model/Doctor.h"
#include "../model/Appointment.h"
#include "../model/Statistics.h"
#include "../common/Types.h"
#include <string>
#include <vector>
#include <optional>
#include <mutex>

namespace HMS {
namespace UI {

/**
 * @class HMSFacade
 * @brief Facade pattern implementation for Hospital Management System
 *
 * Provides a simplified, unified interface to the complex subsystem
 * of services. ConsoleUI interacts only with this facade, which
 * coordinates calls to appropriate services.
 *
 * Implements Singleton pattern.
 */
class HMSFacade {
private:
    // ==================== Singleton ====================
    static HMSFacade* s_instance;
    static std::mutex s_mutex;

    // ==================== Services ====================
    BLL::AuthService* m_authService;
    BLL::PatientService* m_patientService;
    BLL::DoctorService* m_doctorService;
    BLL::AppointmentService* m_appointmentService;
    BLL::AdminService* m_adminService;

    // ==================== State ====================
    bool m_isInitialized;

    // ==================== Private Constructor ====================
    HMSFacade();

public:
    // ==================== Singleton Access ====================

    /**
     * @brief Get the singleton instance
     * @return Pointer to the singleton instance
     */
    static HMSFacade* getInstance();

    /**
     * @brief Delete copy constructor
     */
    HMSFacade(const HMSFacade&) = delete;

    /**
     * @brief Delete assignment operator
     */
    HMSFacade& operator=(const HMSFacade&) = delete;

    /**
     * @brief Destructor
     */
    ~HMSFacade();

    // ==================== System Lifecycle ====================

    /**
     * @brief Initialize the system
     *
     * Loads all data from files and prepares services.
     * Must be called before any other operations.
     *
     * @return True if initialization successful
     */
    bool initialize();

    /**
     * @brief Shutdown the system
     *
     * Saves all data and cleans up resources.
     * Should be called before exiting the application.
     */
    void shutdown();

    /**
     * @brief Check if system is initialized
     * @return True if initialized
     */
    bool isInitialized() const;

    // ==================== Authentication ====================

    /**
     * @brief Login with credentials
     * @param username The username
     * @param password The password
     * @return True if login successful
     */
    bool login(const std::string& username, const std::string& password);

    /**
     * @brief Logout current user
     */
    void logout();

    /**
     * @brief Check if user is logged in
     * @return True if logged in
     */
    bool isLoggedIn() const;

    /**
     * @brief Get current user's username
     * @return Username or empty string
     */
    std::string getCurrentUsername() const;

    /**
     * @brief Get current user's role
     * @return Role enum value
     */
    Role getCurrentRole() const;

    // ==================== Registration ====================

    /**
     * @brief Register a new patient account
     * @param username The username
     * @param password The password
     * @param name Patient's name
     * @param phone Phone number
     * @param gender Gender string
     * @param dateOfBirth Date of birth
     * @param address Address
     * @return True if registration successful
     */
    bool registerPatient(const std::string& username,
                         const std::string& password,
                         const std::string& name,
                         const std::string& phone,
                         const std::string& gender,
                         const std::string& dateOfBirth,
                         const std::string& address);

    /**
     * @brief Check if username is available
     * @param username The username to check
     * @return True if available
     */
    bool isUsernameAvailable(const std::string& username);

    // ==================== Patient Operations ====================

    /**
     * @brief Get current patient's profile
     * @return Patient if logged in as patient, nullopt otherwise
     */
    std::optional<Model::Patient> getMyProfile();

    /**
     * @brief Update current patient's profile
     * @param phone New phone (empty to keep existing)
     * @param address New address (empty to keep existing)
     * @return True if successful
     */
    bool updateMyProfile(const std::string& phone, const std::string& address);

    /**
     * @brief Get current patient's appointments
     * @return Vector of all appointments
     */
    std::vector<Model::Appointment> getMyAppointments();

    /**
     * @brief Get current patient's upcoming appointments
     * @return Vector of upcoming appointments
     */
    std::vector<Model::Appointment> getMyUpcomingAppointments();

    /**
     * @brief Get current patient's total bill
     * @return Total unpaid amount
     */
    double getMyTotalBill();

    /**
     * @brief Book an appointment
     * @param doctorID Doctor's ID
     * @param date Date (YYYY-MM-DD)
     * @param time Time (HH:MM)
     * @param disease Disease/symptoms description
     * @return True if booking successful
     */
    bool bookAppointment(const std::string& doctorID,
                         const std::string& date,
                         const std::string& time,
                         const std::string& disease);

    /**
     * @brief Cancel an appointment
     * @param appointmentID The appointment ID
     * @return True if cancellation successful
     */
    bool cancelAppointment(const std::string& appointmentID);

    // ==================== Doctor Operations ====================

    /**
     * @brief Get all doctors
     * @return Vector of all doctors
     */
    std::vector<Model::Doctor> getAllDoctors();

    /**
     * @brief Get doctors by specialization
     * @param specialization The specialization to filter by
     * @return Vector of matching doctors
     */
    std::vector<Model::Doctor> getDoctorsBySpecialization(const std::string& specialization);

    /**
     * @brief Get all specializations
     * @return Vector of specialization names
     */
    std::vector<std::string> getAllSpecializations();

    /**
     * @brief Get doctor by ID
     * @param doctorID The doctor's ID
     * @return Doctor if found, nullopt otherwise
     */
    std::optional<Model::Doctor> getDoctorByID(const std::string& doctorID);

    /**
     * @brief Get available slots for a doctor
     * @param doctorID Doctor's ID
     * @param date Date (YYYY-MM-DD)
     * @return Vector of available time slots
     */
    std::vector<std::string> getAvailableSlots(const std::string& doctorID,
                                                const std::string& date);

    // ==================== Doctor's Patient Management ====================

    /**
     * @brief Get all patients (for doctor/admin)
     * @return Vector of all patients
     */
    std::vector<Model::Patient> getAllPatients();

    /**
     * @brief Search patients by keyword
     * @param keyword Search keyword
     * @return Vector of matching patients
     */
    std::vector<Model::Patient> searchPatients(const std::string& keyword);

    /**
     * @brief Get patient by ID
     * @param patientID Patient's ID
     * @return Patient if found, nullopt otherwise
     */
    std::optional<Model::Patient> getPatientByID(const std::string& patientID);

    /**
     * @brief Add a new patient (for doctor)
     * @param name Patient's name
     * @param phone Phone number
     * @param gender Gender string
     * @param dateOfBirth Date of birth
     * @param address Address
     * @param medicalHistory Initial medical history
     * @return True if successful
     */
    bool addPatient(const std::string& name,
                    const std::string& phone,
                    const std::string& gender,
                    const std::string& dateOfBirth,
                    const std::string& address,
                    const std::string& medicalHistory);

    /**
     * @brief Update patient info (for doctor)
     * @param patientID Patient's ID
     * @param phone New phone
     * @param address New address
     * @param medicalHistory New medical history
     * @return True if successful
     */
    bool updatePatient(const std::string& patientID,
                       const std::string& phone,
                       const std::string& address,
                       const std::string& medicalHistory);

    /**
     * @brief Delete a patient (for doctor)
     * @param patientID Patient's ID
     * @return True if successful
     */
    bool deletePatient(const std::string& patientID);

    /**
     * @brief Get doctor's schedule for a date
     * @param date Date (YYYY-MM-DD)
     * @return Vector of appointments
     */
    std::vector<Model::Appointment> getMySchedule(const std::string& date);

    /**
     * @brief Mark appointment as completed
     * @param appointmentID Appointment ID
     * @return True if successful
     */
    bool markAppointmentCompleted(const std::string& appointmentID);

    // ==================== Admin Operations ====================

    /**
     * @brief Add a new doctor
     * @param username Account username
     * @param password Account password
     * @param name Doctor's name
     * @param phone Phone number
     * @param gender Gender string
     * @param dateOfBirth Date of birth
     * @param specialization Medical specialization
     * @param schedule Working schedule
     * @param consultationFee Consultation fee
     * @return True if successful
     */
    bool addDoctor(const std::string& username,
                   const std::string& password,
                   const std::string& name,
                   const std::string& phone,
                   const std::string& gender,
                   const std::string& dateOfBirth,
                   const std::string& specialization,
                   const std::string& schedule,
                   double consultationFee);

    /**
     * @brief Update doctor info
     * @param doctorID Doctor's ID
     * @param specialization New specialization
     * @param schedule New schedule
     * @param consultationFee New fee
     * @return True if successful
     */
    bool updateDoctor(const std::string& doctorID,
                      const std::string& specialization,
                      const std::string& schedule,
                      double consultationFee);

    /**
     * @brief Delete a doctor
     * @param doctorID Doctor's ID
     * @return True if successful
     */
    bool deleteDoctor(const std::string& doctorID);

    /**
     * @brief Get system statistics
     * @return Statistics object
     */
    Model::Statistics getStatistics();

    /**
     * @brief Generate summary report
     * @return Report string
     */
    std::string generateReport();

    // ==================== Data Operations ====================

    /**
     * @brief Save all data
     * @return True if successful
     */
    bool saveData();

    /**
     * @brief Load all data
     * @return True if successful
     */
    bool loadData();
};

} // namespace UI
} // namespace HMS
