#include "advance/Prescription.h"
#include "common/Constants.h"
#include "common/Types.h"
#include "common/Utils.h"
#include <gtest/gtest.h>
#include <string>
#include <vector>

using namespace HMS::Model;
using namespace HMS;

// ==================== Constructor Tests ====================

TEST(PrescriptionTest, DefaultConstructor)
{
    Prescription rx;

    EXPECT_EQ(rx.getPrescriptionID(), "");
    EXPECT_EQ(rx.getAppointmentID(), "");
    EXPECT_EQ(rx.getPatientUsername(), "");
    EXPECT_EQ(rx.getDoctorID(), "");
    EXPECT_EQ(rx.getPrescriptionDate(), "");
    EXPECT_EQ(rx.getDiagnosis(), "");
    EXPECT_EQ(rx.getNotes(), "");
    EXPECT_FALSE(rx.isDispensed());
    EXPECT_EQ(rx.getItemCount(), 0);
}

TEST(PrescriptionTest, ParameterizedConstructor)
{
    Prescription rx("PRE001", "APT001", "patient001", "D001", "2024-03-15");

    EXPECT_EQ(rx.getPrescriptionID(), "PRE001");
    EXPECT_EQ(rx.getAppointmentID(), "APT001");
    EXPECT_EQ(rx.getPatientUsername(), "patient001");
    EXPECT_EQ(rx.getDoctorID(), "D001");
    EXPECT_EQ(rx.getPrescriptionDate(), "2024-03-15");
    EXPECT_EQ(rx.getDiagnosis(), "");
    EXPECT_EQ(rx.getNotes(), "");
    EXPECT_FALSE(rx.isDispensed());
    EXPECT_EQ(rx.getItemCount(), 0);
}

// ==================== Getters/Setters Tests ====================

TEST(PrescriptionTest, SettersUpdateValuesCorrectly)
{
    Prescription rx("PRE001", "APT001", "patient001", "D001", "2024-03-15");

    rx.setDiagnosis("Viral Fever");
    rx.setNotes("Rest recommended");
    rx.setDispensed(true);

    EXPECT_EQ(rx.getDiagnosis(), "Viral Fever");
    EXPECT_EQ(rx.getNotes(), "Rest recommended");
    EXPECT_TRUE(rx.isDispensed());
}

// ==================== Item Management Tests ====================

TEST(PrescriptionTest, AddItemSuccess)
{
    Prescription rx("PRE001", "APT001", "patient001", "D001", "2024-03-15");

    PrescriptionItem item1;
    item1.medicineID = "MED001";
    item1.medicineName = "Paracetamol";
    item1.quantity = 20;
    item1.dosage = "2 tablets 3 times daily";
    item1.duration = "5 days";
    item1.instructions = "After meals";

    PrescriptionItem item2;
    item2.medicineID = "MED002";
    item2.medicineName = "Amoxicillin";
    item2.quantity = 10;
    item2.dosage = "1 capsule twice daily";
    item2.duration = "7 days";
    item2.instructions = "Before breakfast";

    rx.addItem(item1);
    rx.addItem(item2);

    EXPECT_EQ(rx.getItemCount(), 2);
}

TEST(PrescriptionTest, AddItemEmptyMedicineIDIgnored)
{
    Prescription rx("PRE001", "APT001", "patient001", "D001", "2024-03-15");

    PrescriptionItem item;
    item.medicineID = "";
    item.medicineName = "Test";
    item.quantity = 10;

    rx.addItem(item);

    EXPECT_EQ(rx.getItemCount(), 0);
}

TEST(PrescriptionTest, AddItemDuplicateUpdates)
{
    Prescription rx("PRE001", "APT001", "patient001", "D001", "2024-03-15");

    PrescriptionItem item1;
    item1.medicineID = "MED001";
    item1.medicineName = "Paracetamol";
    item1.quantity = 10;
    item1.dosage = "1 tablet daily";
    item1.duration = "5 days";
    item1.instructions = "After meals";

    PrescriptionItem item2;
    item2.medicineID = "MED001"; // Same medicine ID
    item2.medicineName = "Paracetamol Updated";
    item2.quantity = 20;              // Updated quantity
    item2.dosage = "2 tablets daily"; // Updated dosage
    item2.duration = "7 days";
    item2.instructions = "Before meals";

    rx.addItem(item1);
    rx.addItem(item2);

    EXPECT_EQ(rx.getItemCount(),
              1); // Should still be 1 (updated, not duplicated)

    auto items = rx.getItems();
    EXPECT_EQ(items[0].quantity, 20); // Should have updated values
}

TEST(PrescriptionTest, RemoveItemSuccess)
{
    Prescription rx("PRE001", "APT001", "patient001", "D001", "2024-03-15");

    PrescriptionItem item1;
    item1.medicineID = "MED001";
    item1.medicineName = "Paracetamol";
    item1.quantity = 10;

    PrescriptionItem item2;
    item2.medicineID = "MED002";
    item2.medicineName = "Amoxicillin";
    item2.quantity = 10;

    rx.addItem(item1);
    rx.addItem(item2);

    rx.removeItem("MED001");

    EXPECT_EQ(rx.getItemCount(), 1);

    auto items = rx.getItems();
    EXPECT_EQ(items[0].medicineID, "MED002");
}

TEST(PrescriptionTest, RemoveItemNotFound)
{
    Prescription rx("PRE001", "APT001", "patient001", "D001", "2024-03-15");

    PrescriptionItem item;
    item.medicineID = "MED001";
    item.medicineName = "Paracetamol";
    item.quantity = 10;

    rx.addItem(item);

    rx.removeItem("MED999"); // Non-existent

    EXPECT_EQ(rx.getItemCount(), 1); // Should remain unchanged
}

TEST(PrescriptionTest, ClearItems)
{
    Prescription rx("PRE001", "APT001", "patient001", "D001", "2024-03-15");

    PrescriptionItem item1;
    item1.medicineID = "MED001";
    item1.medicineName = "Paracetamol";
    item1.quantity = 10;

    PrescriptionItem item2;
    item2.medicineID = "MED002";
    item2.medicineName = "Amoxicillin";
    item2.quantity = 10;

    rx.addItem(item1);
    rx.addItem(item2);

    rx.clearItems();

    EXPECT_EQ(rx.getItemCount(), 0);
}

TEST(PrescriptionTest, GetItems)
{
    Prescription rx("PRE001", "APT001", "patient001", "D001", "2024-03-15");

    PrescriptionItem item;
    item.medicineID = "MED001";
    item.medicineName = "Paracetamol";
    item.quantity = 20;
    item.dosage = "2 tablets daily";
    item.duration = "5 days";
    item.instructions = "After meals";

    rx.addItem(item);

    auto items = rx.getItems();
    EXPECT_EQ(items.size(), 1);
    EXPECT_EQ(items[0].medicineID, "MED001");
    EXPECT_EQ(items[0].medicineName, "Paracetamol");
    EXPECT_EQ(items[0].quantity, 20);
}


// ==================== Serialization Tests ====================

TEST(PrescriptionTest, SerializeReturnsCorrectFormat)
{
    Prescription rx("PRE001", "APT001", "patient001", "D001", "2024-03-15");
    rx.setDiagnosis("Viral Fever");
    rx.setNotes("Rest recommended");
    rx.setDispensed(true);

    PrescriptionItem item;
    item.medicineID = "MED001";
    item.medicineName = "Paracetamol";
    item.quantity = 20;
    item.dosage = "2 tablets daily";
    item.duration = "5 days";
    item.instructions = "After meals";

    rx.addItem(item);

    std::string serialized = rx.serialize();

    EXPECT_NE(serialized.find("PRE001"), std::string::npos);
    EXPECT_NE(serialized.find("APT001"), std::string::npos);
    EXPECT_NE(serialized.find("patient001"), std::string::npos);
    EXPECT_NE(serialized.find("Viral Fever"), std::string::npos);
    EXPECT_NE(serialized.find("MED001"), std::string::npos);

    // Count pipes (should be 8 for 9 fields)
    int pipes = 0;
    for (char c : serialized)
    {
        if (c == '|')
            pipes++;
    }
    EXPECT_EQ(pipes, 8);
}

TEST(PrescriptionTest, SerializeEmptyItems)
{
    Prescription rx("PRE001", "APT001", "patient001", "D001", "2024-03-15");
    rx.setDiagnosis("Check-up");

    std::string serialized = rx.serialize();

    EXPECT_NE(serialized.find("PRE001"), std::string::npos);
}

TEST(PrescriptionTest, SerializeMultipleItems)
{
    Prescription rx("PRE001", "APT001", "patient001", "D001", "2024-03-15");

    PrescriptionItem item1;
    item1.medicineID = "MED001";
    item1.medicineName = "Paracetamol";
    item1.quantity = 20;
    item1.dosage = "2 tablets";
    item1.duration = "5 days";
    item1.instructions = "After meals";

    PrescriptionItem item2;
    item2.medicineID = "MED002";
    item2.medicineName = "Amoxicillin";
    item2.quantity = 10;
    item2.dosage = "1 capsule";
    item2.duration = "7 days";
    item2.instructions = "Before meals";

    rx.addItem(item1);
    rx.addItem(item2);

    std::string serialized = rx.serialize();

    // Items should be separated by semicolon
    EXPECT_NE(serialized.find("MED001"), std::string::npos);
    EXPECT_NE(serialized.find("MED002"), std::string::npos);
    EXPECT_NE(serialized.find(";"), std::string::npos);
}

// ==================== Deserialization Tests ====================

TEST(PrescriptionTest, DeserializeValid)
{
    std::string line =
        "PRE001|APT001|patient001|D001|2024-03-15|Viral Fever|Rest "
        "recommended|1|MED001:Paracetamol:20:2 tablets:5 days:After meals";

    auto result = Prescription::deserialize(line);

    ASSERT_TRUE(result.has_value());

    Prescription rx = result.value();
    EXPECT_EQ(rx.getPrescriptionID(), "PRE001");
    EXPECT_EQ(rx.getAppointmentID(), "APT001");
    EXPECT_EQ(rx.getPatientUsername(), "patient001");
    EXPECT_EQ(rx.getDoctorID(), "D001");
    EXPECT_EQ(rx.getPrescriptionDate(), "2024-03-15");
    EXPECT_EQ(rx.getDiagnosis(), "Viral Fever");
    EXPECT_EQ(rx.getNotes(), "Rest recommended");
    EXPECT_TRUE(rx.isDispensed());
    EXPECT_EQ(rx.getItemCount(), 1);

    auto items = rx.getItems();
    EXPECT_EQ(items[0].medicineID, "MED001");
    EXPECT_EQ(items[0].medicineName, "Paracetamol");
    EXPECT_EQ(items[0].quantity, 20);
}

TEST(PrescriptionTest, DeserializeMultipleItems)
{
    std::string line =
        "PRE001|APT001|patient001|D001|2024-03-15|Fever|Rest|0|MED001:"
        "Paracetamol:20:2 tablets:5 days:After meals;MED002:Amoxicillin:10:1 "
        "capsule:7 days:Before meals";

    auto result = Prescription::deserialize(line);

    ASSERT_TRUE(result.has_value());

    Prescription rx = result.value();
    EXPECT_EQ(rx.getItemCount(), 2);

    auto items = rx.getItems();
    EXPECT_EQ(items[0].medicineID, "MED001");
    EXPECT_EQ(items[1].medicineID, "MED002");
}

TEST(PrescriptionTest, DeserializeEmptyItems)
{
    std::string line = "PRE001|APT001|patient001|D001|2024-03-15|Check-up||0|";

    auto result = Prescription::deserialize(line);

    ASSERT_TRUE(result.has_value());

    Prescription rx = result.value();
    EXPECT_EQ(rx.getItemCount(), 0);
}

TEST(PrescriptionTest, DeserializeEmptyReturnsNullopt)
{
    auto result = Prescription::deserialize("");
    EXPECT_FALSE(result.has_value());
}

TEST(PrescriptionTest, DeserializeCommentReturnsNullopt)
{
    auto result = Prescription::deserialize("#PRE001|APT001|...");
    EXPECT_FALSE(result.has_value());
}

TEST(PrescriptionTest, DeserializeInvalidFieldCountReturnsNullopt)
{
    std::string line = "PRE001|APT001|patient001"; // Not enough fields

    auto result = Prescription::deserialize(line);
    EXPECT_FALSE(result.has_value());
}

TEST(PrescriptionTest, DeserializeInvalidDateReturnsNullopt)
{
    std::string line =
        "PRE001|APT001|patient001|D001|invalid-date|Fever|Notes|0|";

    auto result = Prescription::deserialize(line);
    EXPECT_FALSE(result.has_value());
}

// ==================== Round-trip Serialization ====================

TEST(PrescriptionTest, RoundTripSerialization)
{
    Prescription original("PRE001", "APT001", "patient001", "D001", "2024-03-15");
    original.setDiagnosis("Viral Fever");
    original.setNotes("Rest recommended");
    original.setDispensed(true);

    PrescriptionItem item;
    item.medicineID = "MED001";
    item.medicineName = "Paracetamol";
    item.quantity = 20;
    item.dosage = "2 tablets";
    item.duration = "5 days";
    item.instructions = "After meals";

    original.addItem(item);

    std::string serialized = original.serialize();
    auto result = Prescription::deserialize(serialized);

    ASSERT_TRUE(result.has_value());

    Prescription restored = result.value();
    EXPECT_EQ(restored.getPrescriptionID(), original.getPrescriptionID());
    EXPECT_EQ(restored.getAppointmentID(), original.getAppointmentID());
    EXPECT_EQ(restored.getPatientUsername(), original.getPatientUsername());
    EXPECT_EQ(restored.getDoctorID(), original.getDoctorID());
    EXPECT_EQ(restored.getPrescriptionDate(), original.getPrescriptionDate());
    EXPECT_EQ(restored.getDiagnosis(), original.getDiagnosis());
    EXPECT_EQ(restored.getNotes(), original.getNotes());
    EXPECT_EQ(restored.isDispensed(), original.isDispensed());
    EXPECT_EQ(restored.getItemCount(), original.getItemCount());
}

TEST(PrescriptionTest, RoundTripSerializationMultipleItems)
{
    Prescription original("PRE001", "APT001", "patient001", "D001", "2024-03-15");

    PrescriptionItem item1;
    item1.medicineID = "MED001";
    item1.medicineName = "Paracetamol";
    item1.quantity = 20;
    item1.dosage = "2 tablets";
    item1.duration = "5 days";
    item1.instructions = "After meals";

    PrescriptionItem item2;
    item2.medicineID = "MED002";
    item2.medicineName = "Amoxicillin";
    item2.quantity = 10;
    item2.dosage = "1 capsule";
    item2.duration = "7 days";
    item2.instructions = "Before meals";

    original.addItem(item1);
    original.addItem(item2);

    std::string serialized = original.serialize();
    auto result = Prescription::deserialize(serialized);

    ASSERT_TRUE(result.has_value());

    Prescription restored = result.value();
    EXPECT_EQ(restored.getItemCount(), 2);

    auto items = restored.getItems();
    EXPECT_EQ(items[0].medicineID, "MED001");
    EXPECT_EQ(items[1].medicineID, "MED002");
}

// ==================== toPrintFormat Tests ====================

TEST(PrescriptionTest, ToPrintFormatReturnsNonEmptyString)
{
    Prescription rx("PRE001", "APT001", "patient001", "D001", "2024-03-15");
    rx.setDiagnosis("Viral Fever");

    PrescriptionItem item;
    item.medicineID = "MED001";
    item.medicineName = "Paracetamol";
    item.quantity = 20;
    item.dosage = "2 tablets daily";
    item.duration = "5 days";
    item.instructions = "After meals";

    rx.addItem(item);

    std::string printFormat = rx.toPrintFormat();

    EXPECT_FALSE(printFormat.empty());
    EXPECT_NE(printFormat.find("PRE001"), std::string::npos);
    EXPECT_NE(printFormat.find("patient001"), std::string::npos);
    EXPECT_NE(printFormat.find("Paracetamol"), std::string::npos);
    EXPECT_NE(printFormat.find("PRESCRIPTION"), std::string::npos);
}

/*
cd build && ./HospitalTests --gtest_filter="PrescriptionTest.*"
*/
