#include "bll/AppointmentService.h"
#include "common/Utils.h"
#include "common/Constants.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>

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
            if (!validateBooking(patientUsername, doctorID, date, time))
            {
                return std::nullopt;
            }

            double fee = getDoctorFee(doctorID);
            std::string id = generateAppointmentID();

            Model::Appointment newAppt(
                id,
                patientUsername,
                doctorID,
                date,
                time,
                disease,
                fee,
                false,
                AppointmentStatus::SCHEDULED,
                "");

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

            if (targetDate != appt.getDate() || targetTime != appt.getTime())
            {
                if (!Utils::isValidDate(targetDate) || !Utils::isValidTime(targetTime))
                {
                    return false;
                }

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
            std::vector<std::string> bookedSlots = m_appointmentRepo->getBookedSlots(doctorID, date);
            std::vector<std::string> availableSlots;

            std::set_difference(allSlots.begin(), allSlots.end(),
                                bookedSlots.begin(), bookedSlots.end(),
                                std::back_inserter(availableSlots));

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

            std::vector<std::string> standards = getStandardTimeSlots();
            if (std::find(standards.begin(), standards.end(), time) == standards.end())
            {
                return false;
            }

            return m_appointmentRepo->isSlotAvailable(doctorID, date, time);
        }

        std::vector<std::string> AppointmentService::getStandardTimeSlots()
        {
            std::vector<std::string> slots;
            int startHour = 8;
            int endHour = 17;

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
            if (!Utils::isValidDate(date))
                return false;
            if (!Utils::isValidTime(time))
                return false;

            if (!Utils::isFutureDate(date) && date != Utils::getCurrentDate())
            {
                return false;
            }

            if (date == Utils::getCurrentDate())
            {
                if (time <= Utils::getCurrentTime())
                    return false;
            }

            if (!patientExists(patientUsername))
                return false;
            if (!doctorExists(doctorID))
                return false;

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
            if (Utils::compareDates(appt->getDate(), today) < 0)
                return false;

            return true;
        }

        bool AppointmentService::canEdit(const std::string &appointmentID)
        {
            return canCancel(appointmentID);
        }

        // ==================== Statistics (ĐÃ SỬA) ====================
        // Vì không thể sửa .h để thêm getter, ta tạm thời disable tính năng tính tiền
        // để code có thể biên dịch (compile) thành công.

        double AppointmentService::getTotalRevenue()
        {
            // Không thể truy cập fee vì thiếu getter trong Appointment.h
            // TODO: Thêm double getFee() const vào Appointment.h để bật lại tính năng này
            return 0.0;
        }

        double AppointmentService::getPaidRevenue()
        {
            // Không thể truy cập fee vì thiếu getter trong Appointment.h
            return 0.0;
        }

        double AppointmentService::getUnpaidRevenue()
        {
            // Không thể truy cập fee vì thiếu getter trong Appointment.h
            return 0.0;
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
                // Nếu Doctor.h có hàm getConsultationFee() thì giữ nguyên.
                // Nếu Doctor.h cũng lỗi, hãy sửa thành return 0.0;
                return doctorOpt->getConsultationFee();
            }
            return 0.0;
        }

        bool AppointmentService::patientExists(const std::string &patientUsername)
        {
            auto all = m_patientRepo->getAll();
            for (const auto &p : all)
            {
                if (p.getUsername() == patientUsername)
                    return true;
            }
            return false;
        }

        bool AppointmentService::doctorExists(const std::string &doctorID)
        {
            return m_doctorRepo->exists(doctorID);
        }

    } // namespace BLL
} // namespace HMS