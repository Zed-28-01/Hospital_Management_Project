#pragma once

/**
 * @file Medicine.h
 * @brief Placeholder for future Medicine management feature
 *
 * This file is a placeholder for the Medicine/Pharmacy management
 * feature planned for future implementation. When implemented, this
 * will allow tracking medicines and their inventory.
 *
 * Planned features:
 * - Medicine CRUD operations
 * - Inventory management
 * - Low stock alerts
 * - Medicine categories
 * - Expiry date tracking
 */

#include <string>
#include "../common/Types.h"

namespace HMS {
namespace Model {

/**
 * @class Medicine
 * @brief Represents a medicine in the pharmacy (FUTURE IMPLEMENTATION)
 *
 * @note This is a placeholder. Implementation pending.
 */
class Medicine {
private:
    std::string m_medicineID;
    std::string m_name;
    std::string m_genericName;
    std::string m_category;
    std::string m_manufacturer;
    std::string m_description;
    double m_unitPrice;
    int m_quantityInStock;
    int m_reorderLevel;
    std::string m_expiryDate;
    std::string m_dosageForm;     // tablet, capsule, syrup, etc.
    std::string m_strength;       // e.g., "500mg", "10ml"

public:
    // ==================== Constructors ====================

    Medicine() = default;

    Medicine(const std::string& medicineID,
             const std::string& name,
             const std::string& category,
             double unitPrice,
             int quantityInStock);

    ~Medicine() = default;

    // ==================== Getters ====================

    std::string getMedicineID() const;
    std::string getName() const;
    std::string getGenericName() const;
    std::string getCategory() const;
    std::string getManufacturer() const;
    std::string getDescription() const;
    double getUnitPrice() const;
    int getQuantityInStock() const;
    int getReorderLevel() const;
    std::string getExpiryDate() const;
    std::string getDosageForm() const;
    std::string getStrength() const;

    // ==================== Setters ====================

    void setName(const std::string& name);
    void setGenericName(const std::string& genericName);
    void setCategory(const std::string& category);
    void setManufacturer(const std::string& manufacturer);
    void setDescription(const std::string& description);
    void setUnitPrice(double price);
    void setQuantityInStock(int quantity);
    void setReorderLevel(int level);
    void setExpiryDate(const std::string& date);
    void setDosageForm(const std::string& form);
    void setStrength(const std::string& strength);

    // ==================== Inventory Operations ====================

    void addStock(int quantity);
    bool removeStock(int quantity);
    bool isLowStock() const;
    bool isExpired() const;
    bool isExpiringSoon(int daysThreshold = 30) const;

    // ==================== Serialization ====================

    void displayInfo() const;
    std::string serialize() const;
    static Result<Medicine> deserialize(const std::string& line);
};

} // namespace Model
} // namespace HMS

/**
 * @note Future files to create when implementing Medicine feature:
 *
 * Data Access Layer:
 * - include/dal/MedicineRepository.h
 * - src/dal/MedicineRepository.cpp
 *
 * Business Logic Layer:
 * - include/bll/MedicineService.h
 * - src/bll/MedicineService.cpp
 * - include/bll/InventoryService.h
 * - src/bll/InventoryService.cpp
 *
 * Data File:
 * - data/Medicine.txt
 *
 * Format: medicineID|name|genericName|category|manufacturer|description|unitPrice|quantity|reorderLevel|expiryDate|dosageForm|strength
 * Example: MED001|Paracetamol 500mg|Acetaminophen|Pain Relief|PharmaCorp|For pain and fever|5000|500|100|2025-12-31|Tablet|500mg
 */
