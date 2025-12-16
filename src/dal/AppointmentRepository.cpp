#include "../include/dal/AppointmentRepository.h"
#include <fstream>
#include <iostream>

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
                std::cerr << "Appointment file not found, starting empty.\n";
                m_isLoaded = true;
                return true;
            }

            std::string line;
            while (std::getline(file, line))
            {
                auto result = Model::Appointment::deserialize(line);
                if (result.has_value())
                {
                    m_appointments.push_back(*result);
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
                std::cerr << "Failed to open appointment file for writing.\n";
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

        bool AppointmentRepository::add(const Model::Appointment &appointment)
        {
            load();
            m_appointments.push_back(appointment);
            return true;
        }

        bool AppointmentRepository::clear()
        {
            m_appointments.clear();
            return true;
        }

        size_t AppointmentRepository::count() const
        {
            return m_appointments.size();
        }

        bool AppointmentRepository::exists(const std::string &id) const
        {
            for (const auto &a : m_appointments)
            {
                if (a.getAppointmentID() == id)
                    return true;
            }
            return false;
        }

    } // namespace DAL
} // namespace HMS
