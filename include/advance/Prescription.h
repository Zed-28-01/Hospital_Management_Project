#pragma once

/**
 * @file Prescription.h
 * @brief Placeholder for advance Prescription feature
 *
 * This file is a placeholder for the Prescription management
 * feature planned for advance implementation. When implemented,
 * this will link appointments with prescribed medicines.
 *
 * Planned features:
 * - Create prescriptions linked to appointments
 * - Track prescribed medicines with dosage
 * - Print/export prescriptions
 * - Prescription history for patients
 */

#include <string>
#include <vector>
#include "../common/Types.h"

namespace HMS {
namespace Model {

/**
 * @struct PrescriptionItem
 * @brief Represents a single item in a prescription
 */
struct PrescriptionItem {
    std::string medicineID;
    std::string medicineName;
    int quantity;
    std::string dosage;          // e.g., "2 tablets, 3 times daily"
    std::string duration;        // e.g., "7 days"
    std::string instructions;    // e.g., "After meals"
};

/**
 * @class Prescription
 * @brief Represents a medical prescription (ADVANCE IMPLEMENTATION)
 *
 * @note This is a placeholder. Implementation pending.
 */
class Prescription {
private:
    std::string m_prescriptionID;
    std::string m_appointmentID;
    std::string m_patientUsername;
    std::string m_doctorID;
    std::string m_prescriptionDate;
    std::vector<PrescriptionItem> m_items;
    std::string m_diagnosis;
    std::string m_notes;
    bool m_isDispensed;

public:
    // ==================== Constructors ====================

    Prescription() = default;

    Prescription(const std::string& prescriptionID,
                 const std::string& appointmentID,
                 const std::string& patientUsername,
                 const std::string& doctorID,
                 const std::string& prescriptionDate);

    ~Prescription() = default;

    // ==================== Getters ====================

    std::string getPrescriptionID() const;
    std::string getAppointmentID() const;
    std::string getPatientUsername() const;
    std::string getDoctorID() const;
    std::string getPrescriptionDate() const;
    std::vector<PrescriptionItem> getItems() const;
    std::string getDiagnosis() const;
    std::string getNotes() const;
    bool isDispensed() const;

    // ==================== Setters ====================

    void setDiagnosis(const std::string& diagnosis);
    void setNotes(const std::string& notes);
    void setDispensed(bool dispensed);

    // ==================== Item Management ====================

    void addItem(const PrescriptionItem& item);
    void removeItem(const std::string& medicineID);
    void clearItems();
    size_t getItemCount() const;
    double calculateTotalCost() const;

    // ==================== Output ====================

    void displayInfo() const;
    std::string toPrintFormat() const;
    std::string serialize() const;
    static Result<Prescription> deserialize(const std::string& line);
};

} // namespace Model
} // namespace HMS

/**
 * @note Advance files to create when implementing Prescription feature:
 *
 * Data Access Layer:
 * - include/dal/PrescriptionRepository.h
 * - src/dal/PrescriptionRepository.cpp
 *
 * Business Logic Layer:
 * - include/bll/PrescriptionService.h
 * - src/bll/PrescriptionService.cpp
 *
 * Data File:
 * - data/Prescription.txt
 *
 * Format: prescriptionID|appointmentID|patientUsername|doctorID|date|diagnosis|notes|isDispensed|items
 * Items format: medicineID:quantity:dosage:duration:instructions;...
 * Example: PRE001|APT001|patient001|D001|2024-03-15|Viral Fever|Rest recommended|1|MED001:20:2 tablets 3 times daily:5 days:After meals;MED005:1:10ml twice daily:7 days:Before meals
 */
