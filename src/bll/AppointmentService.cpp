// SỬA: Dùng đường dẫn chuẩn dựa trên include_directories trong CMake
#include "bll/AppointmentService.h"
#include "common/Utils.h"
#include "common/Constants.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <set> // Thêm thư viện set cho logic available slots

namespace HMS
{
    namespace BLL
    {

        // ==================== Singleton Members ====================
        std::unique_ptr<AppointmentService> AppointmentService::s_instance = nullptr;
        std::mutex AppointmentService::s_mutex;

        // ==================== Private Constructor ====================
        AppointmentService::AppointmentService()
        {
            m_appointmentRepo = DAL::AppointmentRepository::getInstance();
            m_patientRepo = DAL::PatientRepository::getInstance();
            m_doctorRepo = DAL::DoctorRepository::getInstance();
        }

        // ==================== Destructor ====================
        AppointmentService::~AppointmentService()
        {
        }

        // ==================== Singleton Access ====================
        AppointmentService *AppointmentService::getInstance()
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            if (!s_instance)
            {
                s_instance = std::unique_ptr<AppointmentService>(new AppointmentService());
            }
            return s_instance.get();
        }

        void AppointmentService::resetInstance()
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_instance.reset();
        }

        // ==================== Booking Operations ====================

        std::optional<Model::Appointment> AppointmentService::bookAppointment(
            const std::string &patientUsername,
            const std::string &doctorID,
            const std::string &date,
            const std::string &time,
            const std::string &disease)
        {
            // 1. Validate inputs
            if (!validateBooking(patientUsername, doctorID, date, time))
            {
                return std::nullopt;
            }

            // 2. Get data needed for creation
            double fee = getDoctorFee(doctorID);
            std::string id = generateAppointmentID();

            // 3. Create Model object
            // Constructor: ID, PatientUser, DocID, Date, Time, Disease, Price, isPaid, Status, Notes
            Model::Appointment newAppt(
                id,
                patientUsername,
                doctorID,
                date,
                time,
                disease,
                fee,
                false,                        // isPaid default false
                AppointmentStatus::SCHEDULED, // Status
                ""                            // Notes
            );

            // 4. Save to Repository
            if (m_appointmentRepo->add(newAppt))
            {
                return newAppt;
            }

            return std::nullopt;
        }

        bool AppointmentService::editAppointment(const std::string &appointmentID,
                                                 const std::string &newDate,
                                                 const std::string &newTime)
        {
            auto apptOpt = m_appointmentRepo->getById(appointmentID);
            if (!apptOpt.has_value())
                return false;

            Model::Appointment appt = apptOpt.value();

            if (!canEdit(appointmentID))
                return false;

            std::string targetDate = newDate.empty() ? appt.getDate() : newDate;
            std::string targetTime = newTime.empty() ? appt.getTime() : newTime;

            // Only check validity if something actually changed
            if (targetDate != appt.getDate() || targetTime != appt.getTime())
            {
                if (!Utils::isValidDate(targetDate) || !Utils::isValidTime(targetTime))
                {
                    return false;
                }

                // Check slot availability (exclude current appointment logic handled by simple check)
                // Note: Strictly speaking, we should check if the NEW slot is free.
                if (!isSlotAvailable(appt.getDoctorID(), targetDate, targetTime))
                {
                    return false;
                }

                appt.setDate(targetDate);
                appt.setTime(targetTime);
            }

            return m_appointmentRepo->update(appt);
        }

        bool AppointmentService::cancelAppointment(const std::string &appointmentID)
        {
            auto apptOpt = m_appointmentRepo->getById(appointmentID);
            if (!apptOpt.has_value())
                return false;

            if (!canCancel(appointmentID))
                return false;

            Model::Appointment appt = apptOpt.value();
            appt.setStatus(AppointmentStatus::CANCELLED);

            return m_appointmentRepo->update(appt);
        }

        bool AppointmentService::rescheduleAppointment(const std::string &appointmentID,
                                                       const std::string &newDate,
                                                       const std::string &newTime)
        {
            return editAppointment(appointmentID, newDate, newTime);
        }

        // ==================== Status Management ====================

        bool AppointmentService::markAsCompleted(const std::string &appointmentID)
        {
            auto apptOpt = m_appointmentRepo->getById(appointmentID);
            if (!apptOpt.has_value())
                return false;

            Model::Appointment appt = apptOpt.value();

            // Only scheduled appointments can be marked as completed
            if (appt.getStatus() != AppointmentStatus::SCHEDULED)
                return false;

            appt.setStatus(AppointmentStatus::COMPLETED);
            return m_appointmentRepo->update(appt);
        }

        bool AppointmentService::markAsNoShow(const std::string &appointmentID)
        {
            auto apptOpt = m_appointmentRepo->getById(appointmentID);
            if (!apptOpt.has_value())
                return false;

            Model::Appointment appt = apptOpt.value();

            if (appt.getStatus() != AppointmentStatus::SCHEDULED)
                return false;

            appt.setStatus(AppointmentStatus::NO_SHOW);
            return m_appointmentRepo->update(appt);
        }

        bool AppointmentService::markAsPaid(const std::string &appointmentID)
        {
            auto apptOpt = m_appointmentRepo->getById(appointmentID);
            if (!apptOpt.has_value())
                return false;

            Model::Appointment appt = apptOpt.value();
            appt.setPaid(true);
            return m_appointmentRepo->update(appt);
        }

        bool AppointmentService::updateNotes(const std::string &appointmentID, const std::string &notes)
        {
            auto apptOpt = m_appointmentRepo->getById(appointmentID);
            if (!apptOpt.has_value())
                return false;

            Model::Appointment appt = apptOpt.value();
            appt.setNotes(notes);
            return m_appointmentRepo->update(appt);
        }

        // ==================== Query Operations ====================

        std::optional<Model::Appointment> AppointmentService::getAppointmentByID(const std::string &appointmentID)
        {
            return m_appointmentRepo->getById(appointmentID);
        }

        std::vector<Model::Appointment> AppointmentService::getAllAppointments()
        {
            return m_appointmentRepo->getAll();
        }

        std::vector<Model::Appointment> AppointmentService::getAppointmentsByStatus(AppointmentStatus status)
        {
            return m_appointmentRepo->getByStatus(status);
        }

        std::vector<Model::Appointment> AppointmentService::getAppointmentsByDate(const std::string &date)
        {
            return m_appointmentRepo->getByDate(date);
        }

        std::vector<Model::Appointment> AppointmentService::getAppointmentsInRange(const std::string &startDate,
                                                                                   const std::string &endDate)
        {
            return m_appointmentRepo->getByDateRange(startDate, endDate);
        }

        std::vector<Model::Appointment> AppointmentService::getTodayAppointments()
        {
            return m_appointmentRepo->getToday();
        }

        size_t AppointmentService::getAppointmentCount() const
        {
            return m_appointmentRepo->count();
        }

        // ==================== Availability ====================

        std::vector<std::string> AppointmentService::getAvailableSlots(const std::string &doctorID,
                                                                       const std::string &date)
        {
            std::vector<std::string> allSlots = getStandardTimeSlots();

            // Get booked slots from Repo
            std::vector<std::string> bookedSlots = m_appointmentRepo->getBookedSlots(doctorID, date);

            // Note: set_difference requires sorted ranges.
            // Standard slots are generated in order.
            // Booked slots returned from Repo might not be sorted. Let's sort them to be safe.
            std::sort(bookedSlots.begin(), bookedSlots.end());

            std::vector<std::string> availableSlots;
            std::set_difference(allSlots.begin(), allSlots.end(),
                                bookedSlots.begin(), bookedSlots.end(),
                                std::back_inserter(availableSlots));

            // If query is for today, remove past times
            std::string today = Utils::getCurrentDate();
            if (date == today)
            {
                std::string currentTime = Utils::getCurrentTime();
                availableSlots.erase(
                    std::remove_if(availableSlots.begin(), availableSlots.end(),
                                   [&currentTime](const std::string &slot)
                                   {
                                       return slot <= currentTime;
                                   }),
                    availableSlots.end());
            }

            return availableSlots;
        }

        bool AppointmentService::isSlotAvailable(const std::string &doctorID,
                                                 const std::string &date,
                                                 const std::string &time)
        {
            if (!Utils::isValidTime(time))
                return false;

            // Check if time is a standard slot (e.g., prevents booking at 08:13)
            std::vector<std::string> standards = getStandardTimeSlots();
            if (std::find(standards.begin(), standards.end(), time) == standards.end())
            {
                return false;
            }

            // Use repository method for the DB check
            return m_appointmentRepo->isSlotAvailable(doctorID, date, time);
        }

        std::vector<std::string> AppointmentService::getStandardTimeSlots()
        {
            std::vector<std::string> slots;
            int startHour = 8;
            int endHour = 17; // 5 PM

            for (int h = startHour; h < endHour; ++h)
            {
                std::stringstream ss1;
                ss1 << std::setfill('0') << std::setw(2) << h << ":00";
                slots.push_back(ss1.str());

                std::stringstream ss2;
                ss2 << std::setfill('0') << std::setw(2) << h << ":30";
                slots.push_back(ss2.str());
            }
            return slots;
        }

        // ==================== Validation ====================

        bool AppointmentService::validateBooking(const std::string &patientUsername,
                                                 const std::string &doctorID,
                                                 const std::string &date,
                                                 const std::string &time)
        {
            // Date/Time syntax check
            if (!Utils::isValidDate(date))
                return false;
            if (!Utils::isValidTime(time))
                return false;

            // Date logic check
            std::string today = Utils::getCurrentDate();
            if (Utils::compareDates(date, today) < 0) // Cannot book past date
            {
                return false;
            }

            // Time logic check (if today)
            if (date == today)
            {
                if (time <= Utils::getCurrentTime())
                    return false;
            }

            // Existence checks
            if (!patientExists(patientUsername))
                return false;
            if (!doctorExists(doctorID))
                return false;

            // Availability check
            if (!isSlotAvailable(doctorID, date, time))
                return false;

            return true;
        }

        bool AppointmentService::canCancel(const std::string &appointmentID)
        {
            auto appt = getAppointmentByID(appointmentID);
            if (!appt)
                return false;

            if (appt->getStatus() != AppointmentStatus::SCHEDULED)
                return false;

            std::string today = Utils::getCurrentDate();
            // Allow cancel if date is future, OR date is today but time is future?
            // Simple rule: Allow cancel if appointment date is not in the past.
            if (Utils::compareDates(appt->getDate(), today) < 0)
                return false;

            return true;
        }

        bool AppointmentService::canEdit(const std::string &appointmentID)
        {
            // Same logic as cancel usually
            return canCancel(appointmentID);
        }

        // ==================== Statistics ====================

        double AppointmentService::getTotalRevenue()
        {
            double total = 0.0;
            auto allAppts = m_appointmentRepo->getAll();

            for (const auto &appt : allAppts)
            {
                // Logic: Count everything except CANCELLED.
                if (appt.getStatus() != AppointmentStatus::CANCELLED)
                {
                    total += appt.getPrice();
                }
            }
            return total;
        }

        double AppointmentService::getPaidRevenue()
        {
            double total = 0.0;
            auto allAppts = m_appointmentRepo->getAll();

            for (const auto &appt : allAppts)
            {
                if (appt.getStatus() != AppointmentStatus::CANCELLED && appt.isPaid())
                {
                    total += appt.getPrice();
                }
            }
            return total;
        }

        double AppointmentService::getUnpaidRevenue()
        {
            double total = 0.0;
            auto allAppts = m_appointmentRepo->getAll();

            for (const auto &appt : allAppts)
            {
                if (appt.getStatus() != AppointmentStatus::CANCELLED && !appt.isPaid())
                {
                    total += appt.getPrice();
                }
            }
            return total;
        }

        size_t AppointmentService::getCountByStatus(AppointmentStatus status)
        {
            return m_appointmentRepo->getByStatus(status).size();
        }

        // ==================== Data Persistence ====================

        bool AppointmentService::saveData()
        {
            return m_appointmentRepo->save();
        }

        bool AppointmentService::loadData()
        {
            return m_appointmentRepo->load();
        }

        // ==================== Helper Methods ====================

        std::string AppointmentService::generateAppointmentID()
        {
            return m_appointmentRepo->getNextId();
        }

        double AppointmentService::getDoctorFee(const std::string &doctorID)
        {
            auto doctorOpt = m_doctorRepo->getById(doctorID);
            if (doctorOpt.has_value())
            {
                return doctorOpt->getConsultationFee();
            }
            return 0.0;
        }

        bool AppointmentService::patientExists(const std::string &patientUsername)
        {
            // OPTIMIZED: Use getByUsername from Repository instead of iterating getAll
            return m_patientRepo->getByUsername(patientUsername).has_value();
        }

        bool AppointmentService::doctorExists(const std::string &doctorID)
        {
            return m_doctorRepo->exists(doctorID);
        }

    } // namespace BLL
} // namespace HMS