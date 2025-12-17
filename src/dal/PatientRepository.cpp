#include "dal/PatientRepository.h"
#include "common/Utils.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <format>
#include <cctype>
#include <iomanip>
#include <filesystem>

namespace HMS {
    namespace DAL {
        std::unique_ptr<PatientRepository> PatientRepository::s_instance = nullptr;
        std::mutex PatientRepository::s_mutex;

        // ==================== Private Constructor ====================
        PatientRepository::PatientRepository()
            :   m_filePath("data/Patient.txt"),
                m_isLoaded(false) {}

        // ==================== Singleton Access ====================
        PatientRepository* PatientRepository::getInstance() {
            std::lock_guard<std::mutex> lock(s_mutex);
            if (!s_instance) {
                s_instance = std::unique_ptr<PatientRepository>(new PatientRepository());
            }
            return s_instance.get();
        }

        void PatientRepository::resetInstance() {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_instance.reset();
        }

        PatientRepository::~PatientRepository() = default;

        // ==================== CRUD Operations ====================
        std::vector<Model::Patient> PatientRepository::getAll() {
            if (!m_isLoaded) {
                load();
            }

            return m_patients;
        }

        std::optional<Model::Patient> PatientRepository::getById(const std::string& id) {
            if (!m_isLoaded) {
                load();
            }

            auto it = std::find_if(m_patients.begin(), m_patients.end(),
                [&id](const Model::Patient& p) {
                    return p.getPatientID() == id;
                });

            if (it != m_patients.end()) {
                return *it;
            }
            return std::nullopt;
        }

        bool PatientRepository::add(const Model::Patient& patient) {
            if (!m_isLoaded) {
                load();
            }

            // Check if patient ID already exists
            if (exists(patient.getPatientID())) {
                return false;
            }

            // Check if username already exists
            if (getByUsername(patient.getUsername()).has_value()) {
                return false;
            }

            m_patients.push_back(patient);
            return save();
        }

        bool PatientRepository::update(const Model::Patient& patient) {
            if (!m_isLoaded) {
                load();
            }

            auto it = std::find_if(m_patients.begin(), m_patients.end(),
                [&patient](const Model::Patient& p) {
                    return p.getPatientID() == patient.getPatientID();
                });

            if (it != m_patients.end()) {
                *it = patient;
                return save();
            }
            return false;
        }

        bool PatientRepository::remove(const std::string& id) {
            if (!m_isLoaded) {
                load();
            }

            auto it = std::find_if(
                m_patients.begin(),
                m_patients.end(),
                [&id](const Model::Patient& p) {
                    return p.getPatientID() == id;
                });

            if (it == m_patients.end()) {
                return false;  // Not found
            }

            m_patients.erase(it);
            return save();
        }

        // ==================== Persistence ====================
        bool PatientRepository::save() {
            // Atomic write using temporary file to prevent data corruption
            std::string tempPath = m_filePath + ".tmp";

            std::ofstream file(tempPath, std::ios::trunc);
            if (!file.is_open()) {
                std::cerr << "Failed to open file for writing: " << tempPath << "\n";
                return false;
            }

            // Write header
            file << "# Format: patientID|username|name|phone|gender|dateOfBirth|address|medicalHistory\n";

            // Write data
            for (const auto& patient : m_patients) {
                file << patient.serialize() << "\n";

                if (file.fail()) {
                    std::cerr << "Failed to write patient data\n";
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

        bool PatientRepository::load() {
            std::ifstream file(m_filePath);
            if (!file.is_open()) {
                m_patients.clear();
                m_isLoaded = true;
                return false;
            }

            m_patients.clear();
            std::string line;

            // Read and deserialize each line
            while (std::getline(file, line)) {
                // Skip empty lines and comments (including header)
                if (line.empty() || line[0] == '#') {
                    continue;
                }

                auto result = Model::Patient::deserialize(line);
                if (result.has_value()) {
                    m_patients.push_back(result.value());
                } else {
                    std::cerr << "Failed to deserialize patient from line: " << line << "\n";
                }
            }

            file.close();
            m_isLoaded = true;

            return true;
        }

        // ==================== Query Operations ====================
        size_t PatientRepository::count() const {
            if (!m_isLoaded) {
                const_cast<PatientRepository*>(this)->load();
            }

            return m_patients.size();
        }

        bool PatientRepository::exists(const std::string& id) const {
            if (!m_isLoaded) {
                const_cast<PatientRepository*>(this)->load();
            }

            return std::any_of(m_patients.begin(), m_patients.end(),
                [&id](const Model::Patient& p) {
                    return p.getPatientID() == id;
                });
        }

        bool PatientRepository::clear() {
            m_patients.clear();
            m_isLoaded = true;
            return true;
        }

        // ==================== Patient-Specific Queries ====================
        std::optional<Model::Patient> PatientRepository::getByUsername(const std::string& username) {
            if (!m_isLoaded) {
                load();
            }

            auto it = std::find_if(m_patients.begin(), m_patients.end(),
                [&username](const Model::Patient& p) {
                    return p.getUsername() == username;
                });

            if (it != m_patients.end()) {
                return *it;
            }
            return std::nullopt;
        }

        std::vector<Model::Patient> PatientRepository::searchByName(const std::string& name) {
            if (!m_isLoaded) {
                load();
            }

            std::vector<Model::Patient> results;

            std::string lowerName = name;
            std::transform(
                lowerName.begin(),
                lowerName.end(),
                lowerName.begin(),
                ::tolower
            );

            for (const auto& p : m_patients) {
                std::string patientName = p.getName();
                std::transform(
                    patientName.begin(),
                    patientName.end(),
                    patientName.begin(),
                    ::tolower
                );

                if (patientName.find(lowerName) != std::string::npos) {
                    results.push_back(p);
                }
            }

            return results;
        }

        std::vector<Model::Patient> PatientRepository::searchByPhone(const std::string& phone) {
            if (!m_isLoaded) {
                load();
            }

            std::vector<Model::Patient> results;

            for (const auto& p : m_patients) {
                if (p.getPhone().find(phone) != std::string::npos) {
                    results.push_back(p);
                }
            }

            return results;
        }

        std::vector<Model::Patient> PatientRepository::search(const std::string& keyword) {
            if (!m_isLoaded) {
                load();
            }

            std::vector<Model::Patient> results;

            std::string lowerKeyword = keyword;
            std::transform(
                lowerKeyword.begin(),
                lowerKeyword.end(),
                lowerKeyword.begin(),
                ::tolower
            );

            for (const auto& p : m_patients) {
                std::string name = p.getName();
                std::string phone = p.getPhone();
                std::string address = p.getAddress();

                std::transform(
                    name.begin(),
                    name.end(),
                    name.begin(),
                    ::tolower
                );

                std::transform(
                    address.begin(),
                    address.end(),
                    address.begin(),
                    ::tolower
                );

                if (name.find(lowerKeyword) != std::string::npos
                    || phone.find(lowerKeyword) != std::string::npos
                    || address.find(lowerKeyword) != std::string::npos
                ) {
                    results.push_back(p);
                }
            }

            return results;
        }

        std::string PatientRepository::getNextId() {
            if (!m_isLoaded) {
                load();
            }

            int maxID = 0;
            for (const auto& patient : m_patients) {
                const std::string& patientID = patient.getPatientID();

                // Only process valid format: P + digits
                if (patientID.length() > 1 && patientID[0] == 'P') {
                    std::string numPart = patientID.substr(1);
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

            return std::format("P{:03d}", maxID + 1);
        }

        // ==================== File Path ====================
        void PatientRepository::setFilePath(const std::string& filePath) {
            m_filePath = filePath;
            m_isLoaded = false;
        }

        std::string PatientRepository::getFilePath() const {
            return m_filePath;
        }
    } // namespace DAL
} // namespace HMS