#include "dal/AppointmentRepository.h"
#include "dal/FileHelper.h"
#include "common/Utils.h"
#include "common/Constants.h"

#include <algorithm>
#include <sstream>
#include <filesystem>

namespace HMS
{
    namespace DAL
    {
        // ==================== Static Members Initialization ====================
        std::unique_ptr<AppointmentRepository> AppointmentRepository::s_instance = nullptr;
        std::mutex AppointmentRepository::s_mutex;

        // ==================== Private Constructor ====================
        AppointmentRepository::AppointmentRepository()
            : m_filePath(Constants::APPOINTMENT_FILE), m_isLoaded(false)
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

        // ==================== Destructor ====================
        AppointmentRepository::~AppointmentRepository() = default;

        // ==================== Private Helper ====================
        void AppointmentRepository::ensureLoaded() const
        {
            if (!m_isLoaded)
            {
                const_cast<AppointmentRepository *>(this)->loadInternal();
            }
        }

        // ==================== CRUD Operations ====================
        std::vector<Model::Appointment> AppointmentRepository::getAll()
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();
            return m_appointments;
        }

        std::optional<Model::Appointment> AppointmentRepository::getById(const std::string &id)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            auto it = std::ranges::find_if(
                m_appointments, [&id](const auto &a)
                {
                    return a.getAppointmentID() == id;
                }
            );

            if (it != m_appointments.end())
            {
                return *it;
            }
            return std::nullopt;
        }

        bool AppointmentRepository::add(const Model::Appointment &appointment)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            // Check if appointment ID already exists (inline for efficiency)
            for (const auto &a : m_appointments)
            {
                if (a.getAppointmentID() == appointment.getAppointmentID())
                {
                    return false;
                }
            }

            m_appointments.push_back(appointment);
            return saveInternal();
        }

        bool AppointmentRepository::update(const Model::Appointment &appointment)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            auto it = std::ranges::find_if(
                m_appointments, [&appointment](const auto &a)
                {
                    return a.getAppointmentID() == appointment.getAppointmentID();
                }
            );

            if (it != m_appointments.end())
            {
                *it = appointment;
                return saveInternal();
            }
            return false;
        }

        bool AppointmentRepository::remove(const std::string &id)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            auto it = std::ranges::find_if(
                m_appointments, [&id](const auto &a)
                {
                    return a.getAppointmentID() == id;
                }
            );

            if (it == m_appointments.end())
            {
                return false;
            }

            m_appointments.erase(it);
            return saveInternal();
        }

        // ==================== Persistence ====================
        bool AppointmentRepository::save()
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            return saveInternal();
        }

        bool AppointmentRepository::saveInternal()
        {
            try
            {
                std::vector<std::string> lines;

                // Add header
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

                // Add data
                for (const auto &appointment : m_appointments)
                {
                    lines.push_back(appointment.serialize());
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
            std::lock_guard<std::mutex> lock(m_dataMutex);
            return loadInternal();
        }

        bool AppointmentRepository::loadInternal()
        {
            try
            {
                // Create parent directory of the file if it doesn't exist
                std::filesystem::path filePath(m_filePath);
                std::filesystem::path parentDir = filePath.parent_path();

                if (!parentDir.empty())
                {
                    FileHelper::createDirectoryIfNotExists(parentDir.string());
                }

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
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();
            return m_appointments.size();
        }

        bool AppointmentRepository::exists(const std::string &id) const
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            return std::ranges::any_of(
                m_appointments, [&id](const auto &a)
                {
                    return a.getAppointmentID() == id;
                }
            );
        }

        bool AppointmentRepository::clear()
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            m_appointments.clear();
            m_isLoaded = true;
            return saveInternal();
        }

        // ==================== Patient-Related Queries ====================
        std::vector<Model::Appointment> AppointmentRepository::getByPatient(const std::string &patientUsername)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            std::vector<Model::Appointment> results;
            std::ranges::copy_if(
                m_appointments, std::back_inserter(results),
                [&patientUsername](const auto &a)
                {
                    return a.getPatientUsername() == patientUsername;
                }
            );

            return results;
        }

        std::vector<Model::Appointment> AppointmentRepository::getUpcomingByPatient(const std::string &patientUsername)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            std::string today = Utils::getCurrentDate();
            std::vector<Model::Appointment> results;

            std::ranges::copy_if(
                m_appointments, std::back_inserter(results),
                [&patientUsername, &today](const auto &a)
                {
                    return a.getPatientUsername() == patientUsername &&
                           a.getStatus() == AppointmentStatus::SCHEDULED &&
                           a.getDate() >= today;
                }
            );

            // Sort by date and time
            std::ranges::sort(results, [](const auto &a, const auto &b)
            {
                if (a.getDate() != b.getDate())
                    return a.getDate() < b.getDate();
                return a.getTime() < b.getTime();
            });

            return results;
        }

        std::vector<Model::Appointment> AppointmentRepository::getHistoryByPatient(const std::string &patientUsername)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            std::vector<Model::Appointment> results;
            std::ranges::copy_if(
                m_appointments, std::back_inserter(results),
                [&patientUsername](const auto &a)
                {
                    return a.getPatientUsername() == patientUsername &&
                           (a.getStatus() == AppointmentStatus::COMPLETED ||
                            a.getStatus() == AppointmentStatus::CANCELLED ||
                            a.getStatus() == AppointmentStatus::NO_SHOW);
                }
            );

            // Sort by date descending (most recent first)
            std::ranges::sort(results, [](const auto &a, const auto &b)
            {
                if (a.getDate() != b.getDate())
                    return a.getDate() > b.getDate();
                return a.getTime() > b.getTime();
            });

            return results;
        }

        std::vector<Model::Appointment> AppointmentRepository::getUnpaidByPatient(const std::string &patientUsername)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            std::vector<Model::Appointment> results;
            std::ranges::copy_if(
                m_appointments, std::back_inserter(results),
                [&patientUsername](const auto &a)
                {
                    return a.getPatientUsername() == patientUsername && !a.isPaid();
                }
            );

            return results;
        }

        // ==================== Doctor-Related Queries ====================
        std::vector<Model::Appointment> AppointmentRepository::getByDoctor(const std::string &doctorID)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            std::vector<Model::Appointment> results;
            std::ranges::copy_if(
                m_appointments, std::back_inserter(results),
                [&doctorID](const auto &a)
                {
                    return a.getDoctorID() == doctorID;
                }
            );

            return results;
        }

        std::vector<Model::Appointment> AppointmentRepository::getByDoctorAndDate(
            const std::string &doctorID, const std::string &date)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            std::vector<Model::Appointment> results;
            std::ranges::copy_if(
                m_appointments, std::back_inserter(results),
                [&doctorID, &date](const auto &a)
                {
                    return a.getDoctorID() == doctorID &&
                           a.getDate() == date &&
                           a.getStatus() != AppointmentStatus::CANCELLED;
                }
            );

            // Sort by time
            std::ranges::sort(results, [](const auto &a, const auto &b)
            {
                return a.getTime() < b.getTime();
            });

            return results;
        }

        std::vector<Model::Appointment> AppointmentRepository::getUpcomingByDoctor(const std::string &doctorID)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            std::string today = Utils::getCurrentDate();
            std::vector<Model::Appointment> results;

            std::ranges::copy_if(
                m_appointments, std::back_inserter(results),
                [&doctorID, &today](const auto &a)
                {
                    return a.getDoctorID() == doctorID &&
                           a.getStatus() == AppointmentStatus::SCHEDULED &&
                           a.getDate() >= today;
                }
            );

            // Sort by date and time
            std::ranges::sort(results, [](const auto &a, const auto &b)
            {
                if (a.getDate() != b.getDate())
                    return a.getDate() < b.getDate();
                return a.getTime() < b.getTime();
            });

            return results;
        }

        // ==================== Date-Based Queries ====================
        std::vector<Model::Appointment> AppointmentRepository::getByDate(const std::string &date)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            std::vector<Model::Appointment> results;
            std::ranges::copy_if(
                m_appointments, std::back_inserter(results),
                [&date](const auto &a)
                {
                    return a.getDate() == date;
                }
            );

            // Sort by time
            std::ranges::sort(results, [](const auto &a, const auto &b)
            {
                return a.getTime() < b.getTime();
            });

            return results;
        }

        std::vector<Model::Appointment> AppointmentRepository::getByDateRange(
            const std::string &startDate, const std::string &endDate)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            std::vector<Model::Appointment> results;
            std::ranges::copy_if(
                m_appointments, std::back_inserter(results),
                [&startDate, &endDate](const auto &a)
                {
                    return a.getDate() >= startDate && a.getDate() <= endDate;
                }
            );

            // Sort by date and time
            std::ranges::sort(results, [](const auto &a, const auto &b)
            {
                if (a.getDate() != b.getDate())
                    return a.getDate() < b.getDate();
                return a.getTime() < b.getTime();
            });

            return results;
        }

        std::vector<Model::Appointment> AppointmentRepository::getToday()
        {
            return getByDate(Utils::getCurrentDate());
        }

        // ==================== Status-Based Queries ====================
        std::vector<Model::Appointment> AppointmentRepository::getByStatus(AppointmentStatus status)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            std::vector<Model::Appointment> results;
            std::ranges::copy_if(
                m_appointments, std::back_inserter(results),
                [status](const auto &a)
                {
                    return a.getStatus() == status;
                }
            );

            return results;
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
        bool AppointmentRepository::isSlotAvailable(
            const std::string &doctorID, const std::string &date, const std::string &time)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            return std::ranges::none_of(
                m_appointments, [&doctorID, &date, &time](const auto &a)
                {
                    return a.getDoctorID() == doctorID &&
                           a.getDate() == date &&
                           a.getTime() == time &&
                           a.getStatus() != AppointmentStatus::CANCELLED;
                }
            );
        }

        bool AppointmentRepository::isSlotAvailable(
            const std::string &doctorID, const std::string &date,
            const std::string &time, const std::string &excludeAppointmentID)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            return std::ranges::none_of(
                m_appointments, [&doctorID, &date, &time, &excludeAppointmentID](const auto &a)
                {
                    return a.getDoctorID() == doctorID &&
                           a.getDate() == date &&
                           a.getTime() == time &&
                           a.getAppointmentID() != excludeAppointmentID &&
                           a.getStatus() != AppointmentStatus::CANCELLED;
                }
            );
        }

        std::vector<std::string> AppointmentRepository::getBookedSlots(
            const std::string &doctorID, const std::string &date)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            std::vector<std::string> slots;
            for (const auto &a : m_appointments)
            {
                if (a.getDoctorID() == doctorID &&
                    a.getDate() == date &&
                    a.getStatus() != AppointmentStatus::CANCELLED)
                {
                    slots.push_back(a.getTime());
                }
            }

            // Sort the slots
            std::ranges::sort(slots);
            return slots;
        }

        // ==================== ID Generation ====================
        std::string AppointmentRepository::getNextId()
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            if (m_appointments.empty())
            {
                return std::format("{}001", Constants::APPOINTMENT_ID_PREFIX);
            }

            // Find max ID number
            int maxID = 0;
            const std::string prefix = Constants::APPOINTMENT_ID_PREFIX;

            for (const auto &appointment : m_appointments)
            {
                const std::string &appointmentID = appointment.getAppointmentID();

                // Only process valid format: prefix + digits
                if (appointmentID.length() > prefix.length() &&
                    appointmentID.starts_with(prefix))
                {
                    std::string numPart = appointmentID.substr(prefix.length());

                    // Validate numeric before parsing
                    if (Utils::isNumeric(numPart))
                    {
                        try
                        {
                            int idNum = std::stoi(numPart);
                            maxID = std::max(maxID, idNum);
                        }
                        catch (const std::exception &)
                        {
                            // Ignore parse errors
                        }
                    }
                }
            }

            return std::format("{}{:03d}", prefix, maxID + 1);
        }

        // ==================== File Path Management ====================
        void AppointmentRepository::setFilePath(const std::string &filePath)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            m_filePath = filePath;
            m_isLoaded = false;
        }

        std::string AppointmentRepository::getFilePath() const
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            return m_filePath;
        }

    } // namespace DAL
} // namespace HMS
