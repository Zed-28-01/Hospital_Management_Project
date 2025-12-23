#include "ui/HMSFacade.h"
#include "bll/AuthService.h"
#include "bll/PatientService.h"
#include "bll/DoctorService.h"
#include "bll/AppointmentService.h"
#include "bll/AdminService.h"


#include "common/Utils.h"
#include "common/Types.h"


namespace HMS {
    namespace UI {

// ================================ SINGLETON INIT ==========================================

        std::unique_ptr<HMSFacade> HMSFacade::s_instance = nullptr;
        std::mutex HMSFacade::s_mutex;


// ============================= CONSTRUCTOR/DESTRUCTOR ============================================


        HMSFacade::HMSFacade() {

            m_authService = BLL::AuthService::getInstance();
            m_patientService = BLL::PatientService::getInstance();
            m_doctorService = BLL::DoctorService::getInstance();
            m_appointmentService = BLL::AppointmentService::getInstance();
            m_adminService = BLL::AdminService::getInstance();

            m_isInitialized = false;
        }

        HMSFacade::~HMSFacade() {
            shutdown();
        }



// ============================ SINGLETON ACCESS ============================================

        HMSFacade* HMSFacade::getInstance() {
            std::lock_guard<std::mutex> lock(s_mutex);
            if (!s_instance) {
                s_instance = std::unique_ptr<HMSFacade>(new HMSFacade());
            }
            return s_instance.get();
        }

        void HMSFacade::resetInstance() {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_instance.reset();
        }

// ============================== SYSTEM LIFECYCLE ===========================================

        bool HMSFacade::initialize() {
            if (m_isInitialized) return true;

            bool success = loadData();
            if (success) {
                m_isInitialized = true;
            }
            return success;
        }

        void HMSFacade::shutdown() {
            if (m_isInitialized) {
                saveData();

                if (m_authService->isLoggedIn()) {
                    m_authService->logout();
                }
                m_isInitialized = false;
            }
        }

        bool HMSFacade::isInitialized() const {
            return m_isInitialized;
        }

// ======================================= AUTHENTICATION =====================================

        bool HMSFacade::login(const std::string& username, const std::string& password) {
            return m_authService->login(username, password);
        }

        void HMSFacade::logout() {
            m_authService->logout();
        }

        bool HMSFacade::isLoggedIn() const {
            return m_authService->isLoggedIn();
        }

        std::string HMSFacade::getCurrentUsername() const {
            return m_authService->getCurrentUsername();
        }

        Role HMSFacade::getCurrentRole() const {
            return m_authService->getCurrentRole();
        }

// ====================================== REGISTRATION =========================================

        bool HMSFacade::registerPatient(const std::string& username,
                                        const std::string& password,
                                        const std::string& name,
                                        const std::string& phone,
                                        const std::string& genderStr,
                                        const std::string& dateOfBirth,
                                        const std::string& address) {

            if (!m_authService->isUsernameAvailable(username)) {
                return false;
            }

            bool accountCreated = m_authService->registerAccount(username, password, Role::PATIENT);

            if (!accountCreated){
                return false;
            }

            Gender gender = stringToGender(genderStr);
            auto result = m_patientService->createPatient(
                username,
                name,
                phone,
                gender,
                dateOfBirth,
                address,
                ""
            );


            if (!result.has_value()) {
                return false;
            }
            return true;
        }
        bool HMSFacade::isUsernameAvailable(const std::string& username) {
            return m_authService->isUsernameAvailable(username);
        }

// ======================================== PATIENT OPERATIONS =========================================

        std::optional<Model::Patient> HMSFacade::getMyProfile() {
            if (!isLoggedIn()) return std::nullopt;

            return m_patientService->getPatientByUsername(getCurrentUsername());
        }

        bool HMSFacade::updateMyProfile(const std::string& phone, const std::string& address) {
            if (!isLoggedIn()) return false;

            auto patientOpt = m_patientService->getPatientByUsername(getCurrentUsername());
            if (!patientOpt.has_value()) {
                return false;
            }

            Model::Patient patient = patientOpt.value();
            patient.setAddress(address);
            patient.setPhone(phone);

            return m_patientService->updatePatient(patient);
        }

        std::vector<Model::Appointment> HMSFacade::getMyAppointments() {
            if (!isLoggedIn()) return {};

            return m_patientService->getAppointmentHistory(getCurrentUsername());
        }

        std::vector<Model::Appointment> HMSFacade::getMyUpcomingAppointments() {
            if (!isLoggedIn()) return {};

            return m_patientService->getUpcomingAppointments(getCurrentUsername());

        }

        double HMSFacade::getMyTotalBill() {
            if (!isLoggedIn()) return 0.0;

            return m_patientService->calculateTotalBill(getCurrentUsername());

        }

        bool HMSFacade::bookAppointment(const std::string& doctorID,
                                        const std::string& date,
                                        const std::string& time,
                                        const std::string& disease) {
            if (!isLoggedIn()) return false;

            auto result = m_appointmentService->bookAppointment(
                getCurrentUsername(),
                doctorID,
                date,
                time,
                disease
            );

            return result.has_value();
        }

        bool HMSFacade::cancelAppointment(const std::string& appointmentID) {

            if (!isLoggedIn()) return false;
            return m_appointmentService->cancelAppointment(appointmentID);

        }

// ========================================= DOCTOR OPERATIONS ==========================================

        std::vector<Model::Doctor> HMSFacade::getAllDoctors() {
            return m_doctorService->getAllDoctors();
        }

        std::vector<Model::Doctor> HMSFacade::getDoctorsBySpecialization(const std::string& specialization) {
            return m_doctorService->searchDoctors(specialization);
        }

        std::vector<std::string> HMSFacade::getAllSpecializations() {
            return m_doctorService->getAllSpecializations();
        }

        std::optional<Model::Doctor> HMSFacade::getDoctorByID(const std::string& doctorID) {
            return m_doctorService->getDoctorByID(doctorID);
        }

        std::vector<std::string> HMSFacade::getAvailableSlots(const std::string& doctorID, const std::string& date) {
            return m_appointmentService->getAvailableSlots(doctorID, date);
        }

// ====================================== DOCTOR'S PATIENT MANAGEMENT ==============================================


        std::vector<Model::Patient> HMSFacade::getAllPatients() {
            return m_patientService->getAllPatients();
        }

        std::vector<Model::Patient> HMSFacade::searchPatients(const std::string& keyword) {
            return m_patientService->searchPatients(keyword);
        }

        std::optional<Model::Patient> HMSFacade::getPatientByID(const std::string& patientID) {
            return m_patientService->getPatientByID(patientID);
        }

        bool HMSFacade::addPatient(const std::string& name,
                                   const std::string& phone,
                                   const std::string& genderStr,
                                   const std::string& dateOfBirth,
                                   const std::string& address,
                                   const std::string& medicalHistory) {


            Gender gender = stringToGender(genderStr);

            // temporary username, use other way if needed
            std::string tempUsername = name + Utils::getCurrentDateTime();

            auto newPatient = m_patientService->createPatient(
                                                    tempUsername,
                                                    name,
                                                    phone,
                                                    gender,
                                                    dateOfBirth,
                                                    address,
                                                    medicalHistory
                                                );
            return newPatient.has_value();
        }

        bool HMSFacade::updatePatient(const std::string& patientID,
                                      const std::string& phone,
                                      const std::string& address,
                                      const std::string& medicalHistory) {

            auto patientOpt = m_patientService->getPatientByID(patientID);
            if (!patientOpt.has_value()) return false;

            Model::Patient patient = patientOpt.value();

            if (!phone.empty()) {
                patient.setPhone(phone);
            }
            if (!address.empty()) {
                patient.setAddress(address);
            }
            if (!medicalHistory.empty()) {
                patient.setMedicalHistory(medicalHistory);
            }

            return m_patientService->updatePatient(patient);
        }

        bool HMSFacade::deletePatient(const std::string& patientID) {
            return m_patientService->deletePatient(patientID);
        }

        std::vector<Model::Appointment> HMSFacade::getMySchedule(const std::string& date) {
            if (!isLoggedIn()) return {};
            auto doctor = m_doctorService->getDoctorByID(getCurrentUsername());
            if (!doctor.has_value()) return {};
            return m_doctorService->getDoctorSchedule(doctor->getDoctorID(), date);
        }

        bool HMSFacade::markAppointmentCompleted(const std::string& appointmentID) {
            return m_appointmentService->markAsCompleted(appointmentID);
        }

// ============================================ ADMIN OPERATIONS =====================================================

        bool HMSFacade::addDoctor(const std::string& username,
                                  const std::string& password,
                                  const std::string& name,
                                  const std::string& phone,
                                  const std::string& genderStr,
                                  const std::string& dateOfBirth,
                                  const std::string& specialization,
                                  const std::string& schedule,
                                  double consultationFee) {


            if (!m_authService->registerAccount(username,password, Role::DOCTOR)) {
                return false;
            }

            Gender gender = stringToGender(genderStr);

            auto newPatient = m_doctorService->createDoctor(
                                             username,
                                             name,
                                             phone,
                                             gender,
                                             dateOfBirth,
                                             specialization,
                                             schedule,
                                             consultationFee);
            return newPatient.has_value();

        }

        bool HMSFacade::updateDoctor(const std::string& doctorID,
                                     const std::string& specialization,
                                     const std::string& schedule,
                                     double consultationFee) {
            auto doctorOpt = m_doctorService->getDoctorByID(doctorID);
            if (!doctorOpt.has_value()) return false;

            Model::Doctor doc = doctorOpt.value();

            if (!specialization.empty()) {
                doc.setSpecialization(specialization);
            }

            if (!schedule.empty()) {
                doc.setSchedule(schedule);
            }
            if (consultationFee > 0 ) {
                doc.setConsultationFee(consultationFee);
            }
            return m_doctorService->updateDoctor(doc);
        }

        bool HMSFacade::deleteDoctor(const std::string& doctorID) {
            return m_doctorService->deleteDoctor(doctorID);
        }

        Model::Statistics HMSFacade::getStatistics(){
            return m_adminService->getStatistics();
        }

        std::string HMSFacade::generateReport() {
            return m_adminService->generateSummaryReport();
        }

        bool HMSFacade::saveData() {
            return m_adminService->saveAllData();
        }

        bool HMSFacade::loadData() {
            return m_adminService->loadAllData();
        }
    }
}