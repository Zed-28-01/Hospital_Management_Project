#include "dal/DoctorRepository.h"
#include "dal/FileHelper.h"
#include "common/Utils.h"
#include "common/Constants.h"

#include <iostream>
#include <algorithm>
#include <set>
#include <format>

namespace HMS {
namespace DAL {

// ==================== Static Members ====================
std::unique_ptr<DoctorRepository> DoctorRepository::s_instance = nullptr;
std::mutex DoctorRepository::s_mutex;

// ==================== Private Constructor ====================
DoctorRepository::DoctorRepository()
    : m_filePath(Constants::DOCTOR_FILE),
    m_isLoaded(false) {
}

// ==================== Singleton Access ====================
DoctorRepository* DoctorRepository::getInstance() {
    std::lock_guard<std::mutex> lock(s_mutex);

    if (!s_instance) {
        s_instance = std::unique_ptr<DoctorRepository>(new DoctorRepository());
    }

    return s_instance.get();
}

void DoctorRepository::resetInstance() {
    std::lock_guard<std::mutex> lock(s_mutex);
    s_instance.reset();
}

// ==================== Destructor ====================
DoctorRepository::~DoctorRepository() = default;

// ==================== CRUD Operations ====================
std::vector<Model::Doctor> DoctorRepository::getAll() {
    if (!m_isLoaded) {
        load();
    }

    return m_doctors;
}

std::optional<Model::Doctor> DoctorRepository::getById(const std::string& id) {
    if (!m_isLoaded) {
        load();
    }

    auto it = std::find_if(m_doctors.begin(), m_doctors.end(),
        [&id](const Model::Doctor& doctor) {
            return doctor.getDoctorID() == id;
        });

    if (it != m_doctors.end()) {
        return *it;
    }

    return std::nullopt;
}

bool DoctorRepository::add(const Model::Doctor& doctor) {
    if (!m_isLoaded) {
        load();
    }

    // Check if doctor ID already exists
    if (exists(doctor.getDoctorID())) {
        return false;
    }

    // Check if username already exists
    if (getByUsername(doctor.getUsername()).has_value()) {
        return false;
    }

    m_doctors.push_back(doctor);
    return save();
}

bool DoctorRepository::update(const Model::Doctor& doctor) {
    if (!m_isLoaded) {
        load();
    }

    // find doctor to update
    auto it = std::find_if(m_doctors.begin(), m_doctors.end(),
        [&doctor](const Model::Doctor& d) {
            return d.getDoctorID() == doctor.getDoctorID();
        });

    if (it == m_doctors.end()) {
        return false;  // Doctor not found
    }

    // Check if the new username is duplicated with another doctor
    auto duplicateUsername = std::find_if(m_doctors.begin(), m_doctors.end(),
        [&doctor](const Model::Doctor& d) {
            return d.getUsername() == doctor.getUsername()
                   && d.getDoctorID() != doctor.getDoctorID();
        });

    if (duplicateUsername != m_doctors.end()) {
        return false;  // Username already taken by another doctor
    }

    *it = doctor;
    return save();
}

bool DoctorRepository::remove(const std::string& id) {
    if (!m_isLoaded) {
        load();
    }

    auto it = std::find_if(m_doctors.begin(), m_doctors.end(),
        [&id](const Model::Doctor& doctor) {
            return doctor.getDoctorID() == id;
        });

    if (it == m_doctors.end()) {
        return false;  // Not found
    }

    m_doctors.erase(it);
    return save();
}

// ==================== Persistence ====================
bool DoctorRepository::save() {
    if (FileHelper::fileExists(m_filePath)) {
        if (!FileHelper::createBackup(m_filePath)) {
            std::cerr << "Warning: Failed to create backup for: " << m_filePath << "\n";
            // Continue anyway - backup failure không nên block save
        }
    }

    // Prepare lines to write
    std::vector<std::string> lines;
    lines.push_back(FileHelper::getFileHeader("Doctor"));

    // Write data
    for (const auto& doctor : m_doctors) {
        lines.push_back(doctor.serialize());
    }

    if (!FileHelper::writeLines(m_filePath, lines)) {
        std::cerr << "Failed to write doctor data to file: " << m_filePath << "\n";
        return false;
    }

    return true;
}

bool DoctorRepository::load() {
    if (!FileHelper::fileExists(m_filePath)) {
        m_doctors.clear();
        m_isLoaded = true;
        return false;
    }

    auto lines = FileHelper::readLines(m_filePath);

    m_doctors.clear();

    for (const auto& line : lines) {
        auto result = Model::Doctor::deserialize(line);

        if (result.has_value()) {
            m_doctors.push_back(result.value());
        } else {
            std::cerr << "Failed to deserialize doctor from line: " << line << "\n";
        }
    }

    m_isLoaded = true;
    return true;
}

// ==================== Query Operations ====================

size_t DoctorRepository::count() const {
    if (!m_isLoaded) {
        const_cast<DoctorRepository*>(this)->load();
    }

    return m_doctors.size();
}

bool DoctorRepository::exists(const std::string& id) const {
    if (!m_isLoaded) {
        const_cast<DoctorRepository*>(this)->load();
    }

    return std::any_of(m_doctors.begin(), m_doctors.end(),
        [&id](const Model::Doctor& doctor) {
            return doctor.getDoctorID() == id;
        });
}

bool DoctorRepository::clear() {
    m_doctors.clear();
    m_isLoaded = true;
    return save();
}

// ==================== Doctor-Specific Queries ====================
std::optional<Model::Doctor> DoctorRepository::getByUsername(const std::string& username) {
    if (!m_isLoaded) {
        load();
    }

    auto it = std::find_if(m_doctors.begin(), m_doctors.end(),
        [&username](const Model::Doctor& doctor) {
            return doctor.getUsername() == username;
        });

    if (it != m_doctors.end()) {
        return *it;
    }

    return std::nullopt;
}

std::vector<Model::Doctor> DoctorRepository::getBySpecialization(const std::string& specialization) {
    if (!m_isLoaded) {
        load();
    }

    std::vector<Model::Doctor> results;

    for (const auto& doctor : m_doctors) {
        if (Utils::containsIgnoreCase(doctor.getSpecialization(), specialization)) {
            results.push_back(doctor);
        }
    }

    return results;
}

std::vector<Model::Doctor> DoctorRepository::searchByName(const std::string& name) {
    if (!m_isLoaded) {
        load();
    }

    std::vector<Model::Doctor> results;

    for (const auto& doctor : m_doctors) {
        if (Utils::containsIgnoreCase(doctor.getName(), name)) {
            results.push_back(doctor);
        }
    }

    return results;
}

std::vector<Model::Doctor> DoctorRepository::search(const std::string& keyword) {
    if (!m_isLoaded) {
        load();
    }

    std::vector<Model::Doctor> results;

    for (const auto& doctor : m_doctors) {
        if (Utils::containsIgnoreCase(doctor.getName(), keyword) ||
            Utils::containsIgnoreCase(doctor.getSpecialization(), keyword) ||
            Utils::containsIgnoreCase(doctor.getDoctorID(), keyword)) {
            results.push_back(doctor);
        }
    }

    return results;
}

std::vector<std::string> DoctorRepository::getAllSpecializations() {
    if (!m_isLoaded) {
        load();
    }

    std::set<std::string> specializations;

    for (const auto& doctor : m_doctors) {
        std::string spec = Utils::trim(doctor.getSpecialization());
        if (!spec.empty()) {
            specializations.insert(spec);
        }
    }

    return std::vector<std::string>(specializations.begin(), specializations.end());
}

std::string DoctorRepository::getNextId() {
    if (!m_isLoaded) {
        load();
    }

    int maxID = 0;
    for (const auto& doctor : m_doctors) {
        const std::string& doctorID = doctor.getDoctorID();

        // Only process valid format: D + digits
        if (doctorID.length() > 1 && doctorID[0] == 'D') {
            std::string numPart = doctorID.substr(1);

            if (Utils::isNumeric(numPart)) {
                try {
                    int idNum = std::stoi(numPart);
                    maxID = std::max(maxID, idNum);
                } catch (const std::exception&) {
                    // Ignore parse errors
                }
            }
        }
    }

    return std::format("D{:03d}", maxID + 1);
}

// ==================== File Path ====================
void DoctorRepository::setFilePath(const std::string& filePath) {
    m_filePath = filePath;
    m_isLoaded = false;
}

std::string DoctorRepository::getFilePath() const {
    return m_filePath;
}

} // namespace DAL
} // namespace HMS
