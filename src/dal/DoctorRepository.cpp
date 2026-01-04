#include "dal/DoctorRepository.h"
#include "common/Constants.h"
#include "common/Utils.h"
#include "dal/FileHelper.h"

#include <algorithm>
#include <filesystem>
#include <format>
#include <set>
#include <sstream>

namespace HMS
{
    namespace DAL
    {
        // ==================== Static Members Initialization ====================
        std::unique_ptr<DoctorRepository> DoctorRepository::s_instance = nullptr;
        std::mutex DoctorRepository::s_mutex;

        // ==================== Private Constructor ====================
        DoctorRepository::DoctorRepository()
            : m_filePath(Constants::DOCTOR_FILE), m_isLoaded(false)
        {
        }

        // ==================== Singleton Access ====================
        DoctorRepository *DoctorRepository::getInstance()
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            if (!s_instance)
            {
                s_instance = std::unique_ptr<DoctorRepository>(new DoctorRepository());
            }
            return s_instance.get();
        }

        void DoctorRepository::resetInstance()
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_instance.reset();
        }

        // ==================== Destructor ====================
        DoctorRepository::~DoctorRepository() = default;

        // ==================== Private Helper ====================
        void DoctorRepository::ensureLoaded() const
        {
            if (!m_isLoaded)
            {
                const_cast<DoctorRepository *>(this)->loadInternal();
            }
        }

        // ==================== CRUD Operations ====================
        std::vector<Model::Doctor> DoctorRepository::getAll()
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();
            return m_doctors;
        }

        std::optional<Model::Doctor> DoctorRepository::getById(const std::string &id)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            auto it = std::ranges::find_if(
                m_doctors, [&id](const auto &d)
                {
                    return d.getDoctorID() == id;
                }
            );

            if (it != m_doctors.end())
            {
                return *it;
            }
            return std::nullopt;
        }

        bool DoctorRepository::add(const Model::Doctor &doctor)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            // Check if doctor ID already exists (primary key check only)
            bool idExists = std::ranges::any_of(
                m_doctors, [&doctor](const auto &d)
                { return d.getDoctorID() == doctor.getDoctorID(); });

            if (idExists)
            {
                return false;
            }

            m_doctors.push_back(doctor);
            return saveInternal();
        }

        bool DoctorRepository::update(const Model::Doctor &doctor)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            auto it = std::ranges::find_if(
                m_doctors, [&doctor](const auto &d)
                {
                    return d.getDoctorID() == doctor.getDoctorID();
                }
            );

            if (it == m_doctors.end())
            {
                return false;
            }

            *it = doctor;
            return saveInternal();
        }

        bool DoctorRepository::remove(const std::string &id)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            auto it = std::ranges::find_if(
                m_doctors, [&id](const auto &d)
                {
                    return d.getDoctorID() == id;
                }
            );

            if (it == m_doctors.end())
            {
                return false;
            }

            m_doctors.erase(it);
            return saveInternal();
        }

        // ==================== Persistence ====================
        bool DoctorRepository::save()
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            return saveInternal();
        }

        bool DoctorRepository::saveInternal()
        {
            try
            {
                std::vector<std::string> lines;

                // Add header
                auto header = FileHelper::getFileHeader("Doctor");
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
                for (const auto &doctor : m_doctors)
                {
                    lines.push_back(doctor.serialize());
                }

                FileHelper::createBackup(m_filePath);
                return FileHelper::writeLines(m_filePath, lines);
            }
            catch (...)
            {
                return false;
            }
        }

        bool DoctorRepository::load()
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            return loadInternal();
        }

        bool DoctorRepository::loadInternal()
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

                m_doctors.clear();

                for (const auto &line : lines)
                {
                    auto doctor = Model::Doctor::deserialize(line);
                    if (doctor)
                    {
                        m_doctors.push_back(doctor.value());
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
        size_t DoctorRepository::count() const
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();
            return m_doctors.size();
        }

        bool DoctorRepository::exists(const std::string &id) const
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            return std::ranges::any_of(
                m_doctors, [&id](const auto &d)
                {
                    return d.getDoctorID() == id;
                }
            );
        }

        bool DoctorRepository::clear()
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            m_doctors.clear();
            m_isLoaded = true;
            return saveInternal();
        }

        // ==================== Doctor-Specific Queries ====================
        std::optional<Model::Doctor> DoctorRepository::getByUsername(const std::string &username)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            auto it = std::ranges::find_if(
                m_doctors, [&username](const auto &d)
                {
                    return d.getUsername() == username;
                }
            );

            if (it != m_doctors.end())
            {
                return *it;
            }
            return std::nullopt;
        }

        std::vector<Model::Doctor> DoctorRepository::getBySpecialization(const std::string &specialization)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            std::vector<Model::Doctor> results;
            std::ranges::copy_if(
                m_doctors, std::back_inserter(results),
                [&specialization](const auto &d)
                {
                    // Check if doctor has this specialization (supports partial match)
                    auto specs = d.getSpecializations();
                    return std::any_of(specs.begin(), specs.end(),
                        [&specialization](const std::string& spec) {
                            return Utils::containsIgnoreCase(spec, specialization);
                        });
                }
            );

            return results;
        }

        std::vector<Model::Doctor> DoctorRepository::searchByName(const std::string &name)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            std::vector<Model::Doctor> results;
            std::ranges::copy_if(
                m_doctors, std::back_inserter(results),
                [&name](const auto &d)
                {
                    return Utils::containsIgnoreCase(d.getName(), name);
                }
            );

            return results;
        }

        std::vector<Model::Doctor> DoctorRepository::search(const std::string &keyword)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            std::vector<Model::Doctor> results;
            std::ranges::copy_if(
                m_doctors, std::back_inserter(results),
                [&keyword](const auto &d)
                {
                    return Utils::containsIgnoreCase(d.getDoctorID(), keyword) ||
                           Utils::containsIgnoreCase(d.getName(), keyword) ||
                           Utils::containsIgnoreCase(d.getSpecialization(), keyword);
                }
            );

            return results;
        }

        std::vector<std::string> DoctorRepository::getAllSpecializations()
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            std::set<std::string> uniqueSpecs;
            for (const auto &d : m_doctors)
            {
                // Get all specializations for each doctor
                auto specs = d.getSpecializations();
                for (const auto &spec : specs)
                {
                    std::string trimmedSpec = Utils::trim(spec);
                    if (!trimmedSpec.empty())
                    {
                        uniqueSpecs.insert(trimmedSpec);
                    }
                }
            }

            return std::vector<std::string>(uniqueSpecs.begin(), uniqueSpecs.end());
        }

        std::string DoctorRepository::getNextId()
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            if (m_doctors.empty())
            {
                return std::format("{}001", Constants::DOCTOR_ID_PREFIX);
            }

            // Find max ID number
            int maxID = 0;
            const std::string prefix = Constants::DOCTOR_ID_PREFIX;

            for (const auto &doctor : m_doctors)
            {
                const std::string &doctorID = doctor.getDoctorID();

                // Only process valid format: prefix + digits
                if (doctorID.length() > prefix.length() &&
                    doctorID.starts_with(prefix))
                {
                    std::string numPart = doctorID.substr(prefix.length());

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
        void DoctorRepository::setFilePath(const std::string &filePath)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            m_filePath = filePath;
            m_isLoaded = false;
        }

        std::string DoctorRepository::getFilePath() const
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            return m_filePath;
        }

    } // namespace DAL
} // namespace HMS
