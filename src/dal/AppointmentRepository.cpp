#include "dal/AppointmentRepository.h"
#include "dal/FileHelper.h"
#include "common/Utils.h"
#include "common/Constants.h"

#include <algorithm>
#include <sstream>

namespace HMS
{
    namespace DAL
    {

        // ==================== Singleton Instance ====================
        std::unique_ptr<AppointmentRepository> AppointmentRepository::s_instance = nullptr;
        std::mutex AppointmentRepository::s_mutex;

        // ==================== Private Constructor ====================
        AppointmentRepository::AppointmentRepository()
            : m_filePath(Constants::APPOINTMENT_FILE),
              m_isLoaded(false)
        {
        }

        // ==================== Singleton Access ====================
        AppointmentRepository *AppointmentRepository::getInstance()
        {
            std::lock_guard<std::mutex> lock(s_mutex);

            if (!s_instance)
            {
                s_instance = std::unique_ptr<AppointmentRepository>(new AppointmentRepository());
            }
            return s_instance.get();
        }

        void AppointmentRepository::resetInstance()
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_instance.reset();
        }

        AppointmentRepository::~AppointmentRepository() = default;

        // ==================== CRUD Operations ====================
        std::vector<Model::Appointment> AppointmentRepository::getAll()
        {
            if (!m_isLoaded)
            {
                load();
            }
            return m_appointments;
        }

        std::optional<Model::Appointment> AppointmentRepository::getById(const std::string &id)
        {
            if (!m_isLoaded)
            {
                load();
            }

            auto it = std::find_if(m_appointments.begin(), m_appointments.end(),
                                   [&id](const Model::Appointment &apt)
                                   {
                                       return apt.getAppointmentID() == id;
                                   });

            if (it != m_appointments.end())
            {
                return *it;
            }
            return std::nullopt;
        }

        bool AppointmentRepository::add(const Model::Appointment &appointment)
        {
            // Load trước để đảm bảo check exists chính xác
            if (!m_isLoaded)
            {
                load();
            }

            if (exists(appointment.getAppointmentID()))
            {
                return false;
            }

            m_appointments.push_back(appointment);
            return save();
        }

        bool AppointmentRepository::update(const Model::Appointment &appointment)
        {
            if (!m_isLoaded)
            {
                load();
            }

            auto it = std::find_if(m_appointments.begin(), m_appointments.end(),
                                   [&appointment](const Model::Appointment &apt)
                                   {
                                       return apt.getAppointmentID() == appointment.getAppointmentID();
                                   });

            if (it != m_appointments.end())
            {
                *it = appointment;
                return save();
            }
            return false;
        }

        bool AppointmentRepository::remove(const std::string &id)
        {
            if (!m_isLoaded)
            {
                load();
            }

            auto it = std::find_if(m_appointments.begin(), m_appointments.end(),
                                   [&id](const Model::Appointment &apt)
                                   {
                                       return apt.getAppointmentID() == id;
                                   });

            if (it != m_appointments.end())
            {
                m_appointments.erase(it);
                return save();
            }
            return false;
        }

        // ==================== Persistence ====================
        bool AppointmentRepository::save()
        {
            try
            {
                std::vector<std::string> lines;

                auto header = FileHelper::getFileHeader("Appointment");
                std::stringstream hss(header);
                std::string headerLine;
                while (std::getline(hss, headerLine))
                {
                    if (!headerLine.empty())
                    {
                        lines.push_back(headerLine);
                    }
                }

                for (const auto &apt : m_appointments)
                {
                    lines.push_back(apt.serialize());
                }

                FileHelper::createBackup(m_filePath);
                return FileHelper::writeLines(m_filePath, lines);
            }
            catch (...)
            {
                return false;
            }
        }

        bool AppointmentRepository::load()
        {
            try
            {
                FileHelper::createDirectoryIfNotExists(Constants::DATA_DIR);
                FileHelper::createFileIfNotExists(m_filePath);

                std::vector<std::string> lines = FileHelper::readLines(m_filePath);

                m_appointments.clear();

                for (const auto &line : lines)
                {
                    auto appointment = Model::Appointment::deserialize(line);
                    if (appointment)
                    {
                        m_appointments.push_back(appointment.value());
                    }
                }

                m_isLoaded = true;
                return true;
            }
            catch (...)
            {
                m_isLoaded = false;
                return false;
            }
        }

        // ==================== Query Operations ====================

    
        size_t AppointmentRepository::count() const
        {
            if (!m_isLoaded)
            {
                const_cast<AppointmentRepository*>(this)->load();
            }
            return m_appointments.size();
        }

        // SỬA ĐỔI QUAN TRỌNG: Tương tự count(), lazy load nếu cần thiết.
        bool AppointmentRepository::exists(const std::string &id) const
        {
            if (!m_isLoaded)
            {
                const_cast<AppointmentRepository*>(this)->load();
            }

            return std::any_of(m_appointments.begin(), m_appointments.end(),
                               [&id](const Model::Appointment &apt)
                               {
                                   return apt.getAppointmentID() == id;
                               });
        }

        bool AppointmentRepository::clear()
        {
            m_appointments.clear();
            // Khi clear, ta coi như đã load (dữ liệu rỗng) để tránh load lại từ file cũ
            m_isLoaded = true;
            return save();
        }

        // ==================== Patient-Related Queries ====================
        std::vector<Model::Appointment> AppointmentRepository::getByPatient(const std::string &patientUsername)
        {
            if (!m_isLoaded) load();

            std::vector<Model::Appointment> result;
            std::copy_if(m_appointments.begin(), m_appointments.end(),
                         std::back_inserter(result),
                         [&patientUsername](const Model::Appointment &apt)
                         {
                             return apt.getPatientUsername() == patientUsername;
                         });
            return result;
        }

        std::vector<Model::Appointment> AppointmentRepository::getUpcomingByPatient(const std::string &patientUsername)
        {
            if (!m_isLoaded) load();

            std::vector<Model::Appointment> result;
            std::string today = Utils::getCurrentDate();

            std::copy_if(m_appointments.begin(), m_appointments.end(),
                         std::back_inserter(result),
                         [&patientUsername, &today](const Model::Appointment &apt)
                         {
                             return apt.getPatientUsername() == patientUsername &&
                                    apt.getStatus() == AppointmentStatus::SCHEDULED &&
                                    Utils::compareDates(apt.getDate(), today) >= 0;
                         });

            std::sort(result.begin(), result.end(),
                      [](const Model::Appointment &a, const Model::Appointment &b)
                      {
                          if (a.getDate() != b.getDate())
                          {
                              return Utils::compareDates(a.getDate(), b.getDate()) < 0;
                          }
                          return a.getTime() < b.getTime();
                      });

            return result;
        }

        std::vector<Model::Appointment> AppointmentRepository::getHistoryByPatient(const std::string &patientUsername)
        {
            if (!m_isLoaded) load();

            std::vector<Model::Appointment> result;

            std::copy_if(m_appointments.begin(), m_appointments.end(),
                         std::back_inserter(result),
                         [&patientUsername](const Model::Appointment &apt)
                         {
                             return apt.getPatientUsername() == patientUsername &&
                                    (apt.getStatus() == AppointmentStatus::COMPLETED ||
                                     apt.getStatus() == AppointmentStatus::CANCELLED ||
                                     apt.getStatus() == AppointmentStatus::NO_SHOW);
                         });

            std::sort(result.begin(), result.end(),
                      [](const Model::Appointment &a, const Model::Appointment &b)
                      {
                          if (a.getDate() != b.getDate())
                          {
                              return Utils::compareDates(a.getDate(), b.getDate()) > 0;
                          }
                          return a.getTime() > b.getTime();
                      });

            return result;
        }

        std::vector<Model::Appointment> AppointmentRepository::getUnpaidByPatient(const std::string &patientUsername)
        {
            if (!m_isLoaded) load();

            std::vector<Model::Appointment> result;

            std::copy_if(m_appointments.begin(), m_appointments.end(),
                         std::back_inserter(result),
                         [&patientUsername](const Model::Appointment &apt)
                         {
                             return apt.getPatientUsername() == patientUsername && !apt.isPaid();
                         });

            return result;
        }

        // ==================== Doctor-Related Queries ====================
        std::vector<Model::Appointment> AppointmentRepository::getByDoctor(const std::string &doctorID)
        {
            if (!m_isLoaded) load();

            std::vector<Model::Appointment> result;

            std::copy_if(m_appointments.begin(), m_appointments.end(),
                         std::back_inserter(result),
                         [&doctorID](const Model::Appointment &apt)
                         {
                             return apt.getDoctorID() == doctorID;
                         });

            return result;
        }

        std::vector<Model::Appointment> AppointmentRepository::getByDoctorAndDate(const std::string &doctorID,
                                                                                  const std::string &date)
        {
            if (!m_isLoaded) load();

            std::vector<Model::Appointment> result;

            std::copy_if(m_appointments.begin(), m_appointments.end(),
                         std::back_inserter(result),
                         [&doctorID, &date](const Model::Appointment &apt)
                         {
                             return apt.getDoctorID() == doctorID && apt.getDate() == date;
                         });

            std::sort(result.begin(), result.end(),
                      [](const Model::Appointment &a, const Model::Appointment &b)
                      {
                          return a.getTime() < b.getTime();
                      });

            return result;
        }

        std::vector<Model::Appointment> AppointmentRepository::getUpcomingByDoctor(const std::string &doctorID)
        {
            if (!m_isLoaded) load();

            std::vector<Model::Appointment> result;
            std::string today = Utils::getCurrentDate();

            std::copy_if(m_appointments.begin(), m_appointments.end(),
                         std::back_inserter(result),
                         [&doctorID, &today](const Model::Appointment &apt)
                         {
                             return apt.getDoctorID() == doctorID &&
                                    apt.getStatus() == AppointmentStatus::SCHEDULED &&
                                    Utils::compareDates(apt.getDate(), today) >= 0;
                         });

            std::sort(result.begin(), result.end(),
                      [](const Model::Appointment &a, const Model::Appointment &b)
                      {
                          if (a.getDate() != b.getDate())
                          {
                              return Utils::compareDates(a.getDate(), b.getDate()) < 0;
                          }
                          return a.getTime() < b.getTime();
                      });

            return result;
        }

        // ==================== Date-Based Queries ====================
        std::vector<Model::Appointment> AppointmentRepository::getByDate(const std::string &date)
        {
            if (!m_isLoaded) load();

            std::vector<Model::Appointment> result;

            std::copy_if(m_appointments.begin(), m_appointments.end(),
                         std::back_inserter(result),
                         [&date](const Model::Appointment &apt)
                         {
                             return apt.getDate() == date;
                         });

            std::sort(result.begin(), result.end(),
                      [](const Model::Appointment &a, const Model::Appointment &b)
                      {
                          return a.getTime() < b.getTime();
                      });

            return result;
        }

        std::vector<Model::Appointment> AppointmentRepository::getByDateRange(const std::string &startDate,
                                                                              const std::string &endDate)
        {
            if (!m_isLoaded) load();

            std::vector<Model::Appointment> result;

            std::copy_if(m_appointments.begin(), m_appointments.end(),
                         std::back_inserter(result),
                         [&startDate, &endDate](const Model::Appointment &apt)
                         {
                             int cmpStart = Utils::compareDates(apt.getDate(), startDate);
                             int cmpEnd = Utils::compareDates(apt.getDate(), endDate);
                             return cmpStart >= 0 && cmpEnd <= 0;
                         });

            std::sort(result.begin(), result.end(),
                      [](const Model::Appointment &a, const Model::Appointment &b)
                      {
                          if (a.getDate() != b.getDate())
                          {
                              return Utils::compareDates(a.getDate(), b.getDate()) < 0;
                          }
                          return a.getTime() < b.getTime();
                      });

            return result;
        }

        std::vector<Model::Appointment> AppointmentRepository::getToday()
        {
            return getByDate(Utils::getCurrentDate());
        }

        // ==================== Status-Based Queries ====================
        std::vector<Model::Appointment> AppointmentRepository::getByStatus(AppointmentStatus status)
        {
            if (!m_isLoaded) load();

            std::vector<Model::Appointment> result;

            std::copy_if(m_appointments.begin(), m_appointments.end(),
                         std::back_inserter(result),
                         [&status](const Model::Appointment &apt)
                         {
                             return apt.getStatus() == status;
                         });

            return result;
        }

        std::vector<Model::Appointment> AppointmentRepository::getScheduled()
        {
            return getByStatus(AppointmentStatus::SCHEDULED);
        }

        std::vector<Model::Appointment> AppointmentRepository::getCompleted()
        {
            return getByStatus(AppointmentStatus::COMPLETED);
        }

        std::vector<Model::Appointment> AppointmentRepository::getCancelled()
        {
            return getByStatus(AppointmentStatus::CANCELLED);
        }

        // ==================== Slot Availability ====================
        bool AppointmentRepository::isSlotAvailable(const std::string &doctorID,
                                                    const std::string &date,
                                                    const std::string &time)
        {
            if (!m_isLoaded) load();

            return std::none_of(m_appointments.begin(), m_appointments.end(),
                                [&doctorID, &date, &time](const Model::Appointment &apt)
                                {
                                    return apt.getDoctorID() == doctorID &&
                                           apt.getDate() == date &&
                                           apt.getTime() == time &&
                                           apt.getStatus() == AppointmentStatus::SCHEDULED;
                                });
        }

        std::vector<std::string> AppointmentRepository::getBookedSlots(const std::string &doctorID,
                                                                       const std::string &date)
        {
            if (!m_isLoaded) load();

            std::vector<std::string> slots;

            for (const auto &apt : m_appointments)
            {
                if (apt.getDoctorID() == doctorID &&
                    apt.getDate() == date &&
                    apt.getStatus() == AppointmentStatus::SCHEDULED)
                {
                    slots.push_back(apt.getTime());
                }
            }

            std::sort(slots.begin(), slots.end());
            slots.erase(std::unique(slots.begin(), slots.end()), slots.end());

            return slots;
        }

        // ==================== ID Generation ====================
        std::string AppointmentRepository::getNextId()
        {
            return Utils::generateAppointmentID();
        }

        // ==================== File Path ====================
        void AppointmentRepository::setFilePath(const std::string &filePath)
        {
            m_filePath = filePath;
            m_isLoaded = false;
        }

        std::string AppointmentRepository::getFilePath() const
        {
            return m_filePath;
        }

    } // namespace DAL
} // namespace HMS