#include <gtest/gtest.h>
#include <algorithm>
#include <string>
#include <vector>
#include <filesystem>

#include "bll/DoctorService.h"
#include "dal/DoctorRepository.h"
#include "dal/AppointmentRepository.h"
#include "model/Doctor.h"
#include "model/Appointment.h"
#include "common/Utils.h"
#include "common/Types.h"

using namespace HMS;

class DoctorServiceTest : public ::testing::Test
{
protected:
    BLL::DoctorService *service;
    DAL::DoctorRepository *docRepo;
    DAL::AppointmentRepository *appRepo;

    const std::string TEST_DOC_FILE = "test_data/doctors_test.txt";
    const std::string TEST_APP_FILE = "test_data/appointments_test.txt";

    void SetUp() override
    {
        std::filesystem::create_directories("test_data");

        BLL::DoctorService::resetInstance();
        DAL::DoctorRepository::resetInstance();
        DAL::AppointmentRepository::resetInstance();

        service = BLL::DoctorService::getInstance();
        docRepo = DAL::DoctorRepository::getInstance();
        appRepo = DAL::AppointmentRepository::getInstance();

        docRepo->setFilePath(TEST_DOC_FILE);
        appRepo->setFilePath(TEST_APP_FILE);

        docRepo->clear();
        appRepo->clear();
    }

    void TearDown() override
    {
        docRepo->clear();
        appRepo->clear();

        BLL::DoctorService::resetInstance();
        DAL::DoctorRepository::resetInstance();
        DAL::AppointmentRepository::resetInstance();
    }
};

TEST_F(DoctorServiceTest, CreateAndGetDoctorSuccess)
{
    Model::Doctor doc(
        "TEST_D01",
        "user_test",
        "Nguyen Van A",
        "0912345678",
        Gender::MALE,
        "1980-01-01",
        "Cardiology",
        "Mon-Fri",
        500000);

    bool created = service->createDoctor(doc);

    EXPECT_TRUE(created);

    auto retrieved = service->getDoctorByID("TEST_D01");
    ASSERT_TRUE(retrieved.has_value());
    EXPECT_EQ(retrieved->getName(), "Nguyen Van A");
    EXPECT_EQ(retrieved->getSpecialization(), "Cardiology");
}

TEST_F(DoctorServiceTest, CreateDoctorDuplicateFail)
{
    Model::Doctor doc(
        "TEST_D01",
        "u1",
        "Doctor A",
        "0123456789",
        Gender::MALE,
        "1990-05-15",
        "General",
        "All",
        100000);

    EXPECT_TRUE(service->createDoctor(doc));
    EXPECT_FALSE(service->createDoctor(doc));
}

TEST_F(DoctorServiceTest, CreateDoctorInvalidPhoneFail)
{
    Model::Doctor doc(
        "TEST_D_INVALID",
        "u_invalid",
        "Invalid Doc",
        "123",
        Gender::MALE,
        "1990-05-15",
        "General",
        "All",
        100000);

    EXPECT_FALSE(service->createDoctor(doc));
}

TEST_F(DoctorServiceTest, CreateDoctorInvalidDateFail)
{
    Model::Doctor doc(
        "TEST_D_INVALID2",
        "u_invalid2",
        "Invalid Doc",
        "0123456789",
        Gender::MALE,
        "1990",
        "General",
        "All",
        100000);

    EXPECT_FALSE(service->createDoctor(doc));
}

TEST_F(DoctorServiceTest, CreateDoctorFutureDOBFail)
{
    Model::Doctor doc(
        "TEST_D_FUTURE",
        "u_future",
        "Future Doc",
        "0123456789",
        Gender::MALE,
        "2099-01-01",
        "General",
        "All",
        100000);

    EXPECT_FALSE(service->createDoctor(doc));
}

TEST_F(DoctorServiceTest, SearchDoctorsLogic)
{
    Model::Doctor doc1(
        "TEST_D02",
        "u2",
        "Tran Van B",
        "0991234567",
        Gender::MALE,
        "1990-06-20",
        "Neurology",
        "Mon",
        100000);
    Model::Doctor doc2(
        "TEST_D03",
        "u3",
        "Le Thi C",
        "0881234567",
        Gender::FEMALE,
        "1992-03-10",
        "Dermatology",
        "Tue",
        200000);

    service->createDoctor(doc1);
    service->createDoctor(doc2);

    auto resultName = service->searchDoctors("tran");
    EXPECT_EQ(resultName.size(), 1);
    EXPECT_EQ(resultName[0].getID(), "TEST_D02");

    auto resultSpec = service->searchDoctors("derma");
    EXPECT_EQ(resultSpec.size(), 1);
    EXPECT_EQ(resultSpec[0].getID(), "TEST_D03");

    auto resultNone = service->searchDoctors("xyz");
    EXPECT_TRUE(resultNone.empty());
}

TEST_F(DoctorServiceTest, GetUpcomingAppointmentsFilter)
{
    std::string docID = "TEST_DOC_UPCOMING";

    Model::Doctor doc(
        docID,
        "u_test",
        "Dr. Time",
        "0001234567",
        Gender::MALE,
        "1980-05-15",
        "Time Management",
        "All",
        100000);
    service->createDoctor(doc);

    Model::Appointment pastApp(
        "A_PAST", "Pat1", docID, "2000-01-01", "10:00", "Flu", 100.0,
        true, AppointmentStatus::COMPLETED, "Done");

    Model::Appointment futureApp(
        "A_FUTURE", "Pat2", docID, "2099-01-01", "09:00", "Checkup", 200.0,
        false, AppointmentStatus::SCHEDULED, "Pending");

    Model::Appointment cancelledApp(
        "A_CANCEL", "Pat3", docID, "2099-01-01", "11:00", "Sick", 200.0,
        false, AppointmentStatus::CANCELLED, "Cancelled");

    appRepo->add(pastApp);
    appRepo->add(futureApp);
    appRepo->add(cancelledApp);

    auto upcoming = service->getUpcomingAppointments(docID);

    ASSERT_EQ(upcoming.size(), 1);
    EXPECT_EQ(upcoming[0].getAppointmentID(), "A_FUTURE");
}

TEST_F(DoctorServiceTest, GetUpcomingAppointmentsSortedByDateTime)
{
    std::string docID = "TEST_DOC_SORT";

    Model::Doctor doc(
        docID,
        "u_sort",
        "Dr. Sort",
        "0112345678",
        Gender::MALE,
        "1975-08-20",
        "Sorting",
        "All",
        100000);
    service->createDoctor(doc);

    Model::Appointment app1(
        "A1", "Pat1", docID, "2099-01-02", "08:00", "A", 100.0,
        false, AppointmentStatus::SCHEDULED, "");
    Model::Appointment app2(
        "A2", "Pat2", docID, "2099-01-01", "10:00", "B", 100.0,
        false, AppointmentStatus::SCHEDULED, "");
    Model::Appointment app3(
        "A3", "Pat3", docID, "2099-01-01", "08:00", "C", 100.0,
        false, AppointmentStatus::SCHEDULED, "");

    appRepo->add(app1);
    appRepo->add(app2);
    appRepo->add(app3);

    auto upcoming = service->getUpcomingAppointments(docID);

    ASSERT_EQ(upcoming.size(), 3);
    EXPECT_EQ(upcoming[0].getAppointmentID(), "A3");
    EXPECT_EQ(upcoming[1].getAppointmentID(), "A2");
    EXPECT_EQ(upcoming[2].getAppointmentID(), "A1");
}

TEST_F(DoctorServiceTest, GetAvailableSlotsLogic)
{
    std::string docID = "TEST_DOC_SLOTS";
    std::string date = "2099-12-31";

    Model::Doctor doc(
        docID,
        "u_slots",
        "Dr. Slots",
        "0223456789",
        Gender::MALE,
        "1985-04-10",
        "Scheduling",
        "All",
        100000);
    service->createDoctor(doc);

    Model::Appointment bookedApp(
        "A1", "Pat1", docID, date, "08:00", "Flu", 100.0,
        false, AppointmentStatus::SCHEDULED, "");
    appRepo->add(bookedApp);

    std::vector<std::string> slots = service->getAvailableSlots(docID, date);

    bool has08 = std::find(slots.begin(), slots.end(), "08:00") != slots.end();
    bool has09 = std::find(slots.begin(), slots.end(), "09:00") != slots.end();

    EXPECT_FALSE(has08);
    EXPECT_TRUE(has09);
}

TEST_F(DoctorServiceTest, GetAvailableSlotsInvalidDate)
{
    std::string docID = "TEST_DOC_INVALID_DATE";

    auto slots = service->getAvailableSlots(docID, "invalid-date");
    EXPECT_TRUE(slots.empty());
}

TEST_F(DoctorServiceTest, GetAvailableSlotsPastDate)
{
    std::string docID = "TEST_DOC_PAST";

    auto slots = service->getAvailableSlots(docID, "2000-01-01");
    EXPECT_TRUE(slots.empty());
}

TEST_F(DoctorServiceTest, IsSlotAvailableValidation)
{
    std::string docID = "TEST_DOC_SLOT_VAL";
    std::string date = "2099-12-31";

    EXPECT_FALSE(service->isSlotAvailable(docID, "invalid", date));
    EXPECT_FALSE(service->isSlotAvailable(docID, "09:00", "invalid"));
}

TEST_F(DoctorServiceTest, StatisticsCalculation)
{
    std::string docID = "TEST_STATS";

    Model::Doctor doc(
        docID,
        "u_stats",
        "Dr. Stats",
        "0334567890",
        Gender::MALE,
        "1970-12-25",
        "Statistics",
        "All",
        100000);
    service->createDoctor(doc);

    Model::Appointment app1("S1", "Pat_A", docID, "2023-01-01", "08:00", "A", 100.0, true, AppointmentStatus::COMPLETED, "");
    Model::Appointment app2("S2", "Pat_A", docID, "2023-02-01", "09:00", "A", 200.0, true, AppointmentStatus::COMPLETED, "");
    Model::Appointment app3("S3", "Pat_B", docID, "2023-03-01", "10:00", "B", 50.0, true, AppointmentStatus::COMPLETED, "");
    Model::Appointment app4("S4", "Pat_C", docID, "2023-04-01", "11:00", "C", 500.0, false, AppointmentStatus::CANCELLED, "");

    appRepo->add(app1);
    appRepo->add(app2);
    appRepo->add(app3);
    appRepo->add(app4);

    double revenue = service->getDoctorRevenue(docID);
    EXPECT_DOUBLE_EQ(revenue, 350.0);

    size_t count = service->getDoctorPatientCount(docID);
    EXPECT_EQ(count, 2);
}

TEST_F(DoctorServiceTest, GetAppointmentsInRangeValidation)
{
    std::string docID = "TEST_RANGE";

    auto result = service->getAppointmentsInRange(docID, "invalid", "2023-12-31");
    EXPECT_TRUE(result.empty());

    result = service->getAppointmentsInRange(docID, "2023-01-01", "invalid");
    EXPECT_TRUE(result.empty());
}

TEST_F(DoctorServiceTest, GetAppointmentsInRangeLogic)
{
    std::string docID = "TEST_RANGE2";

    Model::Doctor doc(
        docID,
        "u_range",
        "Dr. Range",
        "0445678901",
        Gender::FEMALE,
        "1982-07-14",
        "Range",
        "All",
        100000);
    service->createDoctor(doc);

    Model::Appointment app1("R1", "Pat1", docID, "2023-01-15", "08:00", "A", 100.0, true, AppointmentStatus::COMPLETED, "");
    Model::Appointment app2("R2", "Pat2", docID, "2023-02-15", "09:00", "B", 100.0, true, AppointmentStatus::COMPLETED, "");
    Model::Appointment app3("R3", "Pat3", docID, "2023-03-15", "10:00", "C", 100.0, true, AppointmentStatus::COMPLETED, "");

    appRepo->add(app1);
    appRepo->add(app2);
    appRepo->add(app3);

    auto result = service->getAppointmentsInRange(docID, "2023-01-01", "2023-02-28");
    EXPECT_EQ(result.size(), 2);
}

TEST_F(DoctorServiceTest, ValidateDoctorEmptyFields)
{
    Model::Doctor emptyName("D1", "u1", "", "0123456789", Gender::MALE, "1990-01-01", "Spec", "All", 100);
    EXPECT_FALSE(service->validateDoctor(emptyName));

    Model::Doctor emptyPhone("D2", "u2", "Name", "", Gender::MALE, "1990-01-01", "Spec", "All", 100);
    EXPECT_FALSE(service->validateDoctor(emptyPhone));

    Model::Doctor emptySpec("D3", "u3", "Name", "0123456789", Gender::MALE, "1990-01-01", "", "All", 100);
    EXPECT_FALSE(service->validateDoctor(emptySpec));
}

TEST_F(DoctorServiceTest, ValidateDoctorNegativeFee)
{
    Model::Doctor doc("D_NEG", "u_neg", "Name", "0123456789", Gender::MALE, "1990-01-01", "Spec", "All", -100);
    EXPECT_FALSE(service->validateDoctor(doc));
}

TEST_F(DoctorServiceTest, DoctorExistsCheck)
{
    Model::Doctor doc(
        "TEST_EXISTS",
        "u_exists",
        "Dr. Exists",
        "0556789012",
        Gender::MALE,
        "1988-11-30",
        "Existence",
        "All",
        100000);

    EXPECT_FALSE(service->doctorExists("TEST_EXISTS"));
    service->createDoctor(doc);
    EXPECT_TRUE(service->doctorExists("TEST_EXISTS"));
}

TEST_F(DoctorServiceTest, UpdateDoctorSuccess)
{
    Model::Doctor doc(
        "TEST_UPDATE",
        "u_update",
        "Dr. Original",
        "0667890123",
        Gender::MALE,
        "1979-02-28",
        "Original",
        "Mon-Fri",
        100000);
    service->createDoctor(doc);

    Model::Doctor updated(
        "TEST_UPDATE",
        "u_update",
        "Dr. Updated",
        "0667890123",
        Gender::MALE,
        "1979-02-28",
        "Updated Spec",
        "Mon-Sat",
        150000);

    EXPECT_TRUE(service->updateDoctor(updated));

    auto retrieved = service->getDoctorByID("TEST_UPDATE");
    ASSERT_TRUE(retrieved.has_value());
    EXPECT_EQ(retrieved->getName(), "Dr. Updated");
    EXPECT_EQ(retrieved->getSpecialization(), "Updated Spec");
}

TEST_F(DoctorServiceTest, UpdateNonExistentDoctorFail)
{
    Model::Doctor doc(
        "NON_EXISTENT",
        "u_non",
        "Dr. Non",
        "0778901234",
        Gender::MALE,
        "1995-09-05",
        "None",
        "All",
        100000);

    EXPECT_FALSE(service->updateDoctor(doc));
}

TEST_F(DoctorServiceTest, DeleteDoctorSuccess)
{
    Model::Doctor doc(
        "TEST_DELETE",
        "u_delete",
        "Dr. Delete",
        "0889012345",
        Gender::FEMALE,
        "1983-06-18",
        "Deletion",
        "All",
        100000);
    service->createDoctor(doc);

    EXPECT_TRUE(service->doctorExists("TEST_DELETE"));
    EXPECT_TRUE(service->deleteDoctor("TEST_DELETE"));
    EXPECT_FALSE(service->doctorExists("TEST_DELETE"));
}

TEST_F(DoctorServiceTest, DeleteNonExistentDoctorFail)
{
    EXPECT_FALSE(service->deleteDoctor("NON_EXISTENT_ID"));
}

TEST_F(DoctorServiceTest, GetDoctorByUsername)
{
    Model::Doctor doc(
        "TEST_USERNAME",
        "unique_username",
        "Dr. Username",
        "0990123456",
        Gender::MALE,
        "1977-04-22",
        "Users",
        "All",
        100000);
    service->createDoctor(doc);

    auto retrieved = service->getDoctorByUsername("unique_username");
    ASSERT_TRUE(retrieved.has_value());
    EXPECT_EQ(retrieved->getID(), "TEST_USERNAME");

    auto notFound = service->getDoctorByUsername("nonexistent");
    EXPECT_FALSE(notFound.has_value());
}

TEST_F(DoctorServiceTest, GetDoctorsBySpecialization)
{
    Model::Doctor doc1(
        "SPEC_D1",
        "u_spec1",
        "Dr. Cardio1",
        "0101234567",
        Gender::MALE,
        "1980-01-01",
        "Cardiology",
        "Mon",
        100000);
    Model::Doctor doc2(
        "SPEC_D2",
        "u_spec2",
        "Dr. Cardio2",
        "0201234567",
        Gender::FEMALE,
        "1985-05-05",
        "Cardiology",
        "Tue",
        120000);
    Model::Doctor doc3(
        "SPEC_D3",
        "u_spec3",
        "Dr. Neuro",
        "0301234567",
        Gender::MALE,
        "1990-10-10",
        "Neurology",
        "Wed",
        150000);

    service->createDoctor(doc1);
    service->createDoctor(doc2);
    service->createDoctor(doc3);

    auto cardiologists = service->getDoctorsBySpecialization("Cardiology");
    EXPECT_EQ(cardiologists.size(), 2);

    auto neurologists = service->getDoctorsBySpecialization("Neurology");
    EXPECT_EQ(neurologists.size(), 1);

    auto none = service->getDoctorsBySpecialization("Nonexistent");
    EXPECT_TRUE(none.empty());
}

TEST_F(DoctorServiceTest, GetAllSpecializations)
{
    Model::Doctor doc1("ALL_SPEC1", "u_as1", "D1", "0401234567", Gender::MALE, "1980-01-01", "Cardiology", "Mon", 100000);
    Model::Doctor doc2("ALL_SPEC2", "u_as2", "D2", "0501234567", Gender::FEMALE, "1985-05-05", "Neurology", "Tue", 100000);
    Model::Doctor doc3("ALL_SPEC3", "u_as3", "D3", "0601234567", Gender::MALE, "1990-10-10", "Cardiology", "Wed", 100000);

    service->createDoctor(doc1);
    service->createDoctor(doc2);
    service->createDoctor(doc3);

    auto specs = service->getAllSpecializations();
    EXPECT_EQ(specs.size(), 2);

    bool hasCardio = std::find(specs.begin(), specs.end(), "Cardiology") != specs.end();
    bool hasNeuro = std::find(specs.begin(), specs.end(), "Neurology") != specs.end();
    EXPECT_TRUE(hasCardio);
    EXPECT_TRUE(hasNeuro);
}

TEST_F(DoctorServiceTest, GetDoctorCount)
{
    EXPECT_EQ(service->getDoctorCount(), 0);

    Model::Doctor doc1("COUNT1", "u_c1", "D1", "0701234567", Gender::MALE, "1980-01-01", "Spec", "Mon", 100000);
    Model::Doctor doc2("COUNT2", "u_c2", "D2", "0801234567", Gender::FEMALE, "1985-05-05", "Spec", "Tue", 100000);

    service->createDoctor(doc1);
    EXPECT_EQ(service->getDoctorCount(), 1);

    service->createDoctor(doc2);
    EXPECT_EQ(service->getDoctorCount(), 2);

    service->deleteDoctor("COUNT1");
    EXPECT_EQ(service->getDoctorCount(), 1);
}

TEST_F(DoctorServiceTest, GetDoctorAppointmentCount)
{
    std::string docID = "TEST_APP_COUNT";

    Model::Doctor doc(docID, "u_app_count", "Dr. Count", "0901234567", Gender::MALE, "1975-03-15", "Counting", "All", 100000);
    service->createDoctor(doc);

    EXPECT_EQ(service->getDoctorAppointmentCount(docID), 0);

    Model::Appointment app1("AC1", "Pat1", docID, "2023-01-01", "08:00", "A", 100.0, true, AppointmentStatus::COMPLETED, "");
    Model::Appointment app2("AC2", "Pat2", docID, "2023-02-01", "09:00", "B", 100.0, false, AppointmentStatus::SCHEDULED, "");

    appRepo->add(app1);
    appRepo->add(app2);

    EXPECT_EQ(service->getDoctorAppointmentCount(docID), 2);
}

TEST_F(DoctorServiceTest, GetTodayAppointments)
{
    std::string docID = "TEST_TODAY";
    std::string today = Utils::getCurrentDate();

    Model::Doctor doc(docID, "u_today", "Dr. Today", "0912345670", Gender::MALE, "1980-08-08", "Today", "All", 100000);
    service->createDoctor(doc);

    Model::Appointment todayApp("TODAY1", "Pat1", docID, today, "14:00", "Checkup", 100.0, false, AppointmentStatus::SCHEDULED, "");
    Model::Appointment otherApp("OTHER1", "Pat2", docID, "2099-12-31", "10:00", "Future", 100.0, false, AppointmentStatus::SCHEDULED, "");

    appRepo->add(todayApp);
    appRepo->add(otherApp);

    auto todayApps = service->getTodayAppointments(docID);
    EXPECT_EQ(todayApps.size(), 1);
    EXPECT_EQ(todayApps[0].getAppointmentID(), "TODAY1");
}

TEST_F(DoctorServiceTest, CreateDoctorWithParameters)
{
    auto result = service->createDoctor(
        "param_user",
        "Dr. Parameters",
        "0923456789",
        Gender::FEMALE,
        "1988-12-12",
        "Parameters",
        "Mon-Fri",
        200000);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getName(), "Dr. Parameters");
    EXPECT_EQ(result->getUsername(), "param_user");
    EXPECT_FALSE(result->getID().empty());
}

/*
./HospitalTests --gtest_filter="DoctorServiceTest.*"
*/
