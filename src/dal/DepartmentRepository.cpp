#include "dal/DepartmentRepository.h"
#include "common/Constants.h"
#include "common/Utils.h"
#include "dal/FileHelper.h"


#include <sstream>
#include <format>
#include <algorithm>


namespace HMS {
    namespace DAL {
// ============================= SINGLETON INIT =============================

        std::unique_ptr<DepartmentRepository> DepartmentRepository::s_instance = nullptr;
        std::mutex DepartmentRepository::s_mutex;
// ============================ CONSTRUCTOR/DESTRUCTOR ================================

        DepartmentRepository::DepartmentRepository()
            : m_filePath(Constants::DEPARTMENT_FILE),
            m_isLoaded(false) {}

        DepartmentRepository::~DepartmentRepository() = default;

// ============================ SINGLETON ACCESS ======================================

        DepartmentRepository* DepartmentRepository::getInstance() {
            std::lock_guard<std::mutex> lock(s_mutex);
            if (!s_instance) {
                s_instance = std::unique_ptr<DepartmentRepository>(new DepartmentRepository());
            }
            return s_instance.get();
        }

        void DepartmentRepository::resetInstance() {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_instance.reset();
        }

// ============================ CRUD OPERATIONS ===========================================

        std::vector<Model::Department> DepartmentRepository::getAll() {
            if (!m_isLoaded) {
                load();
            }

            return m_departments;
        }

        std::optional<Model::Department> DepartmentRepository::getById(const std::string& id) {
            if (!m_isLoaded) {
                load();
            }

            auto it = std::find_if(m_departments.begin(), m_departments.end(),
                                   [&id](const Model::Department& dep)
                                   {
                                    return dep.getDepartmentID() == id;
                                   });

            if (it != m_departments.end()) {
                return *it;
            }
            return std::nullopt;
        }

        bool DepartmentRepository::add(const Model::Department& department) {
            if (!m_isLoaded) {
                load();
            }

            if (exists(department.getDepartmentID())) {
                return false;
            }

            m_departments.push_back(department);
            return save();
        }

        bool DepartmentRepository::update(const Model::Department& department) {
            if (!m_isLoaded) {
                load();
            }

            auto it = std::find_if(m_departments.begin(), m_departments.end(),
                                   [&department](const Model::Department& dep)
                                   {
                                    return dep.getDepartmentID() == department.getDepartmentID();
                                   });

            if (it != m_departments.end()) {
                *it = department;
                return save();
            }

            return false;
        }

        bool DepartmentRepository::remove(const std::string& id) {
            if (!m_isLoaded) {
                load();
            }

            auto it = std::find_if(m_departments.begin(), m_departments.end(),
            [&id](const Model::Department& dep)
            {return dep.getDepartmentID() == id; });

            if (it != m_departments.end()) {
                m_departments.erase(it, m_departments.end());
                return save();
            }
            return false;
        }

// =================================== PERSISTENCE ======================================

        bool DepartmentRepository::save() {
            try {
                std::vector<std::string> lines;

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

                for (const auto &dep : m_departments)
                {
                    lines.push_back(dep.serialize());
                }

                FileHelper::createBackup(m_filePath);

                return FileHelper::writeLines(m_filePath, lines);
            }
            catch (...)
            {
                return false;
            }
        }

        bool DepartmentRepository::load() {
            try {
                FileHelper::createDirectoryIfNotExists(Constants::DATA_DIR);
                FileHelper::createFileIfNotExists(m_filePath);

                std::vector<std::string> lines = FileHelper::readLines(m_filePath);

                m_departments.clear();

                for (const auto &line : lines)
                {
                    auto patient = Model::Department::deserialize(line);
                    if (patient)
                    {
                        m_departments.push_back(patient.value());
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

// =========================================== QUERY OPERATIONS ===============================================

        size_t DepartmentRepository::count() const {
            return m_departments.size();
        }


        bool DepartmentRepository::exists(const std::string& id) const {
            for (const auto& dep : m_departments) {
                if (dep.getDepartmentID() == id) {
                    return true;
                }
            }
            return false;
        }

        bool DepartmentRepository::clear() {
            m_departments.clear();
            return save();
        }

// ========================================= DEPARTMENT-SPECIFIC QUERIES ==============================================

        std::optional<Model::Department> DepartmentRepository::getByName(const std::string& name) {
            if (!m_isLoaded) {
                load();
            }


            for (const auto &dep : m_departments) {
                if (Utils::toLower(dep.getName()) == Utils::toLower(name)) {
                    return dep;
                }
            }
            return std::nullopt;
        }

        std::optional<Model::Department> DepartmentRepository::getDepartmentByDoctor(const std::string& doctorID) {
            if (!m_isLoaded) {
                load();
            }

            for (const auto& dep : m_departments) {
                if (dep.hasDoctor(doctorID)) {
                    return dep;
                }
            }

            return std::nullopt;
        }

        std::vector<Model::Department> DepartmentRepository::getDepartmentsByDoctor(const std::string& doctorID) {
            if (!m_isLoaded) {
                load();
            }

            std::vector<Model::Department> result;
            for (const auto& dep : m_departments) {
                if (dep.hasDoctor(doctorID)) {
                    result.push_back(dep);
                }
            }
            return result;
        }

        std::vector<Model::Department> DepartmentRepository::searchByName(const std::string& keyword) {
            if (!m_isLoaded) {
                load();
            }

            std::vector<Model::Department> result;
            for (auto const dep : m_departments) {
                if (Utils::toLower(dep.getName()).find(Utils::toLower(keyword)) != std::string::npos) {
                    result.push_back(dep);
                }
            }

            return result;
        }

        std::vector<std::string> DepartmentRepository::getAllNames() {
            if (!m_isLoaded) {
                load();
            }

            std::vector<std::string> names;
            for (const auto& dep : m_departments) {
                names.push_back(dep.getName());
            }
            return names;
        }

        std::string DepartmentRepository::getNextId() {
            if (!m_isLoaded) {
                load();
            }

            int maxID = 0;
            const std::string prefix = Constants::DEPARTMENT_ID_PREFIX;

            for (const auto &dep : m_departments)
            {
                const std::string &departmentID = dep.getDepartmentID();

                if (departmentID.length() > prefix.length() &&
                    departmentID.substr(0, prefix.length()) == prefix)
                {
                    std::string numPart = departmentID.substr(prefix.length());

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

// ================================= FILE PATH ===========================================

        void DepartmentRepository::setFilePath(const std::string& filePath) {
            m_filePath = filePath;
            m_isLoaded = false;
        }

        std::string DepartmentRepository::getFilePath() const {
            return m_filePath;
        }
    }
}