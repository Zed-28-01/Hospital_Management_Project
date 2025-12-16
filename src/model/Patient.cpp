#include "model/Patient.h"

#include <sstream>
#include <format>
#include <iostream>
#include <vector>

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
    std::istringstream iss(line);
    std::string token;
    std::vector<std::string> fields;

    // Split by pipe delimiter
    while (std::getline(iss, token, '|')) {
        fields.push_back(token);
    }

    // Handle trailing empty field (e.g., line ends with "|")
    // getline won't capture empty string after last delimiter
    if (!line.empty() && line.back() == '|') {
        fields.push_back("");
    }

    // Validate field count
    if (fields.size() != 8) {
        std::cerr << std::format("Invalid patient data format: expected 8 fields, got {}\n",
                                fields.size());
        return std::nullopt;
    }

    // Parse gender
    Gender gender = stringToGender(fields[4]);
    if (gender == Gender::UNKNOWN) {
        std::cerr << std::format("Invalid gender value: {}\n", fields[4]);
        return std::nullopt;
    }

    // Create Patient object
    Patient patient(
        fields[0],  // patientID
        fields[1],  // username
        fields[2],  // name
        fields[3],  // phone
        gender,     // gender
        fields[5],  // dateOfBirth
        fields[6],  // address
        fields[7]   // medicalHistory
    );

    return patient;
}