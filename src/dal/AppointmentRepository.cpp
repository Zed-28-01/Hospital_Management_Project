#include "dal/AppointmentRepository.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <format>
#include <algorithm>
#include <format>

namespace HMS
{
    namespace DAL
    {

        // ==================== Static Members ====================
        AppointmentRepository *AppointmentRepository::s_instance = nullptr;
        std::mutex AppointmentRepository::s_mutex;

        // ==================== Constructor / Destructor ====================
        AppointmentRepository::AppointmentRepository()
            : m_filePath("data/appointments.txt"),
              m_isLoaded(false) {}

        AppointmentRepository::~AppointmentRepository()
        {
            save();
        }

        // ==================== Singleton ====================
        AppointmentRepository *AppointmentRepository::getInstance()
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            if (!s_instance)
            {
                s_instance = new AppointmentRepository();
            }
            return s_instance;
        }

        // ==================== Persistence ====================
        bool AppointmentRepository::load()
        {
            if (m_isLoaded)
                return true;

            std::ifstream file(m_filePath);
            if (!file.is_open())
            {
                std::cerr << std::format("Appointment file '{}' not found, starting empty.\n", m_filePath);
                m_isLoaded = true;
                return true;
            }

            std::string line;
            while (std::getline(file, line))
            {
                if (line.empty())
                    continue; // skip blank lines

                auto result = Model::Appointment::deserialize(line);
                if (result.has_value())
                {
                    m_appointments.push_back(*result);
                }
                else
                {
                    std::cerr << std::format("Warning: failed to parse appointment line, skipping: '{}'\n", line);
                }
            }

            file.close();
            m_isLoaded = true;
            return true;
        }

        bool AppointmentRepository::save()
        {
            std::ofstream file(m_filePath);
            if (!file.is_open())
            {
                std::cerr << std::format("Failed to open appointment file '{}' for writing.\n", m_filePath);
                return false;
            }

            for (const auto &appt : m_appointments)
            {
                file << appt.serialize() << "\n";
            }

            file.close();
            return true;
        }

        // ==================== File Path ====================
        void AppointmentRepository::setFilePath(const std::string &filePath)
        {
            m_filePath = filePath;
        }

        std::string AppointmentRepository::getFilePath() const
        {
            return m_filePath;
        }

        // ==================== Minimal CRUD (stub for now) ====================
        std::vector<Model::Appointment> AppointmentRepository::getAll()
        {
            load();
            return m_appointments;
        }

        std::optional<Model::Appointment> AppointmentRepository::getById(const std::string &id)
        {
            load();
            auto it = std::find_if(m_appointments.begin(), m_appointments.end(),
                                   [&id](const Model::Appointment &a)
                                   { return a.getAppointmentID() == id; });
            if (it != m_appointments.end())
                return *it;
            return std::nullopt;
        }

        bool AppointmentRepository::add(const Model::Appointment &appointment)
        {
            load();
            // Do not add duplicate appointment IDs
            if (exists(appointment.getAppointmentID()))
            {
                std::cerr << std::format("Attempted to add appointment with duplicate ID: {}\n", appointment.getAppointmentID());
                return false;
            }

            m_appointments.push_back(appointment);
            // persist immediately and report success of persistence
            if (!save())
            {
                std::cerr << std::format("Warning: failed to save after adding appointment {}\n", appointment.getAppointmentID());
                return false;
            }
            return true;
        }

        bool AppointmentRepository::update(const Model::Appointment &appointment)
        {
            load();
            for (auto &a : m_appointments)
            {
                if (a.getAppointmentID() == appointment.getAppointmentID())
                {
                    a = appointment;
                    if (!save())
                    {
                        std::cerr << std::format("Warning: failed to save after updating appointment {}\n", appointment.getAppointmentID());
                        return false;
                    }
                    return true;
                }
            }
            std::cerr << std::format("Appointment not found for update: {}\n", appointment.getAppointmentID());
            return false;
        }

        bool AppointmentRepository::remove(const std::string &id)
        {
            load();
            auto it = std::find_if(m_appointments.begin(), m_appointments.end(),
                                   [&id](const Model::Appointment &a)
                                   { return a.getAppointmentID() == id; });
            if (it == m_appointments.end())
            {
                std::cerr << std::format("Appointment not found for removal: {}\n", id);
                return false;
            }
            m_appointments.erase(it);
            if (!save())
            {
                std::cerr << std::format("Warning: failed to save after removing appointment {}\n", id);
                return false;
            }
            return true;
        }

        bool AppointmentRepository::clear()
        {
            m_appointments.clear();
            if (!save())
            {
                std::cerr << std::format("Warning: failed to save after clearing appointments\n");
                return false;
            }
            return true;
        }

        size_t AppointmentRepository::count() const
        {
            const_cast<AppointmentRepository *>(this)->load();
            return m_appointments.size();
        }

        bool AppointmentRepository::exists(const std::string &id) const
        {
            const_cast<AppointmentRepository *>(this)->load();
            for (const auto &a : m_appointments)
            {
                if (a.getAppointmentID() == id)
                    return true;
            }
            return false;
        }

    } // namespace DAL
} // namespace HMS
