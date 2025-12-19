#include "bll/PatientService.h"
#include "common/Utils.h"
#include "common/Constants.h"

#include <algorithm>
#include <iostream>

namespace HMS {
namespace BLL {

// ==================== Singleton ====================
std::unique_ptr<PatientService> PatientService::s_instance = nullptr;
std::mutex PatientService::s_mutex;

// ==================== Private Constructor ====================
PatientService::PatientService()
    : m_patientRepo(DAL::PatientRepository::getInstance()),
      m_appointmentRepo(DAL::AppointmentRepository::getInstance()) {
}

PatientService* PatientService::getInstance() {
    std::lock_guard<std::mutex> lock(s_mutex);
    if (s_instance == nullptr) {
        s_instance = std::unique_ptr<PatientService>(new PatientService());
    }

    return s_instance.get();
}

void PatientService::resetInstance() {
    std::lock_guard<std::mutex> lock(s_mutex);
    s_instance.reset();
}

PatientService::~PatientService() = default;

// ==================== CRUD Operations ====================

bool PatientService::createPatient(const Model::Patient& patient) {
    // Validate patient data first
    if (!validatePatient(patient)) {
        return false;
    }

    // Check if patient ID already exists using repository's exists()
    if (m_patientRepo->exists(patient.getPatientID())) {
        return false;
    }

    // Check if username already has a patient record using repository's getByUsername()
    if (m_patientRepo->getByUsername(patient.getUsername()).has_value()) {
        return false;
    }

    // Add patient to repository
    bool success = m_patientRepo->add(patient);

    return success;
}

std::optional<Model::Patient> PatientService::createPatient(
    const std::string& username,
    const std::string& name,
    const std::string& phone,
    Gender gender,
    const std::string& dateOfBirth,
    const std::string& address,
    const std::string& medicalHistory) {

    // Generate unique patient ID
    std::string patientID = Utils::generatePatientID();

    // Create patient object
    Model::Patient patient(patientID, username, name, phone, gender,
                          dateOfBirth, address, medicalHistory);

    // Try to create the patient
    if (createPatient(patient)) {
        return patient;
    }

    return std::nullopt;
}

bool PatientService::updatePatient(const Model::Patient& patient) {
    // Validate patient data
    if (!validatePatient(patient)) {
        return false;
    }

    // Check if patient exists using repository's exists()
    if (!m_patientRepo->exists(patient.getPatientID())) {
        return false;
    }

    // Update patient in repository
    bool success = m_patientRepo->update(patient);

    return success;
}

bool PatientService::deletePatient(const std::string& patientID) {
    // Check if patient exists
    if (!m_patientRepo->exists(patientID)) {
        return false;
    }

    /// Get patient to find username
    auto patientOpt = m_patientRepo->getById(patientID);
    if (!patientOpt.has_value()) {
        return false;
    }

    std::string username = patientOpt->getUsername();

    // Delete appointments first, check all success
    bool allAppointmentsDeleted = true;
    auto appointments = m_appointmentRepo->getByPatient(username);
    for (const auto& apt : appointments) {
        if (!m_appointmentRepo->remove(apt.getAppointmentID())) {
            allAppointmentsDeleted = false;
            break;
        }
    }

    if (!allAppointmentsDeleted) {
        return false;
    }

    // Only delete patient if all appointments deleted
    return m_patientRepo->remove(patientID);
}

// ==================== Query Operations ====================
std::optional<Model::Patient> PatientService::getPatientByID(const std::string& patientID) {
    return m_patientRepo->getById(patientID);
}

std::optional<Model::Patient> PatientService::getPatientByUsername(const std::string& username) {
    return m_patientRepo->getByUsername(username);
}

std::vector<Model::Patient> PatientService::getAllPatients() {
    return m_patientRepo->getAll();
}

std::vector<Model::Patient> PatientService::searchPatients(const std::string& keyword) {
    if (keyword.empty()) {
        return m_patientRepo->getAll();
    }

    return m_patientRepo->search(keyword);
}

size_t PatientService::getPatientCount() const {
    return m_patientRepo->count();
}

// ==================== Appointment Queries ====================

std::vector<Model::Appointment> PatientService::getAppointmentHistory(const std::string& username) {
    return m_appointmentRepo->getByPatient(username);
}

std::vector<Model::Appointment> PatientService::getUpcomingAppointments(const std::string& username) {
    return m_appointmentRepo->getUpcomingByPatient(username);
}

std::vector<Model::Appointment> PatientService::getPastAppointments(const std::string& username) {
    return m_appointmentRepo->getHistoryByPatient(username);
}

// ==================== Billing ====================

double PatientService::calculateTotalBill(const std::string& username) {
    auto unpaidAppointments = m_appointmentRepo->getUnpaidByPatient(username);
    double total = 0.0;

    for (const auto& apt : unpaidAppointments) {
        // Only count scheduled or completed appointments
        AppointmentStatus status = apt.getStatus();
        if (status == AppointmentStatus::SCHEDULED ||
            status == AppointmentStatus::COMPLETED) {
            total += apt.getPrice();
        }
    }

    return total;
}

double PatientService::calculateTotalPaid(const std::string& username) {
    auto allAppointments = m_appointmentRepo->getByPatient(username);
    double total = 0.0;

    for (const auto& apt : allAppointments) {
        // Only count paid appointments
        if (apt.isPaid()) {
            total += apt.getPrice();
        }
    }

    return total;
}

std::vector<Model::Appointment> PatientService::getUnpaidAppointments(const std::string& username) {
    auto unpaidAppointments = m_appointmentRepo->getUnpaidByPatient(username);
    std::vector<Model::Appointment> result;

    for (const auto& apt : unpaidAppointments) {
        AppointmentStatus status = apt.getStatus();
        if (status == AppointmentStatus::SCHEDULED ||
            status == AppointmentStatus::COMPLETED) {
            result.push_back(apt);
        }
    }

    // Sort by date (oldest first)
    std::sort(result.begin(), result.end(),
        [](const Model::Appointment& a, const Model::Appointment& b) {
            if (a.getDate() == b.getDate()) {
                return a.getTime() < b.getTime();
            }
            return a.getDate() < b.getDate();
        });

    return result;
}

// ==================== Medical History ====================
bool PatientService::updateMedicalHistory(const std::string& patientID,
                            const std::string& newHistory) {
    auto patientOpt = m_patientRepo->getById(patientID);
    if (!patientOpt.has_value()) {
        return false;
    }

    Model::Patient patient = patientOpt.value();
    patient.setMedicalHistory(newHistory);

    return updatePatient(patient);
}

bool PatientService::appendMedicalHistory(const std::string& patientID,
                            const std::string& entry) {
    auto patientOpt = m_patientRepo->getById(patientID);
    if (!patientOpt.has_value()) {
        return false;
    }

    Model::Patient patient = patientOpt.value();

    // Get current history and append with timestamp
    std::string currentHistory = patient.getMedicalHistory();

    // Use Utils::getCurrentDate() for timestamp
    std::string dateStamp = "[" + Utils::getCurrentDate() + "] ";
    std::string newHistory;

    if (currentHistory.empty()) {
        newHistory = dateStamp + entry;
    } else {
        newHistory = currentHistory + "\n" + dateStamp + entry;
    }

    patient.setMedicalHistory(newHistory);

    return updatePatient(patient);
}

// ==================== Validation ====================
bool PatientService::validatePatient(const Model::Patient& patient) {
    // Validate patient ID
    if (patient.getPatientID().empty()) {
        return false;
    }

    // Validate username
    if (!Utils::isValidUsername(patient.getUsername())) {
        return false;
    }

    // Validate name
    std::string name = patient.getName();
    if (name.empty() || name.length() < 2) {
        return false;
    }

    // Validate phone
    if (!Utils::isValidPhone(patient.getPhone())) {
        return false;
    }

    // Validate date of birth
    std::string dob = patient.getDateOfBirth();
    if (!Utils::isValidDate(dob)) {
        return false;
    }

    // Check if date of birth is not in the future
    if (Utils::isFutureDate(dob)) {
        return false;
    }

    // Validate address
    if (patient.getAddress().empty()) {
        return false;
    }

    // Validate gender
    Gender gender = patient.getGender();
    if (gender == Gender::UNKNOWN) {
        return false;
    }

    return true;
}

bool PatientService::patientExists(const std::string& patientID) {
    return m_patientRepo->exists(patientID);
}

// ==================== Data Persistence ====================

bool PatientService::saveData() {
    bool success = m_patientRepo->save();

    return success;
}

bool PatientService::loadData() {
    bool success = m_patientRepo->load();

    return success;
}

} // namespace BLL
} // namespace HMS
