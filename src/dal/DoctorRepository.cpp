#include "dal/DoctorRepository.h"
#include "common/Utils.h"
#include "common/Constants.h"

#include <fstream>
#include <iostream>
#include <algorithm>
#include <set>
#include <format>
#include <filesystem>

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
    // Atomic write using temporary file to prevent data corruption
    std::string tempPath = m_filePath + ".tmp";

    std::ofstream file(tempPath, std::ios::trunc);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << tempPath << "\n";
        return false;
    }

    // Write header
    file << "# Format: doctorID|username|name|phone|gender|dateOfBirth|specialization|schedule|consultationFee\n";

    // Write data
    for (const auto& doctor : m_doctors) {
        file << doctor.serialize() << "\n";

        if (file.fail()) {
            std::cerr << "Failed to write doctor data\n";
            file.close();
            std::filesystem::remove(tempPath);  // Clean up temp file
            return false;
        }
    }

    file.flush();
    if (file.fail()) {
        file.close();
        std::filesystem::remove(tempPath);
        return false;
    }
    file.close();

    try {
        std::filesystem::rename(tempPath, m_filePath);
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Failed to rename temp file: " << e.what() << "\n";
        std::filesystem::remove(tempPath);
        return false;
    }

    return true;
}

bool DoctorRepository::load() {
    std::ifstream file(m_filePath);
    if (!file.is_open()) {
        m_doctors.clear();
        m_isLoaded = true;
        return false;
    }

    m_doctors.clear();
    std::string line;

    // Read and deserialize each line
    while (std::getline(file, line)) {
        // Skip empty lines and comments (including header)
        if (line.empty() || line[0] == '#') {
            continue;
        }

        auto result = Model::Doctor::deserialize(line);
        if (result.has_value()) {
            m_doctors.push_back(result.value());
        } else {
            std::cerr << "Failed to deserialize doctor from line: " << line << "\n";
        }
    }

    file.close();
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

    std::string lowerSpec = specialization;
    std::transform(
        lowerSpec.begin(),
        lowerSpec.end(),
        lowerSpec.begin(),
        ::tolower
    );

    for (const auto& doctor : m_doctors) {
        std::string docSpec = doctor.getSpecialization();
        std::transform(
            docSpec.begin(),
            docSpec.end(),
            docSpec.begin(),
            ::tolower
        );

        if (docSpec.find(lowerSpec) != std::string::npos) {
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

    std::string lowerName = name;
    std::transform(
        lowerName.begin(),
        lowerName.end(),
        lowerName.begin(),
        ::tolower
    );

    for (const auto& doctor : m_doctors) {
        std::string doctorName = doctor.getName();
        std::transform(
            doctorName.begin(),
            doctorName.end(),
            doctorName.begin(),
            ::tolower
        );

        if (doctorName.find(lowerName) != std::string::npos) {
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

    std::string lowerKeyword = keyword;
    std::transform(
        lowerKeyword.begin(),
        lowerKeyword.end(),
        lowerKeyword.begin(),
        ::tolower
    );

    for (const auto& doctor : m_doctors) {
        std::string name = doctor.getName();
        std::string spec = doctor.getSpecialization();
        std::string doctorID = doctor.getDoctorID();

        std::transform(
            name.begin(),
            name.end(),
            name.begin(),
            ::tolower
        );

        std::transform(
            spec.begin(),
            spec.end(),
            spec.begin(),
            ::tolower
        );

        std::transform(
            doctorID.begin(),
            doctorID.end(),
            doctorID.begin(),
            ::tolower
        );

        if (name.find(lowerKeyword) != std::string::npos
            || spec.find(lowerKeyword) != std::string::npos
            || doctorID.find(lowerKeyword) != std::string::npos
        ) {
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
        std::string spec = doctor.getSpecialization();
        // Simple trim - remove leading/trailing spaces
        size_t start = spec.find_first_not_of(" \t\n\r");
        size_t end = spec.find_last_not_of(" \t\n\r");

        if (start != std::string::npos && end != std::string::npos) {
            spec = spec.substr(start, end - start + 1);
            if (!spec.empty()) {
                specializations.insert(spec);
            }
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
            // Check if all characters are digits
            bool allDigits = std::all_of(numPart.begin(), numPart.end(), ::isdigit);

            if (allDigits && !numPart.empty()) {
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
