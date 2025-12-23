#pragma once

#include "../dal/PrescriptionRepository.h"
#include "../dal/AppointmentRepository.h"
#include "../dal/MedicineRepository.h"
#include "../advance/Prescription.h"
#include "../model/Appointment.h"
#include "../advance/Medicine.h"
#include "../common/Types.h"

#include <string>
#include <mutex>
#include <memory>
#include <map>

namespace HMS {
namespace BLL {

/**
 * @struct DispenseResult
 * @brief Result of dispensing a prescription
 */
struct DispenseResult {
    bool success;
    std::string message;
    double totalCost;
    std::vector<std::string> failedItems;  // Medicine IDs that couldn't be dispensed
};

/**
 * @struct PrescriptionSummary
 * @brief Summary of prescription for display
 */
struct PrescriptionSummary {
    std::string prescriptionID;
    std::string patientName;
    std::string doctorName;
    std::string date;
    std::string diagnosis;
    int itemCount;
    double totalCost;
    bool isDispensed;
};

/**
 * @class PrescriptionService
 * @brief Service for prescription-related business logic
 *
 * Implements Singleton pattern. Handles prescription creation,
 * dispensing, and integration with medicine inventory.
 */
class PrescriptionService {
private:
    // ==================== Singleton ====================
    static std::unique_ptr<PrescriptionService> s_instance;
    static std::mutex s_mutex;

    // ==================== Dependencies ====================
    DAL::PrescriptionRepository* m_prescriptionRepo;
    DAL::AppointmentRepository* m_appointmentRepo;
    DAL::MedicineRepository* m_medicineRepo;

    // ==================== Private Constructor ====================
    PrescriptionService();

public:
    // ==================== Singleton Access ====================

    /**
     * @brief Get the singleton instance
     * @return Pointer to the singleton instance
     */
    static PrescriptionService* getInstance();

    /**
     * @brief Reset the singleton instance (for testing)
     */
    static void resetInstance();

    /**
     * @brief Delete copy constructor
     */
    PrescriptionService(const PrescriptionService&) = delete;

    /**
     * @brief Delete assignment operator
     */
    PrescriptionService& operator=(const PrescriptionService&) = delete;

    /**
     * @brief Destructor
     */
    ~PrescriptionService();

    // ==================== Prescription Creation ====================

    /**
     * @brief Create a new prescription
     * @param prescription The prescription data
     * @return True if successful
     */
    bool createPrescription(const Model::Prescription& prescription);

    /**
     * @brief Create prescription for an appointment
     * @param appointmentID The appointment ID
     * @param diagnosis Diagnosis text
     * @param notes Additional notes
     * @return Created prescription or nullopt if failed
     */
    Result<Model::Prescription> createPrescription(
        const std::string& appointmentID,
        const std::string& diagnosis,
        const std::string& notes);

    /**
     * @brief Update an existing prescription
     * @param prescription The prescription with updated data
     * @return True if successful
     */
    bool updatePrescription(const Model::Prescription& prescription);

    /**
     * @brief Delete a prescription
     * @param prescriptionID The prescription's ID
     * @return True if successful (only if not dispensed)
     */
    bool deletePrescription(const std::string& prescriptionID);

    // ==================== Item Management ====================

    /**
     * @brief Add an item to a prescription
     * @param prescriptionID The prescription's ID
     * @param item The prescription item to add
     * @return True if successful
     */
    bool addPrescriptionItem(const std::string& prescriptionID,
                              const Model::PrescriptionItem& item);

    /**
     * @brief Add an item to a prescription with individual parameters
     * @param prescriptionID The prescription's ID
     * @param medicineID The medicine ID
     * @param quantity Quantity to prescribe
     * @param dosage Dosage instructions
     * @param duration Duration of treatment
     * @param instructions Additional instructions
     * @return True if successful
     */
    bool addPrescriptionItem(
        const std::string& prescriptionID,
        const std::string& medicineID,
        int quantity,
        const std::string& dosage,
        const std::string& duration,
        const std::string& instructions);

    /**
     * @brief Remove an item from a prescription
     * @param prescriptionID The prescription's ID
     * @param medicineID The medicine ID to remove
     * @return True if successful
     */
    bool removePrescriptionItem(const std::string& prescriptionID,
                                 const std::string& medicineID);

    /**
     * @brief Update an item in a prescription
     * @param prescriptionID The prescription's ID
     * @param item The updated prescription item
     * @return True if successful
     */
    bool updatePrescriptionItem(const std::string& prescriptionID,
                                 const Model::PrescriptionItem& item);

    /**
     * @brief Clear all items from a prescription
     * @param prescriptionID The prescription's ID
     * @return True if successful
     */
    bool clearPrescriptionItems(const std::string& prescriptionID);

    // ==================== Query Operations ====================

    /**
     * @brief Get prescription by ID
     * @param prescriptionID The prescription's ID
     * @return Prescription if found, nullopt otherwise
     */
    Result<Model::Prescription> getPrescriptionByID(const std::string& prescriptionID);

    /**
     * @brief Get prescription by appointment ID
     * @param appointmentID The appointment's ID
     * @return Prescription if found, nullopt otherwise
     */
    Result<Model::Prescription> getPrescriptionByAppointment(const std::string& appointmentID);

    /**
     * @brief Get all prescriptions
     * @return Vector of all prescriptions
     */
    List<Model::Prescription> getAllPrescriptions();

    /**
     * @brief Get prescriptions for a patient
     * @param patientUsername The patient's username
     * @return Vector of patient's prescriptions
     */
    List<Model::Prescription> getPatientPrescriptions(const std::string& patientUsername);

    /**
     * @brief Get prescriptions by a doctor
     * @param doctorID The doctor's ID
     * @return Vector of doctor's prescriptions
     */
    List<Model::Prescription> getDoctorPrescriptions(const std::string& doctorID);

    /**
     * @brief Get prescriptions by date range
     * @param startDate Start date (YYYY-MM-DD)
     * @param endDate End date (YYYY-MM-DD)
     * @return Vector of prescriptions in range
     */
    List<Model::Prescription> getPrescriptionsByDateRange(const std::string& startDate,
                                                           const std::string& endDate);

    /**
     * @brief Get prescription summaries for display
     * @return Vector of prescription summaries
     */
    List<PrescriptionSummary> getPrescriptionSummaries();

    /**
     * @brief Get total prescription count
     * @return Number of prescriptions
     */
    size_t getPrescriptionCount() const;

    // ==================== Dispensing ====================

    /**
     * @brief Dispense a prescription (updates medicine inventory)
     * @param prescriptionID The prescription's ID
     * @return Dispense result with success status and details
     */
    DispenseResult dispensePrescription(const std::string& prescriptionID);

    /**
     * @brief Check if prescription can be dispensed
     * @param prescriptionID The prescription's ID
     * @return True if all medicines have sufficient stock
     */
    bool canDispense(const std::string& prescriptionID) const;

    /**
     * @brief Get medicines with insufficient stock for a prescription
     * @param prescriptionID The prescription's ID
     * @return Vector of medicine IDs with insufficient stock
     */
    List<std::string> getInsufficientStockItems(const std::string& prescriptionID) const;

    /**
     * @brief Get undispensed prescriptions
     * @return Vector of undispensed prescriptions
     */
    List<Model::Prescription> getUndispensedPrescriptions();

    /**
     * @brief Get dispensed prescriptions
     * @return Vector of dispensed prescriptions
     */
    List<Model::Prescription> getDispensedPrescriptions();

    /**
     * @brief Mark prescription as undispensed (reverse dispense, for corrections)
     * @param prescriptionID The prescription's ID
     * @return True if successful
     * @note This does NOT restore medicine stock
     */
    bool markAsUndispensed(const std::string& prescriptionID);

    // ==================== Cost Calculation ====================

    /**
     * @brief Calculate total cost of a prescription
     * @param prescriptionID The prescription's ID
     * @return Total cost, or -1.0 if prescription not found
     */
    double calculatePrescriptionCost(const std::string& prescriptionID) const;

    /**
     * @brief Calculate cost for prescription items
     * @param items Vector of prescription items
     * @return Total cost
     */
    double calculateItemsCost(const List<Model::PrescriptionItem>& items) const;

    // ==================== Print/Export ====================

    /**
     * @brief Get prescription in printable format
     * @param prescriptionID The prescription's ID
     * @return Formatted prescription string
     */
    std::string getPrescriptionPrintFormat(const std::string& prescriptionID) const;

    // ==================== Statistics ====================

    /**
     * @brief Get prescription statistics
     * @return Map of statistic name to value
     */
    std::map<std::string, int> getPrescriptionStatistics() const;

    /**
     * @brief Get most prescribed medicines
     * @param limit Maximum number of medicines to return
     * @return Vector of pairs (medicineID, count)
     */
    List<std::pair<std::string, int>> getMostPrescribedMedicines(int limit = 10) const;

    // ==================== Validation ====================

    /**
     * @brief Validate prescription data
     * @param prescription The prescription to validate
     * @return True if valid
     */
    bool validatePrescription(const Model::Prescription& prescription) const;

    /**
     * @brief Check if prescription ID exists
     * @param prescriptionID The prescription ID to check
     * @return True if exists
     */
    bool prescriptionExists(const std::string& prescriptionID) const;

    /**
     * @brief Check if appointment already has a prescription
     * @param appointmentID The appointment ID to check
     * @return True if prescription exists for this appointment
     */
    bool appointmentHasPrescription(const std::string& appointmentID) const;

    // ==================== Data Persistence ====================

    /**
     * @brief Save prescription data
     * @return True if successful
     */
    bool saveData();

    /**
     * @brief Load prescription data
     * @return True if successful
     */
    bool loadData();
};

} // namespace BLL
} // namespace HMS
