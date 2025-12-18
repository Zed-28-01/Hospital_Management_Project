#include "model/Doctor.h"
#include "common/Types.h"
#include "common/Utils.h"
#include <iostream>
#include <sstream>
#include <optional>

namespace HMS {
    namespace Model {
// ============================== CONSTRUCTOR ====================

        Doctor::Doctor(const std::string& doctorID, const std::string& username,
           const std::string& name, const std::string& phone,
           Gender gender, const std::string& dateOfBirth,
           const std::string& specialization, const std::string& schedule,
           double consultationFee) : Person (name, phone, gender, dateOfBirth), m_doctorID(doctorID),
           m_username(username), m_specialization(specialization),
           m_schedule(schedule), m_consultationFee(consultationFee) {
           }
// =============================== GETTERS ===========================

        std::string Doctor::getID() const {
            return m_doctorID;
        }
        std::string Doctor::getDoctorID() const {
            return m_doctorID;
        }
        std::string Doctor::getUsername() const {
            return m_username;
        }
        std::string Doctor::getSpecialization() const {
            return m_specialization;
        }
        std::string Doctor::getSchedule() const {
            return m_schedule;
        }
        double Doctor::getConsultationFee() const {
            return m_consultationFee;
        }
// ====================================== SETTERS =====================================
        void Doctor::setSpecialization(const std::string& specialization) {
            m_specialization = specialization;
        }
        void Doctor::setSchedule(const std::string& schedule) {
            m_schedule = schedule;
        }
        void Doctor::setConsultationFee(double fee) {
            m_consultationFee = fee;
        }
        void Doctor::displayInfo() const {
            std::cout <<"THONG TIN BAC SI\n";
            std::cout <<"  - Ten: " << m_name << "\n";
            std::cout <<"  - So dien thoai: " << m_phone << "\n";
            std::cout <<"  - Gioi tinh: " <<  genderToString(m_gender) << "\n";
            std::cout <<"  - Ngay sinh: " << m_dateOfBirth << "\n";
            std::cout <<"  - Chuyen khoa: " << m_specialization << "\n";
            std::cout <<"  - Lich lam viec: " << m_schedule << "\n";
            std::cout <<"  - Phi kham benh: " << Utils::formatMoney(m_consultationFee) << "\n";
        }
        std::string Doctor::serialize() const {
            std::stringstream builder;
            builder << m_doctorID << "|" << m_username <<"|"
                    << m_name << "|" << m_phone <<"|"
                    << genderToString(m_gender) << "|"
                    << m_dateOfBirth << "|" << m_specialization <<"|"
                    << m_schedule << "|" << m_consultationFee;
            return builder.str();
        }
        Result<Doctor> Doctor::deserialize(const std::string& line) {
            if (line.empty()) {
                return std::nullopt;
            }
            std::vector<std::string> parts = Utils::split(line, '|');

            if (parts.size() != 9) {
                return std::nullopt;
            }
            Doctor doc;

            doc.m_doctorID = parts[0];
            doc.m_username = parts[1];
            doc.m_name = parts[2];
            doc.m_phone = parts[3];
            doc.m_gender = stringToGender(parts[4]);
            doc.m_dateOfBirth = parts[5];
            doc.m_specialization = parts[6];
            doc.m_schedule = parts[7];
            doc.m_consultationFee = std::stod(parts[8]);

            return doc; // Trả về thành công (Hộp có quà)
        }
    }
}