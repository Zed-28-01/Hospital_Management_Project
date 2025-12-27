#include "advance/Prescription.h"
#include "common/Constants.h"
#include "common/Utils.h"

#include <algorithm>
#include <format>
#include <iostream>
#include <sstream>

namespace HMS
{
    namespace Model
    {

        // ==================== Constructor ====================
        Prescription::Prescription(const std::string &prescriptionID,
                                   const std::string &appointmentID,
                                   const std::string &patientUsername,
                                   const std::string &doctorID,
                                   const std::string &prescriptionDate)
            : m_prescriptionID(prescriptionID), m_appointmentID(appointmentID),
              m_patientUsername(patientUsername), m_doctorID(doctorID),
              m_prescriptionDate(prescriptionDate), m_items(), m_diagnosis(""),
              m_notes(""), m_isDispensed(false) {}

        // ==================== Getters ====================
        std::string Prescription::getPrescriptionID() const { return m_prescriptionID; }

        std::string Prescription::getAppointmentID() const { return m_appointmentID; }

        std::string Prescription::getPatientUsername() const
        {
            return m_patientUsername;
        }

        std::string Prescription::getDoctorID() const { return m_doctorID; }

        std::string Prescription::getPrescriptionDate() const
        {
            return m_prescriptionDate;
        }

        std::vector<PrescriptionItem> Prescription::getItems() const { return m_items; }

        std::string Prescription::getDiagnosis() const { return m_diagnosis; }

        std::string Prescription::getNotes() const { return m_notes; }

        bool Prescription::isDispensed() const { return m_isDispensed; }

        // ==================== Setters ====================
        void Prescription::setDiagnosis(const std::string &diagnosis)
        {
            m_diagnosis = diagnosis;
        }

        void Prescription::setNotes(const std::string &notes) { m_notes = notes; }

        void Prescription::setDispensed(bool dispensed) { m_isDispensed = dispensed; }

        // ==================== Item Management ====================
        // Model is a data container - business logic (duplicate check) is done at BLL layer
        void Prescription::addItem(const PrescriptionItem &item)
        {
            m_items.push_back(item);
        }

        void Prescription::removeItem(const std::string &medicineID)
        {
            auto it = std::remove_if(m_items.begin(), m_items.end(),
                                     [&medicineID](const PrescriptionItem &item)
                                     {
                                         return item.medicineID == medicineID;
                                     });
            m_items.erase(it, m_items.end());
        }

        void Prescription::clearItems() { m_items.clear(); }

        size_t Prescription::getItemCount() const { return m_items.size(); }

        // ==================== Helper Functions for Serialization ====================
        namespace
        {
            /**
             * @brief Serialize a single PrescriptionItem
             * Format: medicineID:medicineName:quantity:dosage:duration:instructions
             */
            std::string serializeItem(const PrescriptionItem &item)
            {
                return std::format("{}{}{}{}{}{}{}{}{}{}{}", item.medicineID,
                                   Constants::ITEM_FIELD_DELIMITER, item.medicineName,
                                   Constants::ITEM_FIELD_DELIMITER, item.quantity,
                                   Constants::ITEM_FIELD_DELIMITER, item.dosage,
                                   Constants::ITEM_FIELD_DELIMITER, item.duration,
                                   Constants::ITEM_FIELD_DELIMITER, item.instructions);
            }

            /**
             * @brief Deserialize a single PrescriptionItem
             * Format: medicineID:medicineName:quantity:dosage:duration:instructions
             */
            Result<PrescriptionItem> deserializeItem(const std::string &itemStr)
            {
                auto parts = Utils::split(itemStr, Constants::ITEM_FIELD_DELIMITER);

                // Need at least 6 fields (medicineName was added to struct)
                if (parts.size() < 6)
                {
                    return std::nullopt;
                }

                try
                {
                    PrescriptionItem item;
                    item.medicineID = Utils::trim(parts[0]);
                    item.medicineName = Utils::trim(parts[1]);
                    item.quantity = std::stoi(Utils::trim(parts[2]));
                    item.dosage = Utils::trim(parts[3]);
                    item.duration = Utils::trim(parts[4]);
                    item.instructions = Utils::trim(parts[5]);

                    if (item.medicineID.empty() || item.quantity <= 0)
                    {
                        return std::nullopt;
                    }

                    return item;
                }
                catch (const std::exception &)
                {
                    return std::nullopt;
                }
            }

            /**
             * @brief Serialize all items to a single string
             * Items are separated by ITEM_DELIMITER (;)
             */
            std::string serializeItems(const std::vector<PrescriptionItem> &items)
            {
                if (items.empty())
                {
                    return "";
                }

                std::ostringstream oss;
                for (size_t i = 0; i < items.size(); ++i)
                {
                    if (i > 0)
                    {
                        oss << Constants::ITEM_DELIMITER;
                    }
                    oss << serializeItem(items[i]);
                }
                return oss.str();
            }

            /**
             * @brief Deserialize items from a single string
             */
            std::vector<PrescriptionItem> deserializeItems(const std::string &itemsStr)
            {
                std::vector<PrescriptionItem> items;
                if (itemsStr.empty())
                {
                    return items;
                }

                auto itemStrings = Utils::split(itemsStr, Constants::ITEM_DELIMITER);
                for (const auto &itemStr : itemStrings)
                {
                    auto item = deserializeItem(itemStr);
                    if (item.has_value())
                    {
                        items.push_back(item.value());
                    }
                }
                return items;
            }
        } // anonymous namespace

        // ==================== Output ====================
        void Prescription::displayInfo() const
        {
            std::cout << "\n========================================\n";
            std::cout << "      PRESCRIPTION INFORMATION         \n";
            std::cout << "========================================\n";
            std::cout << std::format("{:<18}: {}\n", "Prescription ID", m_prescriptionID);
            std::cout << std::format("{:<18}: {}\n", "Appointment ID", m_appointmentID);
            std::cout << std::format("{:<18}: {}\n", "Patient", m_patientUsername);
            std::cout << std::format("{:<18}: {}\n", "Doctor ID", m_doctorID);
            std::cout << std::format("{:<18}: {}\n", "Date",
                                     Utils::formatDateDisplay(m_prescriptionDate));
            if (!m_diagnosis.empty())
            {
                std::cout << std::format("{:<18}: {}\n", "Diagnosis", m_diagnosis);
            }
            std::cout << std::format("{:<18}: {}\n", "Status",
                                     m_isDispensed ? "Dispensed" : "Pending");

            if (!m_items.empty())
            {
                std::cout << "\n--- Prescribed Items (" << m_items.size() << ") ---\n";
                for (size_t i = 0; i < m_items.size(); ++i)
                {
                    const auto &item = m_items[i];
                    std::cout << std::format("  {}. {} ({})\n", i + 1,
                                             item.medicineName.empty() ? item.medicineID
                                                                       : item.medicineName,
                                             item.medicineID);
                    std::cout << std::format("     Qty: {}, Dosage: {}\n", item.quantity,
                                             item.dosage);
                    std::cout << std::format("     Duration: {}\n", item.duration);
                    if (!item.instructions.empty())
                    {
                        std::cout << std::format("     Instructions: {}\n", item.instructions);
                    }
                }
            }

            if (!m_notes.empty())
            {
                std::cout << std::format("\n{:<18}: {}\n", "Notes", m_notes);
            }
            std::cout << "========================================\n\n";
        }

        std::string Prescription::toPrintFormat() const
        {
            std::ostringstream oss;

            oss << "╔══════════════════════════════════════════════════════════════╗\n";
            oss << "║                      MEDICAL PRESCRIPTION                    ║\n";
            oss << "╠══════════════════════════════════════════════════════════════╣\n";
            oss << std::format("║ Rx No: {:<55} ║\n", m_prescriptionID);
            oss << std::format("║ Date:  {:<55} ║\n",
                               Utils::formatDateDisplay(m_prescriptionDate));
            oss << "╠══════════════════════════════════════════════════════════════╣\n";
            oss << std::format("║ Patient: {:<53} ║\n", m_patientUsername);
            oss << std::format("║ Doctor:  {:<53} ║\n", m_doctorID);

            if (!m_diagnosis.empty())
            {
                oss << "╠══════════════════════════════════════════════════════════════╣\n";
                oss << std::format("║ Diagnosis: {:<51} ║\n", m_diagnosis);
            }

            oss << "╠══════════════════════════════════════════════════════════════╣\n";
            oss << "║                        MEDICATIONS                           ║\n";
            oss << "╠══════════════════════════════════════════════════════════════╣\n";

            for (size_t i = 0; i < m_items.size(); ++i)
            {
                const auto &item = m_items[i];
                std::string medicineName =
                    item.medicineName.empty() ? item.medicineID : item.medicineName;

                oss << std::format("║ {}. {:<58} ║\n", i + 1, medicineName);
                oss << std::format("║    Qty: {}  |  Dosage: {:<38} ║\n", item.quantity,
                                   item.dosage);
                oss << std::format("║    Duration: {:<48} ║\n", item.duration);
                if (!item.instructions.empty())
                {
                    oss << std::format("║    Note: {:<52} ║\n", item.instructions);
                }
                if (i < m_items.size() - 1)
                {
                    oss << "║──────────────────────────────────────────────────────────────║"
                           "\n";
                }
            }

            if (!m_notes.empty())
            {
                oss << "╠══════════════════════════════════════════════════════════════╣\n";
                oss << std::format("║ Notes: {:<55} ║\n", m_notes);
            }

            oss << "╚══════════════════════════════════════════════════════════════╝\n";

            return oss.str();
        }

        std::string Prescription::serialize() const
        {
            // Format:
            // prescriptionID|appointmentID|patientUsername|doctorID|date|diagnosis|notes|isDispensed|items
            // Items format:
            // medicineID:medicineName:quantity:dosage:duration:instructions;...
            return std::format("{}|{}|{}|{}|{}|{}|{}|{}|{}", m_prescriptionID,
                               m_appointmentID, m_patientUsername, m_doctorID,
                               m_prescriptionDate, m_diagnosis, m_notes,
                               (m_isDispensed ? "1" : "0"), serializeItems(m_items));
        }

        // ==================== Static Factory Method ====================
        Result<Prescription> Prescription::deserialize(const std::string &line)
        {
            // Skip empty lines and comments
            if (line.empty() || line[0] == Constants::COMMENT_CHAR)
            {
                return std::nullopt;
            }

            // Split by delimiter
            auto parts = Utils::split(line, Constants::FIELD_DELIMITER);

            // Expected 9 fields
            if (parts.size() != 9)
            {
                std::cerr << std::format(
                    "Error: Invalid prescription format. Expected 9 fields, got {}\n",
                    parts.size());
                return std::nullopt;
            }

            try
            {
                std::string prescriptionID = Utils::trim(parts[0]);
                std::string appointmentID = Utils::trim(parts[1]);
                std::string patientUsername = Utils::trim(parts[2]);
                std::string doctorID = Utils::trim(parts[3]);
                std::string prescriptionDate = Utils::trim(parts[4]);
                std::string diagnosis = Utils::trim(parts[5]);
                std::string notes = Utils::trim(parts[6]);
                bool isDispensed = (Utils::trim(parts[7]) == "1");
                std::string itemsStr = Utils::trim(parts[8]);

                // Validate required fields are not empty
                if (prescriptionID.empty() || patientUsername.empty() || doctorID.empty())
                {
                    std::cerr << "Error: Prescription record has empty required fields\n";
                    return std::nullopt;
                }

                // Validate date format
                if (!prescriptionDate.empty() && !Utils::isValidDate(prescriptionDate))
                {
                    std::cerr << std::format(
                        "Error: Invalid date format '{}' for prescription {}\n",
                        prescriptionDate, prescriptionID);
                    return std::nullopt;
                }

                // Create Prescription object
                Prescription prescription(prescriptionID, appointmentID, patientUsername,
                                          doctorID, prescriptionDate);
                prescription.setDiagnosis(diagnosis);
                prescription.setNotes(notes);
                prescription.setDispensed(isDispensed);

                // Parse and add items
                auto items = deserializeItems(itemsStr);
                for (const auto &item : items)
                {
                    prescription.addItem(item);
                }

                return prescription;
            }
            catch (const std::exception &e)
            {
                std::cerr << std::format("Error: Failed to parse prescription record: {}\n",
                                         e.what());
                return std::nullopt;
            }
        }

    } // namespace Model
} // namespace HMS
