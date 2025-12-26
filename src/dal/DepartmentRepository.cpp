#include "dal/DepartmentRepository.h"
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
        // ==================== Static Members Initialization ====================
        std::unique_ptr<DepartmentRepository> DepartmentRepository::s_instance = nullptr;
        std::mutex DepartmentRepository::s_mutex;

        // ==================== Private Constructor ====================
        DepartmentRepository::DepartmentRepository()
            : m_filePath(Constants::DEPARTMENT_FILE),
              m_isLoaded(false)
        {
        }

        // ==================== Singleton Access ====================
        DepartmentRepository *DepartmentRepository::getInstance()
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            if (!s_instance)
            {
                s_instance = std::unique_ptr<DepartmentRepository>(new DepartmentRepository());
            }
            return s_instance.get();
        }

        void DepartmentRepository::resetInstance()
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_instance.reset();
        }

        // ==================== Destructor ====================
        DepartmentRepository::~DepartmentRepository() = default;

        // ==================== Private Helper ====================
        void DepartmentRepository::ensureLoaded() const
        {
            if (!m_isLoaded)
            {
                const_cast<DepartmentRepository *>(this)->loadInternal();
            }
        }

        // ==================== CRUD Operations ====================
        std::vector<Model::Department> DepartmentRepository::getAll()
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();
            return m_departments;
        }

        std::optional<Model::Department> DepartmentRepository::getById(const std::string &id)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            auto it = std::ranges::find_if(
                m_departments, [&id](const auto &d)
                {
                    return d.getDepartmentID() == id;
                }
            );

            if (it != m_departments.end())
            {
                return *it;
            }
            return std::nullopt;
        }

        bool DepartmentRepository::add(const Model::Department &department)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            // Check if department ID already exists (inline for efficiency)
            for (const auto &d : m_departments)
            {
                if (d.getDepartmentID() == department.getDepartmentID())
                {
                    return false;
                }
            }

            m_departments.push_back(department);
            return saveInternal();
        }

        bool DepartmentRepository::update(const Model::Department &department)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            auto it = std::ranges::find_if(
                m_departments, [&department](const auto &d)
                {
                    return d.getDepartmentID() == department.getDepartmentID();
                }
            );

            if (it != m_departments.end())
            {
                *it = department;
                return saveInternal();
            }
            return false;
        }

        bool DepartmentRepository::remove(const std::string &id)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            auto it = std::ranges::find_if(
                m_departments, [&id](const auto &d)
                {
                    return d.getDepartmentID() == id;
                }
            );

            if (it == m_departments.end())
            {
                return false;
            }

            m_departments.erase(it);
            return saveInternal();
        }

        // ==================== Persistence ====================
        bool DepartmentRepository::save()
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            return saveInternal();
        }

        bool DepartmentRepository::saveInternal()
        {
            try
            {
                std::vector<std::string> lines;

                // Add header
                auto header = FileHelper::getFileHeader("Department");
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
                for (const auto &department : m_departments)
                {
                    lines.push_back(department.serialize());
                }

                FileHelper::createBackup(m_filePath);
                return FileHelper::writeLines(m_filePath, lines);
            }
            catch (...)
            {
                return false;
            }
        }

        bool DepartmentRepository::load()
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            return loadInternal();
        }

        bool DepartmentRepository::loadInternal()
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

                m_departments.clear();

                for (const auto &line : lines)
                {
                    auto department = Model::Department::deserialize(line);
                    if (department)
                    {
                        m_departments.push_back(department.value());
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
        size_t DepartmentRepository::count() const
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();
            return m_departments.size();
        }

        bool DepartmentRepository::exists(const std::string &id) const
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            return std::ranges::any_of(
                m_departments, [&id](const auto &d)
                {
                    return d.getDepartmentID() == id;
                }
            );
        }

        bool DepartmentRepository::clear()
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            m_departments.clear();
            m_isLoaded = true;
            return saveInternal();
        }

        // ==================== Department-Specific Queries ====================
        std::optional<Model::Department> DepartmentRepository::getByName(const std::string &name)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            auto it = std::ranges::find_if(
                m_departments, [&name](const auto &d)
                {
                    const std::string &deptName = d.getName();
                    if (deptName.size() != name.size()) return false;
                    return std::equal(deptName.begin(), deptName.end(), name.begin(),
                        [](char a, char b) { return std::tolower(static_cast<unsigned char>(a)) ==
                                                    std::tolower(static_cast<unsigned char>(b)); });
                }
            );

            if (it != m_departments.end())
            {
                return *it;
            }
            return std::nullopt;
        }

        std::optional<Model::Department> DepartmentRepository::getByHeadDoctor(const std::string &doctorID)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            auto it = std::ranges::find_if(
                m_departments, [&doctorID](const auto &d)
                {
                    return d.getHeadDoctorID() == doctorID;
                }
            );

            if (it != m_departments.end())
            {
                return *it;
            }
            return std::nullopt;
        }

        std::optional<Model::Department> DepartmentRepository::getDepartmentByDoctor(const std::string &doctorID)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            for (const auto &d : m_departments)
            {
                const auto &doctors = d.getDoctorIDs();
                if (std::ranges::find(doctors, doctorID) != doctors.end())
                {
                    return d;
                }
            }
            return std::nullopt;
        }

        std::vector<Model::Department> DepartmentRepository::getDepartmentsByDoctor(const std::string &doctorID)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            std::vector<Model::Department> results;
            for (const auto &d : m_departments)
            {
                const auto &doctors = d.getDoctorIDs();
                if (std::ranges::find(doctors, doctorID) != doctors.end())
                {
                    results.push_back(d);
                }
            }
            return results;
        }

        std::vector<Model::Department> DepartmentRepository::searchByName(const std::string &name)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            std::vector<Model::Department> results;
            std::ranges::copy_if(
                m_departments, std::back_inserter(results),
                [&name](const auto &d)
                {
                    return Utils::containsIgnoreCase(d.getName(), name);
                }
            );

            return results;
        }

        std::vector<std::string> DepartmentRepository::getAllNames()
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            std::vector<std::string> names;
            names.reserve(m_departments.size());
            for (const auto &d : m_departments)
            {
                names.push_back(d.getName());
            }
            return names;
        }

        std::string DepartmentRepository::getNextId()
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            if (m_departments.empty())
            {
                return std::format("{}001", Constants::DEPARTMENT_ID_PREFIX);
            }

            // Find max ID number
            int maxID = 0;
            const std::string prefix = Constants::DEPARTMENT_ID_PREFIX;

            for (const auto &department : m_departments)
            {
                const std::string &departmentID = department.getDepartmentID();

                // Only process valid format: prefix + digits
                if (departmentID.length() > prefix.length() &&
                    departmentID.starts_with(prefix))
                {
                    std::string numPart = departmentID.substr(prefix.length());

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
        void DepartmentRepository::setFilePath(const std::string &filePath)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            m_filePath = filePath;
            m_isLoaded = false;
        }

        std::string DepartmentRepository::getFilePath() const
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            return m_filePath;
        }

    } // namespace DAL
} // namespace HMS
