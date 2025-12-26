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
        // Setup test file path
        testFilePath = "test_data/test_medicines.txt";

        // Ensure test directory exists
        std::filesystem::create_directories("test_data");

        // Clear any existing test file
        if (std::filesystem::exists(testFilePath))
        {
            std::filesystem::remove(testFilePath);
        }

        // Get repository instance and set test file path
        repo = MedicineRepository::getInstance();
        repo->setFilePath(testFilePath);
        repo->clear();
        repo->load();

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

        // Remove test file
        if (std::filesystem::exists(testFilePath))
        {
            std::filesystem::remove(testFilePath);
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
        return med;
    }
};

// ==================== CRUD Operations Tests ====================

TEST_F(MedicineServiceTest, CreateMedicine_ValidMedicine_Success)
{
    // Arrange
    Medicine med = createTestMedicine("MED001", "Aspirin");

    // Act
    bool result = service->createMedicine(med);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(1, service->getMedicineCount());

    auto retrieved = service->getMedicineByID("MED001");
    ASSERT_TRUE(retrieved.has_value());
    EXPECT_EQ("Aspirin", retrieved->getName());
}

TEST_F(MedicineServiceTest, CreateMedicine_InvalidMedicine_EmptyName_Fail)
{
    // Arrange
    Medicine med = createTestMedicine("MED001", "");

    // Act
    bool result = service->createMedicine(med);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(0, service->getMedicineCount());
}

TEST_F(MedicineServiceTest, CreateMedicine_InvalidMedicine_EmptyCategory_Fail)
{
    // Arrange
    Medicine med("MED001", "Aspirin", "", 10.0, 100);

    // Act
    bool result = service->createMedicine(med);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(0, service->getMedicineCount());
}

TEST_F(MedicineServiceTest, CreateMedicine_InvalidMedicine_NegativePrice_Fail)
{
    // Arrange
    Medicine med = createTestMedicine("MED001", "Aspirin", "Antibiotic", -5.0, 100);

    // Act
    bool result = service->createMedicine(med);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(0, service->getMedicineCount());
}

TEST_F(MedicineServiceTest, CreateMedicine_InvalidMedicine_NegativeQuantity_Fail)
{
    // Arrange
    Medicine med = createTestMedicine("MED001", "Aspirin", "Antibiotic", 10.0, -1);

    // Act
    bool result = service->createMedicine(med);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(0, service->getMedicineCount());
}

TEST_F(MedicineServiceTest, CreateMedicine_DuplicateID_Fail)
{
    // Arrange
    Medicine med1 = createTestMedicine("MED001", "Aspirin");
    Medicine med2 = createTestMedicine("MED001", "Paracetamol");

    // Act
    service->createMedicine(med1);
    bool result = service->createMedicine(med2);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(1, service->getMedicineCount());
}

TEST_F(MedicineServiceTest, CreateMedicine_DuplicateNameAndManufacturer_Fail)
{
    // Arrange
    Medicine med1 = createTestMedicine("MED001", "Aspirin");
    med1.setManufacturer("Pharma Inc");

    Medicine med2 = createTestMedicine("MED002", "Aspirin");
    med2.setManufacturer("Pharma Inc");

    // Act
    service->createMedicine(med1);
    bool result = service->createMedicine(med2);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(1, service->getMedicineCount());
}

TEST_F(MedicineServiceTest, CreateMedicine_SameNameDifferentManufacturer_Success)
{
    // Arrange
    Medicine med1 = createTestMedicine("MED001", "Aspirin");
    med1.setManufacturer("Pharma A");

    Medicine med2 = createTestMedicine("MED002", "Aspirin");
    med2.setManufacturer("Pharma B");

    // Act
    bool result1 = service->createMedicine(med1);
    bool result2 = service->createMedicine(med2);

    // Assert
    EXPECT_TRUE(result1);
    EXPECT_TRUE(result2);
    EXPECT_EQ(2, service->getMedicineCount());
}

TEST_F(MedicineServiceTest, CreateMedicine_WithParameters_Success)
{
    // Act
    auto result = service->createMedicine(
        "Aspirin",
        "Acetylsalicylic Acid",
        "Analgesic",
        "Pharma Inc",
        15.50,
        200);

    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ("Aspirin", result->getName());
    EXPECT_EQ("Acetylsalicylic Acid", result->getGenericName());
    EXPECT_EQ("Analgesic", result->getCategory());
    EXPECT_EQ("Pharma Inc", result->getManufacturer());
    EXPECT_EQ(15.50, result->getUnitPrice());
    EXPECT_EQ(200, result->getQuantityInStock());
    EXPECT_EQ(1, service->getMedicineCount());
}

TEST_F(MedicineServiceTest, CreateMedicine_WithParameters_InvalidInput_Fail)
{
    // Test empty name
    auto result1 = service->createMedicine("", "Generic", "Category", "Mfr", 10.0, 100);
    EXPECT_FALSE(result1.has_value());

    // Test empty category
    auto result2 = service->createMedicine("Name", "Generic", "", "Mfr", 10.0, 100);
    EXPECT_FALSE(result2.has_value());

    // Test zero price
    auto result3 = service->createMedicine("Name", "Generic", "Category", "Mfr", 0.0, 100);
    EXPECT_FALSE(result3.has_value());

    // Test negative quantity
    auto result4 = service->createMedicine("Name", "Generic", "Category", "Mfr", 10.0, -1);
    EXPECT_FALSE(result4.has_value());

    EXPECT_EQ(0, service->getMedicineCount());
}

TEST_F(MedicineServiceTest, UpdateMedicine_ValidUpdate_Success)
{
    // Arrange
    Medicine med = createTestMedicine("MED001", "Aspirin");
    service->createMedicine(med);

    med.setName("Updated Aspirin");
    med.setUnitPrice(20.0);

    // Act
    bool result = service->updateMedicine(med);

    // Assert
    EXPECT_TRUE(result);

    auto updated = service->getMedicineByID("MED001");
    ASSERT_TRUE(updated.has_value());
    EXPECT_EQ("Updated Aspirin", updated->getName());
    EXPECT_EQ(20.0, updated->getUnitPrice());
}

TEST_F(MedicineServiceTest, UpdateMedicine_InvalidMedicine_Fail) {
    // Arrange
    Medicine med = createTestMedicine("MED001", "Aspirin");
    service->createMedicine(med);

    // Create medicine with negative price (invalid)
    Medicine invalidMed("MED001", "Aspirin", "Category", -10.0, 100);

    // Act
    bool result = service->updateMedicine(invalidMed);

    // Assert
    EXPECT_FALSE(result);
}

TEST_F(MedicineServiceTest, UpdateMedicine_NonExistentMedicine_Fail)
{
    // Arrange
    Medicine med = createTestMedicine("MED999", "NonExistent");

    // Act
    bool result = service->updateMedicine(med);

    // Assert
    EXPECT_FALSE(result);
}

TEST_F(MedicineServiceTest, UpdateMedicine_DuplicateNameManufacturer_Fail)
{
    // Arrange
    Medicine med1 = createTestMedicine("MED001", "Aspirin");
    med1.setManufacturer("Pharma A");

    Medicine med2 = createTestMedicine("MED002", "Paracetamol");
    med2.setManufacturer("Pharma B");

    service->createMedicine(med1);
    service->createMedicine(med2);

    // Try to update med2 to have same name and manufacturer as med1
    med2.setName("Aspirin");
    med2.setManufacturer("Pharma A");

    // Act
    bool result = service->updateMedicine(med2);

    // Assert
    EXPECT_FALSE(result);
}

TEST_F(MedicineServiceTest, DeleteMedicine_ExistingMedicine_Success)
{
    // Arrange
    Medicine med = createTestMedicine("MED001", "Aspirin");
    service->createMedicine(med);

    // Act
    bool result = service->deleteMedicine("MED001");

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(0, service->getMedicineCount());
    EXPECT_FALSE(service->getMedicineByID("MED001").has_value());
}

TEST_F(MedicineServiceTest, DeleteMedicine_NonExistentMedicine_Fail)
{
    // Act
    bool result = service->deleteMedicine("MED999");

    // Assert
    EXPECT_FALSE(result);
}

TEST_F(MedicineServiceTest, DeleteMedicine_EmptyID_Fail)
{
    // Act
    bool result = service->deleteMedicine("");

    // Assert
    EXPECT_FALSE(result);
}

// ==================== Query Operations Tests ====================

TEST_F(MedicineServiceTest, GetMedicineByID_ExistingMedicine_Success)
{
    // Arrange
    Medicine med = createTestMedicine("MED001", "Aspirin");
    service->createMedicine(med);

    // Act
    auto result = service->getMedicineByID("MED001");

    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ("MED001", result->getMedicineID());
    EXPECT_EQ("Aspirin", result->getName());
}

TEST_F(MedicineServiceTest, GetMedicineByID_NonExistentMedicine_ReturnEmpty)
{
    // Act
    auto result = service->getMedicineByID("MED999");

    // Assert
    EXPECT_FALSE(result.has_value());
}

TEST_F(MedicineServiceTest, GetAllMedicines_EmptyRepository_ReturnEmptyList)
{
    // Act
    auto result = service->getAllMedicines();

    // Assert
    EXPECT_TRUE(result.empty());
}

TEST_F(MedicineServiceTest, GetAllMedicines_MultipleMedicines_ReturnAll)
{
    // Arrange
    service->createMedicine(createTestMedicine("MED001", "Aspirin"));
    service->createMedicine(createTestMedicine("MED002", "Paracetamol"));
    service->createMedicine(createTestMedicine("MED003", "Ibuprofen"));

    // Act
    auto result = service->getAllMedicines();

    // Assert
    EXPECT_EQ(3, result.size());
}

TEST_F(MedicineServiceTest, GetMedicinesByCategory_ValidCategory_ReturnMatching)
{
    // Arrange
    service->createMedicine(createTestMedicine("MED001", "Aspirin", "Analgesic"));
    service->createMedicine(createTestMedicine("MED002", "Amoxicillin", "Antibiotic"));
    service->createMedicine(createTestMedicine("MED003", "Ibuprofen", "Analgesic"));

    // Act
    auto result = service->getMedicinesByCategory("Analgesic");

    // Assert
    EXPECT_EQ(2, result.size());
}

TEST_F(MedicineServiceTest, GetMedicinesByCategory_NonExistentCategory_ReturnEmpty)
{
    // Arrange
    service->createMedicine(createTestMedicine("MED001", "Aspirin", "Analgesic"));

    // Act
    auto result = service->getMedicinesByCategory("NonExistent");

    // Assert
    EXPECT_TRUE(result.empty());
}

TEST_F(MedicineServiceTest, SearchMedicines_EmptyKeyword_ReturnAll)
{
    // Arrange
    service->createMedicine(createTestMedicine("MED001", "Aspirin"));
    service->createMedicine(createTestMedicine("MED002", "Paracetamol"));

    // Act
    auto result = service->searchMedicines("");

    // Assert
    EXPECT_EQ(2, result.size());
}

TEST_F(MedicineServiceTest, SearchMedicines_ByName_ReturnMatching)
{
    // Arrange
    service->createMedicine(createTestMedicine("MED001", "Aspirin"));
    service->createMedicine(createTestMedicine("MED002", "Paracetamol"));
    service->createMedicine(createTestMedicine("MED003", "Ibuprofen"));

    // Act
    auto result = service->searchMedicines("para");

    // Assert
    EXPECT_EQ(1, result.size());
    EXPECT_EQ("Paracetamol", result[0].getName());
}

TEST_F(MedicineServiceTest, SearchMedicines_ByGenericName_ReturnMatching)
{
    // Arrange
    Medicine med = createTestMedicine("MED001", "Aspirin");
    med.setGenericName("Acetylsalicylic Acid");
    service->createMedicine(med);

    // Act
    auto result = service->searchMedicines("Acetyl");

    // Assert
    EXPECT_EQ(1, result.size());
    EXPECT_EQ("Aspirin", result[0].getName());
}

TEST_F(MedicineServiceTest, SearchMedicines_ByCategory_ReturnMatching)
{
    // Arrange
    service->createMedicine(createTestMedicine("MED001", "Aspirin", "Analgesic"));
    service->createMedicine(createTestMedicine("MED002", "Amoxicillin", "Antibiotic"));

    // Act
    auto result = service->searchMedicines("anal");

    // Assert
    EXPECT_EQ(1, result.size());
    EXPECT_EQ("Aspirin", result[0].getName());
}

TEST_F(MedicineServiceTest, SearchMedicines_ByManufacturer_ReturnMatching)
{
    // Arrange
    Medicine med1 = createTestMedicine("MED001", "Aspirin");
    med1.setManufacturer("Pfizer");

    Medicine med2 = createTestMedicine("MED002", "Paracetamol");
    med2.setManufacturer("GSK");

    service->createMedicine(med1);
    service->createMedicine(med2);

    // Act
    auto result = service->searchMedicines("pfizer");

    // Assert
    EXPECT_EQ(1, result.size());
    EXPECT_EQ("Aspirin", result[0].getName());
}

TEST_F(MedicineServiceTest, SearchMedicines_NoMatch_ReturnEmpty)
{
    // Arrange
    service->createMedicine(createTestMedicine("MED001", "Aspirin"));

    // Act
    auto result = service->searchMedicines("XYZ123");

    // Assert
    EXPECT_TRUE(result.empty());
}

TEST_F(MedicineServiceTest, GetAllCategories_ReturnUniqueSorted)
{
    // Arrange
    service->createMedicine(createTestMedicine("MED001", "Med1", "Analgesic"));
    service->createMedicine(createTestMedicine("MED002", "Med2", "Antibiotic"));
    service->createMedicine(createTestMedicine("MED003", "Med3", "Analgesic"));
    service->createMedicine(createTestMedicine("MED004", "Med4", "Antiviral"));

    // Act
    auto result = service->getAllCategories();

    // Assert
    EXPECT_EQ(3, result.size());
    EXPECT_EQ("Analgesic", result[0]);
    EXPECT_EQ("Antibiotic", result[1]);
    EXPECT_EQ("Antiviral", result[2]);
}

TEST_F(MedicineServiceTest, GetAllManufacturers_ReturnUniqueSorted)
{
    // Arrange
    Medicine med1 = createTestMedicine("MED001", "Med1");
    med1.setManufacturer("Pfizer");

    Medicine med2 = createTestMedicine("MED002", "Med2");
    med2.setManufacturer("GSK");

    Medicine med3 = createTestMedicine("MED003", "Med3");
    med3.setManufacturer("Pfizer");

    service->createMedicine(med1);
    service->createMedicine(med2);
    service->createMedicine(med3);

    // Act
    auto result = service->getAllManufacturers();

    // Assert
    EXPECT_EQ(2, result.size());
    EXPECT_EQ("GSK", result[0]);
    EXPECT_EQ("Pfizer", result[1]);
}

TEST_F(MedicineServiceTest, GetMedicineCount_ReturnCorrectCount)
{
    // Arrange
    EXPECT_EQ(0, service->getMedicineCount());

    service->createMedicine(createTestMedicine("MED001", "Med1"));
    EXPECT_EQ(1, service->getMedicineCount());

    service->createMedicine(createTestMedicine("MED002", "Med2"));
    EXPECT_EQ(2, service->getMedicineCount());

    service->deleteMedicine("MED001");
    EXPECT_EQ(1, service->getMedicineCount());
}

// ==================== Stock Management Tests ====================

TEST_F(MedicineServiceTest, AddStock_ValidQuantity_Success)
{
    // Arrange
    Medicine med = createTestMedicine("MED001", "Aspirin", "Analgesic", 10.0, 100);
    service->createMedicine(med);

    // Act
    bool result = service->addStock("MED001", 50);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(150, service->getStockLevel("MED001"));
}

TEST_F(MedicineServiceTest, AddStock_ZeroQuantity_Fail)
{
    // Arrange
    Medicine med = createTestMedicine("MED001", "Aspirin", "Analgesic", 10.0, 100);
    service->createMedicine(med);

    // Act
    bool result = service->addStock("MED001", 0);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(100, service->getStockLevel("MED001"));
}

TEST_F(MedicineServiceTest, AddStock_NegativeQuantity_Fail)
{
    // Arrange
    Medicine med = createTestMedicine("MED001", "Aspirin", "Analgesic", 10.0, 100);
    service->createMedicine(med);

    // Act
    bool result = service->addStock("MED001", -10);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(100, service->getStockLevel("MED001"));
}

TEST_F(MedicineServiceTest, AddStock_NonExistentMedicine_Fail)
{
    // Act
    bool result = service->addStock("MED999", 50);

    // Assert
    EXPECT_FALSE(result);
}

TEST_F(MedicineServiceTest, RemoveStock_ValidQuantity_Success)
{
    // Arrange
    Medicine med = createTestMedicine("MED001", "Aspirin", "Analgesic", 10.0, 100);
    service->createMedicine(med);

    // Act
    bool result = service->removeStock("MED001", 30);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(70, service->getStockLevel("MED001"));
}

TEST_F(MedicineServiceTest, RemoveStock_ExactQuantity_Success)
{
    // Arrange
    Medicine med = createTestMedicine("MED001", "Aspirin", "Analgesic", 10.0, 100);
    service->createMedicine(med);

    // Act
    bool result = service->removeStock("MED001", 100);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(0, service->getStockLevel("MED001"));
}

TEST_F(MedicineServiceTest, RemoveStock_InsufficientStock_Fail)
{
    // Arrange
    Medicine med = createTestMedicine("MED001", "Aspirin", "Analgesic", 10.0, 100);
    service->createMedicine(med);

    // Act
    bool result = service->removeStock("MED001", 150);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(100, service->getStockLevel("MED001"));
}

TEST_F(MedicineServiceTest, RemoveStock_ZeroQuantity_Fail)
{
    // Arrange
    Medicine med = createTestMedicine("MED001", "Aspirin", "Analgesic", 10.0, 100);
    service->createMedicine(med);

    // Act
    bool result = service->removeStock("MED001", 0);

    // Assert
    EXPECT_FALSE(result);
}

TEST_F(MedicineServiceTest, RemoveStock_NegativeQuantity_Fail)
{
    // Arrange
    Medicine med = createTestMedicine("MED001", "Aspirin", "Analgesic", 10.0, 100);
    service->createMedicine(med);

    // Act
    bool result = service->removeStock("MED001", -10);

    // Assert
    EXPECT_FALSE(result);
}

TEST_F(MedicineServiceTest, RemoveStock_NonExistentMedicine_Fail)
{
    // Act
    bool result = service->removeStock("MED999", 10);

    // Assert
    EXPECT_FALSE(result);
}

TEST_F(MedicineServiceTest, HasEnoughStock_SufficientStock_ReturnTrue)
{
    // Arrange
    Medicine med = createTestMedicine("MED001", "Aspirin", "Analgesic", 10.0, 100);
    service->createMedicine(med);

    // Act & Assert
    EXPECT_TRUE(service->hasEnoughStock("MED001", 50));
    EXPECT_TRUE(service->hasEnoughStock("MED001", 100));
}

TEST_F(MedicineServiceTest, HasEnoughStock_InsufficientStock_ReturnFalse)
{
    // Arrange
    Medicine med = createTestMedicine("MED001", "Aspirin", "Analgesic", 10.0, 100);
    service->createMedicine(med);

    // Act & Assert
    EXPECT_FALSE(service->hasEnoughStock("MED001", 150));
}

TEST_F(MedicineServiceTest, HasEnoughStock_NonExistentMedicine_ReturnFalse)
{
    // Act & Assert
    EXPECT_FALSE(service->hasEnoughStock("MED999", 10));
}

TEST_F(MedicineServiceTest, GetStockLevel_ExistingMedicine_ReturnLevel)
{
    // Arrange
    Medicine med = createTestMedicine("MED001", "Aspirin", "Analgesic", 10.0, 100);
    service->createMedicine(med);

    // Act
    int level = service->getStockLevel("MED001");

    // Assert
    EXPECT_EQ(100, level);
}

TEST_F(MedicineServiceTest, GetStockLevel_NonExistentMedicine_ReturnNegative)
{
    // Act
    int level = service->getStockLevel("MED999");

    // Assert
    EXPECT_EQ(-1, level);
}

TEST_F(MedicineServiceTest, UpdateReorderLevel_ValidLevel_Success)
{
    // Arrange
    Medicine med = createTestMedicine("MED001", "Aspirin", "Analgesic", 10.0, 100);
    service->createMedicine(med);

    // Act
    bool result = service->updateReorderLevel("MED001", 20);

    // Assert
    EXPECT_TRUE(result);

    auto updated = service->getMedicineByID("MED001");
    ASSERT_TRUE(updated.has_value());
    EXPECT_EQ(20, updated->getReorderLevel());
}

TEST_F(MedicineServiceTest, UpdateReorderLevel_ZeroLevel_Success)
{
    // Arrange
    Medicine med = createTestMedicine("MED001", "Aspirin", "Analgesic", 10.0, 100);
    service->createMedicine(med);

    // Act
    bool result = service->updateReorderLevel("MED001", 0);

    // Assert
    EXPECT_TRUE(result);

    auto updated = service->getMedicineByID("MED001");
    ASSERT_TRUE(updated.has_value());
    EXPECT_EQ(0, updated->getReorderLevel());
}

TEST_F(MedicineServiceTest, UpdateReorderLevel_NegativeLevel_Fail)
{
    // Arrange
    Medicine med = createTestMedicine("MED001", "Aspirin", "Analgesic", 10.0, 100);
    service->createMedicine(med);

    // Act
    bool result = service->updateReorderLevel("MED001", -5);

    // Assert
    EXPECT_FALSE(result);
}

TEST_F(MedicineServiceTest, UpdateReorderLevel_NonExistentMedicine_Fail)
{
    // Act
    bool result = service->updateReorderLevel("MED999", 10);

    // Assert
    EXPECT_FALSE(result);
}

// ==================== Stock Alerts Tests ====================

TEST_F(MedicineServiceTest, GetLowStockAlerts_MixedStock_ReturnLowAndOut)
{
    // Arrange
    Medicine med1 = createTestMedicine("MED001", "Med1", "Cat", 10.0, 5);
    med1.setReorderLevel(10);

    Medicine med2 = createTestMedicine("MED002", "Med2", "Cat", 10.0, 0);
    med2.setReorderLevel(5);

    Medicine med3 = createTestMedicine("MED003", "Med3", "Cat", 10.0, 100);
    med3.setReorderLevel(10);

    service->createMedicine(med1);
    service->createMedicine(med2);
    service->createMedicine(med3);

    // Act
    auto alerts = service->getLowStockAlerts();

    // Assert
    EXPECT_EQ(2, alerts.size());

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
    // Arrange
    Medicine med = createTestMedicine("MED001", "Med1", "Cat", 10.0, 100);
    med.setReorderLevel(10);
    service->createMedicine(med);

    // Act
    auto alerts = service->getLowStockAlerts();

    // Assert
    EXPECT_TRUE(alerts.empty());
}

TEST_F(MedicineServiceTest, GetOutOfStock_ReturnOnlyZeroStock)
{
    // Arrange
    service->createMedicine(createTestMedicine("MED001", "Med1", "Cat", 10.0, 0));
    service->createMedicine(createTestMedicine("MED002", "Med2", "Cat", 10.0, 5));
    service->createMedicine(createTestMedicine("MED003", "Med3", "Cat", 10.0, 100));

    // Act
    auto result = service->getOutOfStock();

    // Assert
    EXPECT_EQ(1, result.size());
    EXPECT_EQ("MED001", result[0].getMedicineID());
}

TEST_F(MedicineServiceTest, GetLowStockMedicines_ReturnBelowReorderLevel)
{
    // Arrange
    Medicine med1 = createTestMedicine("MED001", "Med1", "Cat", 10.0, 5);
    med1.setReorderLevel(10);

    Medicine med2 = createTestMedicine("MED002", "Med2", "Cat", 10.0, 15);
    med2.setReorderLevel(10);

    service->createMedicine(med1);
    service->createMedicine(med2);

    // Act
    auto result = service->getLowStockMedicines();

    // Assert
    EXPECT_EQ(1, result.size());
    EXPECT_EQ("MED001", result[0].getMedicineID());
}

// ==================== Expiry Alerts Tests ====================

TEST_F(MedicineServiceTest, GetExpiryAlerts_ExpiredMedicines_ReturnExpiredAlerts)
{
    // Arrange
    Medicine med = createTestMedicine("MED001", "Aspirin");
    med.setExpiryDate("2020-01-01");
    service->createMedicine(med);

    // Act
    auto alerts = service->getExpiryAlerts(30);

    // Assert
    ASSERT_EQ(1, alerts.size());
    EXPECT_EQ("EXPIRED", alerts[0].alertType);
    EXPECT_EQ("MED001", alerts[0].medicineID);
}

TEST_F(MedicineServiceTest, GetExpiryAlerts_ExpiringSoonMedicines_ReturnExpiringSoonAlerts)
{
    // Arrange
    Medicine med = createTestMedicine("MED001", "Aspirin");

    // Set expiry date to 15 days from now
    auto currentDate = Utils::getCurrentDate();
    int year = std::stoi(currentDate.substr(0, 4));
    int month = std::stoi(currentDate.substr(5, 2));
    int day = std::stoi(currentDate.substr(8, 2)) + 15;

    if (day > 28)
    {
        day = day - 28;
        month++;
        if (month > 12)
        {
            month = 1;
            year++;
        }
    }

    std::string expiryDate = std::to_string(year) + "-" +
                             (month < 10 ? "0" : "") + std::to_string(month) + "-" +
                             (day < 10 ? "0" : "") + std::to_string(day);

    med.setExpiryDate(expiryDate);
    service->createMedicine(med);

    // Act
    auto alerts = service->getExpiryAlerts(30);

    // Assert
    ASSERT_EQ(1, alerts.size());
    EXPECT_EQ("EXPIRING_SOON", alerts[0].alertType);
    EXPECT_EQ("MED001", alerts[0].medicineID);
}

TEST_F(MedicineServiceTest, GetExpiryAlerts_NoExpiryIssues_ReturnEmpty)
{
    // Arrange
    Medicine med = createTestMedicine("MED001", "Aspirin");
    med.setExpiryDate("2030-12-31");
    service->createMedicine(med);

    // Act
    auto alerts = service->getExpiryAlerts(30);

    // Assert
    EXPECT_TRUE(alerts.empty());
}

TEST_F(MedicineServiceTest, GetExpiryAlerts_MixedExpiry_ReturnBothTypes)
{
    // Arrange
    Medicine expired = createTestMedicine("MED001", "Expired");
    expired.setExpiryDate("2020-01-01");

    Medicine expiring = createTestMedicine("MED002", "Expiring");
    auto currentDate = Utils::getCurrentDate();
    int year = std::stoi(currentDate.substr(0, 4));
    int month = std::stoi(currentDate.substr(5, 2));
    int day = std::stoi(currentDate.substr(8, 2)) + 15;
    if (day > 28)
    {
        day = day - 28;
        month++;
        if (month > 12)
        {
            month = 1;
            year++;
        }
    }
    std::string expiryDate = std::to_string(year) + "-" +
                             (month < 10 ? "0" : "") + std::to_string(month) + "-" +
                             (day < 10 ? "0" : "") + std::to_string(day);
    expiring.setExpiryDate(expiryDate);

    Medicine valid = createTestMedicine("MED003", "Valid");
    valid.setExpiryDate("2030-12-31");

    service->createMedicine(expired);
    service->createMedicine(expiring);
    service->createMedicine(valid);

    // Act
    auto alerts = service->getExpiryAlerts(30);

    // Assert
    EXPECT_EQ(2, alerts.size());

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
    // Arrange
    Medicine expired = createTestMedicine("MED001", "Expired");
    expired.setExpiryDate("2020-01-01");

    Medicine valid = createTestMedicine("MED002", "Valid");
    valid.setExpiryDate("2030-12-31");

    service->createMedicine(expired);
    service->createMedicine(valid);

    // Act
    auto result = service->getExpiredMedicines();

    // Assert
    EXPECT_EQ(1, result.size());
    EXPECT_EQ("MED001", result[0].getMedicineID());
}

TEST_F(MedicineServiceTest, GetExpiringSoonMedicines_ReturnWithinThreshold)
{
    // Arrange
    Medicine expiring = createTestMedicine("MED001", "Expiring");
    auto currentDate = Utils::getCurrentDate();
    int year = std::stoi(currentDate.substr(0, 4));
    int month = std::stoi(currentDate.substr(5, 2));
    int day = std::stoi(currentDate.substr(8, 2)) + 15;
    if (day > 28)
    {
        day = day - 28;
        month++;
        if (month > 12)
        {
            month = 1;
            year++;
        }
    }
    std::string expiryDate = std::to_string(year) + "-" +
                             (month < 10 ? "0" : "") + std::to_string(month) + "-" +
                             (day < 10 ? "0" : "") + std::to_string(day);
    expiring.setExpiryDate(expiryDate);

    Medicine valid = createTestMedicine("MED002", "Valid");
    valid.setExpiryDate("2030-12-31");

    service->createMedicine(expiring);
    service->createMedicine(valid);

    // Act
    auto result = service->getExpiringSoonMedicines(30);

    // Assert
    EXPECT_EQ(1, result.size());
    EXPECT_EQ("MED001", result[0].getMedicineID());
}

// ==================== Inventory Statistics Tests ====================

TEST_F(MedicineServiceTest, GetTotalInventoryValue_CalculateCorrectly)
{
    // Arrange
    service->createMedicine(createTestMedicine("MED001", "Med1", "Cat", 10.0, 100));
    service->createMedicine(createTestMedicine("MED002", "Med2", "Cat", 20.0, 50));
    service->createMedicine(createTestMedicine("MED003", "Med3", "Cat", 5.0, 200));

    // Act
    double total = service->getTotalInventoryValue();

    // Assert
    EXPECT_DOUBLE_EQ(3000.0, total);
}

TEST_F(MedicineServiceTest, GetTotalInventoryValue_EmptyInventory_ReturnZero)
{
    // Act
    double total = service->getTotalInventoryValue();

    // Assert
    EXPECT_DOUBLE_EQ(0.0, total);
}

TEST_F(MedicineServiceTest, GetInventoryValueByCategory_CalculateCorrectly)
{
    // Arrange
    service->createMedicine(createTestMedicine("MED001", "Med1", "Analgesic", 10.0, 100));
    service->createMedicine(createTestMedicine("MED002", "Med2", "Analgesic", 20.0, 50));
    service->createMedicine(createTestMedicine("MED003", "Med3", "Antibiotic", 15.0, 80));

    // Act
    auto result = service->getInventoryValueByCategory();

    // Assert
    EXPECT_EQ(2, result.size());
    EXPECT_DOUBLE_EQ(2000.0, result["Analgesic"]);
    EXPECT_DOUBLE_EQ(1200.0, result["Antibiotic"]);
}

TEST_F(MedicineServiceTest, GetStockCountByCategory_CalculateCorrectly)
{
    // Arrange
    service->createMedicine(createTestMedicine("MED001", "Med1", "Analgesic", 10.0, 100));
    service->createMedicine(createTestMedicine("MED002", "Med2", "Analgesic", 20.0, 50));
    service->createMedicine(createTestMedicine("MED003", "Med3", "Antibiotic", 15.0, 80));

    // Act
    auto result = service->getStockCountByCategory();

    // Assert
    EXPECT_EQ(2, result.size());
    EXPECT_EQ(150, result["Analgesic"]);
    EXPECT_EQ(80, result["Antibiotic"]);
}

// ==================== Validation Tests ====================

TEST_F(MedicineServiceTest, ValidateMedicine_ValidMedicine_ReturnTrue)
{
    // Arrange
    Medicine med = createTestMedicine("MED001", "Aspirin", "Analgesic", 10.0, 100);

    // Act
    bool result = service->validateMedicine(med);

    // Assert
    EXPECT_TRUE(result);
}

TEST_F(MedicineServiceTest, ValidateMedicine_EmptyID_ReturnFalse)
{
    // Arrange
    Medicine med = createTestMedicine("", "Aspirin", "Analgesic", 10.0, 100);

    // Act
    bool result = service->validateMedicine(med);

    // Assert
    EXPECT_FALSE(result);
}

TEST_F(MedicineServiceTest, ValidateMedicine_EmptyName_ReturnFalse)
{
    // Arrange
    Medicine med = createTestMedicine("MED001", "", "Analgesic", 10.0, 100);

    // Act
    bool result = service->validateMedicine(med);

    // Assert
    EXPECT_FALSE(result);
}

TEST_F(MedicineServiceTest, ValidateMedicine_EmptyCategory_ReturnFalse)
{
    // Arrange
    Medicine med("MED001", "Aspirin", "", 10.0, 100);

    // Act
    bool result = service->validateMedicine(med);

    // Assert
    EXPECT_FALSE(result);
}

TEST_F(MedicineServiceTest, ValidateMedicine_NegativePrice_ReturnFalse)
{
    // Arrange
    Medicine med = createTestMedicine("MED001", "Aspirin", "Analgesic", -5.0, 100);

    // Act
    bool result = service->validateMedicine(med);

    // Assert
    EXPECT_FALSE(result);
}

TEST_F(MedicineServiceTest, ValidateMedicine_NegativeQuantity_ReturnFalse)
{
    // Arrange
    Medicine med = createTestMedicine("MED001", "Aspirin", "Analgesic", 10.0, -1);

    // Act
    bool result = service->validateMedicine(med);

    // Assert
    EXPECT_FALSE(result);
}

TEST_F(MedicineServiceTest, ValidateMedicine_SetterProtection_ReorderLevel)
{
    // Test that setter protects against negative reorder level
    Medicine med = createTestMedicine("MED001", "Aspirin", "Analgesic", 10.0, 100);
    med.setReorderLevel(10);

    // Try to set negative (setter should reject it)
    med.setReorderLevel(-5);

    // Assert - Medicine should still be valid with old value
    EXPECT_TRUE(service->validateMedicine(med));
    EXPECT_EQ(10, med.getReorderLevel());  // Setter rejected -5, kept 10
}

TEST_F(MedicineServiceTest, ValidateMedicine_SetterProtection_ExpiryDate)
{
    // Test that setter protects against invalid expiry date
    Medicine med = createTestMedicine("MED001", "Aspirin", "Analgesic", 10.0, 100);

    // setExpiryDate will reject invalid date
    med.setExpiryDate("invalid-date");  // Setter rejects this

    // Assert - Medicine is still valid because setter protected it
    EXPECT_TRUE(service->validateMedicine(med));
    EXPECT_TRUE(med.getExpiryDate().empty());  // Date is still empty
}

TEST_F(MedicineServiceTest, ValidateMedicine_ValidExpiryDate_ReturnTrue)
{
    // Arrange
    Medicine med = createTestMedicine("MED001", "Aspirin", "Analgesic", 10.0, 100);
    med.setExpiryDate("2025-12-31");

    // Act
    bool result = service->validateMedicine(med);

    // Assert
    EXPECT_TRUE(result);
}

TEST_F(MedicineServiceTest, MedicineExists_ExistingMedicine_ReturnTrue)
{
    // Arrange
    service->createMedicine(createTestMedicine("MED001", "Aspirin"));

    // Act & Assert
    EXPECT_TRUE(service->medicineExists("MED001"));
}

TEST_F(MedicineServiceTest, MedicineExists_NonExistentMedicine_ReturnFalse)
{
    // Act & Assert
    EXPECT_FALSE(service->medicineExists("MED999"));
}

TEST_F(MedicineServiceTest, MedicineNameExists_ExistingName_ReturnTrue)
{
    // Arrange
    service->createMedicine(createTestMedicine("MED001", "Aspirin"));

    // Act & Assert
    EXPECT_TRUE(service->medicineNameExists("Aspirin"));
    EXPECT_TRUE(service->medicineNameExists("aspirin"));
}

TEST_F(MedicineServiceTest, MedicineNameExists_WithExcludeID_ReturnFalse)
{
    // Arrange
    service->createMedicine(createTestMedicine("MED001", "Aspirin"));

    // Act & Assert
    EXPECT_FALSE(service->medicineNameExists("Aspirin", "MED001"));
}

TEST_F(MedicineServiceTest, MedicineNameExists_NonExistentName_ReturnFalse)
{
    // Act & Assert
    EXPECT_FALSE(service->medicineNameExists("NonExistent"));
}

// ==================== Data Persistence Tests ====================

TEST_F(MedicineServiceTest, SaveData_Success)
{
    // Arrange
    service->createMedicine(createTestMedicine("MED001", "Aspirin"));

    // Act
    bool result = service->saveData();

    // Assert
    EXPECT_TRUE(result);
    EXPECT_TRUE(std::filesystem::exists(testFilePath));
}

TEST_F(MedicineServiceTest, LoadData_Success)
{
    // Arrange
    service->createMedicine(createTestMedicine("MED001", "Aspirin"));
    service->saveData();

    MedicineService::resetInstance();
    MedicineRepository::resetInstance();

    repo = MedicineRepository::getInstance();
    repo->setFilePath(testFilePath);
    service = MedicineService::getInstance();

    // Act
    bool result = service->loadData();

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(1, service->getMedicineCount());

    auto med = service->getMedicineByID("MED001");
    ASSERT_TRUE(med.has_value());
    EXPECT_EQ("Aspirin", med->getName());
}

TEST_F(MedicineServiceTest, SaveAndLoad_PreserveData)
{
    // Arrange
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

    // Act
    auto loaded = service->getMedicineByID("MED001");

    // Assert
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
    EXPECT_EQ(2, service->getMedicineCount());

    // 2. Update stock
    EXPECT_TRUE(service->addStock(result1->getMedicineID(), 50));
    EXPECT_EQ(150, service->getStockLevel(result1->getMedicineID()));

    // 3. Remove stock
    EXPECT_TRUE(service->removeStock(result1->getMedicineID(), 30));
    EXPECT_EQ(120, service->getStockLevel(result1->getMedicineID()));

    // 4. Update medicine
    auto medicine = *service->getMedicineByID(result1->getMedicineID());
    medicine.setUnitPrice(12.0);
    EXPECT_TRUE(service->updateMedicine(medicine));

    auto updated = service->getMedicineByID(result1->getMedicineID());
    EXPECT_DOUBLE_EQ(12.0, updated->getUnitPrice());

    // 5. Search
    auto searchResults = service->searchMedicines("aspirin");
    EXPECT_EQ(1, searchResults.size());

    // 6. Get categories
    auto categories = service->getAllCategories();
    EXPECT_EQ(1, categories.size());
    EXPECT_EQ("Analgesic", categories[0]);

    // 7. Calculate inventory value
    double totalValue = service->getTotalInventoryValue();
    EXPECT_DOUBLE_EQ(12.0 * 120 + 8.0 * 150, totalValue);

    // 8. Delete medicine
    EXPECT_TRUE(service->deleteMedicine(result2->getMedicineID()));
    EXPECT_EQ(1, service->getMedicineCount());
}

TEST_F(MedicineServiceTest, IntegrationTest_StockManagement) {
    // Create medicine with LOW stock (quantity <= reorderLevel)
    Medicine med = createTestMedicine("MED001", "Aspirin", "Analgesic", 10.0, 25);
    med.setReorderLevel(30);
    service->createMedicine(med);

    // Medicine should be in low stock
    auto lowStock = service->getLowStockMedicines();
    EXPECT_EQ(1, lowStock.size());

    auto alerts = service->getLowStockAlerts();
    EXPECT_EQ(1, alerts.size());
    EXPECT_EQ("LOW_STOCK", alerts[0].alertType);

    // Add stock to bring above reorder level
    service->addStock("MED001", 100);
    EXPECT_EQ(125, service->getStockLevel("MED001"));

    // Should no longer be in low stock
    lowStock = service->getLowStockMedicines();
    EXPECT_TRUE(lowStock.empty());

    // Remove stock to make it out of stock
    service->removeStock("MED001", 125);
    EXPECT_EQ(0, service->getStockLevel("MED001"));

    auto outOfStock = service->getOutOfStock();
    EXPECT_EQ(1, outOfStock.size());

    alerts = service->getLowStockAlerts();
    EXPECT_EQ(1, alerts.size());
    EXPECT_EQ("OUT_OF_STOCK", alerts[0].alertType);
}

/*
./HospitalTests --gtest_filter="MedicineServiceTest.*"
*/
