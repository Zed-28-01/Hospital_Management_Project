#include <gtest/gtest.h>
#include "model/Appointment.h"
#include "common/Utils.h"

using namespace HMS;
using namespace HMS::Model;

// ==================== Constructor Tests ====================

TEST(AppointmentTest, DefaultConstructor)
{
    Appointment apt;

    // Default-constructed strings should be empty
    EXPECT_EQ(apt.getAppointmentID(), "");
    EXPECT_EQ(apt.getPatientUsername(), "");
    EXPECT_EQ(apt.getDoctorID(), "");
    EXPECT_EQ(apt.getDate(), "");
    EXPECT_EQ(apt.getTime(), "");
    EXPECT_EQ(apt.getDisease(), "");
    EXPECT_EQ(apt.getNotes(), "");
}

TEST(AppointmentTest, ParameterizedConstructor)
{
    Appointment apt(
        "APT001",
        "patient01",
        "D001",
        "2025-12-20",
        "10:00",
        "Flu symptoms",
        500000.0);

    EXPECT_EQ(apt.getAppointmentID(), "APT001");
    EXPECT_EQ(apt.getPatientUsername(), "patient01");
    EXPECT_EQ(apt.getDoctorID(), "D001");
    EXPECT_EQ(apt.getDate(), "2025-12-20");
    EXPECT_EQ(apt.getTime(), "10:00");
    EXPECT_EQ(apt.getDisease(), "Flu symptoms");
    EXPECT_EQ(apt.getPrice(), 500000.0);
    EXPECT_FALSE(apt.isPaid());
    EXPECT_EQ(apt.getStatus(), AppointmentStatus::SCHEDULED);
    EXPECT_EQ(apt.getNotes(), "");
}

TEST(AppointmentTest, FullParameterizedConstructor)
{
    Appointment apt(
        "APT002",
        "patient02",
        "D002",
        "2025-01-10",
        "14:30",
        "Diabetes check",
        750000.0,
        true,
        AppointmentStatus::COMPLETED,
        "Doctor recommended insulin");

    EXPECT_EQ(apt.getAppointmentID(), "APT002");
    EXPECT_EQ(apt.getPatientUsername(), "patient02");
    EXPECT_EQ(apt.getDoctorID(), "D002");
    EXPECT_EQ(apt.getDate(), "2025-01-10");
    EXPECT_EQ(apt.getTime(), "14:30");
    EXPECT_EQ(apt.getDisease(), "Diabetes check");
    EXPECT_EQ(apt.getPrice(), 750000.0);
    EXPECT_TRUE(apt.isPaid());
    EXPECT_EQ(apt.getStatus(), AppointmentStatus::COMPLETED);
    EXPECT_EQ(apt.getNotes(), "Doctor recommended insulin");
}

// ==================== Getters ====================

TEST(AppointmentTest, GetDateTime)
{
    Appointment apt(
        "APT003",
        "patient03",
        "D003",
        "2025-12-25",
        "09:15",
        "Check-up",
        300000.0);

    EXPECT_EQ(apt.getDateTime(), "2025-12-25 09:15");
}

TEST(AppointmentTest, GetStatusString)
{
    Appointment apt1(
        "APT004", "p", "D", "2025-01-01", "10:00",
        "Disease", 100000.0);

    EXPECT_EQ(apt1.getStatusString(), "scheduled");

    Appointment apt2(
        "APT005", "p", "D", "2024-01-01", "10:00",
        "Disease", 100000.0, false, AppointmentStatus::COMPLETED, "");

    EXPECT_EQ(apt2.getStatusString(), "completed");

    Appointment apt3(
        "APT006", "p", "D", "2024-01-01", "10:00",
        "Disease", 100000.0, false, AppointmentStatus::CANCELLED, "");

    EXPECT_EQ(apt3.getStatusString(), "cancelled");

    Appointment apt4(
        "APT007", "p", "D", "2024-01-01", "10:00",
        "Disease", 100000.0, false, AppointmentStatus::NO_SHOW, "");

    EXPECT_EQ(apt4.getStatusString(), "no_show");
}

// ==================== Setters with Validation ====================

TEST(AppointmentTest, SetDateValid)
{
    Appointment apt(
        "APT008", "p", "D", "2025-01-01", "10:00",
        "Disease", 100000.0);

    apt.setDate("2025-12-31");
    EXPECT_EQ(apt.getDate(), "2025-12-31");
}

TEST(AppointmentTest, SetDateInvalid)
{
    Appointment apt(
        "APT009", "p", "D", "2025-01-01", "10:00",
        "Disease", 100000.0);

    apt.setDate("invalid-date");
    EXPECT_EQ(apt.getDate(), "2025-01-01");
}

TEST(AppointmentTest, SetTimeValid)
{
    Appointment apt(
        "APT010", "p", "D", "2025-01-01", "10:00",
        "Disease", 100000.0);

    apt.setTime("15:45");
    EXPECT_EQ(apt.getTime(), "15:45");
}

TEST(AppointmentTest, SetTimeInvalid)
{
    Appointment apt(
        "APT011", "p", "D", "2025-01-01", "10:00",
        "Disease", 100000.0);

    apt.setTime("25:99");
    EXPECT_EQ(apt.getTime(), "10:00");
}

TEST(AppointmentTest, SetDisease)
{
    Appointment apt(
        "APT012", "p", "D", "2025-01-01", "10:00",
        "Old disease", 100000.0);

    apt.setDisease("New disease");
    EXPECT_EQ(apt.getDisease(), "New disease");
}

TEST(AppointmentTest, SetPriceValid)
{
    Appointment apt(
        "APT013", "p", "D", "2025-01-01", "10:00",
        "Disease", 100000.0);

    apt.setPrice(250000.0);
    EXPECT_EQ(apt.getPrice(), 250000.0);
}

TEST(AppointmentTest, SetPriceNegativeIgnored)
{
    Appointment apt(
        "APT014", "p", "D", "2025-01-01", "10:00",
        "Disease", 100000.0);

    apt.setPrice(-50000.0);
    EXPECT_EQ(apt.getPrice(), 100000.0);
}

TEST(AppointmentTest, SetPriceZero)
{
    Appointment apt(
        "APT015", "p", "D", "2025-01-01", "10:00",
        "Disease", 100000.0);

    apt.setPrice(0.0);
    EXPECT_EQ(apt.getPrice(), 0.0);
}

TEST(AppointmentTest, SetPaidStatus)
{
    Appointment apt(
        "APT016", "p", "D", "2025-01-01", "10:00",
        "Disease", 100000.0);

    EXPECT_FALSE(apt.isPaid());
    apt.setPaid(true);
    EXPECT_TRUE(apt.isPaid());
}

TEST(AppointmentTest, SetStatus)
{
    Appointment apt(
        "APT017", "p", "D", "2025-01-01", "10:00",
        "Disease", 100000.0);

    apt.setStatus(AppointmentStatus::COMPLETED);
    EXPECT_EQ(apt.getStatus(), AppointmentStatus::COMPLETED);

    apt.setStatus(AppointmentStatus::CANCELLED);
    EXPECT_EQ(apt.getStatus(), AppointmentStatus::CANCELLED);
}

TEST(AppointmentTest, SetNotes)
{
    Appointment apt(
        "APT018", "p", "D", "2025-01-01", "10:00",
        "Disease", 100000.0);

    apt.setNotes("Patient arrived late");
    EXPECT_EQ(apt.getNotes(), "Patient arrived late");
}

// ==================== Status Methods ====================

TEST(AppointmentTest, MarkAsCompleted)
{
    Appointment apt(
        "APT019", "p", "D", "2025-01-01", "10:00",
        "Disease", 100000.0);

    apt.markAsCompleted();
    EXPECT_EQ(apt.getStatus(), AppointmentStatus::COMPLETED);
}

TEST(AppointmentTest, MarkAsCancelled)
{
    Appointment apt(
        "APT020", "p", "D", "2025-01-01", "10:00",
        "Disease", 100000.0);

    apt.markAsCancelled();
    EXPECT_EQ(apt.getStatus(), AppointmentStatus::CANCELLED);
}

TEST(AppointmentTest, MarkAsNoShow)
{
    Appointment apt(
        "APT021", "p", "D", "2025-01-01", "10:00",
        "Disease", 100000.0);

    apt.markAsNoShow();
    EXPECT_EQ(apt.getStatus(), AppointmentStatus::NO_SHOW);
}

// ==================== isUpcoming Tests ====================

TEST(AppointmentTest, IsUpcoming_FutureDate_Scheduled)
{
    Appointment apt(
        "APT022", "p", "D", "2099-12-31", "10:00",
        "Disease", 100000.0, false, AppointmentStatus::SCHEDULED, "");

    EXPECT_TRUE(apt.isUpcoming());
}

TEST(AppointmentTest, IsUpcoming_PastDate_Scheduled)
{
    Appointment apt(
        "APT023", "p", "D", "2000-01-01", "10:00",
        "Disease", 100000.0, false, AppointmentStatus::SCHEDULED, "");

    EXPECT_FALSE(apt.isUpcoming());
}

TEST(AppointmentTest, IsUpcoming_FutureDate_Completed)
{
    Appointment apt(
        "APT024", "p", "D", "2099-12-31", "10:00",
        "Disease", 100000.0, false, AppointmentStatus::COMPLETED, "");

    EXPECT_FALSE(apt.isUpcoming());
}

TEST(AppointmentTest, IsUpcoming_FutureDate_Cancelled)
{
    Appointment apt(
        "APT025", "p", "D", "2099-12-31", "10:00",
        "Disease", 100000.0, false, AppointmentStatus::CANCELLED, "");

    EXPECT_FALSE(apt.isUpcoming());
}

// ==================== canCancel Tests ====================

TEST(AppointmentTest, CanCancel_FutureDate_Scheduled)
{
    Appointment apt(
        "APT026", "p", "D", "2099-12-31", "10:00",
        "Disease", 100000.0, false, AppointmentStatus::SCHEDULED, "");

    EXPECT_TRUE(apt.canCancel());
}

TEST(AppointmentTest, CanCancel_PastDate_Scheduled)
{
    Appointment apt(
        "APT027", "p", "D", "2000-01-01", "10:00",
        "Disease", 100000.0, false, AppointmentStatus::SCHEDULED, "");

    EXPECT_FALSE(apt.canCancel());
}

TEST(AppointmentTest, CanCancel_FutureDate_Completed)
{
    Appointment apt(
        "APT028", "p", "D", "2099-12-31", "10:00",
        "Disease", 100000.0, false, AppointmentStatus::COMPLETED, "");

    EXPECT_FALSE(apt.canCancel());
}

TEST(AppointmentTest, CanCancel_AlreadyCancelled)
{
    Appointment apt(
        "APT029", "p", "D", "2099-12-31", "10:00",
        "Disease", 100000.0, false, AppointmentStatus::CANCELLED, "");

    EXPECT_FALSE(apt.canCancel());
}

// ==================== canEdit Tests ====================

TEST(AppointmentTest, CanEdit_FutureDate_Scheduled)
{
    Appointment apt(
        "APT030", "p", "D", "2099-12-31", "10:00",
        "Disease", 100000.0, false, AppointmentStatus::SCHEDULED, "");

    EXPECT_TRUE(apt.canEdit());
}

TEST(AppointmentTest, CanEdit_PastDate_Scheduled)
{
    Appointment apt(
        "APT031", "p", "D", "2000-01-01", "10:00",
        "Disease", 100000.0, false, AppointmentStatus::SCHEDULED, "");

    EXPECT_FALSE(apt.canEdit());
}

TEST(AppointmentTest, CanEdit_FutureDate_Completed)
{
    Appointment apt(
        "APT032", "p", "D", "2099-12-31", "10:00",
        "Disease", 100000.0, false, AppointmentStatus::COMPLETED, "");

    EXPECT_FALSE(apt.canEdit());
}

// ==================== Serialization Tests ====================

TEST(AppointmentTest, SerializeSimple)
{
    Appointment apt(
        "APT033", "patient01", "D001", "2025-12-20", "10:00",
        "Flu", 500000.0);

    std::string serialized = apt.serialize();

    // Updated: std::format("{:.2f}", 500000.0) produces "500000.00"
    EXPECT_EQ(serialized,
              "APT033|patient01|D001|2025-12-20|10:00|Flu|500000.00|0|scheduled|");
}

TEST(AppointmentTest, SerializeWithAllFields)
{
    Appointment apt(
        "APT034", "patient02", "D002", "2025-01-10", "14:30",
        "Diabetes check", 750000.0, true, AppointmentStatus::COMPLETED,
        "Doctor recommended insulin");

    std::string serialized = apt.serialize();

    // Updated: std::format("{:.2f}", 750000.0) produces "750000.00"
    EXPECT_EQ(serialized,
              "APT034|patient02|D002|2025-01-10|14:30|Diabetes check|750000.00|1|completed|Doctor recommended insulin");
}

TEST(AppointmentTest, SerializeEmptyNotes)
{
    Appointment apt(
        "APT035", "p", "D", "2025-01-01", "10:00",
        "Disease", 100000.0, false, AppointmentStatus::SCHEDULED, "");

    std::string serialized = apt.serialize();
    EXPECT_TRUE(serialized.find("|") != std::string::npos);
}

// ==================== Deserialization Tests ====================

TEST(AppointmentTest, DeserializeValid)
{
    std::string line = "APT036|patient03|D003|2025-12-25|09:15|Check-up|300000|0|scheduled|";

    auto result = Appointment::deserialize(line);

    ASSERT_TRUE(result.has_value());

    Appointment apt = result.value();
    EXPECT_EQ(apt.getAppointmentID(), "APT036");
    EXPECT_EQ(apt.getPatientUsername(), "patient03");
    EXPECT_EQ(apt.getDoctorID(), "D003");
    EXPECT_EQ(apt.getDate(), "2025-12-25");
    EXPECT_EQ(apt.getTime(), "09:15");
    EXPECT_EQ(apt.getDisease(), "Check-up");
    EXPECT_DOUBLE_EQ(apt.getPrice(), 300000.0);
    EXPECT_FALSE(apt.isPaid());
    EXPECT_EQ(apt.getStatus(), AppointmentStatus::SCHEDULED);
    EXPECT_EQ(apt.getNotes(), "");
}

TEST(AppointmentTest, DeserializeWithNotes)
{
    std::string line = "APT037|patient04|D004|2025-01-15|11:00|Allergy test|200000|1|completed|Positive for pollen";

    auto result = Appointment::deserialize(line);

    ASSERT_TRUE(result.has_value());

    Appointment apt = result.value();
    EXPECT_EQ(apt.getAppointmentID(), "APT037");
    EXPECT_TRUE(apt.isPaid());
    EXPECT_EQ(apt.getStatus(), AppointmentStatus::COMPLETED);
    EXPECT_EQ(apt.getNotes(), "Positive for pollen");
}

// ==================== Trim Whitespace Tests ====================

TEST(AppointmentTest, DeserializeWithLeadingWhitespace)
{
    std::string line = " APT038| patient05 |D005|2025-01-20|13:00|Disease|150000|0|scheduled|";

    auto result = Appointment::deserialize(line);

    ASSERT_TRUE(result.has_value());

    Appointment apt = result.value();
    EXPECT_EQ(apt.getAppointmentID(), "APT038");
    EXPECT_EQ(apt.getPatientUsername(), "patient05");
    EXPECT_EQ(apt.getDoctorID(), "D005");
}

TEST(AppointmentTest, DeserializeWithTrailingWhitespace)
{
    std::string line = "APT039|patient06 |D006 |2025-01-25|10:00|Disease |150000 |0|scheduled|";

    auto result = Appointment::deserialize(line);

    ASSERT_TRUE(result.has_value());

    Appointment apt = result.value();
    EXPECT_EQ(apt.getAppointmentID(), "APT039");
    EXPECT_EQ(apt.getPatientUsername(), "patient06");
    EXPECT_EQ(apt.getDoctorID(), "D006");
    EXPECT_EQ(apt.getDisease(), "Disease");
    EXPECT_DOUBLE_EQ(apt.getPrice(), 150000.0);
}

TEST(AppointmentTest, DeserializeWithWhitespaceInPrice)
{
    std::string line = "APT040|patient07|D007|2025-02-01|10:00|Disease|  500000.00  |0|scheduled|";

    auto result = Appointment::deserialize(line);

    ASSERT_TRUE(result.has_value());

    Appointment apt = result.value();
    EXPECT_DOUBLE_EQ(apt.getPrice(), 500000.0);
}

TEST(AppointmentTest, DeserializeWithWhitespaceInBooleanField)
{
    std::string line = "APT041|patient08|D008|2025-02-05|10:00|Disease|100000| 1 |scheduled|";

    auto result = Appointment::deserialize(line);

    ASSERT_TRUE(result.has_value());

    Appointment apt = result.value();
    EXPECT_TRUE(apt.isPaid());
}

TEST(AppointmentTest, DeserializeWithWhitespaceInStatus)
{
    std::string line = "APT042|patient09|D009|2025-02-10|10:00|Disease|100000|1| completed |";

    auto result = Appointment::deserialize(line);

    ASSERT_TRUE(result.has_value());

    Appointment apt = result.value();
    EXPECT_EQ(apt.getStatus(), AppointmentStatus::COMPLETED);
}

TEST(AppointmentTest, DeserializeWithWhitespaceInDateTime)
{
    std::string line = "APT043|patient10|D010| 2025-02-15 | 14:30 |Disease|100000|0|scheduled|";

    auto result = Appointment::deserialize(line);

    ASSERT_TRUE(result.has_value());

    Appointment apt = result.value();
    EXPECT_EQ(apt.getDate(), "2025-02-15");
    EXPECT_EQ(apt.getTime(), "14:30");
}

TEST(AppointmentTest, DeserializeWithMultipleSpacesInNotes)
{
    std::string line = "APT044|patient11|D011|2025-02-20|10:00|Disease|100000|0|scheduled|  Note with spaces  ";

    auto result = Appointment::deserialize(line);

    ASSERT_TRUE(result.has_value());

    Appointment apt = result.value();
    EXPECT_EQ(apt.getNotes(), "Note with spaces");
}

// ==================== Original Edge Case Tests ====================

TEST(AppointmentTest, DeserializeEmptyLine)
{
    std::string line = "";

    auto result = Appointment::deserialize(line);
    EXPECT_FALSE(result.has_value());
}

TEST(AppointmentTest, DeserializeCommentLine)
{
    std::string line = "#APT045|patient12|D012|2025-02-25|13:00|Disease|150000|0|scheduled|";

    auto result = Appointment::deserialize(line);
    EXPECT_FALSE(result.has_value());
}

TEST(AppointmentTest, DeserializeInvalidFieldCount_TooFew)
{
    std::string line = "APT046|patient13|D013|2025-02-28";

    auto result = Appointment::deserialize(line);
    EXPECT_FALSE(result.has_value());
}

TEST(AppointmentTest, DeserializeInvalidFieldCount_TooMany)
{
    std::string line = "APT047|patient14|D014|2025-03-01|10:00|Disease|100000|0|scheduled||extra|field";

    auto result = Appointment::deserialize(line);
    EXPECT_FALSE(result.has_value());
}

TEST(AppointmentTest, DeserializeInvalidDate)
{
    std::string line = "APT048|patient15|D015|invalid-date|10:00|Disease|100000|0|scheduled|";

    auto result = Appointment::deserialize(line);
    EXPECT_FALSE(result.has_value());
}

TEST(AppointmentTest, DeserializeInvalidTime)
{
    std::string line = "APT049|patient16|D016|2025-03-05|25:99|Disease|100000|0|scheduled|";

    auto result = Appointment::deserialize(line);
    EXPECT_FALSE(result.has_value());
}

TEST(AppointmentTest, DeserializeInvalidPrice_NotNumeric)
{
    std::string line = "APT050|patient17|D017|2025-03-10|10:00|Disease|not_a_price|0|scheduled|";

    auto result = Appointment::deserialize(line);
    EXPECT_FALSE(result.has_value());
}

TEST(AppointmentTest, DeserializeNegativePrice)
{
    std::string line = "APT051|patient18|D018|2025-03-15|10:00|Disease|-50000|0|scheduled|";

    auto result = Appointment::deserialize(line);
    EXPECT_FALSE(result.has_value());
}

TEST(AppointmentTest, DeserializeInvalidStatus)
{
    std::string line = "APT052|patient19|D019|2025-03-20|10:00|Disease|100000|0|invalid_status|";

    auto result = Appointment::deserialize(line);

    if (result.has_value())
    {
        Appointment apt = result.value();
        EXPECT_EQ(apt.getStatus(), AppointmentStatus::UNKNOWN);
    }
}

TEST(AppointmentTest, DeserializeZeroPrice)
{
    std::string line = "APT053|patient20|D020|2025-03-25|10:00|Disease|0|0|scheduled|";

    auto result = Appointment::deserialize(line);

    ASSERT_TRUE(result.has_value());

    Appointment apt = result.value();
    EXPECT_DOUBLE_EQ(apt.getPrice(), 0.0);
}

TEST(AppointmentTest, DeserializeLargePrice)
{
    std::string line = "APT054|patient21|D021|2025-03-30|10:00|Disease|9999999.99|1|completed|";

    auto result = Appointment::deserialize(line);

    ASSERT_TRUE(result.has_value());

    Appointment apt = result.value();
    EXPECT_DOUBLE_EQ(apt.getPrice(), 9999999.99);
}

// ==================== Edge Cases with Special Characters ====================

TEST(AppointmentTest, DeserializeWithSpecialCharactersInDisease)
{
    std::string line = "APT055|patient22|D022|2025-04-01|10:00|Flu & Fever (Serious)|100000|0|scheduled|";

    auto result = Appointment::deserialize(line);

    ASSERT_TRUE(result.has_value());

    Appointment apt = result.value();
    EXPECT_EQ(apt.getDisease(), "Flu & Fever (Serious)");
}

TEST(AppointmentTest, DeserializeWithSpecialCharactersInNotes)
{
    std::string line = "APT056|patient23|D023|2025-04-05|10:00|Disease|100000|0|scheduled|Note: Patient asked for Friday @ 2:30 PM";

    auto result = Appointment::deserialize(line);

    ASSERT_TRUE(result.has_value());

    Appointment apt = result.value();
    EXPECT_EQ(apt.getNotes(), "Note: Patient asked for Friday @ 2:30 PM");
}

// ==================== Round-trip Serialization Tests ====================

TEST(AppointmentTest, RoundTripSerialization)
{
    Appointment original(
        "APT057", "patient24", "D024", "2025-04-10", "15:30",
        "Routine checkup", 250000.0, true, AppointmentStatus::SCHEDULED,
        "Follow-up appointment");

    std::string serialized = original.serialize();
    auto deserialized = Appointment::deserialize(serialized);

    ASSERT_TRUE(deserialized.has_value());

    Appointment restored = deserialized.value();
    EXPECT_EQ(restored.getAppointmentID(), original.getAppointmentID());
    EXPECT_EQ(restored.getPatientUsername(), original.getPatientUsername());
    EXPECT_EQ(restored.getDoctorID(), original.getDoctorID());
    EXPECT_EQ(restored.getDate(), original.getDate());
    EXPECT_EQ(restored.getTime(), original.getTime());
    EXPECT_EQ(restored.getDisease(), original.getDisease());
    EXPECT_DOUBLE_EQ(restored.getPrice(), original.getPrice());
    EXPECT_EQ(restored.isPaid(), original.isPaid());
    EXPECT_EQ(restored.getStatus(), original.getStatus());
    EXPECT_EQ(restored.getNotes(), original.getNotes());
}

// ==================== Multiple Status Changes ====================

TEST(AppointmentTest, MultipleStatusChanges)
{
    Appointment apt(
        "APT058", "p", "D", "2025-01-01", "10:00",
        "Disease", 100000.0);

    EXPECT_EQ(apt.getStatus(), AppointmentStatus::SCHEDULED);

    apt.markAsNoShow();
    EXPECT_EQ(apt.getStatus(), AppointmentStatus::NO_SHOW);

    apt.markAsCompleted();
    EXPECT_EQ(apt.getStatus(), AppointmentStatus::COMPLETED);

    apt.setStatus(AppointmentStatus::CANCELLED);
    EXPECT_EQ(apt.getStatus(), AppointmentStatus::CANCELLED);
}

// ==================== Boundary Date/Time Tests ====================

TEST(AppointmentTest, BoundaryDate_FirstDayOfYear)
{
    Appointment apt(
        "APT059", "p", "D", "2025-01-01", "10:00",
        "Disease", 100000.0);

    EXPECT_EQ(apt.getDate(), "2025-01-01");
}

TEST(AppointmentTest, BoundaryDate_LastDayOfYear)
{
    Appointment apt(
        "APT060", "p", "D", "2025-12-31", "10:00",
        "Disease", 100000.0);

    EXPECT_EQ(apt.getDate(), "2025-12-31");
}

TEST(AppointmentTest, BoundaryTime_Midnight)
{
    Appointment apt(
        "APT061", "p", "D", "2025-01-01", "00:00",
        "Disease", 100000.0);

    EXPECT_EQ(apt.getTime(), "00:00");
}

TEST(AppointmentTest, BoundaryTime_EndOfDay)
{
    Appointment apt(
        "APT062", "p", "D", "2025-01-01", "23:59",
        "Disease", 100000.0);

    EXPECT_EQ(apt.getTime(), "23:59");
}

// ==================== Complex Scenario Tests ====================

TEST(AppointmentTest, CompleteLifecycle)
{
    Appointment apt(
        "APT063", "patient25", "D025", "2099-12-20", "10:00",
        "Annual checkup", 500000.0);

    EXPECT_TRUE(apt.canEdit());
    EXPECT_TRUE(apt.canCancel());
    EXPECT_TRUE(apt.isUpcoming());
    EXPECT_FALSE(apt.isPaid());

    apt.setPaid(true);
    EXPECT_TRUE(apt.isPaid());

    apt.setNotes("Patient in good health");

    apt.setTime("14:00");
    EXPECT_EQ(apt.getTime(), "14:00");

    apt.markAsCompleted();
    EXPECT_FALSE(apt.canEdit());
    EXPECT_FALSE(apt.canCancel());
    EXPECT_FALSE(apt.isUpcoming());
}

// ==================== Additional Tests for std::format precision ====================

TEST(AppointmentTest, SerializePriceWithDecimals)
{
    Appointment apt(
        "APT064", "p", "D", "2025-01-01", "10:00",
        "Disease", 123456.78);

    std::string serialized = apt.serialize();

    // std::format("{:.2f}", 123456.78) produces "123456.78"
    EXPECT_TRUE(serialized.find("123456.78") != std::string::npos);
}

TEST(AppointmentTest, SerializePriceRounding)
{
    Appointment apt(
        "APT065", "p", "D", "2025-01-01", "10:00",
        "Disease", 99999.999);

    std::string serialized = apt.serialize();

    // std::format("{:.2f}", 99999.999) rounds to "100000.00"
    EXPECT_TRUE(serialized.find("100000.00") != std::string::npos);
}

TEST(AppointmentTest, DeserializePriceWithTwoDecimals)
{
    std::string line = "APT066|p|D|2025-01-01|10:00|Disease|250000.50|0|scheduled|";

    auto result = Appointment::deserialize(line);
    ASSERT_TRUE(result.has_value());

    Appointment apt = result.value();
    EXPECT_DOUBLE_EQ(apt.getPrice(), 250000.50);
}

TEST(AppointmentTest, RoundTripSerializationWithDecimals)
{
    Appointment original(
        "APT067", "p", "D", "2025-01-01", "10:00",
        "Disease", 12345.67);

    std::string serialized = original.serialize();
    auto deserialized = Appointment::deserialize(serialized);

    ASSERT_TRUE(deserialized.has_value());

    Appointment restored = deserialized.value();
    EXPECT_DOUBLE_EQ(restored.getPrice(), 12345.67);
}