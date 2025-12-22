#pragma once

#include "../dal/AppointmentRepository.h"
#include "../dal/PatientRepository.h"
#include "../dal/DoctorRepository.h"
#include "../model/Appointment.h"
#include "../model/Patient.h"
#include "../model/Doctor.h"
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
         * @class AppointmentService
         * @brief Service for appointment-related business logic
         *
         * Implements Singleton pattern. Handles appointment booking,
         * scheduling, status management, and availability checking.
         */
        class AppointmentService
        {
        private:
            // ==================== Singleton ====================
            static std::unique_ptr<AppointmentService> s_instance;
            static std::mutex s_mutex;

            // ==================== Dependencies ====================
            DAL::AppointmentRepository *m_appointmentRepo;
            DAL::PatientRepository *m_patientRepo;
            DAL::DoctorRepository *m_doctorRepo;

            // ==================== Private Constructor ====================
            AppointmentService();

        public:
            // ==================== Singleton Access ====================

            /**
             * @brief Get the singleton instance
             * @return Pointer to the singleton instance
             */
            static AppointmentService *getInstance();

            /**
             * @brief Reset the singleton instance (for testing)
             */
            static void resetInstance();

            /**
             * @brief Delete copy constructor
             */
            AppointmentService(const AppointmentService &) = delete;

            /**
             * @brief Delete assignment operator
             */
            AppointmentService &operator=(const AppointmentService &) = delete;

            /**
             * @brief Destructor
             */
            ~AppointmentService();

            // ==================== Booking Operations ====================

            /**
             * @brief Book a new appointment
             * @param patientUsername The patient's username
             * @param doctorID The doctor's ID
             * @param date Appointment date (YYYY-MM-DD)
             * @param time Appointment time (HH:MM)
             * @param disease Description of disease/symptoms
             * @return Created appointment or nullopt if booking failed
             */
            std::optional<Model::Appointment> bookAppointment(
                const std::string &patientUsername,
                const std::string &doctorID,
                const std::string &date,
                const std::string &time,
                const std::string &disease);

            /**
             * @brief Edit an existing appointment
             * @param appointmentID The appointment ID
             * @param newDate New date (empty string to keep existing)
             * @param newTime New time (empty string to keep existing)
             * @return True if successful
             */
            bool editAppointment(const std::string &appointmentID,
                                 const std::string &newDate,
                                 const std::string &newTime);

            /**
             * @brief Cancel an appointment
             * @param appointmentID The appointment ID
             * @return True if successful
             */
            bool cancelAppointment(const std::string &appointmentID);

            /**
             * @brief Reschedule an appointment
             * @param appointmentID The appointment ID
             * @param newDate New date
             * @param newTime New time
             * @return True if successful
             */
            bool rescheduleAppointment(const std::string &appointmentID,
                                       const std::string &newDate,
                                       const std::string &newTime);

            // ==================== Status Management ====================

            /**
             * @brief Mark appointment as completed
             * @param appointmentID The appointment ID
             * @return True if successful
             */
            bool markAsCompleted(const std::string &appointmentID);

            /**
             * @brief Mark patient as no-show
             * @param appointmentID The appointment ID
             * @return True if successful
             */
            bool markAsNoShow(const std::string &appointmentID);

            /**
             * @brief Mark appointment as paid
             * @param appointmentID The appointment ID
             * @return True if successful
             */
            bool markAsPaid(const std::string &appointmentID);

            /**
             * @brief Update appointment notes
             * @param appointmentID The appointment ID
             * @param notes New notes
             * @return True if successful
             */
            bool updateNotes(const std::string &appointmentID,
                             const std::string &notes);

            // ==================== Query Operations ====================

            /**
             * @brief Get appointment by ID
             * @param appointmentID The appointment ID
             * @return Appointment if found, nullopt otherwise
             */
            std::optional<Model::Appointment> getAppointmentByID(const std::string &appointmentID);

            /**
             * @brief Get all appointments
             * @return Vector of all appointments
             */
            std::vector<Model::Appointment> getAllAppointments();

            /**
             * @brief Get appointments by status
             * @param status The appointment status
             * @return Vector of matching appointments
             */
            std::vector<Model::Appointment> getAppointmentsByStatus(AppointmentStatus status);

            /**
             * @brief Get appointments for a specific date
             * @param date The date (YYYY-MM-DD)
             * @return Vector of appointments on that date
             */
            std::vector<Model::Appointment> getAppointmentsByDate(const std::string &date);

            /**
             * @brief Get appointments in date range
             * @param startDate Start date
             * @param endDate End date
             * @return Vector of appointments in range
             */
            std::vector<Model::Appointment> getAppointmentsInRange(const std::string &startDate,
                                                                   const std::string &endDate);

            /**
             * @brief Get today's appointments
             * @return Vector of today's appointments
             */
            std::vector<Model::Appointment> getTodayAppointments();

            /**
             * @brief Get total appointment count
             * @return Number of appointments
             */
            size_t getAppointmentCount() const;

            // ==================== Availability ====================

            /**
             * @brief Get available time slots for a doctor on a date
             * @param doctorID The doctor's ID
             * @param date The date (YYYY-MM-DD)
             * @return Vector of available time slots
             */
            std::vector<std::string> getAvailableSlots(const std::string &doctorID,
                                                       const std::string &date);

            /**
             * @brief Check if a specific slot is available
             * @param doctorID The doctor's ID
             * @param date The date (YYYY-MM-DD)
             * @param time The time (HH:MM)
             * @return True if slot is available
             */
            bool isSlotAvailable(const std::string &doctorID,
                                 const std::string &date,
                                 const std::string &time);

            /**
             * @brief Get standard appointment time slots
             * @return Vector of standard time slots (e.g., "08:00", "08:30", etc.)
             */
            std::vector<std::string> getStandardTimeSlots();

            // ==================== Validation ====================

            /**
             * @brief Validate appointment booking
             * @param patientUsername The patient's username
             * @param doctorID The doctor's ID
             * @param date The date
             * @param time The time
             * @return True if booking is valid
             */
            bool validateBooking(const std::string &patientUsername,
                                 const std::string &doctorID,
                                 const std::string &date,
                                 const std::string &time);

            /**
             * @brief Check if appointment can be cancelled
             * @param appointmentID The appointment ID
             * @return True if can be cancelled
             */
            bool canCancel(const std::string &appointmentID);

            /**
             * @brief Check if appointment can be edited
             * @param appointmentID The appointment ID
             * @return True if can be edited
             */
            bool canEdit(const std::string &appointmentID);

            // ==================== Statistics ====================

            /**
             * @brief Get total revenue from appointments
             * @return Total revenue
             */
            double getTotalRevenue();

            /**
             * @brief Get paid revenue
             * @return Paid revenue
             */
            double getPaidRevenue();

            /**
             * @brief Get unpaid revenue
             * @return Unpaid revenue
             */
            double getUnpaidRevenue();

            /**
             * @brief Get count by status
             * @param status The appointment status
             * @return Count of appointments with that status
             */
            size_t getCountByStatus(AppointmentStatus status);

            // ==================== Data Persistence ====================

            /**
             * @brief Save appointment data
             * @return True if successful
             */
            bool saveData();

            /**
             * @brief Load appointment data
             * @return True if successful
             */
            bool loadData();

        private:
            // ==================== Helper Methods ====================

            /**
             * @brief Generate a new appointment ID
             * @return New unique appointment ID
             */
            std::string generateAppointmentID();

            /**
             * @brief Get doctor's consultation fee
             * @param doctorID The doctor's ID
             * @return Consultation fee
             */
            double getDoctorFee(const std::string &doctorID);

            /**
             * @brief Validate patient exists
             * @param patientUsername The patient's username
             * @return True if patient exists
             */
            bool patientExists(const std::string &patientUsername);

            /**
             * @brief Validate doctor exists
             * @param doctorID The doctor's ID
             * @return True if doctor exists
             */
            bool doctorExists(const std::string &doctorID);
        };

    } // namespace BLL
} // namespace HMS
