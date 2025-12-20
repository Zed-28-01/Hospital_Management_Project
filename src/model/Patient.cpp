#include "model/Patient.h"
#include "common/Utils.h"
#include "common/Constants.h"

#include <format>
#include <iostream>

// ==================== Constructor ====================
HMS::Model::Patient::Patient(const std::string& patientID,
            const std::string& username,
            const std::string& name,
            const std::string& phone,
            Gender gender,
            const std::string& dateOfBirth,
            const std::string& address,
            const std::string& medicalHistory)
    : Person(name, phone, gender, dateOfBirth),
      m_patientID(patientID),
      m_username(username),
      m_address(address),
      m_medicalHistory(medicalHistory) {}

// ==================== Getters ====================
std::string HMS::Model::Patient::getID() const {
    return m_patientID;
}

std::string HMS::Model::Patient::getPatientID() const {
    return m_patientID;
}

std::string HMS::Model::Patient::getUsername() const {
    return m_username;
}

std::string HMS::Model::Patient::getAddress() const {
    return m_address;
}

std::string HMS::Model::Patient::getMedicalHistory() const {
    return m_medicalHistory;
}

// ==================== Setters ====================

void HMS::Model::Patient::setAddress(const std::string& address) {
    m_address = address;
}

void HMS::Model::Patient::setMedicalHistory(const std::string& medicalHistory) {
    m_medicalHistory = medicalHistory;
}

void HMS::Model::Patient::appendMedicalHistory(const std::string& entry) {
    if (!m_medicalHistory.empty()) {
        m_medicalHistory += "; ";
    }

    m_medicalHistory += entry;
}

// ==================== Override Methods ====================
void HMS::Model::Patient::displayInfo() const {
    std::cout << "\n========================================\n";
    std::cout << "        PATIENT INFORMATION          \n";
    std::cout << "========================================\n";
    std::cout << std::format("{:<18}: {}\n", "Patient ID", m_patientID);
    std::cout << std::format("{:<18}: {}\n", "Username", m_username);
    std::cout << std::format("{:<18}: {}\n", "Name", m_name);
    std::cout << std::format("{:<18}: {}\n", "Phone", m_phone);
    std::cout << std::format("{:<18}: {}\n", "Gender", genderToString(m_gender));
    std::cout << std::format("{:<18}: {}\n", "Date of Birth", m_dateOfBirth);
    std::cout << std::format("{:<18}: {}\n", "Address", m_address);
    std::cout << std::format("{:<18}: {}\n", "Medical History", m_medicalHistory);
    std::cout << "========================================\n\n";
}

std::string HMS::Model::Patient::serialize() const {
    return std::format(
        "{}|{}|{}|{}|{}|{}|{}|{}",
        m_patientID,
        m_username,
        m_name,
        m_phone,
        genderToString(m_gender),
        m_dateOfBirth,
        m_address,
        m_medicalHistory
    );
}

// ==================== Static Factory Method ====================

HMS::Result<HMS::Model::Patient> HMS::Model::Patient::deserialize(const std::string& line) {
    // Skip empty lines and comments
    if (line.empty() || line[0] == Constants::COMMENT_CHAR) {
        return std::nullopt;
    }

    // Split by delimiter
    auto fields = Utils::split(line, Constants::FIELD_DELIMITER);

    // Validate field count
    if (fields.size() != 8) {
        std::cerr << std::format("Error: Invalid patient format. Expected 8 fields, got {}\n",
                                fields.size());
        return std::nullopt;
    }

    // Extract and trim fields
    std::string patientID = Utils::trim(fields[0]);
    std::string username = Utils::trim(fields[1]);
    std::string name = Utils::trim(fields[2]);
    std::string phone = Utils::trim(fields[3]);
    std::string genderStr = Utils::trim(fields[4]);
    std::string dateOfBirth = Utils::trim(fields[5]);
    std::string address = Utils::trim(fields[6]);
    std::string medicalHistory = Utils::trim(fields[7]);

    // Validate required fields are not empty
    if (patientID.empty() || username.empty() || name.empty()) {
        std::cerr << "Error: Patient record has empty required fields\n";
        return std::nullopt;
    }

    // Validate username format
    if (!Utils::isValidUsername(username)) {
        std::cerr << std::format("Error: Invalid username '{}' for patient {}\n",
                                username, patientID);
        return std::nullopt;
    }

    // Parse gender
    Gender gender = stringToGender(genderStr);
    if (gender == Gender::UNKNOWN) {
        std::cerr << std::format("Error: Invalid gender '{}' for patient {}\n",
                                genderStr, patientID);
        return std::nullopt;
    }

    return Patient(patientID, username, name, phone, gender,
                   dateOfBirth, address, medicalHistory);
}