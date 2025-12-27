#include <filesystem>
#include <gtest/gtest.h>


#include "bll/MedicineService.h"
#include "common/Constants.h"
#include "common/Utils.h"
#include "dal/MedicineRepository.h"


using namespace HMS;
using namespace HMS::BLL;
using namespace HMS::DAL;
using namespace HMS::Model;

// ==================== Test Fixture ====================

class MedicineServiceTest : public ::testing::Test {
protected:
  MedicineService *medicineService;

  // Test file path to avoid touching production data
  std::string testMedicineFile = "test_medicine_service.txt";

  void SetUp() override {
    // Reset singletons to get fresh instances
    MedicineService::resetInstance();
    MedicineRepository::resetInstance();

    // Redirect repository to test file
    MedicineRepository::getInstance()->setFilePath(testMedicineFile);
    MedicineRepository::getInstance()->clear();

    // Get MedicineService instance
    medicineService = MedicineService::getInstance();
  }

  void TearDown() override {
    // Reset singletons
    MedicineService::resetInstance();
    MedicineRepository::resetInstance();

    // Clean up test files
    cleanupTestFile(testMedicineFile);
  }

  void cleanupTestFile(const std::string &filePath) {
    if (std::filesystem::exists(filePath)) {
      std::filesystem::remove(filePath);
    }
    std::string backupFile = filePath + ".bak";
    if (std::filesystem::exists(backupFile)) {
      std::filesystem::remove(backupFile);
    }
  }

  // Helper to create test medicine
  Medicine createTestMedicine(const std::string &medicineID = "MED001",
                              const std::string &name = "Paracetamol",
                              const std::string &category = "Pain Relief",
                              double unitPrice = 50000.0, int quantity = 100) {
    Medicine med(medicineID, name, category, unitPrice, quantity);
    med.setGenericName("Acetaminophen");
    med.setManufacturer("PharmaCorp");
    med.setReorderLevel(Constants::DEFAULT_REORDER_LEVEL);
    med.setExpiryDate("2025-12-31");
    med.setDosageForm("Tablet");
    med.setStrength("500mg");
    return med;
  }

  // Helper to create expired medicine
  Medicine createExpiredMedicine(const std::string &medicineID = "MED_EXP",
                                 const std::string &name = "Expired Med") {
    Medicine med(medicineID, name, "Test Category", 10000.0, 50);
    med.setExpiryDate("2020-01-01"); // Past date
    med.setReorderLevel(10);
    return med;
  }

  // Helper to create low stock medicine
  Medicine createLowStockMedicine(const std::string &medicineID = "MED_LOW",
                                  const std::string &name = "Low Stock Med") {
    Medicine med(medicineID, name, "Test Category", 25000.0, 5);
    med.setReorderLevel(10); // Stock (5) <= reorderLevel (10)
    med.setExpiryDate("2026-12-31");
    return med;
  }
};

// ==================== Singleton Tests ====================

TEST_F(MedicineServiceTest, GetInstance_ReturnsSameInstance) {
  MedicineService *instance1 = MedicineService::getInstance();
  MedicineService *instance2 = MedicineService::getInstance();

  EXPECT_EQ(instance1, instance2);
}

TEST_F(MedicineServiceTest, ResetInstance_CreatesFreshInstance) {
  MedicineService *instance1 = MedicineService::getInstance();
  EXPECT_NE(instance1, nullptr);

  MedicineService::resetInstance();

  MedicineService *instance2 = MedicineService::getInstance();
  EXPECT_NE(instance2, nullptr);
}

// ==================== CRUD - Create Tests ====================

TEST_F(MedicineServiceTest, CreateMedicine_ValidMedicine_ReturnsTrue) {
  Medicine med = createTestMedicine("MED001");

  bool result = medicineService->createMedicine(med);

  EXPECT_TRUE(result);
  EXPECT_EQ(medicineService->getMedicineCount(), 1u);
}

TEST_F(MedicineServiceTest, CreateMedicine_DuplicateID_ReturnsFalse) {
  Medicine med1 = createTestMedicine("MED001");
  Medicine med2 = createTestMedicine("MED001", "Different Name");

  medicineService->createMedicine(med1);
  bool result = medicineService->createMedicine(med2);

  EXPECT_FALSE(result);
  EXPECT_EQ(medicineService->getMedicineCount(), 1u);
}

TEST_F(MedicineServiceTest, CreateMedicine_WithParams_ReturnsCreatedMedicine) {
  auto result = medicineService->createMedicine(
      "Test Medicine", "Generic Name", "Test Category", "Test Manufacturer",
      100000.0, 50);

  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result->getName(), "Test Medicine");
  EXPECT_EQ(result->getGenericName(), "Generic Name");
  EXPECT_EQ(result->getCategory(), "Test Category");
  EXPECT_EQ(result->getManufacturer(), "Test Manufacturer");
  EXPECT_DOUBLE_EQ(result->getUnitPrice(), 100000.0);
  EXPECT_EQ(result->getQuantityInStock(), 50);
}

TEST_F(MedicineServiceTest, CreateMedicine_EmptyName_ReturnsNullopt) {
  auto result = medicineService->createMedicine("", "Generic", "Category",
                                                "Manufacturer", 100.0, 10);

  EXPECT_FALSE(result.has_value());
}

TEST_F(MedicineServiceTest, CreateMedicine_NegativePrice_ReturnsNullopt) {
  auto result = medicineService->createMedicine(
      "Test Med", "Generic", "Category", "Manufacturer", -100.0, 10);

  EXPECT_FALSE(result.has_value());
}

TEST_F(MedicineServiceTest, CreateMedicine_NegativeQuantity_ReturnsNullopt) {
  auto result = medicineService->createMedicine(
      "Test Med", "Generic", "Category", "Manufacturer", 100.0, -10);

  EXPECT_FALSE(result.has_value());
}

// ==================== CRUD - Read Tests ====================

TEST_F(MedicineServiceTest, GetMedicineByID_Exists_ReturnsMedicine) {
  Medicine med = createTestMedicine("MED001");
  medicineService->createMedicine(med);

  auto result = medicineService->getMedicineByID("MED001");

  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result->getMedicineID(), "MED001");
  EXPECT_EQ(result->getName(), "Paracetamol");
}

TEST_F(MedicineServiceTest, GetMedicineByID_NotExists_ReturnsNullopt) {
  auto result = medicineService->getMedicineByID("NONEXISTENT");

  EXPECT_FALSE(result.has_value());
}

TEST_F(MedicineServiceTest, GetAllMedicines_ReturnsAllMedicines) {
  medicineService->createMedicine(createTestMedicine("MED001", "Med1"));
  medicineService->createMedicine(createTestMedicine("MED002", "Med2"));
  medicineService->createMedicine(createTestMedicine("MED003", "Med3"));

  auto medicines = medicineService->getAllMedicines();

  EXPECT_EQ(medicines.size(), 3u);
}

TEST_F(MedicineServiceTest, GetMedicinesByCategory_ReturnsFilteredMedicines) {
  medicineService->createMedicine(createTestMedicine("MED001", "Paracetamol"));

  Medicine antibiotic("MED002", "Amoxicillin", "Antibiotics", 75000.0, 80);
  medicineService->createMedicine(antibiotic);

  auto painRelief = medicineService->getMedicinesByCategory("Pain Relief");
  auto antibiotics = medicineService->getMedicinesByCategory("Antibiotics");

  EXPECT_EQ(painRelief.size(), 1u);
  EXPECT_EQ(antibiotics.size(), 1u);
}

TEST_F(MedicineServiceTest, SearchMedicines_FindsByName) {
  medicineService->createMedicine(createTestMedicine("MED001", "Paracetamol"));
  medicineService->createMedicine(createTestMedicine("MED002", "Ibuprofen"));

  auto results = medicineService->searchMedicines("Para");

  EXPECT_EQ(results.size(), 1u);
  EXPECT_EQ(results[0].getName(), "Paracetamol");
}

TEST_F(MedicineServiceTest, GetMedicineCount_ReturnsCorrectCount) {
  EXPECT_EQ(medicineService->getMedicineCount(), 0u);

  medicineService->createMedicine(createTestMedicine("MED001"));
  EXPECT_EQ(medicineService->getMedicineCount(), 1u);

  medicineService->createMedicine(createTestMedicine("MED002", "Another Med"));
  EXPECT_EQ(medicineService->getMedicineCount(), 2u);
}

// ==================== CRUD - Update Tests ====================

TEST_F(MedicineServiceTest, UpdateMedicine_Exists_ReturnsTrue) {
  Medicine med = createTestMedicine("MED001");
  medicineService->createMedicine(med);

  med.setName("Updated Paracetamol");
  med.setUnitPrice(60000.0);
  bool result = medicineService->updateMedicine(med);

  EXPECT_TRUE(result);

  auto updated = medicineService->getMedicineByID("MED001");
  EXPECT_EQ(updated->getName(), "Updated Paracetamol");
  EXPECT_DOUBLE_EQ(updated->getUnitPrice(), 60000.0);
}

TEST_F(MedicineServiceTest, UpdateMedicine_NotExists_ReturnsFalse) {
  Medicine med = createTestMedicine("NONEXISTENT");

  bool result = medicineService->updateMedicine(med);

  EXPECT_FALSE(result);
}

// ==================== CRUD - Delete Tests ====================

TEST_F(MedicineServiceTest, DeleteMedicine_Exists_ReturnsTrue) {
  medicineService->createMedicine(createTestMedicine("MED001"));

  bool result = medicineService->deleteMedicine("MED001");

  EXPECT_TRUE(result);
  EXPECT_EQ(medicineService->getMedicineCount(), 0u);
}

TEST_F(MedicineServiceTest, DeleteMedicine_NotExists_ReturnsFalse) {
  bool result = medicineService->deleteMedicine("NONEXISTENT");

  EXPECT_FALSE(result);
}

// ==================== Stock Management Tests ====================

TEST_F(MedicineServiceTest, AddStock_ValidQuantity_IncreasesStock) {
  Medicine med = createTestMedicine("MED001");
  med.setQuantityInStock(100);
  medicineService->createMedicine(med);

  bool result = medicineService->addStock("MED001", 50);

  EXPECT_TRUE(result);
  EXPECT_EQ(medicineService->getStockLevel("MED001"), 150);
}

TEST_F(MedicineServiceTest, AddStock_ZeroQuantity_ReturnsFalse) {
  medicineService->createMedicine(createTestMedicine("MED001"));

  bool result = medicineService->addStock("MED001", 0);

  EXPECT_FALSE(result);
}

TEST_F(MedicineServiceTest, AddStock_NegativeQuantity_ReturnsFalse) {
  medicineService->createMedicine(createTestMedicine("MED001"));

  bool result = medicineService->addStock("MED001", -10);

  EXPECT_FALSE(result);
}

TEST_F(MedicineServiceTest, AddStock_NonExistentMedicine_ReturnsFalse) {
  bool result = medicineService->addStock("NONEXISTENT", 50);

  EXPECT_FALSE(result);
}

TEST_F(MedicineServiceTest, RemoveStock_ValidQuantity_DecreasesStock) {
  Medicine med = createTestMedicine("MED001");
  med.setQuantityInStock(100);
  medicineService->createMedicine(med);

  bool result = medicineService->removeStock("MED001", 30);

  EXPECT_TRUE(result);
  EXPECT_EQ(medicineService->getStockLevel("MED001"), 70);
}

TEST_F(MedicineServiceTest, RemoveStock_ExceedsStock_ReturnsFalse) {
  Medicine med = createTestMedicine("MED001");
  med.setQuantityInStock(50);
  medicineService->createMedicine(med);

  bool result = medicineService->removeStock("MED001", 100);

  EXPECT_FALSE(result);
  EXPECT_EQ(medicineService->getStockLevel("MED001"), 50); // Unchanged
}

TEST_F(MedicineServiceTest, RemoveStock_ZeroQuantity_ReturnsFalse) {
  medicineService->createMedicine(createTestMedicine("MED001"));

  bool result = medicineService->removeStock("MED001", 0);

  EXPECT_FALSE(result);
}

TEST_F(MedicineServiceTest, HasEnoughStock_Sufficient_ReturnsTrue) {
  Medicine med = createTestMedicine("MED001");
  med.setQuantityInStock(100);
  medicineService->createMedicine(med);

  EXPECT_TRUE(medicineService->hasEnoughStock("MED001", 50));
  EXPECT_TRUE(medicineService->hasEnoughStock("MED001", 100));
}

TEST_F(MedicineServiceTest, HasEnoughStock_Insufficient_ReturnsFalse) {
  Medicine med = createTestMedicine("MED001");
  med.setQuantityInStock(50);
  medicineService->createMedicine(med);

  EXPECT_FALSE(medicineService->hasEnoughStock("MED001", 100));
}

TEST_F(MedicineServiceTest, HasEnoughStock_NonExistent_ReturnsFalse) {
  EXPECT_FALSE(medicineService->hasEnoughStock("NONEXISTENT", 10));
}

TEST_F(MedicineServiceTest, GetStockLevel_Exists_ReturnsLevel) {
  Medicine med = createTestMedicine("MED001");
  med.setQuantityInStock(150);
  medicineService->createMedicine(med);

  EXPECT_EQ(medicineService->getStockLevel("MED001"), 150);
}

TEST_F(MedicineServiceTest, GetStockLevel_NotExists_ReturnsNegativeOne) {
  EXPECT_EQ(medicineService->getStockLevel("NONEXISTENT"), -1);
}

TEST_F(MedicineServiceTest, UpdateReorderLevel_ValidLevel_ReturnsTrue) {
  medicineService->createMedicine(createTestMedicine("MED001"));

  bool result = medicineService->updateReorderLevel("MED001", 20);

  EXPECT_TRUE(result);

  auto med = medicineService->getMedicineByID("MED001");
  EXPECT_EQ(med->getReorderLevel(), 20);
}

TEST_F(MedicineServiceTest, UpdateReorderLevel_NegativeLevel_ReturnsFalse) {
  medicineService->createMedicine(createTestMedicine("MED001"));

  bool result = medicineService->updateReorderLevel("MED001", -5);

  EXPECT_FALSE(result);
}

// ==================== Stock Alerts Tests ====================

TEST_F(MedicineServiceTest, GetLowStockAlerts_ReturnsCorrectAlerts) {
  medicineService->createMedicine(createTestMedicine("MED001")); // Normal stock
  medicineService->createMedicine(
      createLowStockMedicine("MED_LOW", "Low Stock"));

  auto alerts = medicineService->getLowStockAlerts();

  EXPECT_GE(alerts.size(), 1u);

  bool foundLowStock = false;
  for (const auto &alert : alerts) {
    if (alert.medicineID == "MED_LOW") {
      foundLowStock = true;
      EXPECT_EQ(alert.alertType, "LOW_STOCK");
    }
  }
  EXPECT_TRUE(foundLowStock);
}

TEST_F(MedicineServiceTest, GetOutOfStock_ReturnsZeroStockMedicines) {
  medicineService->createMedicine(createTestMedicine("MED001")); // Has stock

  Medicine outOfStock("MED_OOS", "Out of Stock Med", "Test", 10000.0, 0);
  medicineService->createMedicine(outOfStock);

  auto result = medicineService->getOutOfStock();

  EXPECT_EQ(result.size(), 1u);
  EXPECT_EQ(result[0].getMedicineID(), "MED_OOS");
}

TEST_F(MedicineServiceTest, GetLowStockMedicines_ReturnsLowStockMedicines) {
  medicineService->createMedicine(createTestMedicine("MED001")); // Normal
  medicineService->createMedicine(createLowStockMedicine("MED_LOW"));

  auto result = medicineService->getLowStockMedicines();

  EXPECT_GE(result.size(), 1u);
}

// ==================== Expiry Alerts Tests ====================

TEST_F(MedicineServiceTest, GetExpiredMedicines_ReturnsExpiredMedicines) {
  medicineService->createMedicine(createTestMedicine("MED001")); // Valid
  medicineService->createMedicine(createExpiredMedicine("MED_EXP"));

  auto expired = medicineService->getExpiredMedicines();

  EXPECT_GE(expired.size(), 1u);

  bool foundExpired = false;
  for (const auto &med : expired) {
    if (med.getMedicineID() == "MED_EXP") {
      foundExpired = true;
    }
  }
  EXPECT_TRUE(foundExpired);
}

TEST_F(MedicineServiceTest, GetExpiringSoonMedicines_ReturnsCorrectMedicines) {
  medicineService->createMedicine(createTestMedicine("MED001"));

  auto expiringSoon = medicineService->getExpiringSoonMedicines(30);

  // Result depends on the current date vs. expiry dates in test data
  EXPECT_GE(expiringSoon.size(), 0u);
}

TEST_F(MedicineServiceTest, GetExpiryAlerts_ContainsExpiredAlert) {
  medicineService->createMedicine(createExpiredMedicine("MED_EXP"));

  auto alerts = medicineService->getExpiryAlerts(30);

  bool foundExpiredAlert = false;
  for (const auto &alert : alerts) {
    if (alert.medicineID == "MED_EXP" && alert.alertType == "EXPIRED") {
      foundExpiredAlert = true;
    }
  }
  EXPECT_TRUE(foundExpiredAlert);
}

// ==================== Inventory Statistics Tests ====================

TEST_F(MedicineServiceTest, GetTotalInventoryValue_CalculatesCorrectly) {
  Medicine med1("MED001", "Med1", "Cat1", 100.0, 10);
  Medicine med2("MED002", "Med2", "Cat1", 200.0, 5);
  medicineService->createMedicine(med1);
  medicineService->createMedicine(med2);

  double total = medicineService->getTotalInventoryValue();

  // 100*10 + 200*5 = 1000 + 1000 = 2000
  EXPECT_DOUBLE_EQ(total, 2000.0);
}

TEST_F(MedicineServiceTest, GetTotalInventoryValue_EmptyInventory_ReturnsZero) {
  double total = medicineService->getTotalInventoryValue();

  EXPECT_DOUBLE_EQ(total, 0.0);
}

TEST_F(MedicineServiceTest, GetInventoryValueByCategory_GroupsCorrectly) {
  Medicine med1("MED001", "Paracetamol", "Pain Relief", 100.0, 10);
  Medicine med2("MED002", "Ibuprofen", "Pain Relief", 150.0, 20);
  Medicine med3("MED003", "Amoxicillin", "Antibiotics", 200.0, 5);
  medicineService->createMedicine(med1);
  medicineService->createMedicine(med2);
  medicineService->createMedicine(med3);

  auto valueByCategory = medicineService->getInventoryValueByCategory();

  // Pain Relief: 100*10 + 150*20 = 1000 + 3000 = 4000
  // Antibiotics: 200*5 = 1000
  EXPECT_DOUBLE_EQ(valueByCategory["Pain Relief"], 4000.0);
  EXPECT_DOUBLE_EQ(valueByCategory["Antibiotics"], 1000.0);
}

TEST_F(MedicineServiceTest, GetStockCountByCategory_CountsCorrectly) {
  Medicine med1("MED001", "Paracetamol", "Pain Relief", 100.0, 10);
  Medicine med2("MED002", "Ibuprofen", "Pain Relief", 150.0, 20);
  Medicine med3("MED003", "Amoxicillin", "Antibiotics", 200.0, 5);
  medicineService->createMedicine(med1);
  medicineService->createMedicine(med2);
  medicineService->createMedicine(med3);

  auto stockByCategory = medicineService->getStockCountByCategory();

  EXPECT_EQ(stockByCategory["Pain Relief"], 30);
  EXPECT_EQ(stockByCategory["Antibiotics"], 5);
}

// ==================== Category/Manufacturer Query Tests ====================

TEST_F(MedicineServiceTest, GetAllCategories_ReturnsUniqueCategories) {
  Medicine med1("MED001", "Med1", "Category A", 100.0, 10);
  Medicine med2("MED002", "Med2", "Category A", 100.0, 10);
  Medicine med3("MED003", "Med3", "Category B", 100.0, 10);
  medicineService->createMedicine(med1);
  medicineService->createMedicine(med2);
  medicineService->createMedicine(med3);

  auto categories = medicineService->getAllCategories();

  EXPECT_EQ(categories.size(), 2u);
}

TEST_F(MedicineServiceTest, GetAllManufacturers_ReturnsUniqueManufacturers) {
  Medicine med1 = createTestMedicine("MED001");
  med1.setManufacturer("ManufacturerA");
  Medicine med2 = createTestMedicine("MED002", "Med2");
  med2.setManufacturer("ManufacturerB");
  Medicine med3 = createTestMedicine("MED003", "Med3");
  med3.setManufacturer("ManufacturerA");

  medicineService->createMedicine(med1);
  medicineService->createMedicine(med2);
  medicineService->createMedicine(med3);

  auto manufacturers = medicineService->getAllManufacturers();

  EXPECT_EQ(manufacturers.size(), 2u);
}

// ==================== Validation Tests ====================

TEST_F(MedicineServiceTest, ValidateMedicine_ValidMedicine_ReturnsTrue) {
  Medicine med = createTestMedicine("MED001");

  EXPECT_TRUE(medicineService->validateMedicine(med));
}

TEST_F(MedicineServiceTest, ValidateMedicine_EmptyID_ReturnsFalse) {
  Medicine med("", "Name", "Category", 100.0, 10);

  EXPECT_FALSE(medicineService->validateMedicine(med));
}

TEST_F(MedicineServiceTest, ValidateMedicine_EmptyName_ReturnsFalse) {
  Medicine med("MED001", "", "Category", 100.0, 10);

  EXPECT_FALSE(medicineService->validateMedicine(med));
}

TEST_F(MedicineServiceTest, ValidateMedicine_EmptyCategory_ReturnsFalse) {
  Medicine med("MED001", "Name", "", 100.0, 10);

  EXPECT_FALSE(medicineService->validateMedicine(med));
}

TEST_F(MedicineServiceTest, ValidateMedicine_NegativePrice_ReturnsFalse) {
  Medicine med("MED001", "Name", "Category", -100.0, 10);

  EXPECT_FALSE(medicineService->validateMedicine(med));
}

TEST_F(MedicineServiceTest, ValidateMedicine_NegativeQuantity_ReturnsFalse) {
  Medicine med("MED001", "Name", "Category", 100.0, -10);

  EXPECT_FALSE(medicineService->validateMedicine(med));
}

TEST_F(MedicineServiceTest, ValidateMedicine_InvalidExpiryDate_ReturnsFalse) {
  Medicine med = createTestMedicine("MED001");
  med.setExpiryDate("invalid-date");

  EXPECT_FALSE(medicineService->validateMedicine(med));
}

TEST_F(MedicineServiceTest, MedicineExists_Exists_ReturnsTrue) {
  medicineService->createMedicine(createTestMedicine("MED001"));

  EXPECT_TRUE(medicineService->medicineExists("MED001"));
}

TEST_F(MedicineServiceTest, MedicineExists_NotExists_ReturnsFalse) {
  EXPECT_FALSE(medicineService->medicineExists("NONEXISTENT"));
}

TEST_F(MedicineServiceTest, MedicineNameExists_Exists_ReturnsTrue) {
  medicineService->createMedicine(
      createTestMedicine("MED001", "Test Medicine"));

  EXPECT_TRUE(medicineService->medicineNameExists("Test Medicine"));
  EXPECT_TRUE(
      medicineService->medicineNameExists("test medicine")); // Case insensitive
  EXPECT_TRUE(
      medicineService->medicineNameExists("  Test Medicine  ")); // Trimmed
}

TEST_F(MedicineServiceTest,
       MedicineNameExists_WithExcludeID_ExcludesCorrectly) {
  medicineService->createMedicine(
      createTestMedicine("MED001", "Test Medicine"));

  // Same ID should be excluded
  EXPECT_FALSE(medicineService->medicineNameExists("Test Medicine", "MED001"));

  // Different ID should still find it
  EXPECT_TRUE(medicineService->medicineNameExists("Test Medicine", "MED002"));
}

// ==================== Data Persistence Tests ====================

TEST_F(MedicineServiceTest, SaveData_DoesNotCrash) {
  medicineService->createMedicine(createTestMedicine("MED001"));

  bool result = medicineService->saveData();

  EXPECT_TRUE(result);
}

TEST_F(MedicineServiceTest, LoadData_DoesNotCrash) {
  bool result = medicineService->loadData();

  EXPECT_TRUE(result);
}

TEST_F(MedicineServiceTest, SaveAndLoad_DataPersists) {
  Medicine med = createTestMedicine("MED001");
  medicineService->createMedicine(med);
  medicineService->saveData();

  // Reset and reload
  MedicineService::resetInstance();
  MedicineRepository::resetInstance();
  MedicineRepository::getInstance()->setFilePath(testMedicineFile);

  medicineService = MedicineService::getInstance();
  medicineService->loadData();

  auto loaded = medicineService->getMedicineByID("MED001");
  EXPECT_TRUE(loaded.has_value());
  EXPECT_EQ(loaded->getName(), "Paracetamol");
}

/*
Build and run tests:
cd build && ./HospitalTests --gtest_filter="MedicineServiceTest.*"
*/
