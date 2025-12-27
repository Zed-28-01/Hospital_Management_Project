#include "bll/MedicineService.h"
#include "common/Utils.h"
#include "common/Constants.h"

#include <algorithm>
#include <set>

namespace HMS
{
    namespace BLL
    {

        // ==================== Static Members ====================
        std::unique_ptr<MedicineService> MedicineService::s_instance = nullptr;
        std::mutex MedicineService::s_mutex;

        // ==================== Private Constructor ====================
        MedicineService::MedicineService()
            : m_medicineRepo(DAL::MedicineRepository::getInstance())
        {
        }

        // ==================== Singleton Access ====================
        MedicineService *MedicineService::getInstance()
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            if (!s_instance)
            {
                s_instance = std::unique_ptr<MedicineService>(new MedicineService());
            }
            return s_instance.get();
        }

        void MedicineService::resetInstance()
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_instance.reset();
        }

        MedicineService::~MedicineService() = default;

        // ==================== CRUD Operations ====================
        bool MedicineService::createMedicine(const Model::Medicine &medicine)
        {
            if (!validateMedicine(medicine))
            {
                return false;
            }

            if (medicineExists(medicine.getMedicineID()))
            {
                return false;
            }

            return m_medicineRepo->add(medicine);
        }

        Result<Model::Medicine> MedicineService::createMedicine(
            const std::string &name,
            const std::string &genericName,
            const std::string &category,
            const std::string &manufacturer,
            double unitPrice,
            int quantity)
        {
            if (name.empty() || category.empty() || unitPrice <= 0 || quantity < 0)
            {
                return std::nullopt;
            }

            Model::Medicine medicine(
                m_medicineRepo->getNextId(),
                name,
                category,
                unitPrice,
                quantity);

            medicine.setGenericName(genericName);
            medicine.setManufacturer(manufacturer);
            medicine.setReorderLevel(Constants::DEFAULT_REORDER_LEVEL);

            return m_medicineRepo->add(medicine)
                       ? Result<Model::Medicine>(medicine)
                       : std::nullopt;
        }

        bool MedicineService::updateMedicine(const Model::Medicine &medicine)
        {
            if (!validateMedicine(medicine))
            {
                return false;
            }

            if (!medicineExists(medicine.getMedicineID()))
            {
                return false;
            }

            return m_medicineRepo->update(medicine);
        }

        bool MedicineService::deleteMedicine(const std::string &medicineID)
        {
            if (medicineID.empty() || !medicineExists(medicineID))
            {
                return false;
            }

            return m_medicineRepo->remove(medicineID);
        }

        // ==================== Query Operations ====================
        Result<Model::Medicine> MedicineService::getMedicineByID(const std::string &medicineID)
        {
            return m_medicineRepo->getById(medicineID);
        }

        List<Model::Medicine> MedicineService::getAllMedicines()
        {
            return m_medicineRepo->getAll();
        }

        List<Model::Medicine> MedicineService::getMedicinesByCategory(const std::string &category)
        {
            return m_medicineRepo->getByCategory(category);
        }

        List<Model::Medicine> MedicineService::searchMedicines(const std::string &keyword)
        {
            if (keyword.empty())
            {
                return getAllMedicines();
            }
            return m_medicineRepo->search(keyword);
        }

        List<std::string> MedicineService::getAllCategories()
        {
            return m_medicineRepo->getAllCategories();
        }

        List<std::string> MedicineService::getAllManufacturers()
        {
            return m_medicineRepo->getAllManufacturers();
        }

        size_t MedicineService::getMedicineCount() const
        {
            return m_medicineRepo->count();
        }

        // ==================== Stock Management ====================
        bool MedicineService::addStock(const std::string &medicineID, int quantity)
        {
            if (quantity <= 0)
            {
                return false;
            }

            auto medicineOpt = m_medicineRepo->getById(medicineID);
            if (!medicineOpt)
            {
                return false;
            }

            auto medicine = *medicineOpt;
            medicine.addStock(quantity);

            return m_medicineRepo->update(medicine);
        }

        bool MedicineService::removeStock(const std::string &medicineID, int quantity)
        {
            if (quantity <= 0)
            {
                return false;
            }

            auto medicineOpt = m_medicineRepo->getById(medicineID);
            if (!medicineOpt)
            {
                return false;
            }

            auto medicine = *medicineOpt;

            // BLL validates sufficient stock before removing
            if (quantity > medicine.getQuantityInStock())
            {
                return false;
            }

            medicine.removeStock(quantity);
            return m_medicineRepo->update(medicine);
        }

        bool MedicineService::hasEnoughStock(const std::string &medicineID, int quantity) const
        {
            if (quantity <= 0)
            {
                return true; // Zero or negative quantity always satisfiable
            }

            auto medicineOpt = m_medicineRepo->getById(medicineID);
            if (!medicineOpt)
            {
                return false;
            }

            return medicineOpt->getQuantityInStock() >= quantity;
        }

        int MedicineService::getStockLevel(const std::string &medicineID) const
        {
            auto medicineOpt = m_medicineRepo->getById(medicineID);
            return medicineOpt ? medicineOpt->getQuantityInStock() : -1;
        }

        bool MedicineService::updateReorderLevel(const std::string &medicineID, int reorderLevel)
        {
            if (reorderLevel < 0)
            {
                return false;
            }

            auto medicineOpt = m_medicineRepo->getById(medicineID);
            if (!medicineOpt)
            {
                return false;
            }

            auto medicine = *medicineOpt;
            medicine.setReorderLevel(reorderLevel);

            return m_medicineRepo->update(medicine);
        }

        // ==================== Stock Alerts ====================
        List<StockAlert> MedicineService::getLowStockAlerts()
        {
            List<StockAlert> alerts;
            auto lowStockMeds = m_medicineRepo->getLowStock();

            for (const auto &med : lowStockMeds)
            {
                StockAlert alert;
                alert.medicineID = med.getMedicineID();
                alert.medicineName = med.getName();
                alert.currentStock = med.getQuantityInStock();
                alert.reorderLevel = med.getReorderLevel();
                alert.alertType = (alert.currentStock == 0) ? "OUT_OF_STOCK" : "LOW_STOCK";
                alerts.push_back(alert);
            }

            return alerts;
        }

        List<Model::Medicine> MedicineService::getOutOfStock()
        {
            auto allMedicines = m_medicineRepo->getAll();
            List<Model::Medicine> outOfStock;

            std::ranges::copy_if(allMedicines, std::back_inserter(outOfStock),
                                 [](const auto &med)
                                 {
                                     return med.getQuantityInStock() == 0;
                                 });

            return outOfStock;
        }

        List<Model::Medicine> MedicineService::getLowStockMedicines()
        {
            return m_medicineRepo->getLowStock();
        }

        // ==================== Expiry Alerts ====================
        List<ExpiryAlert> MedicineService::getExpiryAlerts(int daysThreshold)
        {
            List<ExpiryAlert> alerts;
            auto expired = m_medicineRepo->getExpired();
            auto expiringSoon = m_medicineRepo->getExpiringSoon(daysThreshold);

            for (const auto &med : expired)
            {
                ExpiryAlert alert;
                alert.medicineID = med.getMedicineID();
                alert.medicineName = med.getName();
                alert.expiryDate = med.getExpiryDate();
                alert.daysUntilExpiry = Utils::compareDates(med.getExpiryDate(), Utils::getCurrentDate());
                alert.alertType = "EXPIRED";
                alerts.push_back(alert);
            }

            for (const auto &med : expiringSoon)
            {
                if (!med.isExpired())
                {
                    ExpiryAlert alert;
                    alert.medicineID = med.getMedicineID();
                    alert.medicineName = med.getName();
                    alert.expiryDate = med.getExpiryDate();
                    alert.daysUntilExpiry = Utils::compareDates(med.getExpiryDate(), Utils::getCurrentDate());
                    alert.alertType = "EXPIRING_SOON";
                    alerts.push_back(alert);
                }
            }

            return alerts;
        }

        List<Model::Medicine> MedicineService::getExpiredMedicines()
        {
            return m_medicineRepo->getExpired();
        }

        List<Model::Medicine> MedicineService::getExpiringSoonMedicines(int daysThreshold)
        {
            return m_medicineRepo->getExpiringSoon(daysThreshold);
        }

        // ==================== Inventory Statistics ====================
        double MedicineService::getTotalInventoryValue() const
        {
            auto allMedicines = m_medicineRepo->getAll();
            double total = 0.0;

            for (const auto &med : allMedicines)
            {
                total += med.getUnitPrice() * med.getQuantityInStock();
            }

            return total;
        }

        std::map<std::string, double> MedicineService::getInventoryValueByCategory() const
        {
            std::map<std::string, double> categoryValues;
            auto allMedicines = m_medicineRepo->getAll();

            for (const auto &med : allMedicines)
            {
                categoryValues[med.getCategory()] +=
                    med.getUnitPrice() * med.getQuantityInStock();
            }

            return categoryValues;
        }

        std::map<std::string, int> MedicineService::getStockCountByCategory() const
        {
            std::map<std::string, int> categoryStocks;
            auto allMedicines = m_medicineRepo->getAll();

            for (const auto &med : allMedicines)
            {
                categoryStocks[med.getCategory()] += med.getQuantityInStock();
            }

            return categoryStocks;
        }

        // ==================== Validation ====================
        bool MedicineService::validateMedicine(const Model::Medicine &medicine) const
        {
            if (medicine.getMedicineID().empty() || medicine.getName().empty())
            {
                return false;
            }

            if (medicine.getCategory().empty())
            {
                return false;
            }

            if (medicine.getUnitPrice() < 0)
            {
                return false;
            }

            if (medicine.getQuantityInStock() < 0)
            {
                return false;
            }

            if (medicine.getReorderLevel() < 0)
            {
                return false;
            }

            const auto &expiryDate = medicine.getExpiryDate();
            if (!expiryDate.empty() && !Utils::isValidDate(expiryDate))
            {
                return false;
            }

            return true;
        }

        bool MedicineService::medicineExists(const std::string &medicineID) const
        {
            return m_medicineRepo->exists(medicineID);
        }

        bool MedicineService::medicineNameExists(const std::string &name, const std::string &excludeID) const
        {
            auto allMedicines = m_medicineRepo->getAll();
            std::string normalizedName = Utils::trim(Utils::toLower(name));

            return std::ranges::any_of(allMedicines,
                                       [&normalizedName, &excludeID](const auto &med)
                                       {
                                           return Utils::trim(Utils::toLower(med.getName())) == normalizedName &&
                                                  med.getMedicineID() != excludeID;
                                       });
        }

        // ==================== Data Persistence ====================
        bool MedicineService::saveData()
        {
            return m_medicineRepo->save();
        }

        bool MedicineService::loadData()
        {
            return m_medicineRepo->load();
        }

    } // namespace BLL
} // namespace HMS