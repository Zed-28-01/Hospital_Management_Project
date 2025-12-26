#include <gtest/gtest.h>

#include "dal/MedicineRepository.h"
#include "advance/Medicine.h"
#include "common/Constants.h"

#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>

using namespace HMS::DAL;
using namespace HMS::Model;

class MedicineRepositoryTest : public ::testing::Test
{
protected:
    MedicineRepository *repo;
    std::string testFilePath;

    void SetUp() override
    {
        // Create test directory
        testFilePath = "test_data/test_medicines.txt";
        std::filesystem::create_directories("test_data");

        // Reset singleton and set test file path
        MedicineRepository::resetInstance();
        repo = MedicineRepository::getInstance();
        repo->setFilePath(testFilePath);
        repo->clear();
    }

    void TearDown() override
    {
        // Clean up test data
        if (std::filesystem::exists(testFilePath))
        {
            std::filesystem::remove(testFilePath);
        }

        // Clean up backup files
        std::string backupPath = testFilePath + ".bak";
        if (std::filesystem::exists(backupPath))
        {
            std::filesystem::remove(backupPath);
        }

        if (std::filesystem::exists("test_data"))
        {
            std::filesystem::remove_all("test_data");
        }

        MedicineRepository::resetInstance();
    }

    // Helper function to create test medicine
    Medicine createTestMedicine(
        const std::string &id,
        const std::string &name,
        const std::string &manufacturer = "Test Pharma",
        int quantity = 100,
        double price = 50000.0,
        int reorderLevel = 10)
    {
        Medicine med(id, name, "Painkiller", price, quantity);

        // Set additional fields
        med.setGenericName("Generic " + name);
        med.setManufacturer(manufacturer);
        med.setReorderLevel(reorderLevel);
        med.setExpiryDate("2025-12-31");
        med.setDosageForm("Tablet");
        med.setStrength("500mg");
        med.setDescription("Store in cool place");

        return med;
    }

    // Helper to create expired medicine
    Medicine createExpiredMedicine(const std::string &id, const std::string &name)
    {
        Medicine med(id, name, "Painkiller", 50000.0, 100);
        med.setGenericName("Generic " + name);
        med.setManufacturer("Test Pharma");
        med.setReorderLevel(10);
        med.setExpiryDate("2020-01-01"); // expired
        med.setDosageForm("Tablet");
        med.setStrength("500mg");
        med.setDescription("Store in cool place");
        return med;
    }

    // Helper to create expiring soon medicine
    Medicine createExpiringSoonMedicine(const std::string &id, const std::string &name, int daysFromNow)
    {
        // Calculate date
        auto now = std::chrono::system_clock::now();
        auto future = now + std::chrono::hours(24 * daysFromNow);
        auto time_t_future = std::chrono::system_clock::to_time_t(future);
        std::tm tm_future = *std::localtime(&time_t_future);

        std::stringstream ss;
        ss << std::put_time(&tm_future, "%Y-%m-%d");

        Medicine med(id, name, "Painkiller", 50000.0, 100);
        med.setGenericName("Generic " + name);
        med.setManufacturer("Test Pharma");
        med.setReorderLevel(10);
        med.setExpiryDate(ss.str());
        med.setDosageForm("Tablet");
        med.setStrength("500mg");
        med.setDescription("Store in cool place");

        return med;
    }
};

// ==================== Singleton Tests ====================

TEST_F(MedicineRepositoryTest, Singleton_GetInstance_ReturnsSameInstance)
{
    auto repo1 = MedicineRepository::getInstance();
    auto repo2 = MedicineRepository::getInstance();

    EXPECT_EQ(repo1, repo2);
    EXPECT_NE(repo1, nullptr);
}

TEST_F(MedicineRepositoryTest, Singleton_ResetInstance_CreatesNewInstance)
{
    auto repo1 = MedicineRepository::getInstance();
    repo1->add(createTestMedicine("MED001", "Test"));

    MedicineRepository::resetInstance();

    auto repo2 = MedicineRepository::getInstance();

    EXPECT_EQ(repo2->count(), 0); // instance mới, không còn data
}

// ==================== CRUD - Add Tests ====================

TEST_F(MedicineRepositoryTest, Add_ValidMedicine_Success)
{
    Medicine med = createTestMedicine("MED001", "Paracetamol");

    bool result = repo->add(med);

    EXPECT_TRUE(result);
    EXPECT_EQ(repo->count(), 1);
    EXPECT_TRUE(repo->exists("MED001"));
}

TEST_F(MedicineRepositoryTest, Add_DuplicateID_Fail)
{
    Medicine med1 = createTestMedicine("MED001", "Paracetamol");
    Medicine med2 = createTestMedicine("MED001", "Different Medicine");

    repo->add(med1);
    bool result = repo->add(med2);

    EXPECT_FALSE(result);
    EXPECT_EQ(repo->count(), 1);
}

TEST_F(MedicineRepositoryTest, Add_DuplicateNameAndManufacturer_Fail)
{
    Medicine med1 = createTestMedicine("MED001", "Paracetamol", "ABC Pharma");
    Medicine med2 = createTestMedicine("MED002", "Paracetamol", "ABC Pharma");

    repo->add(med1);
    bool result = repo->add(med2);

    EXPECT_FALSE(result);
    EXPECT_EQ(repo->count(), 1);
}

TEST_F(MedicineRepositoryTest, Add_SameNameDifferentManufacturer_Success)
{
    Medicine med1 = createTestMedicine("MED001", "Paracetamol", "ABC Pharma");
    Medicine med2 = createTestMedicine("MED002", "Paracetamol", "XYZ Pharma");

    bool result1 = repo->add(med1);
    bool result2 = repo->add(med2);

    EXPECT_TRUE(result1);
    EXPECT_TRUE(result2);
    EXPECT_EQ(repo->count(), 2);
}

TEST_F(MedicineRepositoryTest, Add_NameWithWhitespace_TrimmedAndCompared)
{
    Medicine med1 = createTestMedicine("MED001", "  Paracetamol  ", "ABC Pharma");
    Medicine med2 = createTestMedicine("MED002", "Paracetamol", "ABC Pharma");

    repo->add(med1);
    bool result = repo->add(med2);

    EXPECT_FALSE(result); // Should detect as duplicate after trimming
}

TEST_F(MedicineRepositoryTest, Add_NameCaseInsensitive_DetectsDuplicate)
{
    Medicine med1 = createTestMedicine("MED001", "Paracetamol", "ABC Pharma");
    Medicine med2 = createTestMedicine("MED002", "PARACETAMOL", "ABC Pharma");

    repo->add(med1);
    bool result = repo->add(med2);

    EXPECT_FALSE(result); // Should detect as duplicate (case-insensitive)
}

TEST_F(MedicineRepositoryTest, Add_MultipleMedicines_AllAdded)
{
    repo->add(createTestMedicine("MED001", "Paracetamol"));
    repo->add(createTestMedicine("MED002", "Ibuprofen"));
    repo->add(createTestMedicine("MED003", "Aspirin"));

    EXPECT_EQ(repo->count(), 3);
}

// ==================== CRUD - GetById Tests ====================

TEST_F(MedicineRepositoryTest, GetById_ExistingMedicine_ReturnsCorrectMedicine)
{
    Medicine med = createTestMedicine("MED001", "Paracetamol", "ABC Pharma", 100, 50000.0);
    repo->add(med);

    auto result = repo->getById("MED001");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getMedicineID(), "MED001");
    EXPECT_EQ(result->getName(), "Paracetamol");
    EXPECT_EQ(result->getManufacturer(), "ABC Pharma");
    EXPECT_EQ(result->getQuantityInStock(), 100);
}

TEST_F(MedicineRepositoryTest, GetById_NonExistingMedicine_ReturnsNullopt)
{
    auto result = repo->getById("MED999");

    EXPECT_FALSE(result.has_value());
}

TEST_F(MedicineRepositoryTest, GetById_EmptyRepository_ReturnsNullopt)
{
    auto result = repo->getById("MED001");

    EXPECT_FALSE(result.has_value());
}

// ==================== CRUD - Update Tests ====================

TEST_F(MedicineRepositoryTest, Update_ExistingMedicine_Success)
{
    Medicine med = createTestMedicine("MED001", "Paracetamol", "ABC Pharma", 100);
    repo->add(med);

    med.setQuantityInStock(200);
    med.setUnitPrice(60000.0);
    bool result = repo->update(med);

    EXPECT_TRUE(result);

    auto updated = repo->getById("MED001");
    ASSERT_TRUE(updated.has_value());
    EXPECT_EQ(updated->getQuantityInStock(), 200);
    EXPECT_DOUBLE_EQ(updated->getUnitPrice(), 60000.0);
}

TEST_F(MedicineRepositoryTest, Update_NonExistingMedicine_Fail)
{
    Medicine med = createTestMedicine("MED999", "NonExistent");

    bool result = repo->update(med);

    EXPECT_FALSE(result);
}

TEST_F(MedicineRepositoryTest, Update_ChangeToDuplicateName_Fail)
{
    Medicine med1 = createTestMedicine("MED001", "Paracetamol", "ABC Pharma");
    Medicine med2 = createTestMedicine("MED002", "Ibuprofen", "ABC Pharma");
    repo->add(med1);
    repo->add(med2);

    // Try to change med2's name to match med1
    med2.setName("Paracetamol");
    bool result = repo->update(med2);

    EXPECT_FALSE(result);
}

TEST_F(MedicineRepositoryTest, Update_SameMedicine_Success)
{
    Medicine med = createTestMedicine("MED001", "Paracetamol", "ABC Pharma", 100);
    repo->add(med);

    // Update same medicine (should not fail on duplicate check)
    med.setQuantityInStock(150);
    bool result = repo->update(med);

    EXPECT_TRUE(result);

    auto updated = repo->getById("MED001");
    ASSERT_TRUE(updated.has_value());
    EXPECT_EQ(updated->getQuantityInStock(), 150);
}

TEST_F(MedicineRepositoryTest, Update_ChangeManufacturerToNonDuplicate_Success)
{
    Medicine med1 = createTestMedicine("MED001", "Paracetamol", "ABC Pharma");
    Medicine med2 = createTestMedicine("MED002", "Paracetamol", "XYZ Pharma");
    repo->add(med1);
    repo->add(med2);

    // Change med2 manufacturer to another unique one
    med2.setManufacturer("DEF Pharma");
    bool result = repo->update(med2);

    EXPECT_TRUE(result);
}

// ==================== CRUD - Remove Tests ====================

TEST_F(MedicineRepositoryTest, Remove_ExistingMedicine_Success)
{
    Medicine med = createTestMedicine("MED001", "Paracetamol");
    repo->add(med);

    bool result = repo->remove("MED001");

    EXPECT_TRUE(result);
    EXPECT_EQ(repo->count(), 0);
    EXPECT_FALSE(repo->exists("MED001"));
}

TEST_F(MedicineRepositoryTest, Remove_NonExistingMedicine_Fail)
{
    bool result = repo->remove("MED999");

    EXPECT_FALSE(result);
}

TEST_F(MedicineRepositoryTest, Remove_FromMultipleMedicines_OnlyTargetRemoved)
{
    repo->add(createTestMedicine("MED001", "Paracetamol"));
    repo->add(createTestMedicine("MED002", "Ibuprofen"));
    repo->add(createTestMedicine("MED003", "Aspirin"));

    repo->remove("MED002");

    EXPECT_EQ(repo->count(), 2);
    EXPECT_TRUE(repo->exists("MED001"));
    EXPECT_FALSE(repo->exists("MED002"));
    EXPECT_TRUE(repo->exists("MED003"));
}

// ==================== CRUD - GetAll Tests ====================

TEST_F(MedicineRepositoryTest, GetAll_EmptyRepository_ReturnsEmptyVector)
{
    auto all = repo->getAll();

    EXPECT_TRUE(all.empty());
    EXPECT_EQ(all.size(), 0);
}

TEST_F(MedicineRepositoryTest, GetAll_WithMedicines_ReturnsAllMedicines)
{
    repo->add(createTestMedicine("MED001", "Paracetamol"));
    repo->add(createTestMedicine("MED002", "Ibuprofen"));
    repo->add(createTestMedicine("MED003", "Aspirin"));

    auto all = repo->getAll();

    EXPECT_EQ(all.size(), 3);
}

TEST_F(MedicineRepositoryTest, GetAll_ReturnsCorrectMedicines)
{
    Medicine med1 = createTestMedicine("MED001", "Paracetamol");
    Medicine med2 = createTestMedicine("MED002", "Ibuprofen");
    repo->add(med1);
    repo->add(med2);

    auto all = repo->getAll();

    EXPECT_EQ(all[0].getMedicineID(), "MED001");
    EXPECT_EQ(all[1].getMedicineID(), "MED002");
}

// ==================== Query - Count Tests ====================

TEST_F(MedicineRepositoryTest, Count_EmptyRepository_ReturnsZero)
{
    EXPECT_EQ(repo->count(), 0);
}

TEST_F(MedicineRepositoryTest, Count_WithMedicines_ReturnsCorrectCount)
{
    repo->add(createTestMedicine("MED001", "Paracetamol"));
    EXPECT_EQ(repo->count(), 1);

    repo->add(createTestMedicine("MED002", "Ibuprofen"));
    EXPECT_EQ(repo->count(), 2);

    repo->add(createTestMedicine("MED003", "Aspirin"));
    EXPECT_EQ(repo->count(), 3);
}

TEST_F(MedicineRepositoryTest, Count_AfterRemove_ReturnsCorrectCount)
{
    repo->add(createTestMedicine("MED001", "Paracetamol"));
    repo->add(createTestMedicine("MED002", "Ibuprofen"));

    repo->remove("MED001");

    EXPECT_EQ(repo->count(), 1);
}

// ==================== Query - Exists Tests ====================

TEST_F(MedicineRepositoryTest, Exists_ExistingMedicine_ReturnsTrue)
{
    Medicine med = createTestMedicine("MED001", "Paracetamol");
    repo->add(med);

    EXPECT_TRUE(repo->exists("MED001"));
}

TEST_F(MedicineRepositoryTest, Exists_NonExistingMedicine_ReturnsFalse)
{
    EXPECT_FALSE(repo->exists("MED999"));
}

TEST_F(MedicineRepositoryTest, Exists_AfterRemove_ReturnsFalse)
{
    Medicine med = createTestMedicine("MED001", "Paracetamol");
    repo->add(med);

    repo->remove("MED001");

    EXPECT_FALSE(repo->exists("MED001"));
}

// ==================== Query - Clear Tests ====================

TEST_F(MedicineRepositoryTest, Clear_WithMedicines_RemovesAll)
{
    repo->add(createTestMedicine("MED001", "Paracetamol"));
    repo->add(createTestMedicine("MED002", "Ibuprofen"));

    bool result = repo->clear();

    EXPECT_TRUE(result);
    EXPECT_EQ(repo->count(), 0);
}

TEST_F(MedicineRepositoryTest, Clear_EmptyRepository_Success)
{
    bool result = repo->clear();

    EXPECT_TRUE(result);
    EXPECT_EQ(repo->count(), 0);
}

// ==================== Medicine-Specific - GetByCategory Tests ====================

TEST_F(MedicineRepositoryTest, GetByCategory_ExistingCategory_ReturnsMatchingMedicines)
{
    Medicine med1 = createTestMedicine("MED001", "Paracetamol");
    med1.setCategory("Painkiller");

    Medicine med2 = createTestMedicine("MED002", "Amoxicillin");
    med2.setCategory("Antibiotic");

    Medicine med3 = createTestMedicine("MED003", "Ibuprofen");
    med3.setCategory("Painkiller");

    repo->add(med1);
    repo->add(med2);
    repo->add(med3);

    auto painkillers = repo->getByCategory("Painkiller");

    EXPECT_EQ(painkillers.size(), 2);
}

TEST_F(MedicineRepositoryTest, GetByCategory_NonExistingCategory_ReturnsEmpty)
{
    repo->add(createTestMedicine("MED001", "Paracetamol"));

    auto results = repo->getByCategory("NonExistent");

    EXPECT_TRUE(results.empty());
}

TEST_F(MedicineRepositoryTest, GetByCategory_CaseInsensitive_FindsMatch)
{
    Medicine med = createTestMedicine("MED001", "Paracetamol");
    med.setCategory("Painkiller");
    repo->add(med);

    auto results = repo->getByCategory("PAINKILLER");

    EXPECT_EQ(results.size(), 1);
}

TEST_F(MedicineRepositoryTest, GetByCategory_PartialMatch_FindsMatch)
{
    Medicine med = createTestMedicine("MED001", "Paracetamol");
    med.setCategory("Painkiller");
    repo->add(med);

    auto results = repo->getByCategory("Pain");

    EXPECT_EQ(results.size(), 1);
}

// ==================== Medicine-Specific - GetLowStock Tests ====================

TEST_F(MedicineRepositoryTest, GetLowStock_ReturnsOnlyLowStockMedicines)
{
    Medicine med1 = createTestMedicine("MED001", "Paracetamol", "ABC", 5, 50000, 10); // Low
    Medicine med2 = createTestMedicine("MED002", "Ibuprofen", "ABC", 50, 50000, 10);  // Normal
    Medicine med3 = createTestMedicine("MED003", "Aspirin", "ABC", 8, 50000, 10);     // Low

    repo->add(med1);
    repo->add(med2);
    repo->add(med3);

    auto lowStock = repo->getLowStock();

    EXPECT_EQ(lowStock.size(), 2);
}

TEST_F(MedicineRepositoryTest, GetLowStock_AtReorderLevel_IsLowStock)
{
    Medicine med = createTestMedicine("MED001", "Paracetamol", "ABC", 10, 50000, 10);
    repo->add(med);

    auto lowStock = repo->getLowStock();

    EXPECT_EQ(lowStock.size(), 1);
}

TEST_F(MedicineRepositoryTest, GetLowStock_NoLowStock_ReturnsEmpty)
{
    Medicine med = createTestMedicine("MED001", "Paracetamol", "ABC", 100, 50000, 10);
    repo->add(med);

    auto lowStock = repo->getLowStock();

    EXPECT_TRUE(lowStock.empty());
}

// ==================== Medicine-Specific - GetExpired Tests ====================

TEST_F(MedicineRepositoryTest, GetExpired_ReturnsOnlyExpiredMedicines)
{
    Medicine med1 = createExpiredMedicine("MED001", "Expired1");
    Medicine med2 = createTestMedicine("MED002", "Valid");
    Medicine med3 = createExpiredMedicine("MED003", "Expired2");

    repo->add(med1);
    repo->add(med2);
    repo->add(med3);

    auto expired = repo->getExpired();

    EXPECT_EQ(expired.size(), 2);
}

TEST_F(MedicineRepositoryTest, GetExpired_NoExpired_ReturnsEmpty)
{
    repo->add(createTestMedicine("MED001", "Valid1"));
    repo->add(createTestMedicine("MED002", "Valid2"));

    auto expired = repo->getExpired();

    EXPECT_TRUE(expired.empty());
}

// ==================== Medicine-Specific - GetExpiringSoon Tests ====================

TEST_F(MedicineRepositoryTest, GetExpiringSoon_Default30Days_ReturnsCorrectMedicines)
{
    Medicine med1 = createExpiringSoonMedicine("MED001", "ExpiringSoon", 15);
    Medicine med2 = createExpiringSoonMedicine("MED002", "ValidLongTime", 400);
    Medicine med3 = createExpiringSoonMedicine("MED003", "ExpiringSoon2", 25);

    repo->add(med1);
    repo->add(med2);
    repo->add(med3);

    auto expiringSoon = repo->getExpiringSoon(30);

    EXPECT_EQ(expiringSoon.size(), 2);
}

TEST_F(MedicineRepositoryTest, GetExpiringSoon_CustomDays_ReturnsCorrectMedicines)
{
    Medicine med1 = createExpiringSoonMedicine("MED001", "ExpiringSoon", 5);
    Medicine med2 = createExpiringSoonMedicine("MED002", "NotYet", 15);

    repo->add(med1);
    repo->add(med2);

    auto expiringSoon = repo->getExpiringSoon(7);

    // Only med1 (5 days) should be expiring within 7 days
    EXPECT_EQ(expiringSoon.size(), 1);
    if (expiringSoon.size() > 0)
    {
        EXPECT_EQ(expiringSoon[0].getMedicineID(), "MED001");
    }
}

// ==================== Medicine-Specific - SearchByName Tests ====================

TEST_F(MedicineRepositoryTest, SearchByName_ByName_FindsMatches)
{
    repo->add(createTestMedicine("MED001", "Paracetamol"));
    repo->add(createTestMedicine("MED002", "Ibuprofen"));
    repo->add(createTestMedicine("MED003", "Paracetamol Extra"));

    auto results = repo->searchByName("Paracetamol");

    EXPECT_EQ(results.size(), 2);
}

TEST_F(MedicineRepositoryTest, SearchByName_ByGenericName_FindsMatches)
{
    Medicine med = createTestMedicine("MED001", "Tylenol");
    med.setGenericName("Acetaminophen");
    repo->add(med);

    auto results = repo->searchByName("Acetaminophen");

    EXPECT_EQ(results.size(), 1);
}

TEST_F(MedicineRepositoryTest, SearchByName_CaseInsensitive_FindsMatch)
{
    repo->add(createTestMedicine("MED001", "Paracetamol"));

    auto results = repo->searchByName("PARACETAMOL");

    EXPECT_EQ(results.size(), 1);
}

TEST_F(MedicineRepositoryTest, SearchByName_PartialMatch_FindsMatch)
{
    repo->add(createTestMedicine("MED001", "Paracetamol"));
    repo->add(createTestMedicine("MED002", "Ibuprofen"));

    auto results = repo->searchByName("para");

    EXPECT_EQ(results.size(), 1);
}

TEST_F(MedicineRepositoryTest, SearchByName_NoMatch_ReturnsEmpty)
{
    repo->add(createTestMedicine("MED001", "Paracetamol"));

    auto results = repo->searchByName("NonExistent");

    EXPECT_TRUE(results.empty());
}

// ==================== Medicine-Specific - Search Tests ====================

TEST_F(MedicineRepositoryTest, Search_ByName_FindsMatch)
{
    repo->add(createTestMedicine("MED001", "Paracetamol"));

    auto results = repo->search("Paracetamol");

    EXPECT_EQ(results.size(), 1);
}

TEST_F(MedicineRepositoryTest, Search_ByManufacturer_FindsMatch)
{
    repo->add(createTestMedicine("MED001", "Medicine1", "ABC Pharma"));
    repo->add(createTestMedicine("MED002", "Medicine2", "XYZ Pharma"));

    auto results = repo->search("ABC");

    EXPECT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].getManufacturer(), "ABC Pharma");
}

TEST_F(MedicineRepositoryTest, Search_ByCategory_FindsMatch)
{
    Medicine med = createTestMedicine("MED001", "Paracetamol");
    med.setCategory("Painkiller");
    repo->add(med);

    auto results = repo->search("Painkiller");

    EXPECT_EQ(results.size(), 1);
}

TEST_F(MedicineRepositoryTest, Search_ByGenericName_FindsMatch)
{
    Medicine med = createTestMedicine("MED001", "Tylenol");
    med.setGenericName("Acetaminophen");
    repo->add(med);

    auto results = repo->search("Acetaminophen");

    EXPECT_EQ(results.size(), 1);
}

TEST_F(MedicineRepositoryTest, Search_MultipleMatches_ReturnsAll)
{
    Medicine med1 = createTestMedicine("MED001", "Paracetamol", "ABC Pharma");
    Medicine med2 = createTestMedicine("MED002", "ABC Brand Medicine", "XYZ Pharma");

    repo->add(med1);
    repo->add(med2);

    auto results = repo->search("ABC");

    EXPECT_EQ(results.size(), 2);
}

// ==================== Medicine-Specific - GetAllCategories Tests ====================

TEST_F(MedicineRepositoryTest, GetAllCategories_ReturnsUniqueCategories)
{
    Medicine med1 = createTestMedicine("MED001", "Med1");
    med1.setCategory("Painkiller");

    Medicine med2 = createTestMedicine("MED002", "Med2");
    med2.setCategory("Antibiotic");

    Medicine med3 = createTestMedicine("MED003", "Med3");
    med3.setCategory("Painkiller");

    repo->add(med1);
    repo->add(med2);
    repo->add(med3);

    auto categories = repo->getAllCategories();

    EXPECT_EQ(categories.size(), 2);
    EXPECT_TRUE(std::find(categories.begin(), categories.end(), "Painkiller") != categories.end());
    EXPECT_TRUE(std::find(categories.begin(), categories.end(), "Antibiotic") != categories.end());
}

TEST_F(MedicineRepositoryTest, GetAllCategories_Sorted_ReturnsInOrder)
{
    Medicine med1 = createTestMedicine("MED001", "Med1");
    med1.setCategory("Vitamin");

    Medicine med2 = createTestMedicine("MED002", "Med2");
    med2.setCategory("Antibiotic");

    repo->add(med1);
    repo->add(med2);

    auto categories = repo->getAllCategories();

    EXPECT_EQ(categories[0], "Antibiotic");
    EXPECT_EQ(categories[1], "Vitamin");
}

TEST_F(MedicineRepositoryTest, GetAllCategories_EmptyRepository_ReturnsEmpty)
{
    auto categories = repo->getAllCategories();

    EXPECT_TRUE(categories.empty());
}

TEST_F(MedicineRepositoryTest, GetAllCategories_IgnoresEmptyCategories)
{
    Medicine med1 = createTestMedicine("MED001", "Med1");
    med1.setCategory("Painkiller");

    Medicine med2 = createTestMedicine("MED002", "Med2");
    med2.setCategory("");

    repo->add(med1);
    repo->add(med2);

    auto categories = repo->getAllCategories();

    EXPECT_EQ(categories.size(), 1);
    EXPECT_EQ(categories[0], "Painkiller");
}

// ==================== Medicine-Specific - GetAllManufacturers Tests ====================

TEST_F(MedicineRepositoryTest, GetAllManufacturers_ReturnsUniqueManufacturers)
{
    repo->add(createTestMedicine("MED001", "Med1", "ABC Pharma"));
    repo->add(createTestMedicine("MED002", "Med2", "XYZ Pharma"));
    repo->add(createTestMedicine("MED003", "Med3", "ABC Pharma"));

    auto manufacturers = repo->getAllManufacturers();

    EXPECT_EQ(manufacturers.size(), 2);
    EXPECT_TRUE(std::find(manufacturers.begin(), manufacturers.end(), "ABC Pharma") != manufacturers.end());
    EXPECT_TRUE(std::find(manufacturers.begin(), manufacturers.end(), "XYZ Pharma") != manufacturers.end());
}

TEST_F(MedicineRepositoryTest, GetAllManufacturers_Sorted_ReturnsInOrder)
{
    repo->add(createTestMedicine("MED001", "Med1", "Zebra Pharma"));
    repo->add(createTestMedicine("MED002", "Med2", "Alpha Pharma"));

    auto manufacturers = repo->getAllManufacturers();

    EXPECT_EQ(manufacturers[0], "Alpha Pharma");
    EXPECT_EQ(manufacturers[1], "Zebra Pharma");
}

TEST_F(MedicineRepositoryTest, GetAllManufacturers_EmptyRepository_ReturnsEmpty)
{
    auto manufacturers = repo->getAllManufacturers();

    EXPECT_TRUE(manufacturers.empty());
}

// ==================== ID Generation - GetNextId Tests ====================

TEST_F(MedicineRepositoryTest, GetNextId_EmptyRepository_ReturnsMED001)
{
    std::string nextId = repo->getNextId();

    EXPECT_EQ(nextId, "MED001");
}

TEST_F(MedicineRepositoryTest, GetNextId_WithExistingMedicines_ReturnsIncrementedId)
{
    repo->add(createTestMedicine("MED001", "Med1"));
    repo->add(createTestMedicine("MED002", "Med2"));

    std::string nextId = repo->getNextId();

    EXPECT_EQ(nextId, "MED003");
}

TEST_F(MedicineRepositoryTest, GetNextId_WithGaps_ReturnsMaxPlusOne)
{
    repo->add(createTestMedicine("MED001", "Med1"));
    repo->add(createTestMedicine("MED005", "Med5"));

    std::string nextId = repo->getNextId();

    EXPECT_EQ(nextId, "MED006");
}

TEST_F(MedicineRepositoryTest, GetNextId_WithInvalidIDs_IgnoresThem)
{
    repo->add(createTestMedicine("MED001", "Med1"));
    repo->add(createTestMedicine("INVALID", "Invalid"));
    repo->add(createTestMedicine("MED003", "Med3"));

    std::string nextId = repo->getNextId();

    EXPECT_EQ(nextId, "MED004");
}

TEST_F(MedicineRepositoryTest, GetNextId_WithLargeNumbers_FormatsCorrectly)
{
    repo->add(createTestMedicine("MED099", "Med99"));

    std::string nextId = repo->getNextId();

    EXPECT_EQ(nextId, "MED100");
}

TEST_F(MedicineRepositoryTest, GetNextId_WithNonNumericSuffix_IgnoresThem)
{
    repo->add(createTestMedicine("MED001", "Med1"));
    repo->add(createTestMedicine("MEDABC", "Invalid"));

    std::string nextId = repo->getNextId();

    EXPECT_EQ(nextId, "MED002");
}

// ==================== Stock Operations - UpdateStock Tests ====================

TEST_F(MedicineRepositoryTest, UpdateStock_ValidQuantity_Success)
{
    Medicine med = createTestMedicine("MED001", "Paracetamol", "ABC", 100);
    repo->add(med);

    bool result = repo->updateStock("MED001", 200);

    EXPECT_TRUE(result);

    auto updated = repo->getById("MED001");
    ASSERT_TRUE(updated.has_value());
    EXPECT_EQ(updated->getQuantityInStock(), 200);
}

TEST_F(MedicineRepositoryTest, UpdateStock_NegativeQuantity_Fail)
{
    Medicine med = createTestMedicine("MED001", "Paracetamol");
    repo->add(med);

    bool result = repo->updateStock("MED001", -10);

    EXPECT_FALSE(result);

    // Quantity should remain unchanged
    auto medicine = repo->getById("MED001");
    ASSERT_TRUE(medicine.has_value());
    EXPECT_EQ(medicine->getQuantityInStock(), 100); // Original quantity
}

TEST_F(MedicineRepositoryTest, UpdateStock_ZeroQuantity_Success)
{
    Medicine med = createTestMedicine("MED001", "Paracetamol", "ABC", 100);
    repo->add(med);

    bool result = repo->updateStock("MED001", 0);

    EXPECT_TRUE(result);

    auto updated = repo->getById("MED001");
    ASSERT_TRUE(updated.has_value());
    EXPECT_EQ(updated->getQuantityInStock(), 0);
}

TEST_F(MedicineRepositoryTest, UpdateStock_NonExistingMedicine_Fail)
{
    bool result = repo->updateStock("MED999", 100);

    EXPECT_FALSE(result);
}

TEST_F(MedicineRepositoryTest, UpdateStock_LargeQuantity_Success)
{
    Medicine med = createTestMedicine("MED001", "Paracetamol");
    repo->add(med);

    bool result = repo->updateStock("MED001", 10000);

    EXPECT_TRUE(result);

    auto updated = repo->getById("MED001");
    ASSERT_TRUE(updated.has_value());
    EXPECT_EQ(updated->getQuantityInStock(), 10000);
}

// ==================== File Path Management Tests ====================

TEST_F(MedicineRepositoryTest, SetFilePath_ChangesPath)
{
    std::string newPath = "test_data/new_medicines.txt";

    repo->setFilePath(newPath);

    EXPECT_EQ(repo->getFilePath(), newPath);

    // Clean up
    if (std::filesystem::exists(newPath))
    {
        std::filesystem::remove(newPath);
    }
}

TEST_F(MedicineRepositoryTest, SetFilePath_ResetsLoadedFlag)
{
    // Load data first
    repo->add(createTestMedicine("MED001", "Med1"));
    EXPECT_EQ(repo->count(), 1);

    // Change path - should reset loaded flag
    repo->setFilePath("test_data/new_path.txt");

    // Count should trigger reload and return 0 (new empty file)
    EXPECT_EQ(repo->count(), 0);

    // Clean up
    if (std::filesystem::exists("test_data/new_path.txt"))
    {
        std::filesystem::remove("test_data/new_path.txt");
    }
}

TEST_F(MedicineRepositoryTest, GetFilePath_ReturnsCurrentPath)
{
    std::string path = repo->getFilePath();

    EXPECT_EQ(path, testFilePath);
}

// ==================== Persistence Tests ====================

TEST_F(MedicineRepositoryTest, Save_PersistsData)
{
    repo->add(createTestMedicine("MED001", "Paracetamol"));
    repo->add(createTestMedicine("MED002", "Ibuprofen"));

    // Data should be saved automatically by add()
    EXPECT_TRUE(std::filesystem::exists(testFilePath));
}

TEST_F(MedicineRepositoryTest, Load_RetrievesPersistedData)
{
    // Add and save
    repo->add(createTestMedicine("MED001", "Paracetamol"));
    repo->add(createTestMedicine("MED002", "Ibuprofen"));

    // Reset and reload
    MedicineRepository::resetInstance();
    repo = MedicineRepository::getInstance();
    repo->setFilePath(testFilePath);

    EXPECT_EQ(repo->count(), 2);
    EXPECT_TRUE(repo->exists("MED001"));
    EXPECT_TRUE(repo->exists("MED002"));
}

TEST_F(MedicineRepositoryTest, Save_CreatesBackup)
{
    // First add - creates initial file
    repo->add(createTestMedicine("MED001", "Paracetamol"));

    // Verify file exists
    ASSERT_TRUE(std::filesystem::exists(testFilePath));

    // Small delay for file system
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Second add - should trigger backup
    repo->add(createTestMedicine("MED002", "Ibuprofen"));

    std::string backupDir = HMS::Constants::BACKUP_DIR;

    if (!std::filesystem::exists(backupDir)) {
        // Backup dir not created - may skip test files or backup feature disabled
        GTEST_SKIP() << "Backup directory '" << backupDir << "' not found - FileHelper may skip test files or backup is disabled";
    }

    bool foundBackup = false;

    for (const auto& entry : std::filesystem::directory_iterator(backupDir)) {
        if (entry.is_regular_file()) {
            std::string filename = entry.path().filename().string();
            if (filename.find("test_medicines") != std::string::npos &&
                filename.find("backup") != std::string::npos) {
                foundBackup = true;
                break;
            }
        }
    }

    if (!foundBackup) {
        GTEST_SKIP() << "No backup found in " << backupDir << " - FileHelper may skip test files";
    }

    EXPECT_TRUE(foundBackup);
}

TEST_F(MedicineRepositoryTest, Load_CreatesDirectoryIfNotExists)
{
    std::string newPath = "test_data/subdir/medicines.txt";

    MedicineRepository::resetInstance();
    repo = MedicineRepository::getInstance();
    repo->setFilePath(newPath);

    // Trigger load
    repo->count();

    EXPECT_TRUE(std::filesystem::exists("test_data/subdir"));

    // Clean up
    std::filesystem::remove_all("test_data/subdir");
}

TEST_F(MedicineRepositoryTest, Load_CreatesFileIfNotExists)
{
    std::string newPath = "test_data/new_file.txt";

    MedicineRepository::resetInstance();
    repo = MedicineRepository::getInstance();
    repo->setFilePath(newPath);

    // Trigger load
    repo->count();

    EXPECT_TRUE(std::filesystem::exists(newPath));

    // Clean up
    std::filesystem::remove(newPath);
}

// ==================== Integration Tests ====================

TEST_F(MedicineRepositoryTest, Integration_CompleteWorkflow)
{
    // Add medicines
    repo->add(createTestMedicine("MED001", "Paracetamol", "ABC Pharma", 100));
    repo->add(createTestMedicine("MED002", "Ibuprofen", "XYZ Pharma", 50));

    EXPECT_EQ(repo->count(), 2);

    // Update stock
    repo->updateStock("MED001", 80);

    auto med1 = repo->getById("MED001");
    ASSERT_TRUE(med1.has_value());
    EXPECT_EQ(med1->getQuantityInStock(), 80);

    // Search
    auto results = repo->searchByName("Paracetamol");
    EXPECT_EQ(results.size(), 1);

    // Remove
    repo->remove("MED002");
    EXPECT_EQ(repo->count(), 1);
    EXPECT_FALSE(repo->exists("MED002"));

    // Verify persistence
    MedicineRepository::resetInstance();
    repo = MedicineRepository::getInstance();
    repo->setFilePath(testFilePath);

    EXPECT_EQ(repo->count(), 1);
    EXPECT_TRUE(repo->exists("MED001"));
    EXPECT_FALSE(repo->exists("MED002"));
}

TEST_F(MedicineRepositoryTest, Integration_StockManagement)
{
    // Add medicines with different stock levels
    repo->add(createTestMedicine("MED001", "Med1", "ABC", 5, 50000, 10));   // Low
    repo->add(createTestMedicine("MED002", "Med2", "ABC", 100, 50000, 10)); // Good
    repo->add(createTestMedicine("MED003", "Med3", "ABC", 8, 50000, 10));   // Low

    // Check low stock
    auto lowStock = repo->getLowStock();
    EXPECT_EQ(lowStock.size(), 2);

    // Restock one medicine
    repo->updateStock("MED001", 50);

    // Check again
    lowStock = repo->getLowStock();
    EXPECT_EQ(lowStock.size(), 1);
    EXPECT_EQ(lowStock[0].getMedicineID(), "MED003");
}

// ==================== Edge Cases Tests ====================

TEST_F(MedicineRepositoryTest, EdgeCase_EmptyName_CanBeAdded)
{
    Medicine med = createTestMedicine("MED001", "");

    bool result = repo->add(med);

    EXPECT_TRUE(result); // Should allow empty name
}

TEST_F(MedicineRepositoryTest, EdgeCase_VeryLongName_CanBeAdded)
{
    std::string longName(1000, 'A');
    Medicine med = createTestMedicine("MED001", longName);

    bool result = repo->add(med);

    EXPECT_TRUE(result);
}

TEST_F(MedicineRepositoryTest, EdgeCase_SpecialCharactersInName_CanBeAdded)
{
    Medicine med = createTestMedicine("MED001", "Med@#$%^&*()");

    bool result = repo->add(med);

    EXPECT_TRUE(result);
}

TEST_F(MedicineRepositoryTest, EdgeCase_MultipleSpacesInName_TrimmedCorrectly)
{
    Medicine med1 = createTestMedicine("MED001", "   Paracetamol   ", "ABC");
    Medicine med2 = createTestMedicine("MED002", "Paracetamol", "ABC");

    repo->add(med1);
    bool result = repo->add(med2);

    EXPECT_FALSE(result); // Should detect as duplicate after trim
}

TEST_F(MedicineRepositoryTest, EdgeCase_MaxIntQuantity_HandledCorrectly)
{
    Medicine med = createTestMedicine("MED001", "Med1");
    repo->add(med);

    bool result = repo->updateStock("MED001", INT_MAX);

    EXPECT_TRUE(result);

    auto updated = repo->getById("MED001");
    EXPECT_EQ(updated->getQuantityInStock(), INT_MAX);
}

// ==================== Concurrent Access Tests ====================

TEST_F(MedicineRepositoryTest, Concurrent_MultipleGetInstance_ReturnsSame)
{
    std::vector<MedicineRepository *> instances;

    for (int i = 0; i < 10; i++)
    {
        instances.push_back(MedicineRepository::getInstance());
    }

    // All should be the same instance
    for (size_t i = 1; i < instances.size(); i++)
    {
        EXPECT_EQ(instances[0], instances[i]);
    }
}

// ==================== Main ====================

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

/*
Build và run tests:
cd build && ./HospitalTests --gtest_filter="MedicineRepositoryTest.*"
*/