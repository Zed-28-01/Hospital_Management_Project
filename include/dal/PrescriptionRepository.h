#pragma once

#include "IRepository.h"
#include "../advance/Prescription.h"
#include <vector>
#include <optional>
#include <string>
#include <mutex>
#include <memory>

namespace HMS
{
    namespace DAL
    {

        /**
         * @class PrescriptionRepository
         * @brief Repository for Prescription entity persistence
         *
         * Implements Singleton pattern. Handles CRUD operations
         * and file persistence for Prescription entities.
         */
        class PrescriptionRepository : public IRepository<Model::Prescription>
        {
        private:
            // ==================== Singleton ====================
            static std::unique_ptr<PrescriptionRepository> s_instance;
            static std::mutex s_mutex;

            // ==================== Data ====================
            std::vector<Model::Prescription> m_prescriptions;
            std::string m_filePath;
            bool m_isLoaded;

            // ==================== Private Constructor ====================
            PrescriptionRepository();

        public:
            // ==================== Singleton Access ====================

            /**
             * @brief Get the singleton instance
             * @return Pointer to the singleton instance
             */
            static PrescriptionRepository *getInstance();

            /**
             * @brief Reset the singleton instance (for testing)
             */
            static void resetInstance();

            /**
             * @brief Delete copy constructor
             */
            PrescriptionRepository(const PrescriptionRepository &) = delete;

            /**
             * @brief Delete assignment operator
             */
            PrescriptionRepository &operator=(const PrescriptionRepository &) = delete;

            /**
             * @brief Destructor
             */
            ~PrescriptionRepository() override;

            // ==================== CRUD Operations ====================

            /**
             * @brief Get all prescriptions
             * @return Vector of all prescriptions
             */
            std::vector<Model::Prescription> getAll() override;

            /**
             * @brief Get prescription by prescription ID
             * @param id Prescription ID
             * @return Prescription if found, nullopt otherwise
             */
            std::optional<Model::Prescription> getById(const std::string &id) override;

            /**
             * @brief Add a new prescription
             * @param prescription The prescription to add
             * @return True if successful
             */
            bool add(const Model::Prescription &prescription) override;

            /**
             * @brief Update an existing prescription
             * @param prescription The prescription with updated values
             * @return True if successful
             */
            bool update(const Model::Prescription &prescription) override;

            /**
             * @brief Remove a prescription by ID
             * @param id Prescription ID to remove
             * @return True if successful
             */
            bool remove(const std::string &id) override;

            // ==================== Persistence ====================

            /**
             * @brief Save all prescriptions to file
             * @return True if successful
             */
            bool save() override;

            /**
             * @brief Load all prescriptions from file
             * @return True if successful
             */
            bool load() override;

            // ==================== Query Operations ====================

            /**
             * @brief Get total number of prescriptions
             * @return Prescription count
             */
            size_t count() const override;

            /**
             * @brief Check if prescription ID exists
             * @param id Prescription ID to check
             * @return True if exists
             */
            bool exists(const std::string &id) const override;

            /**
             * @brief Clear all prescriptions from memory
             * @return True if successful
             */
            bool clear() override;

            // ==================== Prescription-Specific Queries ====================

            /**
             * @brief Get prescription by appointment ID
             * @param appointmentID The appointment ID
             * @return Prescription if found, nullopt otherwise
             */
            std::optional<Model::Prescription> getByAppointment(const std::string &appointmentID);

            /**
             * @brief Get all prescriptions for a patient
             * @param patientUsername The patient's username
             * @return Vector of prescriptions for this patient
             */
            std::vector<Model::Prescription> getByPatient(const std::string &patientUsername);

            /**
             * @brief Get all prescriptions by a doctor
             * @param doctorID The doctor's ID
             * @return Vector of prescriptions by this doctor
             */
            std::vector<Model::Prescription> getByDoctor(const std::string &doctorID);

            /**
             * @brief Get all undispensed prescriptions
             * @return Vector of prescriptions that haven't been dispensed
             */
            std::vector<Model::Prescription> getUndispensed();

            /**
             * @brief Get all dispensed prescriptions
             * @return Vector of prescriptions that have been dispensed
             */
            std::vector<Model::Prescription> getDispensed();

            /**
             * @brief Get prescriptions by date
             * @param date The prescription date (YYYY-MM-DD)
             * @return Vector of prescriptions on that date
             */
            std::vector<Model::Prescription> getByDate(const std::string &date);

            /**
             * @brief Get prescriptions within a date range
             * @param startDate Start date (YYYY-MM-DD)
             * @param endDate End date (YYYY-MM-DD)
             * @return Vector of prescriptions within the range
             */
            std::vector<Model::Prescription> getByDateRange(const std::string &startDate,
                                                            const std::string &endDate);

            /**
             * @brief Get prescriptions containing a specific medicine
             * @param medicineID The medicine ID
             * @return Vector of prescriptions containing this medicine
             */
            std::vector<Model::Prescription> getByMedicine(const std::string &medicineID);

            /**
             * @brief Get the next available prescription ID
             * @return New prescription ID string (e.g., "PRE001")
             */
            std::string getNextId();

            // ==================== Dispensing Operations ====================

            /**
             * @brief Mark a prescription as dispensed
             * @param id Prescription ID
             * @return True if successful
             */
            bool markAsDispensed(const std::string &id);

            /**
             * @brief Mark a prescription as undispensed
             * @param id Prescription ID
             * @return True if successful
             */
            bool markAsUndispensed(const std::string &id);

            // ==================== File Path ====================

            /**
             * @brief Set the file path for persistence
             * @param filePath Path to the prescription file
             */
            void setFilePath(const std::string &filePath);

            /**
             * @brief Get the current file path
             * @return File path string
             */
            std::string getFilePath() const;
        };

    } // namespace DAL
} // namespace HMS
