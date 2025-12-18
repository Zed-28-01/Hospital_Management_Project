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

class DoctorServiceTest : public ::testing::Test {
protected:
    BLL::DoctorService* service;
    DAL::DoctorRepository* docRepo;
    DAL::AppointmentRepository* appRepo;

    const std::string TEST_DOC_FILE = "test_data/doctors_test.txt";
    const std::string TEST_APP_FILE = "test_data/appointments_test.txt";

    void SetUp() override {
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

    void TearDown() override {
        docRepo->clear();
        appRepo->clear();

        BLL::DoctorService::resetInstance();
        DAL::DoctorRepository::resetInstance();
        DAL::AppointmentRepository::resetInstance();
    }
};

TEST_F(DoctorServiceTest, CreateAndGetDoctorSuccess) {
    Model::Doctor doc(
        "TEST_D01",
        "user_test",
        "Nguyen Van A",
        "0912345678",
        Gender::MALE,
        "1980-01-01",
        "Cardiology",
        "Mon-Fri",
        500000
    );

    bool created = service->createDoctor(doc);

    EXPECT_TRUE(created);

    auto retrieved = service->getDoctorByID("TEST_D01");
    ASSERT_TRUE(retrieved.has_value());
    EXPECT_EQ(retrieved->getName(), "Nguyen Van A");
    EXPECT_EQ(retrieved->getSpecialization(), "Cardiology");
}

TEST_F(DoctorServiceTest, CreateDoctorDuplicateFail) {
    Model::Doctor doc("TEST_D01", "u1", "A", "123", Gender::MALE, "1990", "Spec", "All", 100);

    EXPECT_TRUE(service->createDoctor(doc));
    EXPECT_FALSE(service->createDoctor(doc));
}

TEST_F(DoctorServiceTest, SearchDoctorsLogic) {
    Model::Doctor doc1("TEST_D02", "u2", "Tran Van B", "099", Gender::MALE, "1990", "Neurology", "Mon", 100);
    Model::Doctor doc2("TEST_D03", "u3", "Le Thi C", "088", Gender::FEMALE, "1992", "Dermatology", "Tue", 200);

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

TEST_F(DoctorServiceTest, GetUpcomingAppointmentsFilter) {
    std::string docID = "TEST_DOC_UPCOMING";

    Model::Doctor doc(docID, "u_test", "Dr. Time", "000", Gender::MALE, "1980", "Time", "All", 100);
    service->createDoctor(doc);

    Model::Appointment pastApp(
        "A_PAST", "Pat1", docID, "2000-01-01", "10:00", "Flu", 100.0,
        true, AppointmentStatus::COMPLETED, "Done"
    );

    Model::Appointment futureApp(
        "A_FUTURE", "Pat2", docID, "2099-01-01", "09:00", "Checkup", 200.0,
        false, AppointmentStatus::SCHEDULED, "Pending"
    );

    Model::Appointment cancelledApp(
        "A_CANCEL", "Pat3", docID, "2099-01-01", "11:00", "Sick", 200.0,
        false, AppointmentStatus::CANCELLED, "Cancelled"
    );

    appRepo->add(pastApp);
    appRepo->add(futureApp);
    appRepo->add(cancelledApp);

    auto upcoming = service->getUpcomingAppointments(docID);

    ASSERT_EQ(upcoming.size(), 1);
    EXPECT_EQ(upcoming[0].getAppointmentID(), "A_FUTURE");
}

TEST_F(DoctorServiceTest, GetAvailableSlotsLogic) {
    std::string docID = "TEST_DOC_SLOTS";
    std::string date = "2099-12-31";

    Model::Appointment bookedApp(
        "A1", "Pat1", docID, date, "08:00", "Flu", 100.0,
        false, AppointmentStatus::SCHEDULED, ""
    );
    appRepo->add(bookedApp);

    std::vector<std::string> slots = service->getAvailableSlots(docID, date);

    bool has08 = std::find(slots.begin(), slots.end(), "08:00") != slots.end();
    bool has09 = std::find(slots.begin(), slots.end(), "09:00") != slots.end();

    EXPECT_FALSE(has08);
    EXPECT_TRUE(has09);
}

TEST_F(DoctorServiceTest, StatisticsCalculation) {
    std::string docID = "TEST_STATS";

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