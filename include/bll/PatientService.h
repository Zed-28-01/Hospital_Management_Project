#pragma once

#include "../dal/PatientRepository.h"
#include "../dal/AppointmentRepository.h"
#include "../model/Patient.h"
#include "../model/Appointment.h"
#include "../common/Types.h"
#include <string>
#include <vector>
#include <optional>
#include <mutex>
#include <memory>

namespace HMS
{
    namespace BLL
    {

        /**
         * @class PatientService
         * @brief Service for patient-related business logic
         *
         * Implements Singleton pattern. Handles patient management,
         * appointment history, and billing calculations.
         */
        class PatientService
        {
        private:
            // ==================== Singleton ====================
            static std::unique_ptr<PatientService> s_instance;
            static std::mutex s_mutex;

            // ==================== Dependencies ====================
            DAL::PatientRepository *m_patientRepo;
            DAL::AppointmentRepository *m_appointmentRepo;

            // ==================== Private Constructor ====================
            PatientService();

        public:
            // ==================== Singleton Access ====================

            /**
             * @brief Get the singleton instance
             * @return Pointer to the singleton instance
             */
            static PatientService *getInstance();

            /**
             * @brief Reset the singleton instance (for testing)
             */
            static void resetInstance();

            /**
             * @brief Delete copy constructor
             */
            PatientService(const PatientService &) = delete;

            /**
             * @brief Delete assignment operator
             */
            PatientService &operator=(const PatientService &) = delete;

            /**
             * @brief Destructor
             */
            ~PatientService();

            // ==================== CRUD Operations ====================

            /**
             * @brief Create a new patient record
             * @param patient The patient data
             * @return True if successful
             */
            bool createPatient(const Model::Patient &patient);

            /**
             * @brief Create patient with individual parameters
             * @param username Account username
             * @param name Patient's name
             * @param phone Phone number
             * @param gender Gender
             * @param dateOfBirth Date of birth
             * @param address Address
             * @param medicalHistory Medical history
             * @return Created patient or nullopt if failed
             */
            std::optional<Model::Patient> createPatient(
                const std::string &username,
                const std::string &name,
                const std::string &phone,
                Gender gender,
                const std::string &dateOfBirth,
                const std::string &address,
                const std::string &medicalHistory);

            /**
             * @brief Update an existing patient record
             * @param patient The patient with updated data
             * @return True if successful
             */
            bool updatePatient(const Model::Patient &patient);

            /**
             * @brief Delete a patient record
             * @param patientID The patient's ID
             * @return True if successful
             */
            bool deletePatient(const std::string &patientID);

            // ==================== Query Operations ====================

            /**
             * @brief Get patient by ID
             * @param patientID The patient's ID
             * @return Patient if found, nullopt otherwise
             */
            std::optional<Model::Patient> getPatientByID(const std::string &patientID);

            /**
             * @brief Get patient by username
             * @param username The account username
             * @return Patient if found, nullopt otherwise
             */
            std::optional<Model::Patient> getPatientByUsername(const std::string &username);

            /**
             * @brief Get all patients
             * @return Vector of all patients
             */
            std::vector<Model::Patient> getAllPatients();

            /**
             * @brief Search patients by keyword
             * @param keyword Search keyword
             * @return Vector of matching patients
             */
            std::vector<Model::Patient> searchPatients(const std::string &keyword);

            /**
             * @brief Get total patient count
             * @return Number of patients
             */
            size_t getPatientCount() const;

            // ==================== Appointment Queries ====================

            /**
             * @brief Get patient's complete appointment history
             * @param username The patient's username
             * @return Vector of all appointments
             */
            std::vector<Model::Appointment> getAppointmentHistory(const std::string &username);

            /**
             * @brief Get patient's upcoming appointments
             * @param username The patient's username
             * @return Vector of scheduled future appointments
             */
            std::vector<Model::Appointment> getUpcomingAppointments(const std::string &username);

            /**
             * @brief Get patient's past appointments
             * @param username The patient's username
             * @return Vector of completed/cancelled appointments
             */
            std::vector<Model::Appointment> getPastAppointments(const std::string &username);

            // ==================== Billing ====================

            /**
             * @brief Calculate total bill for a patient
             * @param username The patient's username
             * @return Total unpaid amount
             */
            double calculateTotalBill(const std::string &username);

            /**
             * @brief Calculate total paid amount
             * @param username The patient's username
             * @return Total paid amount
             */
            double calculateTotalPaid(const std::string &username);

            /**
             * @brief Get patient's unpaid appointments
             * @param username The patient's username
             * @return Vector of unpaid appointments
             */
            std::vector<Model::Appointment> getUnpaidAppointments(const std::string &username);

            // ==================== Medical History ====================

            /**
             * @brief Update patient's medical history
             * @param patientID The patient's ID
             * @param newHistory New medical history text
             * @return True if successful
             */
            bool updateMedicalHistory(const std::string &patientID,
                                      const std::string &newHistory);

            /**
             * @brief Append to patient's medical history
             * @param patientID The patient's ID
             * @param entry Entry to append
             * @return True if successful
             */
            bool appendMedicalHistory(const std::string &patientID,
                                      const std::string &entry);

            // ==================== Validation ====================

            /**
             * @brief Validate patient data
             * @param patient The patient to validate
             * @return True if valid
             */
            bool validatePatient(const Model::Patient &patient);

            /**
             * @brief Check if patient ID exists
             * @param patientID The patient ID to check
             * @return True if exists
             */
            bool patientExists(const std::string &patientID);

            // ==================== Data Persistence ====================

            /**
             * @brief Save patient data
             * @return True if successful
             */
            bool saveData();

            /**
             * @brief Load patient data
             * @return True if successful
             */
            bool loadData();
        };

    } // namespace BLL
} // namespace HMS
