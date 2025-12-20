#include "bll/DoctorService.h"
#include "common/Utils.h"
#include "common/Types.h"
#include <set>


namespace HMS {
    namespace BLL {

// ========================== CONSTRUCTOR ================================

        DoctorService::DoctorService() {
            m_doctorRepo = DAL::DoctorRepository::getInstance();
            m_appointmentRepo = DAL::AppointmentRepository::getInstance();

        }
        DoctorService::~DoctorService() = default;

// =========================== SINGLETON ACCESS ========================

        DoctorService* DoctorService::getInstance() {
            std::lock_guard<std::mutex> lock(s_mutex);
            if (!s_instance) {
                s_instance = std::unique_ptr<DoctorService>(new DoctorService());
            }
            return s_instance.get();
        }
        void DoctorService::resetInstance() {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_instance.reset();
        }

// ============================= CRUD Operations ===============================

        bool DoctorService::createDoctor(const Model::Doctor& doctor) {

            if (!validateDoctor(doctor)) {
                return false;
            }
            if (doctorExists(doctor.getID())) {
                return false;
            }
            return m_doctorRepo->add(doctor);
        }

        std::optional<Model::Doctor> DoctorService::createDoctor(const std::string& username,
                                                                const std::string& name,
                                                                const std::string& phone,
                                                                Gender gender,
                                                                const std::string& dateOfBirth,
                                                                const std::string& specialization,
                                                                const std::string& schedule,
                                                                double consultationFee) {
            std::string id = Utils::generateDoctorID();
            Model::Doctor newDoc(id,
                                username,
                                name,
                                phone,
                                gender,
                                dateOfBirth,
                                specialization,
                                schedule,
                                consultationFee
                            );

            if (createDoctor(newDoc)) {
                return newDoc;
            }

            return std::nullopt;
        }

        bool DoctorService::updateDoctor(const Model::Doctor& doctor) {

            if (!doctorExists(doctor.getID())) {
                return false;
            }

            if (!validateDoctor(doctor)) {
                return false;
            }

            return m_doctorRepo->update(doctor);
        }

        bool DoctorService::deleteDoctor(const std::string& doctorID) {

            if (!doctorExists(doctorID)) {
                return false;
            }

            return m_doctorRepo->remove(doctorID);
        }

// ============================== QUERY OPERATIONS ===========================

        std::optional<Model::Doctor> DoctorService::getDoctorByID(const std::string& doctorID) {

            auto doctors = m_doctorRepo->getAll();
            for (const auto& doc: doctors) {
                if (doc.getID() == doctorID) {
                    return doc;
                }
            }
            return std::nullopt;

        }

        std::optional<Model::Doctor> DoctorService::getDoctorByUsername(const std::string& username) {

            auto doctors = m_doctorRepo->getAll();
            for (const auto& doc : doctors) {
                if (doc.getUsername() == username) {
                    return doc;
                }
            }

            return std::nullopt;
        }

        std::vector<Model::Doctor> DoctorService::getAllDoctors() {
            return m_doctorRepo->getAll();
        }

        std::vector<Model::Doctor> DoctorService::searchDoctors(const std::string& keyword) {

            auto doctors = m_doctorRepo->getAll();
            std::vector<Model::Doctor> result;
            for (auto const& doc : doctors) {
                if (Utils::containsIgnoreCase(doc.getID(), keyword) ||
                    Utils::containsIgnoreCase(doc.getName(), keyword) ||
                    Utils::containsIgnoreCase(doc.getSpecialization(), keyword)) {
                    result.push_back(doc);
                }
            }
            return result;
        }

        std::vector<Model::Doctor> DoctorService::getDoctorsBySpecialization(const std::string& specialization) {
            std::vector<Model::Doctor> result;
            auto doctors = m_doctorRepo->getAll();
            std::string target = Utils::toLower(specialization);
            for (auto const& doc : doctors) {
                if (Utils::toLower(doc.getSpecialization()) == target) {
                    result.push_back(doc);
                }
            }
            return result;
        }

        std::vector<std::string> DoctorService::getAllSpecializations() {

            std::set<std::string> specSet;
            auto doctors = m_doctorRepo->getAll();
            for (const auto& doc : doctors) {
                if (!doc.getSpecialization().empty()) {
                    specSet.insert(doc.getSpecialization());
                }
            }
            return std::vector<std::string>(specSet.begin(), specSet.end());
        }

        size_t DoctorService::getDoctorCount() const {
            return m_doctorRepo->getAll().size();
        }

// ========================== SCHEDULE MANAGEMENT =============================

        std::vector<Model::Appointment> DoctorService::getDoctorSchedule(const std::string& doctorID, const std::string& date) {
            auto appointments = m_appointmentRepo->getAll();
            std::vector<Model::Appointment> result;
            for (auto const& app : appointments) {
                if (app.getDoctorID() == doctorID && app.getDate() == date &&
                    app.getStatus() != AppointmentStatus::CANCELLED) {
                    result.push_back(app);
                }
            }

            std::sort(result.begin(), result.end(), [](const Model::Appointment& a, const Model::Appointment& b) {
                    return a.getTime() < b.getTime();
            });

            return result;
        }

        std::vector<Model::Appointment> DoctorService::getUpcomingAppointments(const std::string& doctorID) {
            auto allApp = m_appointmentRepo->getAll();
            std::vector<Model::Appointment> result;
            std::string today = Utils::getCurrentDate();
            std::string nowTime = Utils::getCurrentTime();

            for (const auto& app : allApp) {
                if (app.getDoctorID() == doctorID &&
                    app.getStatus() != AppointmentStatus::CANCELLED &&
                    app.getStatus() != AppointmentStatus::COMPLETED &&
                    (app. getDate() > today || (app.getDate() == today && app.getTime() > nowTime))) {
                        result.push_back(app);
                    }
            }

            std::sort(result.begin(), result.end(), [](const Model::Appointment& a, const Model::Appointment& b) {
                    return a.getTime() < b.getTime();
            });

            return result;
        }

        std::vector<std::string> DoctorService::getAvailableSlots(const std::string& doctorID, const std::string& date) {
            std::vector<std::string> availableSlots;

            std::vector<std::string> standardSlots {
                "08:00", "09:00",
                "10:00", "11:00",
                "13:00", "14:00",
                "15:00", "16:00"
            };

            auto exitingSchedule = m_appointmentRepo->getBookedSlots(doctorID, date);
            std::set<std::string> occupiedTime;
            for (const auto& schedule : exitingSchedule) {
                occupiedTime.insert(schedule);
            }

            for (auto const& slot : standardSlots) {
                if (occupiedTime.find(slot) == occupiedTime.end()) {
                    if (date == Utils::getCurrentDate()) {
                        if (slot > Utils::getCurrentTime()) {
                            availableSlots.push_back(slot);
                        }
                    }
                    else {
                        availableSlots.push_back(slot);
                    }
                }
            }
            return availableSlots;
        }

        bool DoctorService::isSlotAvailable(const std::string& doctorID, const std::string& time, const std::string& date) {

            auto slot = getAvailableSlots(doctorID, date);
            return std::find(slot.begin(), slot.end(), time) != slot.end();
        }

// ============================= ACTIVITY TRACKING ===============================

        std::vector<Model::Appointment> DoctorService::getDoctorActivity(const std::string& doctorID) {

            auto allAppointments = m_appointmentRepo->getAll();
            std::vector<Model::Appointment> result;

            for (auto const& app : allAppointments) {
                if (app.getDoctorID() == doctorID) {
                    result.push_back(app);
                }
            }

            return result;
        }

        std::vector<Model::Appointment> DoctorService::getCompletedAppointments(const std::string& doctorID) {

            auto allAppointments = m_appointmentRepo->getAll();
            std::vector<Model::Appointment> result;
            for (auto const& app : allAppointments) {
                if (app.getDoctorID() == doctorID && app.getStatus() == AppointmentStatus::COMPLETED) {
                    result.push_back(app);
                }
            }
            return result;
        }

        std::vector<Model::Appointment> DoctorService::getAppointmentsInRange(const std::string& doctorID,
                                                                              const std::string& startDate,
                                                                              const std::string& endDate) {
            auto activities = getDoctorActivity(doctorID);
            std::vector<Model::Appointment> result;
            for (auto const& a : activities) {
                if (a.getDate() >= startDate && a.getDate() <= endDate) {
                    result.push_back(a);
                }
            }
            return result;
        }

        std::vector<Model::Appointment> DoctorService::getTodayAppointments(const std::string& doctorID) {
            return getDoctorSchedule(doctorID, Utils::getCurrentDate());
        }

// ============================ STATISTICS ================================

        double DoctorService::getDoctorRevenue(const std::string& doctorID) {
            auto completed = getCompletedAppointments(doctorID);
            double total = 0.0;
            for (const auto& app : completed) {
                total += app.getPrice();
            }
            return total;
        }

        size_t DoctorService::getDoctorAppointmentCount(const std::string& doctorID)  {
            return getDoctorActivity(doctorID).size();
        }

        size_t DoctorService::getDoctorPatientCount(const std::string& doctorID) {
            auto activity = getDoctorActivity(doctorID);
            std::set<std::string> uniquePatients;

            for (const auto& act : activity) {
                if (act.getStatus() == AppointmentStatus::COMPLETED) {
                    uniquePatients.insert(act.getPatientUsername()); // Chỉ thêm ID bệnh nhân vào
                }
            }
            return uniquePatients.size();
        }

// ================================ VALIDATION & PERSISTENCE ==============================

        bool DoctorService::validateDoctor(const Model::Doctor& doctor) {
            if (doctor.getID().empty() || doctor.getName().empty() ||
                doctor.getPhone().empty() || doctor.getSpecialization().empty() ||
                doctor.getDoctorID().empty() || doctor.getDateOfBirth().empty() ||
                doctor.getGenderString().empty() || doctor.getUsername().empty()) {
                return false;
            }
            if (doctor.getConsultationFee() < 0) return false;
            return true;
        }

        bool DoctorService::doctorExists(const std::string& doctorID){
            auto doc = getDoctorByID(doctorID);
            return doc.has_value();
        }

        bool DoctorService::saveData() {
            return m_doctorRepo->save();
        }

        bool DoctorService::loadData() {
            return m_doctorRepo->load();
        }

    }
}