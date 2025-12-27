#include "dal/PatientRepository.h"
#include "common/Constants.h"
#include "common/Utils.h"
#include "dal/FileHelper.h"

#include <algorithm>
#include <filesystem>
#include <format>
#include <sstream>

namespace HMS
{
    namespace DAL
    {
        // ==================== Static Members Initialization ====================
        std::unique_ptr<PatientRepository> PatientRepository::s_instance = nullptr;
        std::mutex PatientRepository::s_mutex;

        // ==================== Private Constructor ====================
        PatientRepository::PatientRepository()
            : m_filePath(Constants::PATIENT_FILE),
              m_isLoaded(false)
        {
        }

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

        // ==================== Destructor ====================
        PatientRepository::~PatientRepository() = default;

        // ==================== Private Helper ====================
        void PatientRepository::ensureLoaded() const
        {
            if (!m_isLoaded)
            {
                const_cast<PatientRepository *>(this)->loadInternal();
            }
        }

        // ==================== CRUD Operations ====================
        std::vector<Model::Patient> PatientRepository::getAll()
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();
            return m_patients;
        }

        std::optional<Model::Patient> PatientRepository::getById(const std::string &id)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            auto it = std::ranges::find_if(
                m_patients, [&id](const auto &p)
                {
                    return p.getPatientID() == id;
                }
            );

            if (it != m_patients.end())
            {
                return *it;
            }
            return std::nullopt;
        }

        bool PatientRepository::add(const Model::Patient &patient)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            // Check if patient ID already exists
            bool idExists = std::ranges::any_of(
                m_patients, [&patient](const auto &p)
                { return p.getPatientID() == patient.getPatientID(); });

            if (idExists)
            {
                return false;
            }

            // Check if username already exists
            bool usernameExists = std::ranges::any_of(
                m_patients, [&patient](const auto &p)
                { return p.getUsername() == patient.getUsername(); });

            if (usernameExists)
            {
                return false;
            }

            m_patients.push_back(patient);
            return saveInternal();
        }

        bool PatientRepository::update(const Model::Patient &patient)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            auto it = std::ranges::find_if(
                m_patients, [&patient](const auto &p)
                {
                    return p.getPatientID() == patient.getPatientID();
                }
            );

            if (it != m_patients.end())
            {
                *it = patient;
                return saveInternal();
            }
            return false;
        }

        bool PatientRepository::remove(const std::string &id)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            auto it = std::ranges::find_if(
                m_patients, [&id](const auto &p)
                {
                    return p.getPatientID() == id;
                }
            );

            if (it == m_patients.end())
            {
                return false; // Not found
            }

            m_patients.erase(it);
            return saveInternal();
        }

        // ==================== Persistence ====================
        bool PatientRepository::save()
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            return saveInternal();
        }

        bool PatientRepository::saveInternal()
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
            std::lock_guard<std::mutex> lock(m_dataMutex);
            return loadInternal();
        }

        bool PatientRepository::loadInternal()
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
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();
            return m_patients.size();
        }

        bool PatientRepository::exists(const std::string &id) const
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            return std::ranges::any_of(
                m_patients, [&id](const auto &p)
                {
                    return p.getPatientID() == id;
                }
            );
        }

        bool PatientRepository::clear()
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            m_patients.clear();
            m_isLoaded = true;
            return saveInternal();
        }

        // ==================== Patient-Specific Queries ====================
        std::optional<Model::Patient> PatientRepository::getByUsername(const std::string &username)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            auto it = std::ranges::find_if(
                m_patients, [&username](const auto &p)
                {
                    return p.getUsername() == username;
                }
            );

            if (it != m_patients.end())
            {
                return *it;
            }
            return std::nullopt;
        }

        std::vector<Model::Patient> PatientRepository::searchByName(const std::string &name)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            std::vector<Model::Patient> results;
            std::ranges::copy_if(
                m_patients, std::back_inserter(results),
                [&name](const auto &p)
                {
                    return Utils::containsIgnoreCase(p.getName(), name);
                }
            );

            return results;
        }

        std::vector<Model::Patient> PatientRepository::searchByPhone(const std::string &phone)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            std::vector<Model::Patient> results;
            std::ranges::copy_if(
                m_patients, std::back_inserter(results),
                [&phone](const auto &p)
                {
                    return p.getPhone().find(phone) != std::string::npos;
                }
            );

            return results;
        }

        std::vector<Model::Patient> PatientRepository::search(const std::string &keyword)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            std::vector<Model::Patient> results;
            std::ranges::copy_if(
                m_patients, std::back_inserter(results),
                [&keyword](const auto &p)
                {
                    return Utils::containsIgnoreCase(p.getName(), keyword) ||
                           Utils::containsIgnoreCase(p.getPhone(), keyword) ||
                           Utils::containsIgnoreCase(p.getAddress(), keyword);
                }
            );

            return results;
        }

        std::string PatientRepository::getNextId()
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            if (m_patients.empty())
            {
                return std::format("{}001", Constants::PATIENT_ID_PREFIX);
            }

            // Find max ID number
            int maxID = 0;
            const std::string prefix = Constants::PATIENT_ID_PREFIX;

            for (const auto &patient : m_patients)
            {
                const std::string &patientID = patient.getPatientID();

                // Only process valid format: prefix + digits
                if (patientID.length() > prefix.length() &&
                    patientID.starts_with(prefix))
                {
                    std::string numPart = patientID.substr(prefix.length());

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
        void PatientRepository::setFilePath(const std::string &filePath)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            m_filePath = filePath;
            m_isLoaded = false; // Force reload with new file
        }

        std::string PatientRepository::getFilePath() const
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            return m_filePath;
        }

    } // namespace DAL
} // namespace HMS
