#include <gtest/gtest.h>
#include "bll/MedicineService.h"
#include "dal/MedicineRepository.h"
#include "common/Constants.h"
#include "common/Utils.h"
#include <filesystem>
#include <fstream>

using namespace HMS;
using namespace HMS::BLL;
using namespace HMS::Model;
using namespace HMS::DAL;
using namespace HMS::Utils;

// ==================== Test Fixture ====================
class MedicineServiceTest : public ::testing::Test
{
protected:
    MedicineService *service;
    MedicineRepository *repo;
    std::string testFilePath;

    void SetUp() override
    {
        // Reset singletons first to ensure clean state
        MedicineService::resetInstance();
        MedicineRepository::resetInstance();

        // Setup test file path
        testFilePath = "test_data/test_medicines.txt";

        // Ensure test directory exists
        std::filesystem::create_directories("test_data");

        // Clear any existing test file
        cleanupTestFile(testFilePath);

        // Get repository instance and set test file path
        repo = MedicineRepository::getInstance();
        repo->setFilePath(testFilePath);
        repo->clear();

        // Get service instance
        service = MedicineService::getInstance();
    }

    void TearDown() override
    {
        // Clean up
        repo->clear();

        // Reset singletons
        MedicineService::resetInstance();
        MedicineRepository::resetInstance();

        // Remove test files
        cleanupTestFile(testFilePath);
    }

    // Helper to clean up test files including backups
    void cleanupTestFile(const std::string &filePath)
    {
        if (std::filesystem::exists(filePath))
        {
            std::filesystem::remove(filePath);
        }
        std::string backupFile = filePath + ".bak";
        if (std::filesystem::exists(backupFile))
        {
            std::filesystem::remove(backupFile);
        }
    }

    // Helper function to create test medicine
    Medicine createTestMedicine(
        const std::string &id,
        const std::string &name,
        const std::string &category = "Antibiotic",
        double price = 10.0,
        int quantity = 100)
    {
        Medicine med(id, name, category, price, quantity);
        med.setManufacturer("Test Pharma");
        med.setGenericName("Generic " + name);
        med.setReorderLevel(Constants::DEFAULT_REORDER_LEVEL);
        return med;
    }

    // Helper to create expired medicine
    Medicine createExpiredMedicine(
        const std::string &id = "MED_EXP",
        const std::string &name = "Expired Med")
    {
        Medicine med(id, name, "Test Category", 10.0, 50);
        med.setExpiryDate("2020-01-01"); // Past date
        med.setReorderLevel(10);
        return med;
    }

    // Helper to create low stock medicine
    Medicine createLowStockMedicine(
        const std::string &id = "MED_LOW",
        const std::string &name = "Low Stock Med")
    {
        Medicine med(id, name, "Test Category", 25.0, 5);
        med.setReorderLevel(10); // Stock (5) <= reorderLevel (10)
        med.setExpiryDate("2030-12-31");
        return med;
    }

    // Helper to generate a date N days from today
    std::string getDateDaysFromNow(int days)
    {
        auto currentDate = Utils::getCurrentDate();
        int year = std::stoi(currentDate.substr(0, 4));
        int month = std::stoi(currentDate.substr(5, 2));
        int day = std::stoi(currentDate.substr(8, 2)) + days;

        // Simple overflow handling
        while (day > 28)
        {
            day -= 28;
            month++;
            if (month > 12)
            {
                month = 1;
                year++;
            }
        }
        while (day < 1)
        {
            day += 28;
            month--;
            if (month < 1)
            {
                month = 12;
                year--;
            }
        }

        return std::to_string(year) + "-" +
               (month < 10 ? "0" : "") + std::to_string(month) + "-" +
               (day < 10 ? "0" : "") + std::to_string(day);
    }
};

// ==================== Singleton Tests ====================

TEST_F(MedicineServiceTest, GetInstance_ReturnsSameInstance)
{
    MedicineService *instance1 = MedicineService::getInstance();
    MedicineService *instance2 = MedicineService::getInstance();

    EXPECT_EQ(instance1, instance2);
    EXPECT_NE(instance1, nullptr);
}

TEST_F(MedicineServiceTest, ResetInstance_CreatesFreshInstance)
{
    MedicineService *instance1 = MedicineService::getInstance();
    EXPECT_NE(instance1, nullptr);

    MedicineService::resetInstance();
    MedicineService *instance2 = MedicineService::getInstance();

    EXPECT_NE(instance2, nullptr);
    // Note: After reset, a new instance is created
}

// ==================== CRUD Operations Tests ====================

TEST_F(MedicineServiceTest, CreateMedicine_ValidMedicine_Success)
{
    Medicine med = createTestMedicine("MED001", "Aspirin");

    bool result = service->createMedicine(med);

    EXPECT_TRUE(result);
    EXPECT_EQ(1u, service->getMedicineCount());

    auto retrieved = service->getMedicineByID("MED001");
    ASSERT_TRUE(retrieved.has_value());
    EXPECT_EQ("Aspirin", retrieved->getName());
}

TEST_F(MedicineServiceTest, CreateMedicine_InvalidMedicine_EmptyName_Fail)
{
    Medicine med = createTestMedicine("MED001", "");

    bool result = service->createMedicine(med);

    EXPECT_FALSE(result);
    EXPECT_EQ(0u, service->getMedicineCount());
}

TEST_F(MedicineServiceTest, CreateMedicine_InvalidMedicine_EmptyCategory_Fail)
{
    Medicine med("MED001", "Aspirin", "", 10.0, 100);

    bool result = service->createMedicine(med);

    EXPECT_FALSE(result);
    EXPECT_EQ(0u, service->getMedicineCount());
}

TEST_F(MedicineServiceTest, CreateMedicine_InvalidMedicine_NegativePrice_Fail)
{
    Medicine med = createTestMedicine("MED001", "Aspirin", "Antibiotic", -5.0, 100);

    bool result = service->createMedicine(med);

    EXPECT_FALSE(result);
    EXPECT_EQ(0u, service->getMedicineCount());
}

TEST_F(MedicineServiceTest, CreateMedicine_InvalidMedicine_NegativeQuantity_Fail)
{
    Medicine med = createTestMedicine("MED001", "Aspirin", "Antibiotic", 10.0, -1);

    bool result = service->createMedicine(med);

    EXPECT_FALSE(result);
    EXPECT_EQ(0u, service->getMedicineCount());
}

TEST_F(MedicineServiceTest, CreateMedicine_DuplicateID_Fail)
{
    Medicine med1 = createTestMedicine("MED001", "Aspirin");
    Medicine med2 = createTestMedicine("MED001", "Paracetamol");

    service->createMedicine(med1);
    bool result = service->createMedicine(med2);

    EXPECT_FALSE(result);
    EXPECT_EQ(1u, service->getMedicineCount());
}

TEST_F(MedicineServiceTest, CreateMedicine_SameNameDifferentManufacturer_Success)
{
    // Service allows same name with different manufacturers (different formulations)
    Medicine med1 = createTestMedicine("MED001", "Aspirin");
    med1.setManufacturer("Pharma Corp A");

    Medicine med2 = createTestMedicine("MED002", "Aspirin");
    med2.setManufacturer("Pharma Corp B");

    bool result1 = service->createMedicine(med1);
    bool result2 = service->createMedicine(med2);

    EXPECT_TRUE(result1);
    EXPECT_TRUE(result2);
    EXPECT_EQ(2u, service->getMedicineCount());
}

TEST_F(MedicineServiceTest, CreateMedicine_WithParameters_Success)
{
    auto result = service->createMedicine(
        "Aspirin",
        "Acetylsalicylic Acid",
        "Analgesic",
        "Pharma Inc",
        15.50,
        200);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ("Aspirin", result->getName());
    EXPECT_EQ("Acetylsalicylic Acid", result->getGenericName());
    EXPECT_EQ("Analgesic", result->getCategory());
    EXPECT_EQ("Pharma Inc", result->getManufacturer());
    EXPECT_DOUBLE_EQ(15.50, result->getUnitPrice());
    EXPECT_EQ(200, result->getQuantityInStock());
    EXPECT_EQ(Constants::DEFAULT_REORDER_LEVEL, result->getReorderLevel());
    EXPECT_EQ(1u, service->getMedicineCount());
}

TEST_F(MedicineServiceTest, CreateMedicine_WithParameters_InvalidInput_Fail)
{
    // Test empty name
    auto result1 = service->createMedicine("", "Generic", "Category", "Mfr", 10.0, 100);
    EXPECT_FALSE(result1.has_value());

    // Test empty category
    auto result2 = service->createMedicine("Name", "Generic", "", "Mfr", 10.0, 100);
    EXPECT_FALSE(result2.has_value());

    // Test zero price (service rejects unitPrice <= 0)
    auto result3 = service->createMedicine("Name", "Generic", "Category", "Mfr", 0.0, 100);
    EXPECT_FALSE(result3.has_value());

    // Test negative price
    auto result4 = service->createMedicine("Name", "Generic", "Category", "Mfr", -10.0, 100);
    EXPECT_FALSE(result4.has_value());

    // Test negative quantity
    auto result5 = service->createMedicine("Name", "Generic", "Category", "Mfr", 10.0, -1);
    EXPECT_FALSE(result5.has_value());

    EXPECT_EQ(0u, service->getMedicineCount());
}

TEST_F(MedicineServiceTest, UpdateMedicine_ValidUpdate_Success)
{
    Medicine med = createTestMedicine("MED001", "Aspirin");
    service->createMedicine(med);

    med.setName("Updated Aspirin");
    med.setUnitPrice(20.0);

    bool result = service->updateMedicine(med);

    EXPECT_TRUE(result);

    auto updated = service->getMedicineByID("MED001");
    ASSERT_TRUE(updated.has_value());
    EXPECT_EQ("Updated Aspirin", updated->getName());
    EXPECT_DOUBLE_EQ(20.0, updated->getUnitPrice());
}

TEST_F(MedicineServiceTest, UpdateMedicine_InvalidMedicine_Fail)
{
    Medicine med = createTestMedicine("MED001", "Aspirin");
    service->createMedicine(med);

    // Create medicine with negative price (invalid)
    Medicine invalidMed("MED001", "Aspirin", "Category", -10.0, 100);

    bool result = service->updateMedicine(invalidMed);

    EXPECT_FALSE(result);
}

TEST_F(MedicineServiceTest, UpdateMedicine_NonExistentMedicine_Fail)
{
    Medicine med = createTestMedicine("MED999", "NonExistent");

    bool result = service->updateMedicine(med);

    EXPECT_FALSE(result);
}

TEST_F(MedicineServiceTest, DeleteMedicine_ExistingMedicine_Success)
{
    Medicine med = createTestMedicine("MED001", "Aspirin");
    service->createMedicine(med);

    bool result = service->deleteMedicine("MED001");

    EXPECT_TRUE(result);
    EXPECT_EQ(0u, service->getMedicineCount());
    EXPECT_FALSE(service->getMedicineByID("MED001").has_value());
}

TEST_F(MedicineServiceTest, DeleteMedicine_NonExistentMedicine_Fail)
{
    bool result = service->deleteMedicine("MED999");

    EXPECT_FALSE(result);
}

TEST_F(MedicineServiceTest, DeleteMedicine_EmptyID_Fail)
{
    bool result = service->deleteMedicine("");

    EXPECT_FALSE(result);
}

// ==================== Query Operations Tests ====================

TEST_F(MedicineServiceTest, GetMedicineByID_ExistingMedicine_Success)
{
    Medicine med = createTestMedicine("MED001", "Aspirin");
    service->createMedicine(med);

    auto result = service->getMedicineByID("MED001");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ("MED001", result->getMedicineID());
    EXPECT_EQ("Aspirin", result->getName());
}

TEST_F(MedicineServiceTest, GetMedicineByID_NonExistentMedicine_ReturnEmpty)
{
    auto result = service->getMedicineByID("MED999");

    EXPECT_FALSE(result.has_value());
}

TEST_F(MedicineServiceTest, GetAllMedicines_EmptyRepository_ReturnEmptyList)
{
    auto result = service->getAllMedicines();

    EXPECT_TRUE(result.empty());
}

TEST_F(MedicineServiceTest, GetAllMedicines_MultipleMedicines_ReturnAll)
{
    service->createMedicine(createTestMedicine("MED001", "Aspirin"));
    service->createMedicine(createTestMedicine("MED002", "Paracetamol"));
    service->createMedicine(createTestMedicine("MED003", "Ibuprofen"));

    auto result = service->getAllMedicines();

    EXPECT_EQ(3u, result.size());
}

TEST_F(MedicineServiceTest, GetMedicinesByCategory_ValidCategory_ReturnMatching)
{
    service->createMedicine(createTestMedicine("MED001", "Aspirin", "Analgesic"));
    service->createMedicine(createTestMedicine("MED002", "Amoxicillin", "Antibiotic"));
    service->createMedicine(createTestMedicine("MED003", "Ibuprofen", "Analgesic"));

    auto result = service->getMedicinesByCategory("Analgesic");

    EXPECT_EQ(2u, result.size());
}

TEST_F(MedicineServiceTest, GetMedicinesByCategory_NonExistentCategory_ReturnEmpty)
{
    service->createMedicine(createTestMedicine("MED001", "Aspirin", "Analgesic"));

    auto result = service->getMedicinesByCategory("NonExistent");

    EXPECT_TRUE(result.empty());
}

TEST_F(MedicineServiceTest, SearchMedicines_EmptyKeyword_ReturnAll)
{
    service->createMedicine(createTestMedicine("MED001", "Aspirin"));
    service->createMedicine(createTestMedicine("MED002", "Paracetamol"));

    auto result = service->searchMedicines("");

    EXPECT_EQ(2u, result.size());
}

TEST_F(MedicineServiceTest, SearchMedicines_ByName_ReturnMatching)
{
    service->createMedicine(createTestMedicine("MED001", "Aspirin"));
    service->createMedicine(createTestMedicine("MED002", "Paracetamol"));
    service->createMedicine(createTestMedicine("MED003", "Ibuprofen"));

    auto result = service->searchMedicines("para");

    EXPECT_EQ(1u, result.size());
    EXPECT_EQ("Paracetamol", result[0].getName());
}

TEST_F(MedicineServiceTest, SearchMedicines_ByGenericName_ReturnMatching)
{
    Medicine med = createTestMedicine("MED001", "Aspirin");
    med.setGenericName("Acetylsalicylic Acid");
    service->createMedicine(med);

    auto result = service->searchMedicines("Acetyl");

    EXPECT_EQ(1u, result.size());
    EXPECT_EQ("Aspirin", result[0].getName());
}

TEST_F(MedicineServiceTest, SearchMedicines_ByCategory_ReturnMatching)
{
    service->createMedicine(createTestMedicine("MED001", "Aspirin", "Analgesic"));
    service->createMedicine(createTestMedicine("MED002", "Amoxicillin", "Antibiotic"));

    auto result = service->searchMedicines("anal");

    EXPECT_EQ(1u, result.size());
    EXPECT_EQ("Aspirin", result[0].getName());
}

TEST_F(MedicineServiceTest, SearchMedicines_ByManufacturer_ReturnMatching)
{
    Medicine med1 = createTestMedicine("MED001", "Aspirin");
    med1.setManufacturer("Pfizer");

    Medicine med2 = createTestMedicine("MED002", "Paracetamol");
    med2.setManufacturer("GSK");

    service->createMedicine(med1);
    service->createMedicine(med2);

    auto result = service->searchMedicines("pfizer");

    EXPECT_EQ(1u, result.size());
    EXPECT_EQ("Aspirin", result[0].getName());
}

TEST_F(MedicineServiceTest, SearchMedicines_NoMatch_ReturnEmpty)
{
    service->createMedicine(createTestMedicine("MED001", "Aspirin"));

    auto result = service->searchMedicines("XYZ123");

    EXPECT_TRUE(result.empty());
}

TEST_F(MedicineServiceTest, GetAllCategories_ReturnUniqueSorted)
{
    service->createMedicine(createTestMedicine("MED001", "Med1", "Analgesic"));
    service->createMedicine(createTestMedicine("MED002", "Med2", "Antibiotic"));
    service->createMedicine(createTestMedicine("MED003", "Med3", "Analgesic"));
    service->createMedicine(createTestMedicine("MED004", "Med4", "Antiviral"));

    auto result = service->getAllCategories();

    EXPECT_EQ(3u, result.size());
    EXPECT_EQ("Analgesic", result[0]);
    EXPECT_EQ("Antibiotic", result[1]);
    EXPECT_EQ("Antiviral", result[2]);
}

TEST_F(MedicineServiceTest, GetAllManufacturers_ReturnUniqueSorted)
{
    Medicine med1 = createTestMedicine("MED001", "Med1");
    med1.setManufacturer("Pfizer");

    Medicine med2 = createTestMedicine("MED002", "Med2");
    med2.setManufacturer("GSK");

    Medicine med3 = createTestMedicine("MED003", "Med3");
    med3.setManufacturer("Pfizer");

    service->createMedicine(med1);
    service->createMedicine(med2);
    service->createMedicine(med3);

    auto result = service->getAllManufacturers();

    EXPECT_EQ(2u, result.size());
    EXPECT_EQ("GSK", result[0]);
    EXPECT_EQ("Pfizer", result[1]);
}

TEST_F(MedicineServiceTest, GetMedicineCount_ReturnCorrectCount)
{
    EXPECT_EQ(0u, service->getMedicineCount());

    service->createMedicine(createTestMedicine("MED001", "Med1"));
    EXPECT_EQ(1u, service->getMedicineCount());

    service->createMedicine(createTestMedicine("MED002", "Med2"));
    EXPECT_EQ(2u, service->getMedicineCount());

    service->deleteMedicine("MED001");
    EXPECT_EQ(1u, service->getMedicineCount());
}

// ==================== Stock Management Tests ====================

TEST_F(MedicineServiceTest, AddStock_ValidQuantity_Success)
{
    Medicine med = createTestMedicine("MED001", "Aspirin", "Analgesic", 10.0, 100);
    service->createMedicine(med);

    bool result = service->addStock("MED001", 50);

    EXPECT_TRUE(result);
    EXPECT_EQ(150, service->getStockLevel("MED001"));
}

TEST_F(MedicineServiceTest, AddStock_ZeroQuantity_Fail)
{
    Medicine med = createTestMedicine("MED001", "Aspirin", "Analgesic", 10.0, 100);
    service->createMedicine(med);

    bool result = service->addStock("MED001", 0);

    EXPECT_FALSE(result);
    EXPECT_EQ(100, service->getStockLevel("MED001"));
}

TEST_F(MedicineServiceTest, AddStock_NegativeQuantity_Fail)
{
    Medicine med = createTestMedicine("MED001", "Aspirin", "Analgesic", 10.0, 100);
    service->createMedicine(med);

    bool result = service->addStock("MED001", -10);

    EXPECT_FALSE(result);
    EXPECT_EQ(100, service->getStockLevel("MED001"));
}

TEST_F(MedicineServiceTest, AddStock_NonExistentMedicine_Fail)
{
    bool result = service->addStock("MED999", 50);

    EXPECT_FALSE(result);
}

TEST_F(MedicineServiceTest, RemoveStock_ValidQuantity_Success)
{
    Medicine med = createTestMedicine("MED001", "Aspirin", "Analgesic", 10.0, 100);
    service->createMedicine(med);

    bool result = service->removeStock("MED001", 30);

    EXPECT_TRUE(result);
    EXPECT_EQ(70, service->getStockLevel("MED001"));
}

TEST_F(MedicineServiceTest, RemoveStock_ExactQuantity_Success)
{
    Medicine med = createTestMedicine("MED001", "Aspirin", "Analgesic", 10.0, 100);
    service->createMedicine(med);

    bool result = service->removeStock("MED001", 100);

    EXPECT_TRUE(result);
    EXPECT_EQ(0, service->getStockLevel("MED001"));
}

TEST_F(MedicineServiceTest, RemoveStock_InsufficientStock_Fail)
{
    Medicine med = createTestMedicine("MED001", "Aspirin", "Analgesic", 10.0, 100);
    service->createMedicine(med);

    bool result = service->removeStock("MED001", 150);

    EXPECT_FALSE(result);
    EXPECT_EQ(100, service->getStockLevel("MED001"));
}

TEST_F(MedicineServiceTest, RemoveStock_ZeroQuantity_Fail)
{
    Medicine med = createTestMedicine("MED001", "Aspirin", "Analgesic", 10.0, 100);
    service->createMedicine(med);

    bool result = service->removeStock("MED001", 0);

    EXPECT_FALSE(result);
}

TEST_F(MedicineServiceTest, RemoveStock_NegativeQuantity_Fail)
{
    Medicine med = createTestMedicine("MED001", "Aspirin", "Analgesic", 10.0, 100);
    service->createMedicine(med);

    bool result = service->removeStock("MED001", -10);

    EXPECT_FALSE(result);
}

TEST_F(MedicineServiceTest, RemoveStock_NonExistentMedicine_Fail)
{
    bool result = service->removeStock("MED999", 10);

    EXPECT_FALSE(result);
}

TEST_F(MedicineServiceTest, HasEnoughStock_SufficientStock_ReturnTrue)
{
    Medicine med = createTestMedicine("MED001", "Aspirin", "Analgesic", 10.0, 100);
    service->createMedicine(med);

    EXPECT_TRUE(service->hasEnoughStock("MED001", 50));
    EXPECT_TRUE(service->hasEnoughStock("MED001", 100));
}

TEST_F(MedicineServiceTest, HasEnoughStock_InsufficientStock_ReturnFalse)
{
    Medicine med = createTestMedicine("MED001", "Aspirin", "Analgesic", 10.0, 100);
    service->createMedicine(med);

    EXPECT_FALSE(service->hasEnoughStock("MED001", 150));
}

TEST_F(MedicineServiceTest, HasEnoughStock_NonExistentMedicine_ReturnFalse)
{
    EXPECT_FALSE(service->hasEnoughStock("MED999", 10));
}

TEST_F(MedicineServiceTest, HasEnoughStock_ZeroQuantity_ReturnTrue)
{
    // Zero quantity is always satisfiable
    Medicine med = createTestMedicine("MED001", "Aspirin", "Analgesic", 10.0, 100);
    service->createMedicine(med);

    EXPECT_TRUE(service->hasEnoughStock("MED001", 0));
}

TEST_F(MedicineServiceTest, HasEnoughStock_NegativeQuantity_ReturnTrue)
{
    // Negative quantity is always satisfiable (edge case optimization)
    Medicine med = createTestMedicine("MED001", "Aspirin", "Analgesic", 10.0, 100);
    service->createMedicine(med);

    EXPECT_TRUE(service->hasEnoughStock("MED001", -10));
}

TEST_F(MedicineServiceTest, GetStockLevel_ExistingMedicine_ReturnLevel)
{
    Medicine med = createTestMedicine("MED001", "Aspirin", "Analgesic", 10.0, 100);
    service->createMedicine(med);

    int level = service->getStockLevel("MED001");

    EXPECT_EQ(100, level);
}

TEST_F(MedicineServiceTest, GetStockLevel_NonExistentMedicine_ReturnNegative)
{
    int level = service->getStockLevel("MED999");

    EXPECT_EQ(-1, level);
}

TEST_F(MedicineServiceTest, UpdateReorderLevel_ValidLevel_Success)
{
    Medicine med = createTestMedicine("MED001", "Aspirin", "Analgesic", 10.0, 100);
    service->createMedicine(med);

    bool result = service->updateReorderLevel("MED001", 20);

    EXPECT_TRUE(result);

    auto updated = service->getMedicineByID("MED001");
    ASSERT_TRUE(updated.has_value());
    EXPECT_EQ(20, updated->getReorderLevel());
}

TEST_F(MedicineServiceTest, UpdateReorderLevel_ZeroLevel_Success)
{
    Medicine med = createTestMedicine("MED001", "Aspirin", "Analgesic", 10.0, 100);
    service->createMedicine(med);

    bool result = service->updateReorderLevel("MED001", 0);

    EXPECT_TRUE(result);

    auto updated = service->getMedicineByID("MED001");
    ASSERT_TRUE(updated.has_value());
    EXPECT_EQ(0, updated->getReorderLevel());
}

TEST_F(MedicineServiceTest, UpdateReorderLevel_NegativeLevel_Fail)
{
    Medicine med = createTestMedicine("MED001", "Aspirin", "Analgesic", 10.0, 100);
    service->createMedicine(med);

    bool result = service->updateReorderLevel("MED001", -5);

    EXPECT_FALSE(result);
}

TEST_F(MedicineServiceTest, UpdateReorderLevel_NonExistentMedicine_Fail)
{
    bool result = service->updateReorderLevel("MED999", 10);

    EXPECT_FALSE(result);
}

// ==================== Stock Alerts Tests ====================

TEST_F(MedicineServiceTest, GetLowStockAlerts_MixedStock_ReturnLowAndOut)
{
    Medicine med1 = createTestMedicine("MED001", "Med1", "Cat", 10.0, 5);
    med1.setReorderLevel(10);

    Medicine med2 = createTestMedicine("MED002", "Med2", "Cat", 10.0, 0);
    med2.setReorderLevel(5);

    Medicine med3 = createTestMedicine("MED003", "Med3", "Cat", 10.0, 100);
    med3.setReorderLevel(10);

    service->createMedicine(med1);
    service->createMedicine(med2);
    service->createMedicine(med3);

    auto alerts = service->getLowStockAlerts();

    EXPECT_EQ(2u, alerts.size());

    // Find alerts by medicine ID
    auto alert1 = std::find_if(alerts.begin(), alerts.end(),
                               [](const StockAlert &a)
                               { return a.medicineID == "MED001"; });
    auto alert2 = std::find_if(alerts.begin(), alerts.end(),
                               [](const StockAlert &a)
                               { return a.medicineID == "MED002"; });

    ASSERT_NE(alert1, alerts.end());
    ASSERT_NE(alert2, alerts.end());

    EXPECT_EQ("LOW_STOCK", alert1->alertType);
    EXPECT_EQ(5, alert1->currentStock);

    EXPECT_EQ("OUT_OF_STOCK", alert2->alertType);
    EXPECT_EQ(0, alert2->currentStock);
}

TEST_F(MedicineServiceTest, GetLowStockAlerts_AllNormalStock_ReturnEmpty)
{
    Medicine med = createTestMedicine("MED001", "Med1", "Cat", 10.0, 100);
    med.setReorderLevel(10);
    service->createMedicine(med);

    auto alerts = service->getLowStockAlerts();

    EXPECT_TRUE(alerts.empty());
}

TEST_F(MedicineServiceTest, GetOutOfStock_ReturnOnlyZeroStock)
{
    service->createMedicine(createTestMedicine("MED001", "Med1", "Cat", 10.0, 0));
    service->createMedicine(createTestMedicine("MED002", "Med2", "Cat", 10.0, 5));
    service->createMedicine(createTestMedicine("MED003", "Med3", "Cat", 10.0, 100));

    auto result = service->getOutOfStock();

    EXPECT_EQ(1u, result.size());
    EXPECT_EQ("MED001", result[0].getMedicineID());
}

TEST_F(MedicineServiceTest, GetLowStockMedicines_ReturnBelowReorderLevel)
{
    service->createMedicine(createLowStockMedicine("MED001", "Low Stock"));

    Medicine normalMed = createTestMedicine("MED002", "Normal Stock", "Cat", 10.0, 100);
    normalMed.setReorderLevel(10);
    service->createMedicine(normalMed);

    auto result = service->getLowStockMedicines();

    EXPECT_GE(result.size(), 1u);

    bool foundLowStock = std::any_of(result.begin(), result.end(),
                                     [](const Medicine &m)
                                     { return m.getMedicineID() == "MED001"; });
    EXPECT_TRUE(foundLowStock);
}

// ==================== Expiry Alerts Tests ====================

TEST_F(MedicineServiceTest, GetExpiryAlerts_ExpiredMedicines_ReturnExpiredAlerts)
{
    service->createMedicine(createExpiredMedicine("MED001", "Expired Aspirin"));

    auto alerts = service->getExpiryAlerts(30);

    ASSERT_GE(alerts.size(), 1u);

    auto expiredAlert = std::find_if(alerts.begin(), alerts.end(),
                                     [](const ExpiryAlert &a)
                                     { return a.medicineID == "MED001"; });
    ASSERT_NE(expiredAlert, alerts.end());
    EXPECT_EQ("EXPIRED", expiredAlert->alertType);
}

TEST_F(MedicineServiceTest, GetExpiryAlerts_ExpiringSoonMedicines_ReturnExpiringSoonAlerts)
{
    Medicine med = createTestMedicine("MED001", "Aspirin");
    med.setExpiryDate(getDateDaysFromNow(15));
    service->createMedicine(med);

    auto alerts = service->getExpiryAlerts(30);

    ASSERT_EQ(1u, alerts.size());
    EXPECT_EQ("EXPIRING_SOON", alerts[0].alertType);
    EXPECT_EQ("MED001", alerts[0].medicineID);
}

TEST_F(MedicineServiceTest, GetExpiryAlerts_NoExpiryIssues_ReturnEmpty)
{
    Medicine med = createTestMedicine("MED001", "Aspirin");
    med.setExpiryDate("2030-12-31");
    service->createMedicine(med);

    auto alerts = service->getExpiryAlerts(30);

    EXPECT_TRUE(alerts.empty());
}

TEST_F(MedicineServiceTest, GetExpiryAlerts_MixedExpiry_ReturnBothTypes)
{
    // Expired medicine
    service->createMedicine(createExpiredMedicine("MED001", "Expired"));

    // Expiring soon medicine
    Medicine expiring = createTestMedicine("MED002", "Expiring");
    expiring.setExpiryDate(getDateDaysFromNow(15));
    service->createMedicine(expiring);

    // Valid medicine
    Medicine valid = createTestMedicine("MED003", "Valid");
    valid.setExpiryDate("2030-12-31");
    service->createMedicine(valid);

    auto alerts = service->getExpiryAlerts(30);

    EXPECT_EQ(2u, alerts.size());

    auto expiredAlert = std::find_if(alerts.begin(), alerts.end(),
                                     [](const ExpiryAlert &a)
                                     { return a.alertType == "EXPIRED"; });
    auto expiringAlert = std::find_if(alerts.begin(), alerts.end(),
                                      [](const ExpiryAlert &a)
                                      { return a.alertType == "EXPIRING_SOON"; });

    EXPECT_NE(expiredAlert, alerts.end());
    EXPECT_NE(expiringAlert, alerts.end());
}

TEST_F(MedicineServiceTest, GetExpiredMedicines_ReturnOnlyExpired)
{
    service->createMedicine(createExpiredMedicine("MED001", "Expired"));

    Medicine valid = createTestMedicine("MED002", "Valid");
    valid.setExpiryDate("2030-12-31");
    service->createMedicine(valid);

    auto result = service->getExpiredMedicines();

    EXPECT_EQ(1u, result.size());
    EXPECT_EQ("MED001", result[0].getMedicineID());
}

TEST_F(MedicineServiceTest, GetExpiringSoonMedicines_ReturnWithinThreshold)
{
    Medicine expiring = createTestMedicine("MED001", "Expiring");
    expiring.setExpiryDate(getDateDaysFromNow(15));
    service->createMedicine(expiring);

    Medicine valid = createTestMedicine("MED002", "Valid");
    valid.setExpiryDate("2030-12-31");
    service->createMedicine(valid);

    auto result = service->getExpiringSoonMedicines(30);

    EXPECT_EQ(1u, result.size());
    EXPECT_EQ("MED001", result[0].getMedicineID());
}

// ==================== Inventory Statistics Tests ====================

TEST_F(MedicineServiceTest, GetTotalInventoryValue_CalculateCorrectly)
{
    service->createMedicine(createTestMedicine("MED001", "Med1", "Cat", 10.0, 100));
    service->createMedicine(createTestMedicine("MED002", "Med2", "Cat", 20.0, 50));
    service->createMedicine(createTestMedicine("MED003", "Med3", "Cat", 5.0, 200));

    double total = service->getTotalInventoryValue();

    // 10*100 + 20*50 + 5*200 = 1000 + 1000 + 1000 = 3000
    EXPECT_DOUBLE_EQ(3000.0, total);
}

TEST_F(MedicineServiceTest, GetTotalInventoryValue_EmptyInventory_ReturnZero)
{
    double total = service->getTotalInventoryValue();

    EXPECT_DOUBLE_EQ(0.0, total);
}

TEST_F(MedicineServiceTest, GetInventoryValueByCategory_CalculateCorrectly)
{
    service->createMedicine(createTestMedicine("MED001", "Med1", "Analgesic", 10.0, 100));
    service->createMedicine(createTestMedicine("MED002", "Med2", "Analgesic", 20.0, 50));
    service->createMedicine(createTestMedicine("MED003", "Med3", "Antibiotic", 15.0, 80));

    auto result = service->getInventoryValueByCategory();

    EXPECT_EQ(2u, result.size());
    EXPECT_DOUBLE_EQ(2000.0, result["Analgesic"]);  // 10*100 + 20*50
    EXPECT_DOUBLE_EQ(1200.0, result["Antibiotic"]); // 15*80
}

TEST_F(MedicineServiceTest, GetStockCountByCategory_CalculateCorrectly)
{
    service->createMedicine(createTestMedicine("MED001", "Med1", "Analgesic", 10.0, 100));
    service->createMedicine(createTestMedicine("MED002", "Med2", "Analgesic", 20.0, 50));
    service->createMedicine(createTestMedicine("MED003", "Med3", "Antibiotic", 15.0, 80));

    auto result = service->getStockCountByCategory();

    EXPECT_EQ(2u, result.size());
    EXPECT_EQ(150, result["Analgesic"]);
    EXPECT_EQ(80, result["Antibiotic"]);
}

// ==================== Validation Tests ====================

TEST_F(MedicineServiceTest, ValidateMedicine_ValidMedicine_ReturnTrue)
{
    Medicine med = createTestMedicine("MED001", "Aspirin", "Analgesic", 10.0, 100);

    bool result = service->validateMedicine(med);

    EXPECT_TRUE(result);
}

TEST_F(MedicineServiceTest, ValidateMedicine_EmptyID_ReturnFalse)
{
    Medicine med = createTestMedicine("", "Aspirin", "Analgesic", 10.0, 100);

    bool result = service->validateMedicine(med);

    EXPECT_FALSE(result);
}

TEST_F(MedicineServiceTest, ValidateMedicine_EmptyName_ReturnFalse)
{
    Medicine med = createTestMedicine("MED001", "", "Analgesic", 10.0, 100);

    bool result = service->validateMedicine(med);

    EXPECT_FALSE(result);
}

TEST_F(MedicineServiceTest, ValidateMedicine_EmptyCategory_ReturnFalse)
{
    Medicine med("MED001", "Aspirin", "", 10.0, 100);

    bool result = service->validateMedicine(med);

    EXPECT_FALSE(result);
}

TEST_F(MedicineServiceTest, ValidateMedicine_NegativePrice_ReturnFalse)
{
    Medicine med = createTestMedicine("MED001", "Aspirin", "Analgesic", -5.0, 100);

    bool result = service->validateMedicine(med);

    EXPECT_FALSE(result);
}

TEST_F(MedicineServiceTest, ValidateMedicine_NegativeQuantity_ReturnFalse)
{
    Medicine med = createTestMedicine("MED001", "Aspirin", "Analgesic", 10.0, -1);

    bool result = service->validateMedicine(med);

    EXPECT_FALSE(result);
}

TEST_F(MedicineServiceTest, ValidateMedicine_NegativeReorderLevel_ReturnFalse)
{
    // Model accepts any value - BLL validates
    Medicine med = createTestMedicine("MED001", "Aspirin", "Analgesic", 10.0, 100);
    med.setReorderLevel(-5);  // Model accepts negative value

    // BLL should detect invalid reorder level
    bool result = service->validateMedicine(med);

    EXPECT_FALSE(result);  // BLL rejects negative reorder level
    EXPECT_EQ(-5, med.getReorderLevel());  // Model stored the value
}

TEST_F(MedicineServiceTest, ValidateMedicine_InvalidExpiryDate_ReturnFalse)
{
    // Model accepts any value - BLL validates
    Medicine med = createTestMedicine("MED001", "Aspirin", "Analgesic", 10.0, 100);
    med.setExpiryDate("invalid-date");  // Model accepts any string

    // BLL should detect invalid expiry date format
    bool result = service->validateMedicine(med);

    EXPECT_FALSE(result);  // BLL rejects invalid date format
    EXPECT_EQ("invalid-date", med.getExpiryDate());  // Model stored the value
}

TEST_F(MedicineServiceTest, ValidateMedicine_ValidExpiryDate_ReturnTrue)
{
    Medicine med = createTestMedicine("MED001", "Aspirin", "Analgesic", 10.0, 100);
    med.setExpiryDate("2025-12-31");

    bool result = service->validateMedicine(med);

    EXPECT_TRUE(result);
}

TEST_F(MedicineServiceTest, MedicineExists_ExistingMedicine_ReturnTrue)
{
    service->createMedicine(createTestMedicine("MED001", "Aspirin"));

    EXPECT_TRUE(service->medicineExists("MED001"));
}

TEST_F(MedicineServiceTest, MedicineExists_NonExistentMedicine_ReturnFalse)
{
    EXPECT_FALSE(service->medicineExists("MED999"));
}

TEST_F(MedicineServiceTest, MedicineNameExists_ExistingName_ReturnTrue)
{
    service->createMedicine(createTestMedicine("MED001", "Aspirin"));

    // Exact match
    EXPECT_TRUE(service->medicineNameExists("Aspirin"));
    // Case insensitive
    EXPECT_TRUE(service->medicineNameExists("aspirin"));
    EXPECT_TRUE(service->medicineNameExists("ASPIRIN"));
}

TEST_F(MedicineServiceTest, MedicineNameExists_WithWhitespace_ReturnTrue)
{
    // Tests the Utils::trim() functionality
    service->createMedicine(createTestMedicine("MED001", "Aspirin"));

    EXPECT_TRUE(service->medicineNameExists("  Aspirin  "));
    EXPECT_TRUE(service->medicineNameExists("Aspirin "));
    EXPECT_TRUE(service->medicineNameExists(" Aspirin"));
}

TEST_F(MedicineServiceTest, MedicineNameExists_WithExcludeID_ReturnFalse)
{
    service->createMedicine(createTestMedicine("MED001", "Aspirin"));

    // Same ID should be excluded
    EXPECT_FALSE(service->medicineNameExists("Aspirin", "MED001"));

    // Different ID should still find it
    EXPECT_TRUE(service->medicineNameExists("Aspirin", "MED002"));
}

TEST_F(MedicineServiceTest, MedicineNameExists_NonExistentName_ReturnFalse)
{
    EXPECT_FALSE(service->medicineNameExists("NonExistent"));
}

// ==================== Data Persistence Tests ====================

TEST_F(MedicineServiceTest, SaveData_Success)
{
    service->createMedicine(createTestMedicine("MED001", "Aspirin"));

    bool result = service->saveData();

    EXPECT_TRUE(result);
    EXPECT_TRUE(std::filesystem::exists(testFilePath));
}

TEST_F(MedicineServiceTest, LoadData_Success)
{
    service->createMedicine(createTestMedicine("MED001", "Aspirin"));
    service->saveData();

    MedicineService::resetInstance();
    MedicineRepository::resetInstance();

    repo = MedicineRepository::getInstance();
    repo->setFilePath(testFilePath);
    service = MedicineService::getInstance();

    bool result = service->loadData();

    EXPECT_TRUE(result);
    EXPECT_EQ(1u, service->getMedicineCount());

    auto med = service->getMedicineByID("MED001");
    ASSERT_TRUE(med.has_value());
    EXPECT_EQ("Aspirin", med->getName());
}

TEST_F(MedicineServiceTest, SaveAndLoad_PreserveData)
{
    Medicine med1 = createTestMedicine("MED001", "Aspirin", "Analgesic", 10.0, 100);
    med1.setGenericName("Acetylsalicylic Acid");
    med1.setManufacturer("Pfizer");
    med1.setExpiryDate("2025-12-31");
    med1.setReorderLevel(20);

    service->createMedicine(med1);
    service->saveData();

    MedicineService::resetInstance();
    MedicineRepository::resetInstance();

    repo = MedicineRepository::getInstance();
    repo->setFilePath(testFilePath);
    service = MedicineService::getInstance();
    service->loadData();

    auto loaded = service->getMedicineByID("MED001");

    ASSERT_TRUE(loaded.has_value());
    EXPECT_EQ("Aspirin", loaded->getName());
    EXPECT_EQ("Acetylsalicylic Acid", loaded->getGenericName());
    EXPECT_EQ("Analgesic", loaded->getCategory());
    EXPECT_EQ("Pfizer", loaded->getManufacturer());
    EXPECT_DOUBLE_EQ(10.0, loaded->getUnitPrice());
    EXPECT_EQ(100, loaded->getQuantityInStock());
    EXPECT_EQ(20, loaded->getReorderLevel());
    EXPECT_EQ("2025-12-31", loaded->getExpiryDate());
}

// ==================== Integration Tests ====================

TEST_F(MedicineServiceTest, IntegrationTest_CompleteWorkflow)
{
    // 1. Create medicines
    auto result1 = service->createMedicine("Aspirin", "Acetyl", "Analgesic", "Pfizer", 10.0, 100);
    auto result2 = service->createMedicine("Paracetamol", "Para", "Analgesic", "GSK", 8.0, 150);

    ASSERT_TRUE(result1.has_value());
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(2u, service->getMedicineCount());

    // 2. Verify default reorder level is set
    EXPECT_EQ(Constants::DEFAULT_REORDER_LEVEL, result1->getReorderLevel());

    // 3. Update stock
    EXPECT_TRUE(service->addStock(result1->getMedicineID(), 50));
    EXPECT_EQ(150, service->getStockLevel(result1->getMedicineID()));

    // 4. Remove stock
    EXPECT_TRUE(service->removeStock(result1->getMedicineID(), 30));
    EXPECT_EQ(120, service->getStockLevel(result1->getMedicineID()));

    // 5. Update medicine
    auto medicine = *service->getMedicineByID(result1->getMedicineID());
    medicine.setUnitPrice(12.0);
    EXPECT_TRUE(service->updateMedicine(medicine));

    auto updated = service->getMedicineByID(result1->getMedicineID());
    EXPECT_DOUBLE_EQ(12.0, updated->getUnitPrice());

    // 6. Search
    auto searchResults = service->searchMedicines("aspirin");
    EXPECT_EQ(1u, searchResults.size());

    // 7. Get categories
    auto categories = service->getAllCategories();
    EXPECT_EQ(1u, categories.size());
    EXPECT_EQ("Analgesic", categories[0]);

    // 8. Calculate inventory value
    double totalValue = service->getTotalInventoryValue();
    EXPECT_DOUBLE_EQ(12.0 * 120 + 8.0 * 150, totalValue);

    // 9. Delete medicine
    EXPECT_TRUE(service->deleteMedicine(result2->getMedicineID()));
    EXPECT_EQ(1u, service->getMedicineCount());
}

TEST_F(MedicineServiceTest, IntegrationTest_StockManagement)
{
    // Create medicine with LOW stock (quantity <= reorderLevel)
    Medicine med = createLowStockMedicine("MED001", "Aspirin");
    service->createMedicine(med);

    // Medicine should be in low stock
    auto lowStock = service->getLowStockMedicines();
    EXPECT_GE(lowStock.size(), 1u);

    auto alerts = service->getLowStockAlerts();
    EXPECT_GE(alerts.size(), 1u);

    auto lowAlert = std::find_if(alerts.begin(), alerts.end(),
                                 [](const StockAlert &a)
                                 { return a.medicineID == "MED001"; });
    ASSERT_NE(lowAlert, alerts.end());
    EXPECT_EQ("LOW_STOCK", lowAlert->alertType);

    // Add stock to bring above reorder level
    service->addStock("MED001", 100);
    EXPECT_EQ(105, service->getStockLevel("MED001"));

    // Should no longer be in low stock
    lowStock = service->getLowStockMedicines();
    bool stillLow = std::any_of(lowStock.begin(), lowStock.end(),
                                [](const Medicine &m)
                                { return m.getMedicineID() == "MED001"; });
    EXPECT_FALSE(stillLow);

    // Remove stock to make it out of stock
    service->removeStock("MED001", 105);
    EXPECT_EQ(0, service->getStockLevel("MED001"));

    auto outOfStock = service->getOutOfStock();
    EXPECT_EQ(1u, outOfStock.size());

    alerts = service->getLowStockAlerts();
    auto outAlert = std::find_if(alerts.begin(), alerts.end(),
                                 [](const StockAlert &a)
                                 { return a.medicineID == "MED001"; });
    ASSERT_NE(outAlert, alerts.end());
    EXPECT_EQ("OUT_OF_STOCK", outAlert->alertType);
}

TEST_F(MedicineServiceTest, IntegrationTest_ExpiryManagement)
{
    // Create medicines with different expiry states
    service->createMedicine(createExpiredMedicine("MED001", "Expired Med"));

    Medicine expiring = createTestMedicine("MED002", "Expiring Med");
    expiring.setExpiryDate(getDateDaysFromNow(15));
    service->createMedicine(expiring);

    Medicine valid = createTestMedicine("MED003", "Valid Med");
    valid.setExpiryDate("2030-12-31");
    service->createMedicine(valid);

    // Check expired
    auto expired = service->getExpiredMedicines();
    EXPECT_EQ(1u, expired.size());
    EXPECT_EQ("MED001", expired[0].getMedicineID());

    // Check expiring soon
    auto expiringSoon = service->getExpiringSoonMedicines(30);
    EXPECT_EQ(1u, expiringSoon.size());
    EXPECT_EQ("MED002", expiringSoon[0].getMedicineID());

    // Check all alerts
    auto alerts = service->getExpiryAlerts(30);
    EXPECT_EQ(2u, alerts.size());
}

/*
Build and run tests:
cd build && cmake --build . && ./HospitalTests --gtest_filter="MedicineServiceTest.*"
*/
