#include "bll/PatientService.h"
#include "common/Utils.h"
#include "common/Constants.h"

#include <algorithm>

namespace HMS
{
    namespace BLL
    {

        // ==================== Singleton ====================
        std::unique_ptr<PatientService> PatientService::s_instance = nullptr;
        std::mutex PatientService::s_mutex;

        // ==================== Private Constructor ====================
        PatientService::PatientService()
            : m_patientRepo(DAL::PatientRepository::getInstance()),
              m_appointmentRepo(DAL::AppointmentRepository::getInstance()),
              m_accountRepo(DAL::AccountRepository::getInstance())
        {
        }

        PatientService *PatientService::getInstance()
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            if (s_instance == nullptr)
            {
                s_instance = std::unique_ptr<PatientService>(new PatientService());
            }

            return s_instance.get();
        }

        void PatientService::resetInstance()
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_instance.reset();
        }

        PatientService::~PatientService() = default;

        // ==================== CRUD Operations ====================

        bool PatientService::createPatient(const Model::Patient &patient)
        {
            // Validate patient data first
            if (!validatePatient(patient))
            {
                return false;
            }

            // Check if patient ID already exists using repository's exists()
            if (m_patientRepo->exists(patient.getPatientID()))
            {
                return false;
            }

            // Check if username already has a patient record using repository's getByUsername()
            if (m_patientRepo->getByUsername(patient.getUsername()).has_value())
            {
                return false;
            }

            // Add patient to repository
            return m_patientRepo->add(patient);
        }

        Result<Model::Patient> PatientService::createPatient(
            const std::string &username,
            const std::string &name,
            const std::string &phone,
            Gender gender,
            const std::string &dateOfBirth,
            const std::string &address,
            const std::string &medicalHistory)
        {

            // Generate unique patient ID
            std::string patientID = m_patientRepo->getNextId();

            // Create patient object
            Model::Patient patient(patientID, username, name, phone, gender,
                                   dateOfBirth, address, medicalHistory);

            // Try to create the patient
            if (createPatient(patient))
            {
                return patient;
            }

            return std::nullopt;
        }

        bool PatientService::updatePatient(const Model::Patient &patient)
        {
            // Validate patient data
            if (!validatePatient(patient))
            {
                return false;
            }

            // Check if patient exists using repository's exists()
            if (!m_patientRepo->exists(patient.getPatientID()))
            {
                return false;
            }

            // Update patient in repository
            return m_patientRepo->update(patient);
        }

        bool PatientService::deletePatient(const std::string &patientID)
        {
            // Get patient to find username (also validates existence)
            auto patientOpt = m_patientRepo->getById(patientID);
            if (!patientOpt.has_value())
            {
                return false;
            }

            std::string username = patientOpt->getUsername();

            // Delete patient profile
            if (!m_patientRepo->remove(patientID))
            {
                return false;
            }

            // NOTE: Appointments are intentionally NOT deleted to preserve:
            // - Historical medical records and audit trail
            // - Revenue/financial reporting accuracy
            // - Referential integrity for Prescriptions
            // Orphaned appointments (with deleted patient) remain as historical data

            // Cascading delete: Remove associated account to prevent orphaned logins
            // This is a security requirement - deleted patients should not be able to log in
            if (!username.empty())
            {
                m_accountRepo->remove(username);
            }

            return true;
        }

        // ==================== Query Operations ====================
        Result<Model::Patient> PatientService::getPatientByID(const std::string &patientID)
        {
            return m_patientRepo->getById(patientID);
        }

        Result<Model::Patient> PatientService::getPatientByUsername(const std::string &username)
        {
            return m_patientRepo->getByUsername(username);
        }

        List<Model::Patient> PatientService::getAllPatients()
        {
            return m_patientRepo->getAll();
        }

        List<Model::Patient> PatientService::searchPatients(const std::string &keyword)
        {
            if (keyword.empty())
            {
                return m_patientRepo->getAll();
            }

            return m_patientRepo->search(keyword);
        }

        size_t PatientService::getPatientCount() const
        {
            return m_patientRepo->count();
        }

        // ==================== Appointment Queries ====================

        List<Model::Appointment> PatientService::getAppointmentHistory(const std::string &username)
        {
            return m_appointmentRepo->getByPatient(username);
        }

        List<Model::Appointment> PatientService::getUpcomingAppointments(const std::string &username)
        {
            return m_appointmentRepo->getUpcomingByPatient(username);
        }

        List<Model::Appointment> PatientService::getPastAppointments(const std::string &username)
        {
            return m_appointmentRepo->getHistoryByPatient(username);
        }

        // ==================== Billing ====================

        double PatientService::calculateTotalBill(const std::string &username)
        {
            auto unpaidAppointments = getUnpaidAppointments(username);
            double total = 0.0;

            for (const auto &apt : unpaidAppointments)
            {
                total += apt.getPrice();
            }

            return total;
        }

        double PatientService::calculateTotalPaid(const std::string &username)
        {
            auto allAppointments = m_appointmentRepo->getByPatient(username);
            double total = 0.0;

            for (const auto &apt : allAppointments)
            {
                // Only count paid appointments
                if (apt.isPaid())
                {
                    total += apt.getPrice();
                }
            }

            return total;
        }

        List<Model::Appointment> PatientService::getUnpaidAppointments(const std::string &username)
        {
            auto unpaidAppointments = m_appointmentRepo->getUnpaidByPatient(username);
            List<Model::Appointment> result;

            for (const auto &apt : unpaidAppointments)
            {
                AppointmentStatus status = apt.getStatus();
                if (status == AppointmentStatus::SCHEDULED ||
                    status == AppointmentStatus::COMPLETED)
                {
                    result.push_back(apt);
                }
            }

            // Sort by date (oldest first) using Utils::compareDates for consistency
            std::sort(result.begin(), result.end(),
                      [](const Model::Appointment &a, const Model::Appointment &b)
                      {
                          int dateCompare = Utils::compareDates(a.getDate(), b.getDate());
                          if (dateCompare == 0)
                          {
                              return a.getTime() < b.getTime();
                          }
                          return dateCompare < 0;
                      });

            return result;
        }

        // ==================== Medical History ====================
        bool PatientService::updateMedicalHistory(const std::string &patientID,
                                                  const std::string &newHistory)
        {
            auto patientOpt = m_patientRepo->getById(patientID);
            if (!patientOpt.has_value())
            {
                return false;
            }

            Model::Patient patient = patientOpt.value();
            patient.setMedicalHistory(newHistory);

            return updatePatient(patient);
        }

        bool PatientService::appendMedicalHistory(const std::string &patientID,
                                                  const std::string &entry)
        {
            auto patientOpt = m_patientRepo->getById(patientID);
            if (!patientOpt.has_value())
            {
                return false;
            }

            Model::Patient patient = patientOpt.value();

            // Get current history and append with timestamp
            std::string currentHistory = patient.getMedicalHistory();

            // Use Utils::getCurrentDate() for timestamp
            std::string dateStamp = "[" + Utils::getCurrentDate() + "] ";
            std::string newHistory;

            if (currentHistory.empty())
            {
                newHistory = dateStamp + entry;
            }
            else
            {
                newHistory = currentHistory + "\n" + dateStamp + entry;
            }

            patient.setMedicalHistory(newHistory);

            return updatePatient(patient);
        }

        // ==================== Validation ====================
        bool PatientService::validatePatient(const Model::Patient &patient)
        {
            // Validate patient ID
            if (patient.getPatientID().empty())
            {
                return false;
            }

            // Validate username
            if (!patient.getUsername().empty() && !Utils::isValidUsername(patient.getUsername()))
            {
                return false;
            }

            // Validate name
            std::string name = patient.getName();
            if (name.empty() || name.length() < 2)
            {
                return false;
            }

            // Validate phone
            if (!Utils::isValidPhone(patient.getPhone()))
            {
                return false;
            }

            // Validate date of birth
            std::string dob = patient.getDateOfBirth();
            if (!Utils::isValidDate(dob))
            {
                return false;
            }

            // Check if date of birth is not in the future
            if (Utils::isFutureDate(dob))
            {
                return false;
            }

            // Validate address
            if (patient.getAddress().empty())
            {
                return false;
            }

            // Validate gender
            Gender gender = patient.getGender();
            if (gender == Gender::UNKNOWN)
            {
                return false;
            }

            return true;
        }

        bool PatientService::patientExists(const std::string &patientID)
        {
            return m_patientRepo->exists(patientID);
        }

        // ==================== Data Persistence ====================

        bool PatientService::saveData()
        {
            return m_patientRepo->save();
        }

        bool PatientService::loadData()
        {
            return m_patientRepo->load();
        }

    } // namespace BLL
} // namespace HMS
