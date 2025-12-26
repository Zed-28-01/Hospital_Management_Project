#include "dal/PatientRepository.h"
#include "dal/FileHelper.h"
#include "common/Utils.h"
#include "common/Constants.h"

#include <algorithm>
#include <sstream>
#include <format>
#include <filesystem>

namespace HMS
{
    namespace DAL
    {
        std::unique_ptr<PatientRepository> PatientRepository::s_instance = nullptr;
        std::mutex PatientRepository::s_mutex;

        // ==================== Private Constructor ====================
        PatientRepository::PatientRepository()
            : m_filePath(Constants::PATIENT_FILE),
              m_isLoaded(false) {}

        // ==================== Singleton Access ====================
        PatientRepository *PatientRepository::getInstance()
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            if (!s_instance)
            {
                s_instance = std::unique_ptr<PatientRepository>(new PatientRepository());
            }
            return s_instance.get();
        }

        void PatientRepository::resetInstance()
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_instance.reset();
        }

        PatientRepository::~PatientRepository() = default;

        // ==================== CRUD Operations ====================
        std::vector<Model::Patient> PatientRepository::getAll()
        {
            if (!m_isLoaded)
            {
                load();
            }

            return m_patients;
        }

        std::optional<Model::Patient> PatientRepository::getById(const std::string &id)
        {
            if (!m_isLoaded)
            {
                load();
            }

            auto it = std::find_if(m_patients.begin(), m_patients.end(),
                                   [&id](const Model::Patient &p)
                                   {
                                       return p.getPatientID() == id;
                                   });

            if (it != m_patients.end())
            {
                return *it;
            }
            return std::nullopt;
        }

        bool PatientRepository::add(const Model::Patient &patient)
        {
            if (!m_isLoaded)
            {
                load();
            }

            // Check if patient ID already exists
            if (exists(patient.getPatientID()))
            {
                return false;
            }

            // Check if username already exists
            if (getByUsername(patient.getUsername()).has_value())
            {
                return false;
            }

            m_patients.push_back(patient);
            return save();
        }

        bool PatientRepository::update(const Model::Patient &patient)
        {
            if (!m_isLoaded)
            {
                load();
            }

            auto it = std::find_if(m_patients.begin(), m_patients.end(),
                                   [&patient](const Model::Patient &p)
                                   {
                                       return p.getPatientID() == patient.getPatientID();
                                   });

            if (it != m_patients.end())
            {
                *it = patient;
                return save();
            }
            return false;
        }

        bool PatientRepository::remove(const std::string &id)
        {
            if (!m_isLoaded)
            {
                load();
            }

            auto it = std::find_if(
                m_patients.begin(),
                m_patients.end(),
                [&id](const Model::Patient &p)
                {
                    return p.getPatientID() == id;
                });

            if (it == m_patients.end())
            {
                return false; // Not found
            }

            m_patients.erase(it);
            return save();
        }

        // ==================== Persistence ====================
        bool PatientRepository::save()
        {
            try
            {
                std::vector<std::string> lines;

                // Add header
                auto header = FileHelper::getFileHeader("Patient");
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
                for (const auto &patient : m_patients)
                {
                    lines.push_back(patient.serialize());
                }

                FileHelper::createBackup(m_filePath);
                return FileHelper::writeLines(m_filePath, lines);
            }
            catch (...)
            {
                return false;
            }
        }

        bool PatientRepository::load()
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

                m_patients.clear();

                for (const auto &line : lines)
                {
                    auto patient = Model::Patient::deserialize(line);
                    if (patient)
                    {
                        m_patients.push_back(patient.value());
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
        size_t PatientRepository::count() const
        {
            if (!m_isLoaded)
            {
                const_cast<PatientRepository *>(this)->load();
            }

            return m_patients.size();
        }

        bool PatientRepository::exists(const std::string &id) const
        {
            if (!m_isLoaded)
            {
                const_cast<PatientRepository *>(this)->load();
            }

            return std::any_of(m_patients.begin(), m_patients.end(),
                               [&id](const Model::Patient &p)
                               {
                                   return p.getPatientID() == id;
                               });
        }

        bool PatientRepository::clear()
        {
            m_patients.clear();
            m_isLoaded = true;
            return save();
        }

        // ==================== Patient-Specific Queries ====================
        std::optional<Model::Patient> PatientRepository::getByUsername(const std::string &username)
        {
            if (!m_isLoaded)
            {
                load();
            }

            auto it = std::find_if(m_patients.begin(), m_patients.end(),
                                   [&username](const Model::Patient &p)
                                   {
                                       return p.getUsername() == username;
                                   });

            if (it != m_patients.end())
            {
                return *it;
            }
            return std::nullopt;
        }

        std::vector<Model::Patient> PatientRepository::searchByName(const std::string &name)
        {
            if (!m_isLoaded)
            {
                load();
            }

            std::vector<Model::Patient> results;

            for (const auto &p : m_patients)
            {
                if (Utils::containsIgnoreCase(p.getName(), name))
                {
                    results.push_back(p);
                }
            }

            return results;
        }

        std::vector<Model::Patient> PatientRepository::searchByPhone(const std::string &phone)
        {
            if (!m_isLoaded)
            {
                load();
            }

            std::vector<Model::Patient> results;

            for (const auto &p : m_patients)
            {
                if (p.getPhone().find(phone) != std::string::npos)
                {
                    results.push_back(p);
                }
            }

            return results;
        }

        std::vector<Model::Patient> PatientRepository::search(const std::string &keyword)
        {
            if (!m_isLoaded)
            {
                load();
            }

            std::vector<Model::Patient> results;

            for (const auto &p : m_patients)
            {
                if (Utils::containsIgnoreCase(p.getName(), keyword) ||
                    Utils::containsIgnoreCase(p.getPhone(), keyword) ||
                    Utils::containsIgnoreCase(p.getAddress(), keyword))
                {
                    results.push_back(p);
                }
            }

            return results;
        }

        std::string PatientRepository::getNextId()
        {
            if (!m_isLoaded)
            {
                load();
            }

            int maxID = 0;
            const std::string prefix = Constants::PATIENT_ID_PREFIX;

            for (const auto &patient : m_patients)
            {
                const std::string &patientID = patient.getPatientID();

                // Only process valid format: prefix + digits
                if (patientID.length() > prefix.length() &&
                    patientID.substr(0, prefix.length()) == prefix)
                {
                    std::string numPart = patientID.substr(prefix.length());

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

        // ==================== File Path ====================
        void PatientRepository::setFilePath(const std::string &filePath)
        {
            m_filePath = filePath;
            m_isLoaded = false;
        }

        std::string PatientRepository::getFilePath() const
        {
            return m_filePath;
        }
    } // namespace DAL
} // namespace HMS