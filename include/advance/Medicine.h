#pragma once

/**
 * @file Medicine.h
 * @brief Medicine model for pharmacy and inventory management
 *
 * Provides functionality for tracking medicines in the hospital pharmacy,
 * including inventory management, expiry tracking, and stock alerts.
 */

#include <string>
#include "../common/Types.h"

namespace HMS
{
    namespace Model
    {

        /**
         * @class Medicine
         * @brief Represents a medicine in the hospital pharmacy system
         *
         * Manages medicine information including pricing, inventory levels,
         * expiry dates, and stock alerts. Supports serialization for
         * persistent storage.
         */
        class Medicine
        {
        private:
            std::string m_medicineID;      ///< Unique medicine identifier (e.g., "MED001")
            std::string m_name;            ///< Brand/trade name
            std::string m_genericName;     ///< Generic/scientific name
            std::string m_category;        ///< Category (e.g., "Pain Relief", "Antibiotics")
            std::string m_manufacturer;    ///< Manufacturer name
            std::string m_description;     ///< Medicine description
            double m_unitPrice = 0.0;      ///< Price per unit (VND)
            int m_quantityInStock = 0;     ///< Current stock quantity
            int m_reorderLevel = 0;        ///< Minimum stock level before reorder alert
            std::string m_expiryDate;      ///< Expiry date (YYYY-MM-DD format)
            std::string m_dosageForm;      ///< Form (e.g., "Tablet", "Capsule", "Syrup")
            std::string m_strength;        ///< Strength (e.g., "500mg", "10ml")

        public:
            // ==================== Constructors ====================

            /**
             * @brief Default constructor
             */
            Medicine() = default;

            /**
             * @brief Parameterized constructor
             * @param medicineID Unique medicine identifier
             * @param name Brand/trade name
             * @param category Medicine category
             * @param unitPrice Price per unit
             * @param quantityInStock Initial stock quantity
             */
            Medicine(const std::string &medicineID,
                     const std::string &name,
                     const std::string &category,
                     double unitPrice,
                     int quantityInStock);

            /**
             * @brief Destructor
             */
            ~Medicine() = default;

            // ==================== Getters ====================

            /**
             * @brief Get medicine's unique ID
             * @return Medicine ID string
             */
            std::string getMedicineID() const;

            /**
             * @brief Get medicine brand/trade name
             * @return Name string
             */
            std::string getName() const;

            /**
             * @brief Get medicine generic name
             * @return Generic name string
             */
            std::string getGenericName() const;

            /**
             * @brief Get medicine category
             * @return Category string
             */
            std::string getCategory() const;

            /**
             * @brief Get manufacturer name
             * @return Manufacturer string
             */
            std::string getManufacturer() const;

            /**
             * @brief Get medicine description
             * @return Description string
             */
            std::string getDescription() const;

            /**
             * @brief Get unit price
             * @return Price per unit (VND)
             */
            double getUnitPrice() const;

            /**
             * @brief Get current stock quantity
             * @return Quantity in stock
             */
            int getQuantityInStock() const;

            /**
             * @brief Get reorder level threshold
             * @return Reorder level
             */
            int getReorderLevel() const;

            /**
             * @brief Get expiry date
             * @return Expiry date string (YYYY-MM-DD)
             */
            std::string getExpiryDate() const;

            /**
             * @brief Get dosage form
             * @return Dosage form string
             */
            std::string getDosageForm() const;

            /**
             * @brief Get medicine strength
             * @return Strength string
             */
            std::string getStrength() const;

            // ==================== Setters ====================

            /**
             * @brief Set medicine name
             * @param name New name
             */
            void setName(const std::string &name);

            /**
             * @brief Set generic name
             * @param genericName New generic name
             */
            void setGenericName(const std::string &genericName);

            /**
             * @brief Set medicine category
             * @param category New category
             */
            void setCategory(const std::string &category);

            /**
             * @brief Set manufacturer
             * @param manufacturer New manufacturer
             */
            void setManufacturer(const std::string &manufacturer);

            /**
             * @brief Set description
             * @param description New description
             */
            void setDescription(const std::string &description);

            /**
             * @brief Set unit price
             * @param price New price (must be non-negative)
             */
            void setUnitPrice(double price);

            /**
             * @brief Set stock quantity
             * @param quantity New quantity (must be non-negative)
             */
            void setQuantityInStock(int quantity);

            /**
             * @brief Set reorder level
             * @param level New reorder level (must be non-negative)
             */
            void setReorderLevel(int level);

            /**
             * @brief Set expiry date
             * @param date New expiry date (YYYY-MM-DD format)
             */
            void setExpiryDate(const std::string &date);

            /**
             * @brief Set dosage form
             * @param form New dosage form
             */
            void setDosageForm(const std::string &form);

            /**
             * @brief Set medicine strength
             * @param strength New strength
             */
            void setStrength(const std::string &strength);

            // ==================== Inventory Operations ====================

            /**
             * @brief Add stock quantity
             * @param quantity Quantity to add (must be positive)
             */
            void addStock(int quantity);

            /**
             * @brief Remove stock quantity
             * @param quantity Quantity to remove
             * @return True if successful, false if insufficient stock
             */
            bool removeStock(int quantity);

            /**
             * @brief Check if stock is below reorder level
             * @return True if stock is low
             */
            bool isLowStock() const;

            /**
             * @brief Check if medicine has expired
             * @return True if expired (expiry date is before today)
             */
            bool isExpired() const;

            /**
             * @brief Check if medicine is expiring soon
             * @param daysThreshold Number of days to check (default: 30)
             * @return True if expiring within threshold days
             */
            bool isExpiringSoon(int daysThreshold = 30) const;

            // ==================== Serialization ====================

            /**
             * @brief Display medicine information to console
             */
            void displayInfo() const;

            /**
             * @brief Serialize medicine to string for file storage
             * @return Pipe-delimited string representation
             *
             * Format: medicineID|name|genericName|category|manufacturer|description|unitPrice|quantity|reorderLevel|expiryDate|dosageForm|strength
             */
            std::string serialize() const;

            /**
             * @brief Deserialize medicine from string
             * @param line Pipe-delimited string from file
             * @return Medicine object or nullopt if parsing fails
             */
            static Result<Medicine> deserialize(const std::string &line);
        };

    } // namespace Model
} // namespace HMS
