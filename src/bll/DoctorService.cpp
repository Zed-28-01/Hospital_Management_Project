#include "bll/DoctorService.h"
#include "common/Utils.h"
#include "common/Types.h"
#include "common/Constants.h"
#include <set>
#include <algorithm>

namespace HMS
{
    namespace BLL
    {

        // ========================== CONSTRUCTOR ================================

        DoctorService::DoctorService()
        {
            m_doctorRepo = DAL::DoctorRepository::getInstance();
            m_appointmentRepo = DAL::AppointmentRepository::getInstance();
        }
        DoctorService::~DoctorService() = default;

        // =========================== SINGLETON ACCESS ========================

        DoctorService *DoctorService::getInstance()
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            if (!s_instance)
            {
                s_instance = std::unique_ptr<DoctorService>(new DoctorService());
            }
            return s_instance.get();
        }

        void DoctorService::resetInstance()
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_instance.reset();
        }

        // ============================= CRUD Operations ===============================

        bool DoctorService::createDoctor(const Model::Doctor &doctor)
        {
            if (!validateDoctor(doctor))
            {
                return false;
            }
            if (doctorExists(doctor.getID()))
            {
                return false;
            }
            return m_doctorRepo->add(doctor);
        }

        Result<Model::Doctor> DoctorService::createDoctor(const std::string &username,
                                                          const std::string &name,
                                                          const std::string &phone,
                                                          Gender gender,
                                                          const std::string &dateOfBirth,
                                                          const std::string &specialization,
                                                          const std::string &schedule,
                                                          double consultationFee)
        {
            std::string id = Utils::generateDoctorID();
            Model::Doctor newDoc(id,
                                 username,
                                 name,
                                 phone,
                                 gender,
                                 dateOfBirth,
                                 specialization,
                                 schedule,
                                 consultationFee);

            if (createDoctor(newDoc))
            {
                return newDoc;
            }

            return std::nullopt;
        }

        bool DoctorService::updateDoctor(const Model::Doctor &doctor)
        {
            if (!doctorExists(doctor.getID()))
            {
                return false;
            }

            if (!validateDoctor(doctor))
            {
                return false;
            }

            return m_doctorRepo->update(doctor);
        }

        bool DoctorService::deleteDoctor(const std::string &doctorID)
        {
            if (!doctorExists(doctorID))
            {
                return false;
            }

            return m_doctorRepo->remove(doctorID);
        }

        // ============================== QUERY OPERATIONS ===========================

        Result<Model::Doctor> DoctorService::getDoctorByID(const std::string &doctorID)
        {
            return m_doctorRepo->getById(doctorID);
        }

        Result<Model::Doctor> DoctorService::getDoctorByUsername(const std::string &username)
        {
            return m_doctorRepo->getByUsername(username);
        }

        List<Model::Doctor> DoctorService::getAllDoctors()
        {
            return m_doctorRepo->getAll();
        }

        List<Model::Doctor> DoctorService::searchDoctors(const std::string &keyword)
        {
            return m_doctorRepo->search(keyword);
        }

        List<Model::Doctor> DoctorService::getDoctorsBySpecialization(const std::string &specialization)
        {
            return m_doctorRepo->getBySpecialization(specialization);
        }

        List<std::string> DoctorService::getAllSpecializations()
        {
            return m_doctorRepo->getAllSpecializations();
        }

        size_t DoctorService::getDoctorCount() const
        {
            return m_doctorRepo->count();
        }

        // ========================== SCHEDULE MANAGEMENT =============================

        List<Model::Appointment> DoctorService::getDoctorSchedule(const std::string &doctorID, const std::string &date)
        {
            auto appointments = m_appointmentRepo->getAll();
            List<Model::Appointment> result;

            for (const auto &app : appointments)
            {
                if (app.getDoctorID() == doctorID && app.getDate() == date &&
                    app.getStatus() != AppointmentStatus::CANCELLED)
                {
                    result.push_back(app);
                }
            }

            std::sort(result.begin(), result.end(), [](const Model::Appointment &a, const Model::Appointment &b)
                      { return a.getTime() < b.getTime(); });

            return result;
        }

        List<Model::Appointment> DoctorService::getUpcomingAppointments(const std::string &doctorID)
        {
            auto allApp = m_appointmentRepo->getAll();
            List<Model::Appointment> result;
            std::string today = Utils::getCurrentDate();
            std::string nowTime = Utils::getCurrentTime();

            for (const auto &app : allApp)
            {
                if (app.getDoctorID() == doctorID &&
                    app.getStatus() != AppointmentStatus::CANCELLED &&
                    app.getStatus() != AppointmentStatus::COMPLETED &&
                    (app.getDate() > today || (app.getDate() == today && app.getTime() > nowTime)))
                {
                    result.push_back(app);
                }
            }

            // Sort by date first, then by time
            std::sort(result.begin(), result.end(), [](const Model::Appointment &a, const Model::Appointment &b)
                      {
                          if (a.getDate() != b.getDate())
                          {
                              return a.getDate() < b.getDate();
                          }
                          return a.getTime() < b.getTime();
                      });

            return result;
        }

        List<std::string> DoctorService::getAvailableSlots(const std::string &doctorID, const std::string &date)
        {
            // Validate the date format
            if (!Utils::isValidDate(date))
            {
                return {};
            }

            // Don't return slots for past dates
            if (Utils::compareDates(date, Utils::getCurrentDate()) < 0)
            {
                return {};
            }

            List<std::string> availableSlots;

            static const List<std::string> standardSlots{
                "08:00", "09:00",
                "10:00", "11:00",
                "13:00", "14:00",
                "15:00", "16:00"};

            auto existingSchedule = m_appointmentRepo->getBookedSlots(doctorID, date);
            std::set<std::string> occupiedTime(existingSchedule.begin(), existingSchedule.end());

            bool isToday = (date == Utils::getCurrentDate());
            std::string currentTime = isToday ? Utils::getCurrentTime() : "";

            for (const auto &slot : standardSlots)
            {
                if (occupiedTime.find(slot) == occupiedTime.end())
                {
                    if (!isToday || slot > currentTime)
                    {
                        availableSlots.push_back(slot);
                    }
                }
            }
            return availableSlots;
        }

        bool DoctorService::isSlotAvailable(const std::string &doctorID, const std::string &time, const std::string &date)
        {
            if (!Utils::isValidTime(time) || !Utils::isValidDate(date))
            {
                return false;
            }

            auto slots = getAvailableSlots(doctorID, date);
            return std::find(slots.begin(), slots.end(), time) != slots.end();
        }

        // ============================= ACTIVITY TRACKING ===============================

        List<Model::Appointment> DoctorService::getDoctorActivity(const std::string &doctorID)
        {
            auto allAppointments = m_appointmentRepo->getAll();
            List<Model::Appointment> result;

            for (const auto &app : allAppointments)
            {
                if (app.getDoctorID() == doctorID)
                {
                    result.push_back(app);
                }
            }

            return result;
        }

        List<Model::Appointment> DoctorService::getCompletedAppointments(const std::string &doctorID)
        {
            auto allAppointments = m_appointmentRepo->getAll();
            List<Model::Appointment> result;

            for (const auto &app : allAppointments)
            {
                if (app.getDoctorID() == doctorID && app.getStatus() == AppointmentStatus::COMPLETED)
                {
                    result.push_back(app);
                }
            }
            return result;
        }

        List<Model::Appointment> DoctorService::getAppointmentsInRange(const std::string &doctorID,
                                                                       const std::string &startDate,
                                                                       const std::string &endDate)
        {
            if (!Utils::isValidDate(startDate) || !Utils::isValidDate(endDate))
            {
                return {};
            }

            auto activities = getDoctorActivity(doctorID);
            List<Model::Appointment> result;

            for (const auto &a : activities)
            {
                int cmpStart = Utils::compareDates(a.getDate(), startDate);
                int cmpEnd = Utils::compareDates(a.getDate(), endDate);
                if (cmpStart >= 0 && cmpEnd <= 0)
                {
                    result.push_back(a);
                }
            }
            return result;
        }

        List<Model::Appointment> DoctorService::getTodayAppointments(const std::string &doctorID)
        {
            return getDoctorSchedule(doctorID, Utils::getCurrentDate());
        }

        // ============================ STATISTICS ================================

        double DoctorService::getDoctorRevenue(const std::string &doctorID)
        {
            auto completed = getCompletedAppointments(doctorID);
            double total = 0.0;
            for (const auto &app : completed)
            {
                total += app.getPrice();
            }
            return total;
        }

        size_t DoctorService::getDoctorAppointmentCount(const std::string &doctorID)
        {
            return getDoctorActivity(doctorID).size();
        }

        size_t DoctorService::getDoctorPatientCount(const std::string &doctorID)
        {
            auto activity = getDoctorActivity(doctorID);
            std::set<std::string> uniquePatients;

            for (const auto &act : activity)
            {
                if (act.getStatus() == AppointmentStatus::COMPLETED)
                {
                    uniquePatients.insert(act.getPatientUsername());
                }
            }
            return uniquePatients.size();
        }

        // ================================ VALIDATION & PERSISTENCE ==============================

        bool DoctorService::validateDoctor(const Model::Doctor &doctor)
        {
            // Check required fields are not empty
            if (doctor.getID().empty() ||
                doctor.getName().empty() ||
                doctor.getPhone().empty() ||
                doctor.getSpecialization().empty() ||
                doctor.getDateOfBirth().empty() ||
                doctor.getUsername().empty())
            {
                return false;
            }

            // Validate phone format
            if (!Utils::isValidPhone(doctor.getPhone()))
            {
                return false;
            }

            // Validate date of birth format
            if (!Utils::isValidDate(doctor.getDateOfBirth()))
            {
                return false;
            }

            // Date of birth should not be in the future
            if (Utils::isFutureDate(doctor.getDateOfBirth()))
            {
                return false;
            }

            // Consultation fee must be non-negative
            if (doctor.getConsultationFee() < 0)
            {
                return false;
            }

            return true;
        }

        bool DoctorService::doctorExists(const std::string &doctorID)
        {
            return m_doctorRepo->exists(doctorID);
        }

        bool DoctorService::saveData()
        {
            return m_doctorRepo->save();
        }

        bool DoctorService::loadData()
        {
            return m_doctorRepo->load();
        }

    }
}
