#include "bll/DepartmentService.h"
#include "dal/AppointmentRepository.h"
#include "common/Utils.h"

#include <algorithm>

namespace HMS
{
    namespace BLL
    {

        // ==================== Static Members ====================
        std::unique_ptr<DepartmentService> DepartmentService::s_instance = nullptr;
        std::mutex DepartmentService::s_mutex;

        // ==================== Private Constructor ====================
        DepartmentService::DepartmentService()
            : m_departmentRepo(DAL::DepartmentRepository::getInstance()), m_doctorRepo(DAL::DoctorRepository::getInstance())
        {
        }

        // ==================== Destructor ====================
        DepartmentService::~DepartmentService() = default;

        // ==================== Singleton Access ====================
        DepartmentService *DepartmentService::getInstance()
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            if (!s_instance)
            {
                s_instance = std::unique_ptr<DepartmentService>(new DepartmentService());
            }
            return s_instance.get();
        }

        void DepartmentService::resetInstance()
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_instance.reset();
        }

        // ==================== CRUD Operations ====================
        bool DepartmentService::createDepartment(const Model::Department &department)
        {
            if (!validateDepartment(department))
            {
                return false;
            }

            if (departmentExists(department.getDepartmentID()))
            {
                return false;
            }

            // Check for duplicate name
            if (departmentNameExists(department.getName()))
            {
                return false;
            }

            return m_departmentRepo->add(department);
        }

        Result<Model::Department> DepartmentService::createDepartment(
            const std::string &name,
            const std::string &description,
            const std::string &location,
            const std::string &phone)
        {
            // Validate input parameters
            if (name.empty())
            {
                return std::nullopt;
            }

            // Check for duplicate name
            if (departmentNameExists(name))
            {
                return std::nullopt;
            }

            // Validate phone if provided
            if (!phone.empty() && !Utils::isValidPhone(phone))
            {
                return std::nullopt;
            }

            // Generate new ID through repository
            std::string id = m_departmentRepo->getNextId();

            // Create department with constructor
            Model::Department dep(id, name, description, "");
            dep.setLocation(location);
            dep.setPhone(phone);

            return m_departmentRepo->add(dep)
                       ? Result<Model::Department>(dep)
                       : std::nullopt;
        }

        bool DepartmentService::updateDepartment(const Model::Department &department)
        {
            if (!departmentExists(department.getDepartmentID()))
            {
                return false;
            }

            if (!validateDepartment(department))
            {
                return false;
            }

            // Check for duplicate name (excluding current department)
            if (departmentNameExists(department.getName(), department.getDepartmentID()))
            {
                return false;
            }

            return m_departmentRepo->update(department);
        }

        bool DepartmentService::deleteDepartment(const std::string &departmentID)
        {
            if (departmentID.empty() || !departmentExists(departmentID))
            {
                return false;
            }

            return m_departmentRepo->remove(departmentID);
        }

        // ==================== Query Operations ====================
        Result<Model::Department> DepartmentService::getDepartmentByID(const std::string &departmentID)
        {
            return m_departmentRepo->getById(departmentID);
        }

        Result<Model::Department> DepartmentService::getDepartmentByName(const std::string &name)
        {
            return m_departmentRepo->getByName(name);
        }

        List<Model::Department> DepartmentService::getAllDepartments()
        {
            return m_departmentRepo->getAll();
        }

        List<Model::Department> DepartmentService::searchDepartments(const std::string &keyword)
        {
            if (keyword.empty())
            {
                return getAllDepartments();
            }
            return m_departmentRepo->searchByName(keyword);
        }

        size_t DepartmentService::getDepartmentCount() const
        {
            return m_departmentRepo->count();
        }

        List<std::string> DepartmentService::getAllDepartmentNames()
        {
            return m_departmentRepo->getAllNames();
        }

        // ==================== Doctor Assignments ====================
        bool DepartmentService::assignDoctor(const std::string &departmentID, const std::string &doctorID)
        {
            // Validate department exists
            auto depOpt = m_departmentRepo->getById(departmentID);
            if (!depOpt.has_value())
            {
                return false;
            }

            // Validate doctor exists
            if (!m_doctorRepo->exists(doctorID))
            {
                return false;
            }

            Model::Department dep = depOpt.value();

            // Idempotent: return true if already assigned to this department
            if (dep.hasDoctor(doctorID))
            {
                return true;
            }

            // Handle cross-department transfer: unassign from current department first
            auto currDep = m_departmentRepo->getDepartmentByDoctor(doctorID);
            if (currDep.has_value() && currDep->getDepartmentID() != departmentID)
            {
                unassignDoctor(currDep->getDepartmentID(), doctorID);
                // Re-fetch department after potential modification
                dep = m_departmentRepo->getById(departmentID).value();
            }

            // Add doctor to department
            dep.addDoctor(doctorID);
            return m_departmentRepo->update(dep);
        }

        bool DepartmentService::unassignDoctor(const std::string &departmentID, const std::string &doctorID)
        {
            // Validate department exists
            auto depOpt = m_departmentRepo->getById(departmentID);
            if (!depOpt.has_value())
            {
                return false;
            }

            Model::Department dep = depOpt.value();

            // Check if doctor is in department
            if (!dep.hasDoctor(doctorID))
            {
                return false;
            }

            // Remove doctor from department
            dep.removeDoctor(doctorID);

            // Clear head if unassigned doctor was the head
            if (dep.getHeadDoctorID() == doctorID)
            {
                dep.setHeadDoctorID("");
            }

            return m_departmentRepo->update(dep);
        }

        bool DepartmentService::setDepartmentHead(const std::string &departmentID, const std::string &doctorID)
        {
            // Validate department exists
            auto depOpt = m_departmentRepo->getById(departmentID);
            if (!depOpt.has_value())
            {
                return false;
            }

            Model::Department dep = depOpt.value();

            // Empty doctorID clears the head
            if (doctorID.empty())
            {
                dep.setHeadDoctorID("");
                return m_departmentRepo->update(dep);
            }

            // Validate doctor exists
            if (!m_doctorRepo->exists(doctorID))
            {
                return false;
            }

            // Auto-assign doctor to department if not already assigned
            if (!dep.hasDoctor(doctorID))
            {
                if (!assignDoctor(departmentID, doctorID))
                {
                    return false;
                }
                // Re-fetch department after assignment
                dep = m_departmentRepo->getById(departmentID).value();
            }

            dep.setHeadDoctorID(doctorID);
            return m_departmentRepo->update(dep);
        }

        Result<Model::Doctor> DepartmentService::getDepartmentHead(const std::string &departmentID)
        {
            auto depOpt = m_departmentRepo->getById(departmentID);
            if (!depOpt.has_value())
            {
                return std::nullopt;
            }

            const std::string &headID = depOpt->getHeadDoctorID();
            if (headID.empty())
            {
                return std::nullopt;
            }

            return m_doctorRepo->getById(headID);
        }

        bool DepartmentService::isDoctorInDepartment(const std::string &departmentID, const std::string &doctorID)
        {
            auto depOpt = m_departmentRepo->getById(departmentID);
            if (!depOpt.has_value())
            {
                return false;
            }
            return depOpt->hasDoctor(doctorID);
        }

        // ==================== Doctor Queries ====================
        List<Model::Doctor> DepartmentService::getDoctorsInDepartment(const std::string &departmentID)
        {
            auto depOpt = m_departmentRepo->getById(departmentID);
            if (!depOpt.has_value())
            {
                return {};
            }

            List<Model::Doctor> doctors;
            const auto &doctorIDs = depOpt->getDoctorIDs();

            for (const auto &docID : doctorIDs)
            {
                auto docOpt = m_doctorRepo->getById(docID);
                if (docOpt.has_value())
                {
                    doctors.push_back(docOpt.value());
                }
            }

            return doctors;
        }

        Result<Model::Department> DepartmentService::getDoctorDepartment(const std::string &doctorID)
        {
            return m_departmentRepo->getDepartmentByDoctor(doctorID);
        }

        List<Model::Department> DepartmentService::getDoctorDepartments(const std::string &doctorID)
        {
            return m_departmentRepo->getDepartmentsByDoctor(doctorID);
        }

        List<Model::Doctor> DepartmentService::getUnassignedDoctors()
        {
            auto allDoctors = m_doctorRepo->getAll();
            List<Model::Doctor> unassigned;

            std::ranges::copy_if(allDoctors, std::back_inserter(unassigned),
                                 [this](const auto &doc)
                                 {
                                     return !m_departmentRepo->getDepartmentByDoctor(doc.getID()).has_value();
                                 });

            return unassigned;
        }

        // ==================== Statistics ====================
        DepartmentStats DepartmentService::getDepartmentStats(const std::string &departmentID)
        {
            DepartmentStats stats{};
            stats.departmentID = departmentID;
            stats.departmentName = "";
            stats.doctorCount = 0;
            stats.appointmentCount = 0;
            stats.doctorNames = {};
            stats.totalRevenue = 0.0;

            auto depOpt = m_departmentRepo->getById(departmentID);
            if (!depOpt.has_value())
            {
                return stats;
            }

            Model::Department dep = depOpt.value();

            stats.departmentID = dep.getDepartmentID();
            stats.departmentName = dep.getName();
            stats.doctorCount = static_cast<int>(dep.getDoctorCount());

            // Get doctor names
            auto doctors = getDoctorsInDepartment(departmentID);
            for (const auto &doc : doctors)
            {
                stats.doctorNames.push_back(doc.getName());
            }

            // Calculate appointment count and revenue from completed appointments
            auto appRepo = DAL::AppointmentRepository::getInstance();
            auto allApps = appRepo->getAll();

            for (const auto &app : allApps)
            {
                if (dep.hasDoctor(app.getDoctorID()))
                {
                    if (app.getStatus() == AppointmentStatus::COMPLETED)
                    {
                        stats.appointmentCount++;
                        stats.totalRevenue += app.getPrice();
                    }
                }
            }

            return stats;
        }

        std::map<std::string, DepartmentStats> DepartmentService::getAllDepartmentStats()
        {
            std::map<std::string, DepartmentStats> statsMap;
            auto deps = m_departmentRepo->getAll();

            for (const auto &dep : deps)
            {
                statsMap[dep.getDepartmentID()] = getDepartmentStats(dep.getDepartmentID());
            }

            return statsMap;
        }

        std::map<std::string, int> DepartmentService::getDoctorCountByDepartment()
        {
            std::map<std::string, int> counts;
            auto deps = m_departmentRepo->getAll();

            for (const auto &dep : deps)
            {
                counts[dep.getDepartmentID()] = static_cast<int>(dep.getDoctorCount());
            }

            return counts;
        }

        // ==================== Validation ====================
        bool DepartmentService::validateDepartment(const Model::Department &department)
        {
            // Check required fields
            if (department.getDepartmentID().empty() || department.getName().empty())
            {
                return false;
            }

            // Validate phone format if provided
            const std::string &phone = department.getPhone();
            if (!phone.empty() && !Utils::isValidPhone(phone))
            {
                return false;
            }

            // If head doctor is set, validate it exists
            const std::string &headID = department.getHeadDoctorID();
            if (!headID.empty() && !m_doctorRepo->exists(headID))
            {
                return false;
            }

            return true;
        }

        bool DepartmentService::departmentExists(const std::string &departmentID)
        {
            return m_departmentRepo->exists(departmentID);
        }

        bool DepartmentService::departmentNameExists(const std::string &name, const std::string &excludeID)
        {
            auto allDepartments = m_departmentRepo->getAll();
            std::string normalizedName = Utils::trim(Utils::toLower(name));

            return std::ranges::any_of(allDepartments,
                                       [&normalizedName, &excludeID](const auto &dep)
                                       {
                                           return Utils::trim(Utils::toLower(dep.getName())) == normalizedName &&
                                                  dep.getDepartmentID() != excludeID;
                                       });
        }

        // ==================== Data Persistence ====================
        bool DepartmentService::saveData()
        {
            return m_departmentRepo->save();
        }

        bool DepartmentService::loadData()
        {
            return m_departmentRepo->load();
        }

    } // namespace BLL
} // namespace HMS
