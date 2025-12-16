// src/model/Appointment.cpp
#include "model/Appointment.h"
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cctype>

namespace HMS
{
    namespace Model
    {

        // ---------------- Constructors ----------------
        Appointment::Appointment(const std::string &appointmentID,
                                 const std::string &patientUsername,
                                 const std::string &doctorID,
                                 const std::string &date,
                                 const std::string &time,
                                 const std::string &disease,
                                 double price)
            : m_appointmentID(appointmentID),
              m_patientUsername(patientUsername),
              m_doctorID(doctorID),
              m_appointmentDate(date),
              m_appointmentTime(time),
              m_disease(disease),
              m_price(price),
              m_isPaid(false),
              m_status(AppointmentStatus::SCHEDULED),
              m_notes("")
        {
        }

        Appointment::Appointment(const std::string &appointmentID,
                                 const std::string &patientUsername,
                                 const std::string &doctorID,
                                 const std::string &date,
                                 const std::string &time,
                                 const std::string &disease,
                                 double price,
                                 bool isPaid,
                                 AppointmentStatus status,
                                 const std::string &notes)
            : m_appointmentID(appointmentID),
              m_patientUsername(patientUsername),
              m_doctorID(doctorID),
              m_appointmentDate(date),
              m_appointmentTime(time),
              m_disease(disease),
              m_price(price),
              m_isPaid(isPaid),
              m_status(status),
              m_notes(notes)
        {
        }

        // ---------------- Getters ----------------
        std::string Appointment::getAppointmentID() const { return m_appointmentID; }
        std::string Appointment::getPatientUsername() const { return m_patientUsername; }
        std::string Appointment::getDoctorID() const { return m_doctorID; }
        std::string Appointment::getDate() const { return m_appointmentDate; }
        std::string Appointment::getTime() const { return m_appointmentTime; }
        std::string Appointment::getDateTime() const
        {
            return m_appointmentDate + " " + m_appointmentTime;
        }
        std::string Appointment::getDisease() const { return m_disease; }
        double Appointment::getPrice() const { return m_price; }
        bool Appointment::isPaid() const { return m_isPaid; }
        AppointmentStatus Appointment::getStatus() const { return m_status; }

        std::string Appointment::getStatusString() const
        {
            std::string s = HMS::statusToString(m_status);
            // Convert "no_show" -> "No show" and capitalize first letter
            std::replace(s.begin(), s.end(), '_', ' ');
            if (!s.empty())
                s[0] = std::toupper(static_cast<unsigned char>(s[0]));
            return s;
        }

        std::string Appointment::getNotes() const { return m_notes; }

        // ---------------- Setters ----------------
        void Appointment::setDate(const std::string &date) { m_appointmentDate = date; }
        void Appointment::setTime(const std::string &time) { m_appointmentTime = time; }
        void Appointment::setDisease(const std::string &disease) { m_disease = disease; }
        void Appointment::setPrice(double price) { m_price = price; }
        void Appointment::setPaid(bool paid) { m_isPaid = paid; }
        void Appointment::setStatus(AppointmentStatus status) { m_status = status; }
        void Appointment::setNotes(const std::string &notes) { m_notes = notes; }

        // ---------------- Status methods ----------------
        void Appointment::markAsCompleted() { m_status = AppointmentStatus::COMPLETED; }
        void Appointment::markAsCancelled() { m_status = AppointmentStatus::CANCELLED; }
        void Appointment::markAsNoShow() { m_status = AppointmentStatus::NO_SHOW; }

        // Helper: parse "YYYY-MM-DD HH:MM" to time_point; returns optional time_point
        static bool parseDateTime(const std::string &date, const std::string &time, std::chrono::system_clock::time_point &out)
        {
            std::istringstream ss(date + " " + time);
            std::tm tm = {};
            ss >> std::get_time(&tm, "%Y-%m-%d %H:%M");
            if (ss.fail())
                return false;
            std::time_t tt = std::mktime(&tm);
            if (tt == -1)
                return false;
            out = std::chrono::system_clock::from_time_t(tt);
            return true;
        }

        bool Appointment::isUpcoming() const
        {
            std::chrono::system_clock::time_point apptTime;
            if (!parseDateTime(m_appointmentDate, m_appointmentTime, apptTime))
                return false;
            return apptTime > std::chrono::system_clock::now();
        }

        bool Appointment::canCancel() const
        {
            // Allow cancel if scheduled and more than 1 hour before appointment (example rule)
            if (m_status != AppointmentStatus::SCHEDULED)
                return false;
            std::chrono::system_clock::time_point apptTime;
            if (!parseDateTime(m_appointmentDate, m_appointmentTime, apptTime))
                return false;
            auto diff = std::chrono::duration_cast<std::chrono::minutes>(apptTime - std::chrono::system_clock::now());
            return diff.count() > 60; // more than 60 minutes
        }

        bool Appointment::canEdit() const
        {
            // Allow edit if scheduled (example rule)
            return (m_status == AppointmentStatus::SCHEDULED);
        }

        // ---------------- Serialization ----------------
        void Appointment::displayInfo() const
        {
            // Simple console printing; adapt to your project's logging
            std::cout << "AppointmentID: " << m_appointmentID << "\n"
                      << "Patient: " << m_patientUsername << "\n"
                      << "Doctor: " << m_doctorID << "\n"
                      << "DateTime: " << getDateTime() << "\n"
                      << "Disease: " << m_disease << "\n"
                      << "Price: " << m_price << "\n"
                      << "Paid: " << (m_isPaid ? "Yes" : "No") << "\n"
                      << "Status: " << getStatusString() << "\n"
                      << "Notes: " << m_notes << "\n";
        }

        std::string Appointment::serialize() const
        {
            // We use pipe '|' as delimiter (as header documented). Note: escape pipes in data if necessary.
            std::ostringstream ss;
            ss << m_appointmentID << '|'
               << m_patientUsername << '|'
               << m_doctorID << '|'
               << m_appointmentDate << '|'
               << m_appointmentTime << '|'
               << m_disease << '|'
               << m_price << '|'
               << (m_isPaid ? "1" : "0") << '|'
               << static_cast<int>(m_status) << '|' // store enum as int
               << m_notes;
            return ss.str();
        }

        Result<Appointment> Appointment::deserialize(const std::string &line)
        {
            // Returns std::nullopt on failure, or an Appointment on success.
            std::vector<std::string> parts;
            std::string cur;
            std::istringstream ss(line);
            while (std::getline(ss, cur, '|'))
                parts.push_back(cur);

            // Expect at least 10 fields
            if (parts.size() < 10)
            {
                return std::nullopt;
            }

            try
            {
                Appointment appt(
                    parts[0],           // id
                    parts[1],           // patient
                    parts[2],           // doctor
                    parts[3],           // date
                    parts[4],           // time
                    parts[5],           // disease
                    std::stod(parts[6]) // price
                );

                appt.m_isPaid = (parts[7] == "1");
                int statusInt = std::stoi(parts[8]);
                appt.m_status = static_cast<AppointmentStatus>(statusInt);
                appt.m_notes = parts[9];

                return appt;
            }
            catch (const std::exception & /*ex*/)
            {
                return std::nullopt;
            }
        }

    } // namespace Model
} // namespace HMS
