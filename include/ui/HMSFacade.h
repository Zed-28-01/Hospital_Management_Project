#pragma once

#include "../bll/AuthService.h"
#include "../bll/PatientService.h"
#include "../bll/DoctorService.h"
#include "../bll/AppointmentService.h"
#include "../bll/AdminService.h"
#include "../bll/MedicineService.h"
#include "../bll/DepartmentService.h"
#include "../bll/PrescriptionService.h"
#include "../advance/ReportGenerator.h"
#include "../model/Patient.h"
#include "../model/Doctor.h"
#include "../model/Appointment.h"
#include "../model/Statistics.h"
#include "../advance/Medicine.h"
#include "../advance/Department.h"
#include "../advance/Prescription.h"
#include "../common/Types.h"
#include <string>
#include <vector>
#include <optional>
#include <mutex>
#include <memory>

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
    static std::unique_ptr<HMSFacade> s_instance;
    static std::mutex s_mutex;

    // ==================== Services ====================
    BLL::AuthService* m_authService;
    BLL::PatientService* m_patientService;
    BLL::DoctorService* m_doctorService;
    BLL::AppointmentService* m_appointmentService;
    BLL::AdminService* m_adminService;
    BLL::MedicineService* m_medicineService;
    BLL::DepartmentService* m_departmentService;
    BLL::PrescriptionService* m_prescriptionService;
    BLL::ReportGenerator* m_reportGenerator;

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
     * @brief Reset the singleton instance (for testing)
     */
    static void resetInstance();

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

    // ==================== Medicine Management ====================

    /**
     * @brief Get all medicines
     * @return Vector of all medicines
     */
    std::vector<Model::Medicine> getAllMedicines();

    /**
     * @brief Get medicine by ID
     * @param medicineID Medicine's ID
     * @return Medicine if found, nullopt otherwise
     */
    std::optional<Model::Medicine> getMedicineByID(const std::string& medicineID);

    /**
     * @brief Search medicines by keyword
     * @param keyword Search keyword (name, category, etc.)
     * @return Vector of matching medicines
     */
    std::vector<Model::Medicine> searchMedicines(const std::string& keyword);

    /**
     * @brief Add a new medicine
     * @param medicineID Medicine ID (e.g., MED001)
     * @param name Brand/trade name
     * @param genericName Generic name
     * @param category Category
     * @param manufacturer Manufacturer name
     * @param description Description
     * @param unitPrice Price per unit
     * @param quantityInStock Initial stock
     * @param reorderLevel Reorder alert level
     * @param expiryDate Expiry date (YYYY-MM-DD)
     * @param dosageForm Dosage form (e.g., Tablet)
     * @param strength Strength (e.g., 500mg)
     * @return True if successful
     */
    bool createMedicine(const std::string& medicineID,
                        const std::string& name,
                        const std::string& genericName,
                        const std::string& category,
                        const std::string& manufacturer,
                        const std::string& description,
                        double unitPrice,
                        int quantityInStock,
                        int reorderLevel,
                        const std::string& expiryDate,
                        const std::string& dosageForm,
                        const std::string& strength);

    /**
     * @brief Update medicine information
     * @param medicineID Medicine ID
     * @param name New name
     * @param category New category
     * @param unitPrice New unit price
     * @param reorderLevel New reorder level
     * @param expiryDate New expiry date
     * @return True if successful
     */
    bool updateMedicine(const std::string& medicineID,
                        const std::string& name,
                        const std::string& category,
                        double unitPrice,
                        int reorderLevel,
                        const std::string& expiryDate);

    /**
     * @brief Delete a medicine
     * @param medicineID Medicine ID
     * @return True if successful
     */
    bool deleteMedicine(const std::string& medicineID);

    /**
     * @brief Add stock to a medicine
     * @param medicineID Medicine ID
     * @param quantity Quantity to add
     * @return True if successful
     */
    bool addMedicineStock(const std::string& medicineID, int quantity);

    /**
     * @brief Remove stock from a medicine
     * @param medicineID Medicine ID
     * @param quantity Quantity to remove
     * @return True if successful
     */
    bool removeMedicineStock(const std::string& medicineID, int quantity);

    /**
     * @brief Get low stock alerts
     * @return Vector of stock alerts
     */
    std::vector<BLL::StockAlert> getLowStockAlerts();

    /**
     * @brief Get expiry alerts
     * @return Vector of expiry alerts
     */
    std::vector<BLL::ExpiryAlert> getExpiryAlerts();

    // ==================== Department Management ====================

    /**
     * @brief Get all departments
     * @return Vector of all departments
     */
    std::vector<Model::Department> getAllDepartments();

    /**
     * @brief Get department by ID
     * @param departmentID Department ID
     * @return Department if found, nullopt otherwise
     */
    std::optional<Model::Department> getDepartmentByID(const std::string& departmentID);

    /**
     * @brief Create a new department
     * @param departmentID Department ID (e.g., DEP001)
     * @param name Department name
     * @param description Description
     * @param headDoctorID Head doctor ID (optional)
     * @param location Location
     * @param contactNumber Contact phone
     * @return True if successful
     */
    bool createDepartment(const std::string& departmentID,
                          const std::string& name,
                          const std::string& description,
                          const std::string& headDoctorID,
                          const std::string& location,
                          const std::string& contactNumber);

    /**
     * @brief Update department information
     * @param departmentID Department ID
     * @param name New name
     * @param description New description
     * @param headDoctorID New head doctor ID
     * @param location New location
     * @param contactNumber New contact number
     * @return True if successful
     */
    bool updateDepartment(const std::string& departmentID,
                          const std::string& name,
                          const std::string& description,
                          const std::string& headDoctorID,
                          const std::string& location,
                          const std::string& contactNumber);

    /**
     * @brief Delete a department
     * @param departmentID Department ID
     * @return True if successful
     */
    bool deleteDepartment(const std::string& departmentID);

    /**
     * @brief Assign a doctor to a department
     * @param doctorID Doctor ID
     * @param departmentID Department ID
     * @return True if successful
     */
    bool assignDoctorToDepartment(const std::string& doctorID,
                                   const std::string& departmentID);

    /**
     * @brief Get department statistics
     * @param departmentID Department ID
     * @return Department statistics struct
     */
    BLL::DepartmentStats getDepartmentStats(const std::string& departmentID);

    // ==================== Prescription Management ====================

    /**
     * @brief Create a new prescription
     * @param prescriptionID Prescription ID (e.g., PRE001)
     * @param patientID Patient ID
     * @param doctorID Doctor ID
     * @param appointmentID Appointment ID
     * @param date Prescription date
     * @param diagnosis Diagnosis
     * @param instructions Instructions
     * @return True if successful
     */
    bool createPrescription(const std::string& prescriptionID,
                            const std::string& patientID,
                            const std::string& doctorID,
                            const std::string& appointmentID,
                            const std::string& date,
                            const std::string& diagnosis,
                            const std::string& instructions);

    /**
     * @brief Add item to prescription
     * @param prescriptionID Prescription ID
     * @param medicineID Medicine ID
     * @param dosage Dosage instructions
     * @param duration Duration (e.g., "5 days")
     * @param quantity Quantity to dispense
     * @return True if successful
     */
    bool addPrescriptionItem(const std::string& prescriptionID,
                             const std::string& medicineID,
                             const std::string& dosage,
                             const std::string& duration,
                             int quantity);

    /**
     * @brief Get prescriptions for a patient
     * @param patientID Patient ID
     * @return Vector of prescriptions
     */
    std::vector<Model::Prescription> getPatientPrescriptions(const std::string& patientID);

    /**
     * @brief Get prescriptions created by a doctor
     * @param doctorID Doctor ID
     * @return Vector of prescriptions
     */
    std::vector<Model::Prescription> getDoctorPrescriptions(const std::string& doctorID);

    /**
     * @brief Get prescription by ID
     * @param prescriptionID Prescription ID
     * @return Prescription if found, nullopt otherwise
     */
    std::optional<Model::Prescription> getPrescriptionByID(const std::string& prescriptionID);

    /**
     * @brief Dispense a prescription
     * @param prescriptionID Prescription ID
     * @return True if successful
     */
    bool dispensePrescription(const std::string& prescriptionID);

    // ==================== Report Generation ====================

    /**
     * @brief Generate daily report for a specific date
     * @param date Date (YYYY-MM-DD)
     * @param format Format ("txt", "csv", or "html")
     * @return Report content
     */
    std::string generateDailyReport(const std::string& date, const std::string& format);

    /**
     * @brief Generate weekly report
     * @param startDate Week start date (YYYY-MM-DD)
     * @param format Format ("txt", "csv", or "html")
     * @return Report content
     */
    std::string generateWeeklyReport(const std::string& startDate, const std::string& format);

    /**
     * @brief Generate monthly report
     * @param year Year (YYYY)
     * @param month Month (1-12)
     * @param format Format ("txt", "csv", or "html")
     * @return Report content
     */
    std::string generateMonthlyReport(int year, int month, const std::string& format);

    /**
     * @brief Generate revenue report
     * @param startDate Start date (YYYY-MM-DD)
     * @param endDate End date (YYYY-MM-DD)
     * @param format Format ("txt", "csv", or "html")
     * @return Report content
     */
    std::string generateRevenueReport(const std::string& startDate,
                                       const std::string& endDate,
                                       const std::string& format);

    /**
     * @brief Export report to file
     * @param reportContent Report content
     * @param filename Output filename
     * @param format Format ("txt", "csv", or "html")
     * @return True if successful
     */
    bool exportReport(const std::string& reportContent,
                      const std::string& filename,
                      const std::string& format);

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
