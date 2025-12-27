#include "advance/Medicine.h"
#include "common/Constants.h"
#include "common/Utils.h"

#include <format>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace HMS
{
    namespace Model
    {

        // ==================== Constructor ====================
        Medicine::Medicine(const std::string &medicineID, const std::string &name,
                           const std::string &category, double unitPrice,
                           int quantityInStock)
            : m_medicineID(medicineID), m_name(name), m_genericName(""),
              m_category(category), m_manufacturer(""), m_description(""),
              m_unitPrice(unitPrice), m_quantityInStock(quantityInStock),
              m_reorderLevel(Constants::DEFAULT_REORDER_LEVEL), m_expiryDate(""),
              m_dosageForm(""), m_strength("") {}

        // ==================== Getters ====================
        std::string Medicine::getMedicineID() const { return m_medicineID; }

        std::string Medicine::getName() const { return m_name; }

        std::string Medicine::getGenericName() const { return m_genericName; }

        std::string Medicine::getCategory() const { return m_category; }

        std::string Medicine::getManufacturer() const { return m_manufacturer; }

        std::string Medicine::getDescription() const { return m_description; }

        double Medicine::getUnitPrice() const { return m_unitPrice; }

        int Medicine::getQuantityInStock() const { return m_quantityInStock; }

        int Medicine::getReorderLevel() const { return m_reorderLevel; }

        std::string Medicine::getExpiryDate() const { return m_expiryDate; }

        std::string Medicine::getDosageForm() const { return m_dosageForm; }

        std::string Medicine::getStrength() const { return m_strength; }

        // ==================== Setters ====================
        void Medicine::setName(const std::string &name) { m_name = name; }

        void Medicine::setGenericName(const std::string &genericName)
        {
            m_genericName = genericName;
        }

        void Medicine::setCategory(const std::string &category)
        {
            m_category = category;
        }

        void Medicine::setManufacturer(const std::string &manufacturer)
        {
            m_manufacturer = manufacturer;
        }

        void Medicine::setDescription(const std::string &description)
        {
            m_description = description;
        }

        void Medicine::setUnitPrice(double price)
        {
            // Model is a data container - validation is done at BLL layer
            m_unitPrice = price;
        }

        void Medicine::setQuantityInStock(int quantity)
        {
            // Model is a data container - validation is done at BLL layer
            m_quantityInStock = quantity;
        }

        void Medicine::setReorderLevel(int level)
        {
            // Model is a data container - validation is done at BLL layer
            m_reorderLevel = level;
        }

        void Medicine::setExpiryDate(const std::string &date)
        {
            // Model is a data container - validation is done at BLL layer
            m_expiryDate = date;
        }

        void Medicine::setDosageForm(const std::string &form) { m_dosageForm = form; }

        void Medicine::setStrength(const std::string &strength)
        {
            m_strength = strength;
        }

        // ==================== Inventory Operations ====================
        // Note: These are simple data operations. Business validation
        // (e.g., quantity > 0, sufficient stock) is done at BLL layer.
        void Medicine::addStock(int quantity)
        {
            m_quantityInStock += quantity;
        }

        bool Medicine::removeStock(int quantity)
        {
            m_quantityInStock -= quantity;
            return true;
        }

        bool Medicine::isLowStock() const
        {
            return m_quantityInStock <= m_reorderLevel;
        }

        bool Medicine::isExpired() const
        {
            if (m_expiryDate.empty())
            {
                return false;
            }
            // If expiry date is strictly before current date, it's expired
            // (Medicine is still valid on its expiry date)
            return Utils::compareDates(m_expiryDate, Utils::getCurrentDate()) < 0;
        }

        bool Medicine::isExpiringSoon(int daysThreshold) const
        {
            if (m_expiryDate.empty() || isExpired())
            {
                return false;
            }

            // Parse current date
            std::string currentDate = Utils::getCurrentDate();
            int currentYear = std::stoi(currentDate.substr(0, 4));
            int currentMonth = std::stoi(currentDate.substr(5, 2));
            int currentDay = std::stoi(currentDate.substr(8, 2));

            // Calculate threshold date by adding days
            int thresholdDay = currentDay + daysThreshold;
            int thresholdMonth = currentMonth;
            int thresholdYear = currentYear;

            // Handle month overflow
            while (thresholdDay > Utils::getDaysInMonth(thresholdMonth, thresholdYear))
            {
                thresholdDay -= Utils::getDaysInMonth(thresholdMonth, thresholdYear);
                thresholdMonth++;
                if (thresholdMonth > 12)
                {
                    thresholdMonth = 1;
                    thresholdYear++;
                }
            }

            // Format threshold date as YYYY-MM-DD
            std::ostringstream oss;
            oss << thresholdYear << "-" << std::setfill('0') << std::setw(2)
                << thresholdMonth << "-" << std::setfill('0') << std::setw(2)
                << thresholdDay;
            std::string thresholdDate = oss.str();

            // If expiry date is before or equal to threshold date, it's expiring soon
            return Utils::compareDates(m_expiryDate, thresholdDate) <= 0;
        }

        // ==================== Serialization ====================
        void Medicine::displayInfo() const
        {
            std::cout << "\n========================================\n";
            std::cout << "       MEDICINE INFORMATION           \n";
            std::cout << "========================================\n";
            std::cout << std::format("{:<18}: {}\n", "Medicine ID", m_medicineID);
            std::cout << std::format("{:<18}: {}\n", "Name", m_name);
            if (!m_genericName.empty())
            {
                std::cout << std::format("{:<18}: {}\n", "Generic Name", m_genericName);
            }
            std::cout << std::format("{:<18}: {}\n", "Category", m_category);
            if (!m_manufacturer.empty())
            {
                std::cout << std::format("{:<18}: {}\n", "Manufacturer", m_manufacturer);
            }
            if (!m_description.empty())
            {
                std::cout << std::format("{:<18}: {}\n", "Description", m_description);
            }
            std::cout << std::format("{:<18}: {}\n", "Unit Price",
                                     Utils::formatMoney(m_unitPrice));
            std::cout << std::format("{:<18}: {}{}\n", "Stock", m_quantityInStock,
                                     isLowStock() ? " [LOW STOCK]" : "");
            std::cout << std::format("{:<18}: {}\n", "Reorder Level", m_reorderLevel);
            if (!m_expiryDate.empty())
            {
                std::cout << std::format(
                    "{:<18}: {}{}\n", "Expiry Date", Utils::formatDateDisplay(m_expiryDate),
                    isExpired() ? " [EXPIRED]"
                                : (isExpiringSoon() ? " [EXPIRING SOON]" : ""));
            }
            if (!m_dosageForm.empty())
            {
                std::cout << std::format("{:<18}: {}\n", "Dosage Form", m_dosageForm);
            }
            if (!m_strength.empty())
            {
                std::cout << std::format("{:<18}: {}\n", "Strength", m_strength);
            }
            std::cout << "========================================\n\n";
        }

        std::string Medicine::serialize() const
        {
            // Format:
            // medicineID|name|genericName|category|manufacturer|description|unitPrice|quantity|reorderLevel|expiryDate|dosageForm|strength
            return std::format("{}|{}|{}|{}|{}|{}|{:.0f}|{}|{}|{}|{}|{}", m_medicineID,
                               m_name, m_genericName, m_category, m_manufacturer,
                               m_description, m_unitPrice, m_quantityInStock,
                               m_reorderLevel, m_expiryDate, m_dosageForm, m_strength);
        }

        // ==================== Static Factory Method ====================
        Result<Medicine> Medicine::deserialize(const std::string &line)
        {
            // Skip empty lines and comments
            if (line.empty() || line[0] == Constants::COMMENT_CHAR)
            {
                return std::nullopt;
            }

            // Split by delimiter
            auto parts = Utils::split(line, Constants::FIELD_DELIMITER);

            // Expected 12 fields
            if (parts.size() != 12)
            {
                std::cerr << std::format(
                    "Error: Invalid medicine format. Expected 12 fields, got {}\n",
                    parts.size());
                return std::nullopt;
            }

            try
            {
                std::string medicineID = Utils::trim(parts[0]);
                std::string name = Utils::trim(parts[1]);
                std::string genericName = Utils::trim(parts[2]);
                std::string category = Utils::trim(parts[3]);
                std::string manufacturer = Utils::trim(parts[4]);
                std::string description = Utils::trim(parts[5]);
                double unitPrice = std::stod(Utils::trim(parts[6]));
                int quantityInStock = std::stoi(Utils::trim(parts[7]));
                int reorderLevel = std::stoi(Utils::trim(parts[8]));
                std::string expiryDate = Utils::trim(parts[9]);
                std::string dosageForm = Utils::trim(parts[10]);
                std::string strength = Utils::trim(parts[11]);

                // Validate required fields are not empty
                if (medicineID.empty() || name.empty())
                {
                    std::cerr << "Error: Medicine record has empty required fields\n";
                    return std::nullopt;
                }

                // Validate price is non-negative
                if (unitPrice < 0)
                {
                    std::cerr << std::format("Error: Invalid unit price for medicine {}\n",
                                             medicineID);
                    return std::nullopt;
                }

                // Validate quantity is non-negative
                if (quantityInStock < 0)
                {
                    std::cerr << std::format("Error: Invalid quantity for medicine {}\n",
                                             medicineID);
                    return std::nullopt;
                }

                // Validate reorder level is non-negative
                if (reorderLevel < 0)
                {
                    std::cerr << std::format("Error: Invalid reorder level for medicine {}\n",
                                             medicineID);
                    return std::nullopt;
                }

                // Validate expiry date format if provided
                if (!expiryDate.empty() && !Utils::isValidDate(expiryDate))
                {
                    std::cerr << std::format(
                        "Error: Invalid expiry date format '{}' for medicine {}\n",
                        expiryDate, medicineID);
                    return std::nullopt;
                }

                // Create Medicine object using parameterized constructor and set additional
                // fields
                Medicine med(medicineID, name, category, unitPrice, quantityInStock);
                med.setGenericName(genericName);
                med.setManufacturer(manufacturer);
                med.setDescription(description);
                med.setReorderLevel(reorderLevel);
                med.setExpiryDate(expiryDate);
                med.setDosageForm(dosageForm);
                med.setStrength(strength);

                return med;
            }
            catch (const std::exception &e)
            {
                std::cerr << std::format("Error: Failed to parse medicine record: {}\n",
                                         e.what());
                return std::nullopt;
            }
        }

    } // namespace Model
} // namespace HMS
