#include "bll/AppointmentService.h"
#include "common/Utils.h"
#include "common/Constants.h"

#include <algorithm>
#include <sstream>
#include <iomanip>
#include <set>

namespace HMS
{
    namespace BLL
    {

        // ==================== Static Cached Time Slots ====================
        static const std::vector<std::string> &getCachedStandardTimeSlots()
        {
            static const std::vector<std::string> slots = []()
            {
                std::vector<std::string> result;
                for (int h = Constants::WORK_START_HOUR; h < Constants::WORK_END_HOUR; ++h)
                {
                    std::stringstream ss1;
                    ss1 << std::setfill('0') << std::setw(2) << h << ":00";
                    result.push_back(ss1.str());

                    std::stringstream ss2;
                    ss2 << std::setfill('0') << std::setw(2) << h << ":30";
                    result.push_back(ss2.str());
                }
                return result;
            }();
            return slots;
        }

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
        AppointmentService::~AppointmentService() = default;

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

            // 2. Validate and trim disease description
            std::string trimmedDisease = Utils::trim(disease);
            if (trimmedDisease.empty())
            {
                return std::nullopt;
            }

            // 3. Get data needed for creation
            double fee = getDoctorFee(doctorID);
            std::string id = generateAppointmentID();

            // 4. Create Model object
            Model::Appointment newAppt(
                id,
                patientUsername,
                doctorID,
                date,
                time,
                trimmedDisease,
                fee,
                false,                        // isPaid default false
                AppointmentStatus::SCHEDULED, // Status
                ""                            // Notes
            );

            // 5. Save to Repository
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

                // Check slot availability, excluding the current appointment
                if (!m_appointmentRepo->isSlotAvailable(appt.getDoctorID(), targetDate, targetTime, appointmentID))
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
            const std::vector<std::string> &allSlots = getCachedStandardTimeSlots();

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
            const std::vector<std::string> &standards = getCachedStandardTimeSlots();
            if (std::find(standards.begin(), standards.end(), time) == standards.end())
            {
                return false;
            }

            // Use repository method for the DB check
            return m_appointmentRepo->isSlotAvailable(doctorID, date, time);
        }

        std::vector<std::string> AppointmentService::getStandardTimeSlots()
        {
            // Return a copy of the cached slots
            return getCachedStandardTimeSlots();
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
            int dateCompare = Utils::compareDates(appt->getDate(), today);

            // Cannot cancel past appointments
            if (dateCompare < 0)
                return false;

            // For today's appointments, check if the time has already passed
            if (dateCompare == 0)
            {
                std::string currentTime = Utils::getCurrentTime();
                if (appt->getTime() <= currentTime)
                    return false;
            }

            return true;
        }

        bool AppointmentService::canEdit(const std::string &appointmentID)
        {
            // Same logic as cancel
            return canCancel(appointmentID);
        }

        // ==================== Statistics ====================

        double AppointmentService::getTotalRevenue()
        {
            double totalRevenue = 0.0;
            double paidRevenue = 0.0;
            double unpaidRevenue = 0.0;

            calculateRevenueStatistics(totalRevenue, paidRevenue, unpaidRevenue);

            return totalRevenue;
        }

        double AppointmentService::getPaidRevenue()
        {
            double totalRevenue = 0.0;
            double paidRevenue = 0.0;
            double unpaidRevenue = 0.0;

            calculateRevenueStatistics(totalRevenue, paidRevenue, unpaidRevenue);

            return paidRevenue;
        }

        double AppointmentService::getUnpaidRevenue()
        {
            double totalRevenue = 0.0;
            double paidRevenue = 0.0;
            double unpaidRevenue = 0.0;

            calculateRevenueStatistics(totalRevenue, paidRevenue, unpaidRevenue);

            return unpaidRevenue;
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
            return Utils::generateAppointmentID();
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
            return m_patientRepo->getByUsername(patientUsername).has_value();
        }

        bool AppointmentService::doctorExists(const std::string &doctorID)
        {
            return m_doctorRepo->exists(doctorID);
        }

        void AppointmentService::calculateRevenueStatistics(double &totalRevenue,
                                                            double &paidRevenue,
                                                            double &unpaidRevenue)
        {
            totalRevenue = 0.0;
            paidRevenue = 0.0;
            unpaidRevenue = 0.0;

            auto allAppts = m_appointmentRepo->getAll();

            for (const auto &appt : allAppts)
            {
                // Skip cancelled appointments
                if (appt.getStatus() == AppointmentStatus::CANCELLED)
                {
                    continue;
                }

                double price = appt.getPrice();
                totalRevenue += price;

                if (appt.isPaid())
                {
                    paidRevenue += price;
                }
                else
                {
                    unpaidRevenue += price;
                }
            }
        }

    } // namespace BLL
} // namespace HMS
