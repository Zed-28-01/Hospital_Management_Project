#include "ui/HMSFacade.h"
#include "common/Utils.h"

namespace HMS {
namespace UI {

// ==================== Helper Functions ====================
namespace {
    Gender stringToGender(const std::string& genderStr) {
        std::string lower = Utils::toLower(genderStr);
        // Vietnamese
        if (lower == "nam") {
            return Gender::MALE;
        } else if (lower == "nu" || lower == "nữ") {
            return Gender::FEMALE;
        } else if (lower == "khac" || lower == "khác") {
            return Gender::OTHER;
        }
        // English (backward compatibility)
        else if (lower == "male" || lower == "m") {
            return Gender::MALE;
        } else if (lower == "female" || lower == "f") {
            return Gender::FEMALE;
        } else if (lower == "other" || lower == "o") {
            return Gender::OTHER;
        }
        return Gender::UNKNOWN;
    }
}

// ==================== Static Members ====================
std::unique_ptr<HMSFacade> HMSFacade::s_instance = nullptr;
std::mutex HMSFacade::s_mutex;

// ==================== Private Constructor ====================
HMSFacade::HMSFacade()
    : m_authService(BLL::AuthService::getInstance()),
      m_patientService(BLL::PatientService::getInstance()),
      m_doctorService(BLL::DoctorService::getInstance()),
      m_appointmentService(BLL::AppointmentService::getInstance()),
      m_adminService(BLL::AdminService::getInstance()),
      m_medicineService(BLL::MedicineService::getInstance()),
      m_departmentService(BLL::DepartmentService::getInstance()),
      m_prescriptionService(BLL::PrescriptionService::getInstance()),
      m_reportGenerator(BLL::ReportGenerator::getInstance()),
      m_isInitialized(false)
{
}

// ==================== Singleton Access ====================
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

HMSFacade::~HMSFacade() = default;

// ==================== System Lifecycle ====================
bool HMSFacade::initialize() {
    if (m_isInitialized) {
        return true;
    }

    m_isInitialized = loadData();
    return m_isInitialized;
}

void HMSFacade::shutdown() {
    if (m_isInitialized) {
        saveData();
        m_isInitialized = false;
    }
}

bool HMSFacade::isInitialized() const {
    return m_isInitialized;
}

// ==================== Authentication ====================
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

// ==================== Registration ====================
bool HMSFacade::registerPatient(const std::string& username,
                                 const std::string& password,
                                 const std::string& name,
                                 const std::string& phone,
                                 const std::string& gender,
                                 const std::string& dateOfBirth,
                                 const std::string& address) {
    // First register account with PATIENT role
    if (!m_authService->registerAccount(username, password, Role::PATIENT)) {
        return false;
    }

    // Check if a doctor-created patient record already exists with matching identity
    auto existingPatient = m_patientService->findUnlinkedPatient(
        phone, name, dateOfBirth, stringToGender(gender));

    if (existingPatient.has_value()) {
        // Link existing patient record to the new account
        return m_patientService->linkPatientToAccount(
            existingPatient->getPatientID(), username);
    }

    // No existing record - create new patient record
    auto result = m_patientService->createPatient(username, name, phone, stringToGender(gender),
                                                   dateOfBirth, address, "");
    return result.has_value();
}

bool HMSFacade::isUsernameAvailable(const std::string& username) {
    return m_authService->isUsernameAvailable(username);
}

// ==================== Patient Operations ====================
std::optional<Model::Patient> HMSFacade::getMyProfile() {
    if (getCurrentRole() != Role::PATIENT) {
        return std::nullopt;
    }
    return m_patientService->getPatientByUsername(getCurrentUsername());
}

bool HMSFacade::updateMyProfile(const std::string& phone, const std::string& address) {
    if (getCurrentRole() != Role::PATIENT) {
        return false;
    }

    auto patient = getMyProfile();
    if (!patient) {
        return false;
    }

    // Update the patient object
    patient->setPhone(phone);
    patient->setAddress(address);

    return m_patientService->updatePatient(*patient);
}

std::vector<Model::Appointment> HMSFacade::getMyAppointments() {
    if (getCurrentRole() != Role::PATIENT) {
        return {};
    }

    // Get all appointments (both upcoming and past)
    auto upcoming = m_patientService->getUpcomingAppointments(getCurrentUsername());
    auto past = m_patientService->getPastAppointments(getCurrentUsername());

    // Combine them
    std::vector<Model::Appointment> all;
    all.insert(all.end(), upcoming.begin(), upcoming.end());
    all.insert(all.end(), past.begin(), past.end());
    return all;
}

std::vector<Model::Appointment> HMSFacade::getMyUpcomingAppointments() {
    if (getCurrentRole() == Role::PATIENT) {
        return m_patientService->getUpcomingAppointments(getCurrentUsername());
    }

    if (getCurrentRole() == Role::DOCTOR) {
        auto doctor = m_doctorService->getDoctorByUsername(getCurrentUsername());
        if (doctor) {
            return m_doctorService->getUpcomingAppointments(doctor->getDoctorID());
        }
    }

    return {};
}

double HMSFacade::getMyTotalBill() {
    if (getCurrentRole() != Role::PATIENT) {
        return 0.0;
    }

    return m_patientService->calculateTotalBill(getCurrentUsername());
}

bool HMSFacade::bookAppointment(const std::string& doctorID,
                                const std::string& date,
                                const std::string& time,
                                const std::string& disease) {
    if (getCurrentRole() != Role::PATIENT) {
        return false;
    }

    auto appointment = m_appointmentService->bookAppointment(
        getCurrentUsername(), doctorID, date, time, disease);

    return appointment.has_value();
}

bool HMSFacade::cancelAppointment(const std::string& appointmentID) {
    return m_appointmentService->cancelAppointment(appointmentID);
}

// ==================== Doctor Operations ====================
std::vector<Model::Doctor> HMSFacade::getAllDoctors() {
    return m_doctorService->getAllDoctors();
}

std::vector<Model::Doctor> HMSFacade::getDoctorsBySpecialization(const std::string& specialization) {
    return m_doctorService->getDoctorsBySpecialization(specialization);
}

std::vector<std::string> HMSFacade::getAllSpecializations() {
    return m_doctorService->getAllSpecializations();
}

std::optional<Model::Doctor> HMSFacade::getDoctorByID(const std::string& doctorID) {
    return m_doctorService->getDoctorByID(doctorID);
}

std::vector<std::string> HMSFacade::getAvailableSlots(const std::string& doctorID,
                                                      const std::string& date) {
    return m_appointmentService->getAvailableSlots(doctorID, date);
}

// ==================== Doctor's Patient Management ====================
std::vector<Model::Patient> HMSFacade::getAllPatients() {
    return m_patientService->getAllPatients();
}

std::vector<Model::Patient> HMSFacade::searchPatients(const std::string& keyword) {
    return m_patientService->searchPatients(keyword);
}

std::optional<Model::Patient> HMSFacade::getPatientByID(const std::string& patientID) {
    return m_patientService->getPatientByID(patientID);
}

std::optional<Model::Patient> HMSFacade::getPatientByUsername(const std::string& username) {
    return m_patientService->getPatientByUsername(username);
}

bool HMSFacade::addPatient(const std::string& name,
                           const std::string& phone,
                           const std::string& gender,
                           const std::string& dateOfBirth,
                           const std::string& address,
                           const std::string& medicalHistory) {
    auto result = m_patientService->createPatient("", name, phone, stringToGender(gender),
                                                   dateOfBirth, address, medicalHistory);
    return result.has_value();
}

bool HMSFacade::updatePatient(const std::string& patientID,
                              const std::string& phone,
                              const std::string& address,
                              const std::string& medicalHistory) {
    auto patient = m_patientService->getPatientByID(patientID);
    if (!patient) {
        return false;
    }

    if (!phone.empty()) {
        patient->setPhone(phone);
    }

    if (!address.empty()) {
        patient->setAddress(address);
    }

    if (!medicalHistory.empty()) {
        patient->setMedicalHistory(medicalHistory);
    }


    return m_patientService->updatePatient(*patient);
}

bool HMSFacade::deletePatient(const std::string& patientID) {
    return m_patientService->deletePatient(patientID);
}

std::vector<Model::Appointment> HMSFacade::getMySchedule(const std::string& date) {
    if (getCurrentRole() != Role::DOCTOR) {
        return {};
    }

    // Get doctor ID from current username
    auto doctor = m_doctorService->getDoctorByUsername(getCurrentUsername());
    if (!doctor) {
        return {};
    }

    return m_doctorService->getDoctorSchedule(doctor->getDoctorID(), date);
}

bool HMSFacade::markAppointmentCompleted(const std::string& appointmentID) {
    return m_appointmentService->markAsCompleted(appointmentID);
}

// ==================== Admin Operations ====================
bool HMSFacade::addDoctor(const std::string& username,
                          const std::string& password,
                          const std::string& name,
                          const std::string& phone,
                          const std::string& gender,
                          const std::string& dateOfBirth,
                          const std::string& specialization,
                          const std::string& schedule,
                          double consultationFee) {
    // First register account with DOCTOR role
    if (!m_authService->registerAccount(username, password, Role::DOCTOR)) {
        return false;
    }

    // Then create doctor record
    auto result = m_doctorService->createDoctor(username, name, phone, stringToGender(gender),
                                                 dateOfBirth, specialization, schedule, consultationFee);
    return result.has_value();
}

bool HMSFacade::updateDoctor(const std::string& doctorID,
                             const std::string& specialization,
                             const std::string& schedule,
                             double consultationFee) {
    auto doctor = m_doctorService->getDoctorByID(doctorID);
    if (!doctor) {
        return false;
    }

    // Only update fields that have new values (non-empty for strings, >= 0 for fee)
    if (!specialization.empty()) {
        doctor->setSpecialization(specialization);
    }
    if (!schedule.empty()) {
        doctor->setSchedule(schedule);
    }
    // consultationFee < 0 means "keep original value" (getDoubleInput returns -1 for empty input)
    if (consultationFee >= 0) {
        doctor->setConsultationFee(consultationFee);
    }

    return m_doctorService->updateDoctor(*doctor);
}

bool HMSFacade::deleteDoctor(const std::string& doctorID) {
    return m_doctorService->deleteDoctor(doctorID);
}

Model::Statistics HMSFacade::getStatistics() {
    return m_adminService->getStatistics();
}

std::string HMSFacade::generateReport() {
    return m_adminService->generateSummaryReport();
}

// ==================== Medicine Management ====================
std::vector<Model::Medicine> HMSFacade::getAllMedicines() {
    return m_medicineService->getAllMedicines();
}

std::optional<Model::Medicine> HMSFacade::getMedicineByID(const std::string& medicineID) {
    return m_medicineService->getMedicineByID(medicineID);
}

std::vector<Model::Medicine> HMSFacade::searchMedicines(const std::string& keyword) {
    return m_medicineService->searchMedicines(keyword);
}

bool HMSFacade::createMedicine(const std::string& medicineID,
                               const std::string& name,
                               const std::string& genericName,
                               const std::string& category,
                               const std::string& manufacturer,
                               const std::string& description,
                               double unitPrice,
                               int quantityInStock,
                               int reorderLevel,
                               const std::string& expiryDate,
                               const std::string& dosageForm,
                               const std::string& strength) {
    Model::Medicine medicine(medicineID, name, category, unitPrice, quantityInStock);
    medicine.setGenericName(genericName);
    medicine.setManufacturer(manufacturer);
    medicine.setDescription(description);
    medicine.setReorderLevel(reorderLevel);
    medicine.setExpiryDate(expiryDate);
    medicine.setDosageForm(dosageForm);
    medicine.setStrength(strength);

    return m_medicineService->createMedicine(medicine);
}

bool HMSFacade::updateMedicine(const std::string& medicineID,
                               const std::string& name,
                               const std::string& category,
                               double unitPrice,
                               int reorderLevel,
                               const std::string& expiryDate) {
    auto medicine = m_medicineService->getMedicineByID(medicineID);
    if (!medicine) {
        return false;
    }

    medicine->setName(name);
    medicine->setCategory(category);
    medicine->setUnitPrice(unitPrice);
    medicine->setReorderLevel(reorderLevel);
    medicine->setExpiryDate(expiryDate);

    return m_medicineService->updateMedicine(*medicine);
}

bool HMSFacade::deleteMedicine(const std::string& medicineID) {
    return m_medicineService->deleteMedicine(medicineID);
}

bool HMSFacade::addMedicineStock(const std::string& medicineID, int quantity) {
    return m_medicineService->addStock(medicineID, quantity);
}

bool HMSFacade::removeMedicineStock(const std::string& medicineID, int quantity) {
    return m_medicineService->removeStock(medicineID, quantity);
}

std::vector<BLL::StockAlert> HMSFacade::getLowStockAlerts() {
    return m_medicineService->getLowStockAlerts();
}

std::vector<BLL::ExpiryAlert> HMSFacade::getExpiryAlerts() {
    return m_medicineService->getExpiryAlerts();
}

// ==================== Department Management ====================
std::vector<Model::Department> HMSFacade::getAllDepartments() {
    return m_departmentService->getAllDepartments();
}

std::optional<Model::Department> HMSFacade::getDepartmentByID(const std::string& departmentID) {
    return m_departmentService->getDepartmentByID(departmentID);
}

bool HMSFacade::createDepartment(const std::string& departmentID,
                                 const std::string& name,
                                 const std::string& description,
                                 const std::string& headDoctorID,
                                 const std::string& location,
                                 const std::string& contactNumber) {
    Model::Department department(departmentID, name, description, headDoctorID);
    department.setLocation(location);
    department.setPhone(contactNumber);

    return m_departmentService->createDepartment(department);
}

bool HMSFacade::updateDepartment(const std::string& departmentID,
                                 const std::string& name,
                                 const std::string& description,
                                 const std::string& headDoctorID,
                                 const std::string& location,
                                 const std::string& contactNumber) {
    auto department = m_departmentService->getDepartmentByID(departmentID);
    if (!department) {
        return false;
    }

    department->setName(name);
    department->setDescription(description);
    department->setHeadDoctorID(headDoctorID);
    department->setLocation(location);
    department->setPhone(contactNumber);

    return m_departmentService->updateDepartment(*department);
}

bool HMSFacade::deleteDepartment(const std::string& departmentID) {
    return m_departmentService->deleteDepartment(departmentID);
}

bool HMSFacade::assignDoctorToDepartment(const std::string& doctorID,
                                         const std::string& departmentID) {
    return m_departmentService->assignDoctor(departmentID, doctorID);
}

BLL::DepartmentStats HMSFacade::getDepartmentStats(const std::string& departmentID) {
    return m_departmentService->getDepartmentStats(departmentID);
}

// ==================== Prescription Management ====================
std::string HMSFacade::generatePrescriptionID() {
    return m_prescriptionService->generateNextID();
}

bool HMSFacade::createPrescription(const std::string& prescriptionID,
                                   const std::string& patientID,
                                   const std::string& doctorID,
                                   const std::string& appointmentID,
                                   const std::string& date,
                                   const std::string& diagnosis,
                                   const std::string& instructions) {
    // Constructor signature: (prescriptionID, appointmentID, patientUsername, doctorID, date)
    Model::Prescription prescription(prescriptionID, appointmentID, patientID, doctorID, date);
    prescription.setDiagnosis(diagnosis);
    prescription.setNotes(instructions);

    return m_prescriptionService->createPrescription(prescription);
}

bool HMSFacade::addPrescriptionItem(const std::string& prescriptionID,
                                    const std::string& medicineID,
                                    const std::string& dosage,
                                    const std::string& duration,
                                    int quantity) {
    // Note: PrescriptionService expects (prescriptionID, medicineID, quantity, dosage, duration, instructions)
    return m_prescriptionService->addPrescriptionItem(prescriptionID, medicineID,
                                                      quantity, dosage, duration, "");
}

std::vector<Model::Prescription> HMSFacade::getPatientPrescriptions(const std::string& patientID) {
    return m_prescriptionService->getPatientPrescriptions(patientID);
}

std::vector<Model::Prescription> HMSFacade::getDoctorPrescriptions(const std::string& doctorID) {
    return m_prescriptionService->getDoctorPrescriptions(doctorID);
}

std::optional<Model::Prescription> HMSFacade::getPrescriptionByID(const std::string& prescriptionID) {
    return m_prescriptionService->getPrescriptionByID(prescriptionID);
}

bool HMSFacade::dispensePrescription(const std::string& prescriptionID) {
    BLL::DispenseResult result = m_prescriptionService->dispensePrescription(prescriptionID);
    return result.success;
}

// ==================== Report Generation ====================
std::string HMSFacade::generateDailyReport(const std::string& date, const std::string& format) {
    BLL::Report report = m_reportGenerator->generateDailyReport(date);

    if (format == "csv") {
        return m_reportGenerator->exportToCSV(report);
    } else if (format == "html") {
        return m_reportGenerator->exportToHTML(report);
    } else {
        return m_reportGenerator->exportToText(report);
    }
}

std::string HMSFacade::generateWeeklyReport(const std::string& startDate, const std::string& format) {
    BLL::Report report = m_reportGenerator->generateWeeklyReport(startDate);

    if (format == "csv") {
        return m_reportGenerator->exportToCSV(report);
    } else if (format == "html") {
        return m_reportGenerator->exportToHTML(report);
    } else {
        return m_reportGenerator->exportToText(report);
    }
}

std::string HMSFacade::generateMonthlyReport(int year, int month, const std::string& format) {
    BLL::Report report = m_reportGenerator->generateMonthlyReport(month, year);

    if (format == "csv") {
        return m_reportGenerator->exportToCSV(report);
    } else if (format == "html") {
        return m_reportGenerator->exportToHTML(report);
    } else {
        return m_reportGenerator->exportToText(report);
    }
}

std::string HMSFacade::generateRevenueReport(const std::string& startDate,
                                             const std::string& endDate,
                                             const std::string& format) {
    BLL::Report report = m_reportGenerator->generateRevenueReport(startDate, endDate);

    if (format == "csv") {
        return m_reportGenerator->exportToCSV(report);
    } else if (format == "html") {
        return m_reportGenerator->exportToHTML(report);
    } else {
        return m_reportGenerator->exportToText(report);
    }
}

bool HMSFacade::exportReport(const std::string& reportContent,
                             const std::string& filename,
                             const std::string& format) {
    // Create a temporary Report struct from the content
    BLL::Report report;
    report.content = reportContent;

    BLL::ExportFormat exportFormat;
    if (format == "csv") {
        exportFormat = BLL::ExportFormat::CSV;
    } else if (format == "html") {
        exportFormat = BLL::ExportFormat::HTML;
    } else {
        exportFormat = BLL::ExportFormat::TEXT;
    }

    return m_reportGenerator->exportReport(report, exportFormat, filename);
}

// ==================== Data Operations ====================
bool HMSFacade::saveData() {
    // Services auto-save via repositories
    return true;
}

bool HMSFacade::loadData() {
    // Services auto-load via repositories
    return true;
}

} // namespace UI
} // namespace HMS
