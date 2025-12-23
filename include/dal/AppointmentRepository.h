#pragma once

#include "IRepository.h"
#include "../model/Appointment.h"
#include "../common/Types.h"
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
         * @class AppointmentRepository
         * @brief Repository for Appointment entity persistence
         *
         * Implements Singleton pattern. Handles CRUD operations
         * and file persistence for Appointment entities.
         * Provides rich query capabilities for appointment lookups.
         */
        class AppointmentRepository : public IRepository<Model::Appointment>
        {
        private:
            // ==================== Singleton ====================
            static std::unique_ptr<AppointmentRepository> s_instance;
            static std::mutex s_mutex;

            // ==================== Data ====================
            std::vector<Model::Appointment> m_appointments;
            std::string m_filePath;
            bool m_isLoaded;

            // ==================== Private Constructor ====================
            AppointmentRepository();

        public:
            // ==================== Singleton Access ====================

            /**
             * @brief Get the singleton instance
             * @return Pointer to the singleton instance
             */
            static AppointmentRepository *getInstance();

            /**
             * @brief Reset the singleton instance (for testing)
             */
            static void resetInstance();

            /**
             * @brief Delete copy constructor
             */
            AppointmentRepository(const AppointmentRepository &) = delete;

            /**
             * @brief Delete assignment operator
             */
            AppointmentRepository &operator=(const AppointmentRepository &) = delete;

            /**
             * @brief Destructor
             */
            ~AppointmentRepository() override;

            // ==================== CRUD Operations ====================

            /**
             * @brief Get all appointments
             * @return Vector of all appointments
             */
            std::vector<Model::Appointment> getAll() override;

            /**
             * @brief Get appointment by appointment ID
             * @param id Appointment ID
             * @return Appointment if found, nullopt otherwise
             */
            std::optional<Model::Appointment> getById(const std::string &id) override;

            /**
             * @brief Add a new appointment
             * @param appointment The appointment to add
             * @return True if successful
             */
            bool add(const Model::Appointment &appointment) override;

            /**
             * @brief Update an existing appointment
             * @param appointment The appointment with updated values
             * @return True if successful
             */
            bool update(const Model::Appointment &appointment) override;

            /**
             * @brief Remove an appointment by ID
             * @param id Appointment ID to remove
             * @return True if successful
             */
            bool remove(const std::string &id) override;

            // ==================== Persistence ====================

            /**
             * @brief Save all appointments to file
             * @return True if successful
             */
            bool save() override;

            /**
             * @brief Load all appointments from file
             * @return True if successful
             */
            bool load() override;

            // ==================== Query Operations ====================

            /**
             * @brief Get total number of appointments
             * @return Appointment count
             */
            size_t count() const override;

            /**
             * @brief Check if appointment ID exists
             * @param id Appointment ID to check
             * @return True if exists
             */
            bool exists(const std::string &id) const override;

            /**
             * @brief Clear all appointments from memory
             * @return True if successful
             */
            bool clear() override;

            // ==================== Patient-Related Queries ====================

            /**
             * @brief Get all appointments for a patient
             * @param patientUsername The patient's username
             * @return Vector of patient's appointments
             */
            std::vector<Model::Appointment> getByPatient(const std::string &patientUsername);

            /**
             * @brief Get patient's upcoming appointments
             * @param patientUsername The patient's username
             * @return Vector of upcoming appointments (scheduled, future date)
             */
            std::vector<Model::Appointment> getUpcomingByPatient(const std::string &patientUsername);

            /**
             * @brief Get patient's appointment history
             * @param patientUsername The patient's username
             * @return Vector of past appointments (completed, cancelled, no_show)
             */
            std::vector<Model::Appointment> getHistoryByPatient(const std::string &patientUsername);

            /**
             * @brief Get patient's unpaid appointments
             * @param patientUsername The patient's username
             * @return Vector of unpaid appointments
             */
            std::vector<Model::Appointment> getUnpaidByPatient(const std::string &patientUsername);

            // ==================== Doctor-Related Queries ====================

            /**
             * @brief Get all appointments for a doctor
             * @param doctorID The doctor's ID
             * @return Vector of doctor's appointments
             */
            std::vector<Model::Appointment> getByDoctor(const std::string &doctorID);

            /**
             * @brief Get doctor's appointments for a specific date
             * @param doctorID The doctor's ID
             * @param date The date (YYYY-MM-DD)
             * @return Vector of appointments on that date
             */
            std::vector<Model::Appointment> getByDoctorAndDate(const std::string &doctorID,
                                                               const std::string &date);

            /**
             * @brief Get doctor's upcoming appointments
             * @param doctorID The doctor's ID
             * @return Vector of upcoming appointments
             */
            std::vector<Model::Appointment> getUpcomingByDoctor(const std::string &doctorID);

            // ==================== Date-Based Queries ====================

            /**
             * @brief Get all appointments for a specific date
             * @param date The date (YYYY-MM-DD)
             * @return Vector of appointments on that date
             */
            std::vector<Model::Appointment> getByDate(const std::string &date);

            /**
             * @brief Get appointments within a date range
             * @param startDate Start date (YYYY-MM-DD)
             * @param endDate End date (YYYY-MM-DD)
             * @return Vector of appointments in range
             */
            std::vector<Model::Appointment> getByDateRange(const std::string &startDate,
                                                           const std::string &endDate);

            /**
             * @brief Get today's appointments
             * @return Vector of today's appointments
             */
            std::vector<Model::Appointment> getToday();

            // ==================== Status-Based Queries ====================

            /**
             * @brief Get appointments by status
             * @param status The appointment status
             * @return Vector of matching appointments
             */
            std::vector<Model::Appointment> getByStatus(AppointmentStatus status);

            /**
             * @brief Get all scheduled appointments
             * @return Vector of scheduled appointments
             */
            std::vector<Model::Appointment> getScheduled();

            /**
             * @brief Get all completed appointments
             * @return Vector of completed appointments
             */
            std::vector<Model::Appointment> getCompleted();

            /**
             * @brief Get all cancelled appointments
             * @return Vector of cancelled appointments
             */
            std::vector<Model::Appointment> getCancelled();

            // ==================== Slot Availability ====================

            /**
             * @brief Check if a time slot is available
             * @param doctorID Doctor's ID
             * @param date Date (YYYY-MM-DD)
             * @param time Time (HH:MM)
             * @return True if slot is available
             */
            bool isSlotAvailable(const std::string &doctorID,
                                 const std::string &date,
                                 const std::string &time);

            /**
             * @brief Check if a time slot is available, excluding a specific appointment
             * @param doctorID Doctor's ID
             * @param date Date (YYYY-MM-DD)
             * @param time Time (HH:MM)
             * @param excludeAppointmentID Appointment ID to exclude from the check
             * @return True if slot is available
             */
            bool isSlotAvailable(const std::string &doctorID,
                                 const std::string &date,
                                 const std::string &time,
                                 const std::string &excludeAppointmentID);

            /**
             * @brief Get booked slots for a doctor on a date
             * @param doctorID Doctor's ID
             * @param date Date (YYYY-MM-DD)
             * @return Vector of booked time slots
             */
            std::vector<std::string> getBookedSlots(const std::string &doctorID,
                                                    const std::string &date);

            // ==================== ID Generation ====================

            /**
             * @brief Get the next available appointment ID
             * @return New appointment ID string
             */
            std::string getNextId();

            // ==================== File Path ====================

            /**
             * @brief Set the file path for persistence
             * @param filePath Path to the appointment file
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
