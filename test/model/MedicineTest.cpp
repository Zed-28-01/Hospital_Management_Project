#include "advance/Medicine.h"
#include "common/Constants.h"
#include "common/Types.h"
#include "common/Utils.h"
#include <gtest/gtest.h>
#include <string>

using namespace HMS::Model;
using namespace HMS;

// ==================== Constructor Tests ====================

TEST(MedicineTest, DefaultConstructor)
{
    Medicine med;

    EXPECT_EQ(med.getMedicineID(), "");
    EXPECT_EQ(med.getName(), "");
    EXPECT_EQ(med.getCategory(), "");
    EXPECT_DOUBLE_EQ(med.getUnitPrice(), 0.0);
    EXPECT_EQ(med.getQuantityInStock(), 0);
}

TEST(MedicineTest, ParameterizedConstructor)
{
    Medicine med("MED001", "Paracetamol 500mg", "Pain Relief", 5000.0, 500);

    EXPECT_EQ(med.getMedicineID(), "MED001");
    EXPECT_EQ(med.getName(), "Paracetamol 500mg");
    EXPECT_EQ(med.getCategory(), "Pain Relief");
    EXPECT_DOUBLE_EQ(med.getUnitPrice(), 5000.0);
    EXPECT_EQ(med.getQuantityInStock(), 500);
    EXPECT_EQ(med.getReorderLevel(), Constants::DEFAULT_REORDER_LEVEL);
    EXPECT_EQ(med.getGenericName(), "");
    EXPECT_EQ(med.getExpiryDate(), "");
}

// ==================== Getters/Setters Tests ====================

TEST(MedicineTest, SettersUpdateValuesCorrectly)
{
    Medicine med;

    med.setName("Aspirin");
    med.setGenericName("Acetylsalicylic acid");
    med.setCategory("Pain Relief");
    med.setManufacturer("PharmaCorp");
    med.setDescription("For pain and fever");
    med.setUnitPrice(3000.0);
    med.setQuantityInStock(200);
    med.setReorderLevel(50);
    med.setExpiryDate("2025-12-31");
    med.setDosageForm("Tablet");
    med.setStrength("500mg");

    EXPECT_EQ(med.getName(), "Aspirin");
    EXPECT_EQ(med.getGenericName(), "Acetylsalicylic acid");
    EXPECT_EQ(med.getCategory(), "Pain Relief");
    EXPECT_EQ(med.getManufacturer(), "PharmaCorp");
    EXPECT_EQ(med.getDescription(), "For pain and fever");
    EXPECT_DOUBLE_EQ(med.getUnitPrice(), 3000.0);
    EXPECT_EQ(med.getQuantityInStock(), 200);
    EXPECT_EQ(med.getReorderLevel(), 50);
    EXPECT_EQ(med.getExpiryDate(), "2025-12-31");
    EXPECT_EQ(med.getDosageForm(), "Tablet");
    EXPECT_EQ(med.getStrength(), "500mg");
}

TEST(MedicineTest, SetUnitPriceNegativeIgnored)
{
    Medicine med("MED001", "Test", "Cat", 1000.0, 10);

    med.setUnitPrice(-500.0);
    EXPECT_DOUBLE_EQ(med.getUnitPrice(), 1000.0);
}

TEST(MedicineTest, SetQuantityNegativeIgnored)
{
    Medicine med("MED001", "Test", "Cat", 1000.0, 100);

    med.setQuantityInStock(-50);
    EXPECT_EQ(med.getQuantityInStock(), 100);
}

TEST(MedicineTest, SetExpiryDateInvalidIgnored)
{
    Medicine med("MED001", "Test", "Cat", 1000.0, 100);

    med.setExpiryDate("2025-12-31");
    EXPECT_EQ(med.getExpiryDate(), "2025-12-31");

    med.setExpiryDate("invalid-date");
    EXPECT_EQ(med.getExpiryDate(), "2025-12-31"); // Should remain unchanged
}

// ==================== Stock Operations Tests ====================

TEST(MedicineTest, AddStockPositive)
{
    Medicine med("MED001", "Test", "Cat", 1000.0, 100);

    med.addStock(50);
    EXPECT_EQ(med.getQuantityInStock(), 150);
}

TEST(MedicineTest, AddStockNegativeIgnored)
{
    Medicine med("MED001", "Test", "Cat", 1000.0, 100);

    med.addStock(-50);
    EXPECT_EQ(med.getQuantityInStock(), 100);
}

TEST(MedicineTest, AddStockZeroIgnored)
{
    Medicine med("MED001", "Test", "Cat", 1000.0, 100);

    med.addStock(0);
    EXPECT_EQ(med.getQuantityInStock(), 100);
}

TEST(MedicineTest, RemoveStockSuccess)
{
    Medicine med("MED001", "Test", "Cat", 1000.0, 100);

    bool result = med.removeStock(30);
    EXPECT_TRUE(result);
    EXPECT_EQ(med.getQuantityInStock(), 70);
}

TEST(MedicineTest, RemoveStockInsufficient)
{
    Medicine med("MED001", "Test", "Cat", 1000.0, 100);

    bool result = med.removeStock(150);
    EXPECT_FALSE(result);
    EXPECT_EQ(med.getQuantityInStock(), 100); // Unchanged
}

TEST(MedicineTest, RemoveStockNegativeOrZero)
{
    Medicine med("MED001", "Test", "Cat", 1000.0, 100);

    EXPECT_FALSE(med.removeStock(0));
    EXPECT_FALSE(med.removeStock(-10));
    EXPECT_EQ(med.getQuantityInStock(), 100);
}

TEST(MedicineTest, RemoveStockExactAmount)
{
    Medicine med("MED001", "Test", "Cat", 1000.0, 100);

    bool result = med.removeStock(100);
    EXPECT_TRUE(result);
    EXPECT_EQ(med.getQuantityInStock(), 0);
}

// ==================== isLowStock Tests ====================

TEST(MedicineTest, IsLowStockTrue)
{
    Medicine med("MED001", "Test", "Cat", 1000.0, 5);
    med.setReorderLevel(10);

    EXPECT_TRUE(med.isLowStock());
}

TEST(MedicineTest, IsLowStockFalse)
{
    Medicine med("MED001", "Test", "Cat", 1000.0, 100);
    med.setReorderLevel(10);

    EXPECT_FALSE(med.isLowStock());
}

TEST(MedicineTest, IsLowStockEqual)
{
    Medicine med("MED001", "Test", "Cat", 1000.0, 10);
    med.setReorderLevel(10);

    EXPECT_TRUE(med.isLowStock()); // Equal to reorder level is considered low
}

// ==================== isExpired Tests ====================

TEST(MedicineTest, IsExpiredTrue)
{
    Medicine med("MED001", "Test", "Cat", 1000.0, 100);
    med.setExpiryDate("2020-01-01"); // Past date

    EXPECT_TRUE(med.isExpired());
}

TEST(MedicineTest, IsExpiredFalse)
{
    Medicine med("MED001", "Test", "Cat", 1000.0, 100);
    med.setExpiryDate("2099-12-31"); // Far future date

    EXPECT_FALSE(med.isExpired());
}

TEST(MedicineTest, IsExpiredEmptyDate)
{
    Medicine med("MED001", "Test", "Cat", 1000.0, 100);
    // No expiry date set

    EXPECT_FALSE(med.isExpired());
}

// ==================== isExpiringSoon Tests ====================

TEST(MedicineTest, IsExpiringSoonTrue)
{
    Medicine med("MED001", "Test", "Cat", 1000.0, 100);
    // Set expiry to 10 days from now (less than default 30 days threshold)
    std::string currentDate = Utils::getCurrentDate();
    int year = std::stoi(currentDate.substr(0, 4));
    int month = std::stoi(currentDate.substr(5, 2));
    int day = std::stoi(currentDate.substr(8, 2)) + 10;

    // Handle overflow
    if (day > 28)
    {
        day = 15;
        month++;
        if (month > 12)
        {
            month = 1;
            year++;
        }
    }

    std::string expiryDate = std::to_string(year) + "-" +
                             (month < 10 ? "0" : "") + std::to_string(month) +
                             "-" + (day < 10 ? "0" : "") + std::to_string(day);
    med.setExpiryDate(expiryDate);

    EXPECT_TRUE(med.isExpiringSoon(30));
}

TEST(MedicineTest, IsExpiringSoonFalse)
{
    Medicine med("MED001", "Test", "Cat", 1000.0, 100);
    med.setExpiryDate("2099-12-31"); // Far future

    EXPECT_FALSE(med.isExpiringSoon(30));
}

TEST(MedicineTest, IsExpiringSoonEmptyDate)
{
    Medicine med("MED001", "Test", "Cat", 1000.0, 100);
    // No expiry date set

    EXPECT_FALSE(med.isExpiringSoon(30));
}

// ==================== Serialization Tests ====================

TEST(MedicineTest, SerializeReturnsCorrectFormat)
{
    Medicine med("MED001", "Paracetamol", "Pain Relief", 5000.0, 500);
    med.setGenericName("Acetaminophen");
    med.setManufacturer("PharmaCorp");
    med.setDescription("For pain and fever");
    med.setReorderLevel(100);
    med.setExpiryDate("2025-12-31");
    med.setDosageForm("Tablet");
    med.setStrength("500mg");

    std::string serialized = med.serialize();

    EXPECT_NE(serialized.find("MED001"), std::string::npos);
    EXPECT_NE(serialized.find("Paracetamol"), std::string::npos);
    EXPECT_NE(serialized.find("Pain Relief"), std::string::npos);
    EXPECT_NE(serialized.find("5000"), std::string::npos);

    // Count pipes (should be 11 for 12 fields)
    int pipes = 0;
    for (char c : serialized)
    {
        if (c == '|')
            pipes++;
    }
    EXPECT_EQ(pipes, 11);
}

// ==================== Deserialization Tests ====================

TEST(MedicineTest, DeserializeValid)
{
    std::string line =
        "MED001|Paracetamol|Acetaminophen|Pain Relief|PharmaCorp|For "
        "pain|5000|500|100|2025-12-31|Tablet|500mg";

    auto result = Medicine::deserialize(line);

    ASSERT_TRUE(result.has_value());

    Medicine med = result.value();
    EXPECT_EQ(med.getMedicineID(), "MED001");
    EXPECT_EQ(med.getName(), "Paracetamol");
    EXPECT_EQ(med.getGenericName(), "Acetaminophen");
    EXPECT_EQ(med.getCategory(), "Pain Relief");
    EXPECT_EQ(med.getManufacturer(), "PharmaCorp");
    EXPECT_DOUBLE_EQ(med.getUnitPrice(), 5000.0);
    EXPECT_EQ(med.getQuantityInStock(), 500);
    EXPECT_EQ(med.getReorderLevel(), 100);
    EXPECT_EQ(med.getExpiryDate(), "2025-12-31");
    EXPECT_EQ(med.getDosageForm(), "Tablet");
    EXPECT_EQ(med.getStrength(), "500mg");
}

TEST(MedicineTest, DeserializeEmptyReturnsNullopt)
{
    auto result = Medicine::deserialize("");
    EXPECT_FALSE(result.has_value());
}

TEST(MedicineTest, DeserializeCommentReturnsNullopt)
{
    auto result = Medicine::deserialize("#MED001|Test|...");
    EXPECT_FALSE(result.has_value());
}

TEST(MedicineTest, DeserializeInvalidFieldCountReturnsNullopt)
{
    std::string line = "MED001|Paracetamol|Pain Relief"; // Not enough fields

    auto result = Medicine::deserialize(line);
    EXPECT_FALSE(result.has_value());
}

TEST(MedicineTest, DeserializeNegativePriceReturnsNullopt)
{
    std::string line = "MED001|Paracetamol|Generic|Cat|Mfg|Desc|-5000|500|100|"
                       "2025-12-31|Tablet|500mg";

    auto result = Medicine::deserialize(line);
    EXPECT_FALSE(result.has_value());
}

TEST(MedicineTest, DeserializeInvalidDateReturnsNullopt)
{
    std::string line = "MED001|Paracetamol|Generic|Cat|Mfg|Desc|5000|500|100|"
                       "invalid-date|Tablet|500mg";

    auto result = Medicine::deserialize(line);
    EXPECT_FALSE(result.has_value());
}

// ==================== Round-trip Serialization ====================

TEST(MedicineTest, RoundTripSerialization)
{
    Medicine original("MED001", "Paracetamol", "Pain Relief", 5000.0, 500);
    original.setGenericName("Acetaminophen");
    original.setManufacturer("PharmaCorp");
    original.setDescription("For pain");
    original.setReorderLevel(100);
    original.setExpiryDate("2025-12-31");
    original.setDosageForm("Tablet");
    original.setStrength("500mg");

    std::string serialized = original.serialize();
    auto result = Medicine::deserialize(serialized);

    ASSERT_TRUE(result.has_value());

    Medicine restored = result.value();
    EXPECT_EQ(restored.getMedicineID(), original.getMedicineID());
    EXPECT_EQ(restored.getName(), original.getName());
    EXPECT_EQ(restored.getGenericName(), original.getGenericName());
    EXPECT_EQ(restored.getCategory(), original.getCategory());
    EXPECT_DOUBLE_EQ(restored.getUnitPrice(), original.getUnitPrice());
    EXPECT_EQ(restored.getQuantityInStock(), original.getQuantityInStock());
    EXPECT_EQ(restored.getReorderLevel(), original.getReorderLevel());
    EXPECT_EQ(restored.getExpiryDate(), original.getExpiryDate());
}

/*
cd build && ./HospitalTests --gtest_filter="MedicineTest.*"
*/
