#pragma once

#include "../dal/MedicineRepository.h"
#include "../advance/Medicine.h"
#include "../common/Types.h"

#include <string>
#include <mutex>
#include <memory>
#include <map>

namespace HMS {
namespace BLL {

/**
 * @struct StockAlert
 * @brief Represents a stock alert for a medicine
 */
struct StockAlert {
    std::string medicineID;
    std::string medicineName;
    int currentStock;
    int reorderLevel;
    std::string alertType;  // "LOW_STOCK" or "OUT_OF_STOCK"
};

/**
 * @struct ExpiryAlert
 * @brief Represents an expiry alert for a medicine
 */
struct ExpiryAlert {
    std::string medicineID;
    std::string medicineName;
    std::string expiryDate;
    int daysUntilExpiry;
    std::string alertType;  // "EXPIRED" or "EXPIRING_SOON"
};

/**
 * @class MedicineService
 * @brief Service for medicine-related business logic
 *
 * Implements Singleton pattern. Handles medicine management,
 * inventory operations, and stock/expiry alerts.
 */
class MedicineService {
private:
    // ==================== Singleton ====================
    static std::unique_ptr<MedicineService> s_instance;
    static std::mutex s_mutex;

    // ==================== Dependencies ====================
    DAL::MedicineRepository* m_medicineRepo;

    // ==================== Private Constructor ====================
    MedicineService();

public:
    // ==================== Singleton Access ====================

    /**
     * @brief Get the singleton instance
     * @return Pointer to the singleton instance
     */
    static MedicineService* getInstance();

    /**
     * @brief Reset the singleton instance (for testing)
     */
    static void resetInstance();

    /**
     * @brief Delete copy constructor
     */
    MedicineService(const MedicineService&) = delete;

    /**
     * @brief Delete assignment operator
     */
    MedicineService& operator=(const MedicineService&) = delete;

    /**
     * @brief Destructor
     */
    ~MedicineService();

    // ==================== CRUD Operations ====================

    /**
     * @brief Create a new medicine
     * @param medicine The medicine data
     * @return True if successful
     */
    bool createMedicine(const Model::Medicine& medicine);

    /**
     * @brief Create medicine with individual parameters
     * @param name Medicine name
     * @param genericName Generic/scientific name
     * @param category Medicine category
     * @param manufacturer Manufacturer name
     * @param unitPrice Price per unit
     * @param quantity Initial stock quantity
     * @return Created medicine or nullopt if failed
     */
    Result<Model::Medicine> createMedicine(
        const std::string& name,
        const std::string& genericName,
        const std::string& category,
        const std::string& manufacturer,
        double unitPrice,
        int quantity);

    /**
     * @brief Update an existing medicine
     * @param medicine The medicine with updated data
     * @return True if successful
     */
    bool updateMedicine(const Model::Medicine& medicine);

    /**
     * @brief Delete a medicine
     * @param medicineID The medicine's ID
     * @return True if successful
     */
    bool deleteMedicine(const std::string& medicineID);

    // ==================== Query Operations ====================

    /**
     * @brief Get medicine by ID
     * @param medicineID The medicine's ID
     * @return Medicine if found, nullopt otherwise
     */
    Result<Model::Medicine> getMedicineByID(const std::string& medicineID);

    /**
     * @brief Get all medicines
     * @return Vector of all medicines
     */
    List<Model::Medicine> getAllMedicines();

    /**
     * @brief Get medicines by category
     * @param category The category name
     * @return Vector of medicines in category
     */
    List<Model::Medicine> getMedicinesByCategory(const std::string& category);

    /**
     * @brief Search medicines by keyword
     * @param keyword Search keyword
     * @return Vector of matching medicines
     */
    List<Model::Medicine> searchMedicines(const std::string& keyword);

    /**
     * @brief Get all unique categories
     * @return Vector of category names
     */
    List<std::string> getAllCategories();

    /**
     * @brief Get all unique manufacturers
     * @return Vector of manufacturer names
     */
    List<std::string> getAllManufacturers();

    /**
     * @brief Get total medicine count
     * @return Number of medicines
     */
    size_t getMedicineCount() const;

    // ==================== Stock Management ====================

    /**
     * @brief Add stock to a medicine
     * @param medicineID The medicine's ID
     * @param quantity Quantity to add
     * @return True if successful
     */
    bool addStock(const std::string& medicineID, int quantity);

    /**
     * @brief Remove stock from a medicine
     * @param medicineID The medicine's ID
     * @param quantity Quantity to remove
     * @return True if successful (false if insufficient stock)
     */
    bool removeStock(const std::string& medicineID, int quantity);

    /**
     * @brief Check if medicine has enough stock
     * @param medicineID The medicine's ID
     * @param quantity Required quantity
     * @return True if enough stock available
     */
    bool hasEnoughStock(const std::string& medicineID, int quantity) const;

    /**
     * @brief Get current stock level
     * @param medicineID The medicine's ID
     * @return Stock quantity, or -1 if medicine not found
     */
    int getStockLevel(const std::string& medicineID) const;

    /**
     * @brief Update reorder level for a medicine
     * @param medicineID The medicine's ID
     * @param reorderLevel New reorder level
     * @return True if successful
     */
    bool updateReorderLevel(const std::string& medicineID, int reorderLevel);

    // ==================== Stock Alerts ====================

    /**
     * @brief Get all low stock alerts
     * @return Vector of stock alerts
     */
    List<StockAlert> getLowStockAlerts();

    /**
     * @brief Get all out-of-stock medicines
     * @return Vector of medicines with zero stock
     */
    List<Model::Medicine> getOutOfStock();

    /**
     * @brief Get medicines below reorder level
     * @return Vector of low stock medicines
     */
    List<Model::Medicine> getLowStockMedicines();

    // ==================== Expiry Alerts ====================

    /**
     * @brief Get all expiry alerts
     * @param daysThreshold Days until expiry threshold (default: 30)
     * @return Vector of expiry alerts
     */
    List<ExpiryAlert> getExpiryAlerts(int daysThreshold = 30);

    /**
     * @brief Get expired medicines
     * @return Vector of expired medicines
     */
    List<Model::Medicine> getExpiredMedicines();

    /**
     * @brief Get medicines expiring soon
     * @param daysThreshold Days until expiry threshold (default: 30)
     * @return Vector of medicines expiring soon
     */
    List<Model::Medicine> getExpiringSoonMedicines(int daysThreshold = 30);

    // ==================== Inventory Statistics ====================

    /**
     * @brief Get total inventory value
     * @return Sum of (unitPrice * quantity) for all medicines
     */
    double getTotalInventoryValue() const;

    /**
     * @brief Get inventory value by category
     * @return Map of category to inventory value
     */
    std::map<std::string, double> getInventoryValueByCategory() const;

    /**
     * @brief Get stock count by category
     * @return Map of category to total stock count
     */
    std::map<std::string, int> getStockCountByCategory() const;

    // ==================== Validation ====================

    /**
     * @brief Validate medicine data
     * @param medicine The medicine to validate
     * @return True if valid
     */
    bool validateMedicine(const Model::Medicine& medicine) const;

    /**
     * @brief Check if medicine ID exists
     * @param medicineID The medicine ID to check
     * @return True if exists
     */
    bool medicineExists(const std::string& medicineID) const;

    /**
     * @brief Check if medicine name already exists
     * @param name The medicine name to check
     * @param excludeID Medicine ID to exclude from check (for updates)
     * @return True if name exists
     */
    bool medicineNameExists(const std::string& name, const std::string& excludeID = "") const;

    // ==================== Data Persistence ====================

    /**
     * @brief Save medicine data
     * @return True if successful
     */
    bool saveData();

    /**
     * @brief Load medicine data
     * @return True if successful
     */
    bool loadData();
};

} // namespace BLL
} // namespace HMS
