#include "bll/DepartmentService.h"
#include "dal/AppointmentRepository.h"
#include "common/Utils.h"

namespace HMS {
    namespace BLL {

// ============================== SINGLETON INIT ==============================
        std::unique_ptr<DepartmentService> DepartmentService::s_instance = nullptr;
        std::mutex DepartmentService::s_mutex;

// ============================= CONSTRUCTOR / DESTRUCTOR ==============================

        DepartmentService::DepartmentService() {
            m_departmentRepo = DAL::DepartmentRepository::getInstance();
            m_doctorRepo = DAL::DoctorRepository::getInstance();
        }

        DepartmentService::~DepartmentService() = default;

// ============================== SINGLETON ACCESS =====================================

        DepartmentService* DepartmentService::getInstance() {
            std::lock_guard<std::mutex> lock(s_mutex);
            if (!s_instance) {
                s_instance = std::unique_ptr<DepartmentService> (new DepartmentService());
            }
            return s_instance.get();
        }

        void DepartmentService::resetInstance() {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_instance.reset();
        }

// =============================== CRUD OPERATIONS =======================================

        bool DepartmentService::createDepartment(const Model::Department& department) {

            if (!validateDepartment(department)) {
                return false;
            }

            if (m_departmentRepo->exists(department.getDepartmentID())) {
                return false;
            }

            if (departmentNameExists(department.getName())) {
                return false;
            }

            return m_departmentRepo->add(department);
        }

        Result<Model::Department> DepartmentService::createDepartment(const std::string&name,
                                                                      const std::string& description,
                                                                      const std::string& location,
                                                                      const std::string& phone) {
            if (name.empty()) {
                return std::nullopt;
            }

            if (departmentNameExists(name)){
                return std::nullopt;
            }

            std::string id = m_departmentRepo->getNextId();

            Model::Department dep(id, name, description, "");
            dep.setLocation(location);
            dep.setPhone(phone);

            if (createDepartment(dep)) {
                return dep;
            }

            return std::nullopt;
        }

        bool DepartmentService::updateDepartment(const Model::Department& department) {
            if (!m_departmentRepo->exists(department.getDepartmentID())) {
                return false;
            }
            if (!validateDepartment(department)) {
                return false;
            }
            if (departmentNameExists(department.getName(), department.getDepartmentID())) {
                return false;
            }
            return m_departmentRepo->update(department);
        }

        bool DepartmentService::deleteDepartment(const std::string& departmentID) {
            return m_departmentRepo->remove(departmentID);
        }

// =================================== QUERY OPERATIONS ====================================

        Result<Model::Department> DepartmentService::getDepartmentByID(const std::string& departmentID) {
            return m_departmentRepo->getById(departmentID);
        }

        Result<Model::Department> DepartmentService::getDepartmentByName(const std::string& name) {
            return m_departmentRepo->getByName(name);
        }

        List<Model::Department> DepartmentService::getAllDepartments() {
            return m_departmentRepo->getAll();
        }

        List<Model::Department> DepartmentService::searchDepartments(const std::string& keyword) {
            return m_departmentRepo->searchByName(keyword);
        }

        size_t DepartmentService::getDepartmentCount() const {
            return m_departmentRepo->count();
        }

        List<std::string> DepartmentService::getAllDepartmentNames() {
            return m_departmentRepo->getAllNames();
        }

// ======================================= DOCTOR ASSIGNMENTS =========================================

        bool DepartmentService::assignDoctor(const std::string& departmentID, const std::string& doctorID) {

            auto depOpt = m_departmentRepo->getById(departmentID);
            if (!depOpt.has_value()) {
                return false;
            }

            if (!m_doctorRepo->exists(doctorID)) {
                return false;
            }

            Model::Department dep = depOpt.value();

            if (dep.hasDoctor(doctorID)) {
                return true;
            }

            auto currDep = m_departmentRepo->getDepartmentByDoctor(doctorID);
            if (currDep.has_value() && currDep->getDepartmentID() != departmentID) {
                unassignDoctor(currDep->getDepartmentID(), doctorID);
                dep = m_departmentRepo->getById(departmentID).value();
            }
            dep.addDoctor(doctorID);
            return m_departmentRepo->update(dep);
        }

        bool DepartmentService::unassignDoctor(const std::string& departmentID, const std::string& doctorID) {
            auto depOpt = m_departmentRepo->getById(departmentID);
            if (!depOpt.has_value()) {
                return false;
            }

            Model::Department dep = depOpt.value();

            if (!dep.hasDoctor(doctorID)) {
                return false;
            }

            dep.removeDoctor(doctorID);
            if (dep.getHeadDoctorID() == doctorID) {
                dep.setHeadDoctorID("");
            }

            return m_departmentRepo->update(dep);
        }

        bool DepartmentService::setDepartmentHead(const std::string& departmentID, const std::string& doctorID) {
            auto depOpt = m_departmentRepo->getById(departmentID);

            if (!depOpt.has_value()) {
                return false;
            }

            Model::Department dep = depOpt.value();


            if (!dep.hasDoctor(doctorID)) {
                if (!assignDoctor(departmentID, doctorID)) {
                    return false;
                }
                dep = m_departmentRepo->getById(departmentID).value();
            }

            dep.setHeadDoctorID(doctorID);
            return m_departmentRepo->update(dep);

        }


        Result<Model::Doctor> DepartmentService::getDepartmentHead(const std::string& departmentID) {
            auto depOpt = m_departmentRepo->getById(departmentID);
            if (!depOpt.has_value()) {
                return std::nullopt;
            }

            std::string headID = depOpt->getHeadDoctorID();
            if (headID.empty()) {
                return std::nullopt;
            }

            return m_doctorRepo->getById(headID);
        }

        bool DepartmentService::isDoctorInDepartment(const std::string& departmentID, const std::string& doctorID) {
            auto depOpt = m_departmentRepo->getById(departmentID);
            if (!depOpt.has_value()) {
                return false;
            }
            return depOpt->hasDoctor(doctorID);
        }

// ================================= DOCTOR QUERIES =======================================

        List<Model::Doctor> DepartmentService::getDoctorsInDepartment(const std::string& departmentID) {
            auto depOpt = m_departmentRepo->getById(departmentID);
            if (!depOpt.has_value()) {
                return {};
            }
            List<Model::Doctor> doctors;

            for (const auto& doc : depOpt->getDoctorIDs()) {
                auto docOpt = m_doctorRepo->getById(doc);
                if (docOpt.has_value()) {
                    doctors.push_back(docOpt.value());
                }

            }
            return doctors;
        }

        Result<Model::Department> DepartmentService::getDoctorDepartment(const std::string& doctorID) {
            return m_departmentRepo->getDepartmentByDoctor(doctorID);
        }

        List<Model::Department> DepartmentService::getDoctorDepartments(const std::string& doctorID) {
            return m_departmentRepo->getDepartmentsByDoctor(doctorID);
        }

        List<Model::Doctor> DepartmentService::getUnassignedDoctors() {
            auto doctors = m_doctorRepo->getAll();

            List<Model::Doctor> unassigned;

            for (const auto& doc : doctors) {
                auto depOpt = m_departmentRepo->getDepartmentByDoctor(doc.getID());
                if (!depOpt.has_value()) {
                    unassigned.push_back(doc);
                }
            }
            return unassigned;
        }

// ======================================== STATISTICS ================================================

        DepartmentStats DepartmentService::getDepartmentStats(const std::string& departmentID) {
            DepartmentStats stats;

            stats.departmentID = departmentID;
            stats.departmentName = "";
            stats.doctorCount = 0;
            stats.appointmentCount = 0;
            stats.doctorNames = {};
            stats.totalRevenue = 0;

            auto depOpt = m_departmentRepo->getById(departmentID);
            if (!depOpt.has_value())  {
                return stats;
            }

            Model::Department dep = depOpt.value();

            stats.departmentID = dep.getDepartmentID();
            stats.departmentName = dep.getName();
            stats.doctorCount = static_cast<int>(dep.getDoctorCount());

            auto doctors = getDoctorsInDepartment(departmentID);
            for (const auto& doc : doctors) {
                stats.doctorNames.push_back(doc.getName());
            }

            auto appRepo = DAL::AppointmentRepository::getInstance();
            auto allApps = appRepo->getAll();

            for (const auto& app : allApps) {
                if (dep.hasDoctor(app.getDoctorID())) {

                    if (app.getStatus() == AppointmentStatus::COMPLETED) {
                        stats.appointmentCount++;
                        stats.totalRevenue+= app.getPrice();
                    }
                }
            }
            return stats;
        }

        std::map<std::string, DepartmentStats> DepartmentService::getAllDepartmentStats() {
            std::map<std::string, DepartmentStats> statsMap;
            auto deps = m_departmentRepo->getAll();

            for (const auto& dep : deps) {
                statsMap[dep.getDepartmentID()] = getDepartmentStats(dep.getDepartmentID());
            }
            return statsMap;
        }

        std::map<std::string, int> DepartmentService::getDoctorCountByDepartment() {
            std::map<std::string, int> counts;

            auto deps = m_departmentRepo->getAll();
            for (const auto& dep : deps) {
                counts[dep.getDepartmentID()] = static_cast<int>(dep.getDoctorCount());
            }
            return counts;
        }

// ======================================== VALIDATION ==================================================

        bool DepartmentService::validateDepartment(const Model::Department& department) {
            if (department.getDepartmentID().empty() ||
                department.getName().empty() ||
                department.getDescription().empty() ||
                department.getLocation().empty()) {
                return false;
            }
            if (!Utils::isValidPhone(department.getPhone())) {
                return false;
            }
            return true;
        }

        bool DepartmentService::departmentExists(const std::string& departmentID) {
            return m_departmentRepo->exists(departmentID);
        }

        bool DepartmentService::departmentNameExists(const std::string& name, const std::string& excludeID) {
            auto depOpt = m_departmentRepo->getByName(name);
            if (!depOpt.has_value()) {
                return false;
            }

            if (!excludeID.empty() && depOpt->getDepartmentID() == excludeID) {
                return false;
            }
            return true;
        }

// ====================================== DATA PERSISTENCE ============================================

        bool DepartmentService::saveData() {
            return m_departmentRepo->save();
        }

        bool DepartmentService::loadData() {
            return m_departmentRepo->load();
        }

    }
}