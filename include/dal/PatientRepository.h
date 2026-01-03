#pragma once

#include "IRepository.h"
#include "../model/Patient.h"
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
         * @class PatientRepository
         * @brief Repository for Patient entity persistence
         *
         * Implements Singleton pattern. Handles CRUD operations
         * and file persistence for Patient entities.
         */
        class PatientRepository : public IRepository<Model::Patient>
        {
        private:
            // ==================== Singleton ====================
            static std::unique_ptr<PatientRepository> s_instance;
            static std::mutex s_mutex;

            // ==================== Data ====================
            std::vector<Model::Patient> m_patients;
            std::string m_filePath;
            bool m_isLoaded;
            mutable std::mutex m_dataMutex;

            // ==================== Private Constructor ====================
            PatientRepository();

            // ==================== Private Helpers ====================

            /**
             * @brief Ensure data is loaded (const-safe helper)
             */
            void ensureLoaded() const;

            /**
             * @brief Internal load implementation (without lock)
             * @return True if successful
             */
            bool loadInternal();

            /**
             * @brief Internal save implementation (without lock)
             * @return True if successful
             */
            bool saveInternal();

        public:
            // ==================== Singleton Access ====================

            /**
             * @brief Get the singleton instance
             * @return Pointer to the singleton instance
             */
            static PatientRepository *getInstance();

            /**
             * @brief Reset the singleton instance (for testing)
             */
            static void resetInstance();

            /**
             * @brief Delete copy constructor
             */
            PatientRepository(const PatientRepository &) = delete;

            /**
             * @brief Delete assignment operator
             */
            PatientRepository &operator=(const PatientRepository &) = delete;

            /**
             * @brief Destructor
             */
            ~PatientRepository() override;

            // ==================== CRUD Operations ====================

            /**
             * @brief Get all patients
             * @return Vector of all patients
             */
            std::vector<Model::Patient> getAll() override;

            /**
             * @brief Get patient by patient ID
             * @param id Patient ID
             * @return Patient if found, nullopt otherwise
             */
            std::optional<Model::Patient> getById(const std::string &id) override;

            /**
             * @brief Add a new patient
             * @param patient The patient to add
             * @return True if successful
             */
            bool add(const Model::Patient &patient) override;

            /**
             * @brief Update an existing patient
             * @param patient The patient with updated values
             * @return True if successful
             */
            bool update(const Model::Patient &patient) override;

            /**
             * @brief Remove a patient by ID
             * @param id Patient ID to remove
             * @return True if successful
             */
            bool remove(const std::string &id) override;

            // ==================== Persistence ====================

            /**
             * @brief Save all patients to file
             * @return True if successful
             */
            bool save() override;

            /**
             * @brief Load all patients from file
             * @return True if successful
             */
            bool load() override;

            // ==================== Query Operations ====================

            /**
             * @brief Get total number of patients
             * @return Patient count
             */
            size_t count() const override;

            /**
             * @brief Check if patient ID exists
             * @param id Patient ID to check
             * @return True if exists
             */
            bool exists(const std::string &id) const override;

            /**
             * @brief Clear all patients from memory
             * @return True if successful
             */
            bool clear() override;

            // ==================== Patient-Specific Queries ====================

            /**
             * @brief Get patient by username
             * @param username The account username
             * @return Patient if found, nullopt otherwise
             */
            std::optional<Model::Patient> getByUsername(const std::string &username);

            /**
             * @brief Search patients by name (partial match)
             * @param name Name to search for
             * @return Vector of matching patients
             */
            std::vector<Model::Patient> searchByName(const std::string &name);

            /**
             * @brief Search patients by phone number (partial match)
             * @param phone Phone number to search for
             * @return Vector of matching patients
             */
            std::vector<Model::Patient> searchByPhone(const std::string &phone);

            /**
             * @brief Find existing patient without account by matching identity fields
             * @param phone Phone number
             * @param name Patient name
             * @param dateOfBirth Date of birth
             * @param gender Gender
             * @return Patient if found (with empty username), nullopt otherwise
             */
            std::optional<Model::Patient> findUnlinkedPatient(
                const std::string &phone,
                const std::string &name,
                const std::string &dateOfBirth,
                Gender gender);

            /**
             * @brief Search patients by any keyword
             * @param keyword Keyword to search in name, phone, address
             * @return Vector of matching patients
             */
            std::vector<Model::Patient> search(const std::string &keyword);

            /**
             * @brief Get the next available patient ID
             * @return New patient ID string
             */
            std::string getNextId();

            // ==================== File Path ====================

            /**
             * @brief Set the file path for persistence
             * @param filePath Path to the patient file
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
