#include "dal/AppointmentRepository.h"
#include "dal/FileHelper.h"
#include "common/Utils.h"
#include "common/Constants.h"

#include <algorithm>
#include <sstream>

namespace HMS {
namespace DAL {

// ==================== Singleton Instance ====================
std::unique_ptr<AppointmentRepository> AppointmentRepository::s_instance = nullptr;
std::mutex AppointmentRepository::s_mutex;

// ==================== Private Constructor ====================
AppointmentRepository::AppointmentRepository()
    : m_filePath(Constants::APPOINTMENT_FILE),
      m_isLoaded(false) {
}

// ==================== Singleton Access ====================
AppointmentRepository* AppointmentRepository::getInstance() {
    if (!s_instance) {
        std::lock_guard<std::mutex> lock(s_mutex);
        if (!s_instance) {
            s_instance = std::unique_ptr<AppointmentRepository>(new AppointmentRepository());
            s_instance->load();
        }
    }
    return s_instance.get();
}

void AppointmentRepository::resetInstance() {
    std::lock_guard<std::mutex> lock(s_mutex);
    s_instance.reset();
}

AppointmentRepository::~AppointmentRepository() {
    save();
}

// ==================== CRUD Operations ====================
std::vector<Model::Appointment> AppointmentRepository::getAll() {
    return m_appointments;
}

std::optional<Model::Appointment> AppointmentRepository::getById(const std::string& id) {
    auto it = std::find_if(m_appointments.begin(), m_appointments.end(),
                          [&id](const Model::Appointment& apt) {
                              return apt.getAppointmentID() == id;
                          });

    if (it != m_appointments.end()) {
        return *it;
    }
    return std::nullopt;
}

bool AppointmentRepository::add(const Model::Appointment& appointment) {
    // Check if appointment already exists
    if (exists(appointment.getAppointmentID())) {
        return false;
    }

    m_appointments.push_back(appointment);
    return save();
}

bool AppointmentRepository::update(const Model::Appointment& appointment) {
    auto it = std::find_if(m_appointments.begin(), m_appointments.end(),
                          [&appointment](const Model::Appointment& apt) {
                              return apt.getAppointmentID() == appointment.getAppointmentID();
                          });

    if (it != m_appointments.end()) {
        *it = appointment;
        return save();
    }
    return false;
}

bool AppointmentRepository::remove(const std::string& id) {
    auto it = std::find_if(m_appointments.begin(), m_appointments.end(),
                          [&id](const Model::Appointment& apt) {
                              return apt.getAppointmentID() == id;
                          });

    if (it != m_appointments.end()) {
        m_appointments.erase(it);
        return save();
    }
    return false;
}

// ==================== Persistence ====================
bool AppointmentRepository::save() {
    try {
        std::vector<std::string> lines;

        // Get header and push its lines (handle multi-line header)
        auto header = FileHelper::getFileHeader("Appointment");
        std::stringstream hss(header);
        std::string headerLine;
        while (std::getline(hss, headerLine)) {
            // skip empty header lines if any
            if (!headerLine.empty()) {
                lines.push_back(headerLine);
            }
        }

        // Serialize all appointments
        for (const auto& apt : m_appointments) {
            lines.push_back(apt.serialize());
        }

        // Ensure backup (if any) before writing
        FileHelper::createBackup(m_filePath);

        // Write to file
        return FileHelper::writeLines(m_filePath, lines);
    } catch (...) {
        return false;
    }
}

bool AppointmentRepository::load() {
    try {
        // Ensure data directory exists first
        FileHelper::createDirectoryIfNotExists(Constants::DATA_DIR);

        // Create file if not exists
        FileHelper::createFileIfNotExists(m_filePath);

        // Read all lines (readLines excludes comments/empty per FileHelper.h)
        std::vector<std::string> lines = FileHelper::readLines(m_filePath);

        // Clear existing data
        m_appointments.clear();

        // Deserialize each line
        for (const auto& line : lines) {
            auto appointment = Model::Appointment::deserialize(line);
            if (appointment) {
                m_appointments.push_back(appointment.value());
            }
        }

        m_isLoaded = true;
        return true;
    } catch (...) {
        return false;
    }
}

// ==================== Query Operations ====================
size_t AppointmentRepository::count() const {
    return m_appointments.size();
}

bool AppointmentRepository::exists(const std::string& id) const {
    return std::any_of(m_appointments.begin(), m_appointments.end(),
                      [&id](const Model::Appointment& apt) {
                          return apt.getAppointmentID() == id;
                      });
}

bool AppointmentRepository::clear() {
    m_appointments.clear();
    return save();
}

// ==================== Patient-Related Queries ====================
std::vector<Model::Appointment> AppointmentRepository::getByPatient(const std::string& patientUsername) {
    std::vector<Model::Appointment> result;
    std::copy_if(m_appointments.begin(), m_appointments.end(),
                std::back_inserter(result),
                [&patientUsername](const Model::Appointment& apt) {
                    return apt.getPatientUsername() == patientUsername;
                });
    return result;
}

std::vector<Model::Appointment> AppointmentRepository::getUpcomingByPatient(const std::string& patientUsername) {
    std::vector<Model::Appointment> result;
    std::string today = Utils::getCurrentDate();

    std::copy_if(m_appointments.begin(), m_appointments.end(),
                std::back_inserter(result),
                [&patientUsername, &today](const Model::Appointment& apt) {
                    return apt.getPatientUsername() == patientUsername &&
                           apt.getStatus() == AppointmentStatus::SCHEDULED &&
                           Utils::compareDates(apt.getDate(), today) >= 0;
                });

    // Sort by date and time
    std::sort(result.begin(), result.end(),
             [](const Model::Appointment& a, const Model::Appointment& b) {
                 if (a.getDate() != b.getDate()) {
                     return Utils::compareDates(a.getDate(), b.getDate()) < 0;
                 }
                 return a.getTime() < b.getTime();
             });

    return result;
}

std::vector<Model::Appointment> AppointmentRepository::getHistoryByPatient(const std::string& patientUsername) {
    std::vector<Model::Appointment> result;

    std::copy_if(m_appointments.begin(), m_appointments.end(),
                std::back_inserter(result),
                [&patientUsername](const Model::Appointment& apt) {
                    return apt.getPatientUsername() == patientUsername &&
                           (apt.getStatus() == AppointmentStatus::COMPLETED ||
                            apt.getStatus() == AppointmentStatus::CANCELLED ||
                            apt.getStatus() == AppointmentStatus::NO_SHOW);
                });

    // Sort by date descending (newest first)
    std::sort(result.begin(), result.end(),
             [](const Model::Appointment& a, const Model::Appointment& b) {
                 if (a.getDate() != b.getDate()) {
                     return Utils::compareDates(a.getDate(), b.getDate()) > 0;
                 }
                 return a.getTime() > b.getTime();
             });

    return result;
}

std::vector<Model::Appointment> AppointmentRepository::getUnpaidByPatient(const std::string& patientUsername) {
    std::vector<Model::Appointment> result;

    std::copy_if(m_appointments.begin(), m_appointments.end(),
                std::back_inserter(result),
                [&patientUsername](const Model::Appointment& apt) {
                    return apt.getPatientUsername() == patientUsername && !apt.isPaid();
                });

    return result;
}

// ==================== Doctor-Related Queries ====================
std::vector<Model::Appointment> AppointmentRepository::getByDoctor(const std::string& doctorID) {
    std::vector<Model::Appointment> result;

    std::copy_if(m_appointments.begin(), m_appointments.end(),
                std::back_inserter(result),
                [&doctorID](const Model::Appointment& apt) {
                    return apt.getDoctorID() == doctorID;
                });

    return result;
}

std::vector<Model::Appointment> AppointmentRepository::getByDoctorAndDate(const std::string& doctorID,
                                                                          const std::string& date) {
    std::vector<Model::Appointment> result;

    std::copy_if(m_appointments.begin(), m_appointments.end(),
                std::back_inserter(result),
                [&doctorID, &date](const Model::Appointment& apt) {
                    return apt.getDoctorID() == doctorID && apt.getDate() == date;
                });

    // Sort by time
    std::sort(result.begin(), result.end(),
             [](const Model::Appointment& a, const Model::Appointment& b) {
                 return a.getTime() < b.getTime();
             });

    return result;
}

std::vector<Model::Appointment> AppointmentRepository::getUpcomingByDoctor(const std::string& doctorID) {
    std::vector<Model::Appointment> result;
    std::string today = Utils::getCurrentDate();

    std::copy_if(m_appointments.begin(), m_appointments.end(),
                std::back_inserter(result),
                [&doctorID, &today](const Model::Appointment& apt) {
                    return apt.getDoctorID() == doctorID &&
                           apt.getStatus() == AppointmentStatus::SCHEDULED &&
                           Utils::compareDates(apt.getDate(), today) >= 0;
                });

    // Sort by date and time
    std::sort(result.begin(), result.end(),
             [](const Model::Appointment& a, const Model::Appointment& b) {
                 if (a.getDate() != b.getDate()) {
                     return Utils::compareDates(a.getDate(), b.getDate()) < 0;
                 }
                 return a.getTime() < b.getTime();
             });

    return result;
}

// ==================== Date-Based Queries ====================
std::vector<Model::Appointment> AppointmentRepository::getByDate(const std::string& date) {
    std::vector<Model::Appointment> result;

    std::copy_if(m_appointments.begin(), m_appointments.end(),
                std::back_inserter(result),
                [&date](const Model::Appointment& apt) {
                    return apt.getDate() == date;
                });

    // Sort by time
    std::sort(result.begin(), result.end(),
             [](const Model::Appointment& a, const Model::Appointment& b) {
                 return a.getTime() < b.getTime();
             });

    return result;
}

std::vector<Model::Appointment> AppointmentRepository::getByDateRange(const std::string& startDate,
                                                                      const std::string& endDate) {
    std::vector<Model::Appointment> result;

    std::copy_if(m_appointments.begin(), m_appointments.end(),
                std::back_inserter(result),
                [&startDate, &endDate](const Model::Appointment& apt) {
                    int cmpStart = Utils::compareDates(apt.getDate(), startDate);
                    int cmpEnd = Utils::compareDates(apt.getDate(), endDate);
                    return cmpStart >= 0 && cmpEnd <= 0;
                });

    // Sort by date and time
    std::sort(result.begin(), result.end(),
             [](const Model::Appointment& a, const Model::Appointment& b) {
                 if (a.getDate() != b.getDate()) {
                     return Utils::compareDates(a.getDate(), b.getDate()) < 0;
                 }
                 return a.getTime() < b.getTime();
             });

    return result;
}

std::vector<Model::Appointment> AppointmentRepository::getToday() {
    return getByDate(Utils::getCurrentDate());
}

// ==================== Status-Based Queries ====================
std::vector<Model::Appointment> AppointmentRepository::getByStatus(AppointmentStatus status) {
    std::vector<Model::Appointment> result;

    std::copy_if(m_appointments.begin(), m_appointments.end(),
                std::back_inserter(result),
                [status](const Model::Appointment& apt) {
                    return apt.getStatus() == status;
                });

    return result;
}

std::vector<Model::Appointment> AppointmentRepository::getScheduled() {
    return getByStatus(AppointmentStatus::SCHEDULED);
}

std::vector<Model::Appointment> AppointmentRepository::getCompleted() {
    return getByStatus(AppointmentStatus::COMPLETED);
}

std::vector<Model::Appointment> AppointmentRepository::getCancelled() {
    return getByStatus(AppointmentStatus::CANCELLED);
}

// ==================== Slot Availability ====================
bool AppointmentRepository::isSlotAvailable(const std::string& doctorID,
                                           const std::string& date,
                                           const std::string& time) {
    return std::none_of(m_appointments.begin(), m_appointments.end(),
                       [&doctorID, &date, &time](const Model::Appointment& apt) {
                           return apt.getDoctorID() == doctorID &&
                                  apt.getDate() == date &&
                                  apt.getTime() == time &&
                                  apt.getStatus() == AppointmentStatus::SCHEDULED;
                       });
}

std::vector<std::string> AppointmentRepository::getBookedSlots(const std::string& doctorID,
                                                               const std::string& date) {
    std::vector<std::string> slots;

    for (const auto& apt : m_appointments) {
        if (apt.getDoctorID() == doctorID &&
            apt.getDate() == date &&
            apt.getStatus() == AppointmentStatus::SCHEDULED) {
            slots.push_back(apt.getTime());
        }
    }

    // Sort slots by time
    std::sort(slots.begin(), slots.end());

    // Remove duplicates
    slots.erase(std::unique(slots.begin(), slots.end()), slots.end());

    return slots;
}

// ==================== ID Generation ====================
std::string AppointmentRepository::getNextId() {
    return Utils::generateAppointmentID();
}

// ==================== File Path ====================
void AppointmentRepository::setFilePath(const std::string& filePath) {
    m_filePath = filePath;
}

std::string AppointmentRepository::getFilePath() const {
    return m_filePath;
}

} // namespace DAL
} // namespace HMS
