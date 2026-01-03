#include "bll/DoctorService.h"
#include "bll/AppointmentService.h"
#include "common/Constants.h"
#include "common/Types.h"
#include "common/Utils.h"

#include <algorithm>
#include <iomanip>
#include <set>
#include <sstream>

namespace HMS
{
    namespace BLL
    {

        // ==================== Static Cached Time Slots ====================
        // Uses the same pattern as AppointmentService for consistency
        static const List<std::string> &getCachedStandardTimeSlots()
        {
            static const List<std::string> slots = []()
            {
                List<std::string> result;
                for (int h = Constants::WORK_START_HOUR; h < Constants::WORK_END_HOUR;
                     ++h)
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

        // ========================== STATIC MEMBER DEFINITIONS
        // ================================
        std::unique_ptr<DoctorService> DoctorService::s_instance = nullptr;
        std::mutex DoctorService::s_mutex;

        // ========================== CONSTRUCTOR ================================

        DoctorService::DoctorService()
        {
            m_doctorRepo = DAL::DoctorRepository::getInstance();
            m_appointmentRepo = DAL::AppointmentRepository::getInstance();
            m_accountRepo = DAL::AccountRepository::getInstance();
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

            // Check if username already exists (business rule)
            if (m_doctorRepo->getByUsername(doctor.getUsername()).has_value())
            {
                return false;
            }

            return m_doctorRepo->add(doctor);
        }

        Result<Model::Doctor> DoctorService::createDoctor(
            const std::string &username, const std::string &name,
            const std::string &phone, Gender gender, const std::string &dateOfBirth,
            const std::string &specialization, const std::string &schedule,
            double consultationFee)
        {
            std::string id = m_doctorRepo->getNextId(); 
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

            // Check if username conflicts with another doctor (business rule)
            auto existingDoctor = m_doctorRepo->getByUsername(doctor.getUsername());
            if (existingDoctor.has_value() &&
                existingDoctor->getDoctorID() != doctor.getDoctorID())
            {
                return false;
            }

            return m_doctorRepo->update(doctor);
        }

        bool DoctorService::deleteDoctor(const std::string &doctorID)
        {
            // Get doctor to find username (also validates existence)
            auto doctorOpt = m_doctorRepo->getById(doctorID);
            if (!doctorOpt.has_value())
            {
                return false;
            }

            std::string username = doctorOpt->getUsername();

            // Auto-cancel upcoming scheduled appointments before deleting doctor
            // This preserves appointment history while preventing orphaned future bookings
            AppointmentService::getInstance()->cancelUpcomingByDoctor(doctorID);

            // Delete doctor profile
            if (!m_doctorRepo->remove(doctorID))
            {
                return false;
            }

            // Cascading delete: Remove associated account to prevent orphaned logins
            // This is a security requirement - deleted doctors should not be able to log in
            if (!username.empty())
            {
                m_accountRepo->remove(username);
            }

            return true;
        }

        // ============================== QUERY OPERATIONS ===========================

        Result<Model::Doctor>
        DoctorService::getDoctorByID(const std::string &doctorID)
        {
            return m_doctorRepo->getById(doctorID);
        }

        Result<Model::Doctor>
        DoctorService::getDoctorByUsername(const std::string &username)
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

        List<Model::Doctor>
        DoctorService::getDoctorsBySpecialization(const std::string &specialization)
        {
            return m_doctorRepo->getBySpecialization(specialization);
        }

        List<std::string> DoctorService::getAllSpecializations()
        {
            return m_doctorRepo->getAllSpecializations();
        }

        size_t DoctorService::getDoctorCount() const { return m_doctorRepo->count(); }

        // ========================== SCHEDULE MANAGEMENT =============================

        List<Model::Appointment>
        DoctorService::getDoctorSchedule(const std::string &doctorID,
                                         const std::string &date)
        {
            // Use efficient repository query instead of filtering all appointments
            auto appointments = m_appointmentRepo->getByDoctorAndDate(doctorID, date);
            List<Model::Appointment> result;

            // Filter out cancelled appointments
            for (const auto &app : appointments)
            {
                if (app.getStatus() != AppointmentStatus::CANCELLED)
                {
                    result.push_back(app);
                }
            }

            // getByDoctorAndDate already returns sorted by time, but ensure consistency
            std::sort(result.begin(), result.end(),
                      [](const Model::Appointment &a, const Model::Appointment &b)
                      {
                          return a.getTime() < b.getTime();
                      });

            return result;
        }

        List<Model::Appointment>
        DoctorService::getUpcomingAppointments(const std::string &doctorID)
        {

            auto upcomingAppts = m_appointmentRepo->getUpcomingByDoctor(doctorID);

            std::string today = Utils::getCurrentDate();
            std::string nowTime = Utils::getCurrentTime();

            List<Model::Appointment> result;
            result.reserve(upcomingAppts.size());

            for (const auto &app : upcomingAppts)
            {
                // For future dates, include all; for today, check time
                if (app.getDate() > today ||
                    (app.getDate() == today && app.getTime() > nowTime))
                {
                    result.push_back(app);
                }
            }

            // Already sorted by repository, no need to re-sort
            return result;
        }

        List<std::string> DoctorService::getAvailableSlots(const std::string &doctorID,
                                                           const std::string &date)
        {
            // Validate the date format
            if (!Utils::isValidDateInternal(date))
            {
                return {};
            }

            // Don't return slots for past dates
            if (Utils::compareDates(date, Utils::getCurrentDate()) < 0)
            {
                return {};
            }

            // Use cached standard time slots (consistent with AppointmentService)
            const List<std::string> &allSlots = getCachedStandardTimeSlots();

            // Get booked slots from repository
            auto bookedSlots = m_appointmentRepo->getBookedSlots(doctorID, date);
            std::set<std::string> occupiedTime(bookedSlots.begin(), bookedSlots.end());

            List<std::string> availableSlots;
            bool isToday = (date == Utils::getCurrentDate());
            std::string currentTime = isToday ? Utils::getCurrentTime() : "";

            for (const auto &slot : allSlots)
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

        bool DoctorService::isSlotAvailable(const std::string &doctorID,
                                            const std::string &time,
                                            const std::string &date)
        {
            if (!Utils::isValidTime(time) || !Utils::isValidDateInternal(date))
            {
                return false;
            }

            auto slots = getAvailableSlots(doctorID, date);
            return std::find(slots.begin(), slots.end(), time) != slots.end();
        }

        // ============================= ACTIVITY TRACKING
        // ===============================

        List<Model::Appointment>
        DoctorService::getDoctorActivity(const std::string &doctorID)
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

        List<Model::Appointment>
        DoctorService::getCompletedAppointments(const std::string &doctorID)
        {
            auto allAppointments = m_appointmentRepo->getAll();
            List<Model::Appointment> result;

            for (const auto &app : allAppointments)
            {
                if (app.getDoctorID() == doctorID &&
                    app.getStatus() == AppointmentStatus::COMPLETED)
                {
                    result.push_back(app);
                }
            }
            return result;
        }

        List<Model::Appointment>
        DoctorService::getAppointmentsInRange(const std::string &doctorID,
                                              const std::string &startDate,
                                              const std::string &endDate)
        {
            if (!Utils::isValidDateInternal(startDate) || !Utils::isValidDateInternal(endDate))
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

        List<Model::Appointment>
        DoctorService::getTodayAppointments(const std::string &doctorID)
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

        // ================================ VALIDATION & PERSISTENCE
        // ==============================

        bool DoctorService::validateDoctor(const Model::Doctor &doctor)
        {
            // Check required fields are not empty
            if (doctor.getID().empty() || doctor.getName().empty() ||
                doctor.getPhone().empty() || doctor.getSpecialization().empty() ||
                doctor.getDateOfBirth().empty() || doctor.getUsername().empty())
            {
                return false;
            }

            // Validate phone format
            if (!Utils::isValidPhone(doctor.getPhone()))
            {
                return false;
            }

            // Validate date of birth format
            if (!Utils::isValidDateInternal(doctor.getDateOfBirth()))
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

        bool DoctorService::saveData() { return m_doctorRepo->save(); }

        bool DoctorService::loadData() { return m_doctorRepo->load(); }

    } // namespace BLL
} // namespace HMS
