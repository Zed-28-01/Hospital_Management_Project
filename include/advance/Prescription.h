#pragma once

/**
 * @file Prescription.h
 * @brief Prescription model for medical prescription management
 *
 * Provides functionality for creating and managing medical prescriptions,
 * linking appointments with prescribed medicines, and tracking dispensing status.
 */

#include <string>
#include <vector>
#include "../common/Types.h"

namespace HMS
{
    namespace Model
    {

        /**
         * @struct PrescriptionItem
         * @brief Represents a single medicine item in a prescription
         *
         * Contains details about a prescribed medicine including quantity,
         * dosage instructions, duration, and special instructions.
         */
        struct PrescriptionItem
        {
            std::string medicineID;    ///< Medicine identifier (e.g., "MED001")
            std::string medicineName;  ///< Medicine name for display
            int quantity;              ///< Quantity to dispense
            std::string dosage;        ///< Dosage instructions (e.g., "2 tablets, 3 times daily")
            std::string duration;      ///< Treatment duration (e.g., "7 days")
            std::string instructions;  ///< Special instructions (e.g., "After meals")
        };

        /**
         * @class Prescription
         * @brief Represents a medical prescription in the hospital system
         *
         * Manages prescription information including patient/doctor details,
         * prescribed items, diagnosis, and dispensing status. Supports
         * serialization for persistent storage and formatted printing.
         */
        class Prescription
        {
        private:
            std::string m_prescriptionID;              ///< Unique prescription identifier (e.g., "PRE001")
            std::string m_appointmentID;               ///< Associated appointment ID
            std::string m_patientUsername;             ///< Patient's account username
            std::string m_doctorID;                    ///< Prescribing doctor's ID
            std::string m_prescriptionDate;            ///< Date prescribed (YYYY-MM-DD)
            std::vector<PrescriptionItem> m_items;     ///< List of prescribed medicine items
            std::string m_diagnosis;                   ///< Medical diagnosis
            std::string m_notes;                       ///< Additional notes
            bool m_isDispensed;                        ///< Whether prescription has been dispensed

        public:
            // ==================== Constructors ====================

            /**
             * @brief Default constructor
             */
            Prescription() = default;

            /**
             * @brief Parameterized constructor
             * @param prescriptionID Unique prescription identifier
             * @param appointmentID Associated appointment ID
             * @param patientUsername Patient's account username
             * @param doctorID Prescribing doctor's ID
             * @param prescriptionDate Date of prescription (YYYY-MM-DD)
             */
            Prescription(const std::string &prescriptionID,
                         const std::string &appointmentID,
                         const std::string &patientUsername,
                         const std::string &doctorID,
                         const std::string &prescriptionDate);

            /**
             * @brief Destructor
             */
            ~Prescription() = default;

            // ==================== Getters ====================

            /**
             * @brief Get prescription's unique ID
             * @return Prescription ID string
             */
            std::string getPrescriptionID() const;

            /**
             * @brief Get associated appointment ID
             * @return Appointment ID string
             */
            std::string getAppointmentID() const;

            /**
             * @brief Get patient's username
             * @return Patient username string
             */
            std::string getPatientUsername() const;

            /**
             * @brief Get prescribing doctor's ID
             * @return Doctor ID string
             */
            std::string getDoctorID() const;

            /**
             * @brief Get prescription date
             * @return Date string (YYYY-MM-DD)
             */
            std::string getPrescriptionDate() const;

            /**
             * @brief Get list of prescribed items
             * @return Vector of PrescriptionItem
             */
            std::vector<PrescriptionItem> getItems() const;

            /**
             * @brief Get diagnosis
             * @return Diagnosis string
             */
            std::string getDiagnosis() const;

            /**
             * @brief Get additional notes
             * @return Notes string
             */
            std::string getNotes() const;

            /**
             * @brief Check if prescription has been dispensed
             * @return True if dispensed
             */
            bool isDispensed() const;

            // ==================== Setters ====================

            /**
             * @brief Set diagnosis
             * @param diagnosis New diagnosis
             */
            void setDiagnosis(const std::string &diagnosis);

            /**
             * @brief Set additional notes
             * @param notes New notes
             */
            void setNotes(const std::string &notes);

            /**
             * @brief Set dispensed status
             * @param dispensed New dispensed status
             */
            void setDispensed(bool dispensed);

            // ==================== Item Management ====================

            /**
             * @brief Add a medicine item to the prescription
             * @param item PrescriptionItem to add
             * @note If medicine already exists, updates existing item
             */
            void addItem(const PrescriptionItem &item);

            /**
             * @brief Remove a medicine item from the prescription
             * @param medicineID Medicine ID to remove
             */
            void removeItem(const std::string &medicineID);

            /**
             * @brief Remove all items from the prescription
             */
            void clearItems();

            /**
             * @brief Get number of items in the prescription
             * @return Number of items
             */
            size_t getItemCount() const;

            // ==================== Output ====================

            /**
             * @brief Display prescription information to console
             */
            void displayInfo() const;

            /**
             * @brief Generate formatted prescription for printing
             * @return Formatted string with box-drawing characters
             */
            std::string toPrintFormat() const;

            /**
             * @brief Serialize prescription to string for file storage
             * @return Pipe-delimited string representation
             *
             * Format: prescriptionID|appointmentID|patientUsername|doctorID|date|diagnosis|notes|isDispensed|items
             * @note Items format: medicineID:medicineName:quantity:dosage:duration:instructions;...
             */
            std::string serialize() const;

            /**
             * @brief Deserialize prescription from string
             * @param line Pipe-delimited string from file
             * @return Prescription object or nullopt if parsing fails
             */
            static Result<Prescription> deserialize(const std::string &line);
        };

    } // namespace Model
} // namespace HMS
