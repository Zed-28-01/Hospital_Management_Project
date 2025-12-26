#include <gtest/gtest.h>
#include "bll/DepartmentService.h"
#include "dal/DepartmentRepository.h"
#include "dal/DoctorRepository.h"
#include "dal/AppointmentRepository.h"
#include "advance/Department.h"
#include "model/Doctor.h"
#include "model/Appointment.h"
#include "common/Utils.h"
#include <filesystem>
#include <fstream>

using namespace HMS;
using namespace HMS::BLL;
using namespace HMS::DAL;
using namespace HMS::Model;

namespace {
    const std::string TEST_DIR = "test/fixtures/service_dept/";
    const std::string DEPT_FILE = TEST_DIR + "departments.txt";
    const std::string DOC_FILE = TEST_DIR + "doctors.txt";
    const std::string APP_FILE = TEST_DIR + "appointments.txt";

    Doctor makeDoc(const std::string& id, const std::string& name) {
        return Doctor(id, "user_" + id, name, "0909090909", Gender::MALE,
                      "1990-01-01", "General", "Mon-Fri", 100.0);
    }

    Appointment makeAppt(const std::string& id, const std::string& docId, double price, AppointmentStatus status) {
        return Appointment(id, "patient_user", docId, "2023-01-01", "10:00",
                           "Flu", price, true, status, "Note");
    }
}

class DepartmentServiceTest : public ::testing::Test {
protected:
    DepartmentService* service;
    DepartmentRepository* deptRepo;
    DoctorRepository* docRepo;
    AppointmentRepository* appRepo;

    void SetUp() override {
        std::filesystem::create_directories(TEST_DIR);

        DepartmentRepository::resetInstance();
        DoctorRepository::resetInstance();
        AppointmentRepository::resetInstance();
        DepartmentService::resetInstance();

        deptRepo = DepartmentRepository::getInstance();
        docRepo = DoctorRepository::getInstance();
        appRepo = AppointmentRepository::getInstance();

        deptRepo->setFilePath(DEPT_FILE);
        docRepo->setFilePath(DOC_FILE);
        appRepo->setFilePath(APP_FILE);

        std::ofstream(DEPT_FILE, std::ios::trunc).close();
        std::ofstream(DOC_FILE, std::ios::trunc).close();
        std::ofstream(APP_FILE, std::ios::trunc).close();

        deptRepo->clear();
        docRepo->clear();
        appRepo->clear();

        service = DepartmentService::getInstance();
    }

    void TearDown() override {
        deptRepo->clear();
        docRepo->clear();
        appRepo->clear();

        DepartmentService::resetInstance();
        DepartmentRepository::resetInstance();
        DoctorRepository::resetInstance();
        AppointmentRepository::resetInstance();

        if (std::filesystem::exists(TEST_DIR)) {
            std::filesystem::remove_all(TEST_DIR);
        }
    }
};

TEST_F(DepartmentServiceTest, CreateDepartment_Valid_Success) {
    auto result = service->createDepartment("Cardiology", "Heart", "Building A", "0901234567");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getName(), "Cardiology");
    EXPECT_EQ(service->getDepartmentCount(), 1);
}

TEST_F(DepartmentServiceTest, CreateDepartment_DuplicateName_Fail) {
    service->createDepartment("Cardiology", "Desc", "Loc", "0901234567");
    auto result = service->createDepartment("Cardiology", "Desc2", "Loc2", "0987654321");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(service->getDepartmentCount(), 1);
}

TEST_F(DepartmentServiceTest, CreateDepartment_InvalidPhone_Fail) {
    auto result = service->createDepartment("Neurology", "Brain", "Loc", "INVALID");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(service->getDepartmentCount(), 0);
}

TEST_F(DepartmentServiceTest, UpdateDepartment_Success) {
    auto depOpt = service->createDepartment("OldName", "Desc", "Loc", "0901234567");
    ASSERT_TRUE(depOpt.has_value());

    Model::Department updateDep = depOpt.value();
    updateDep.setName("NewName");

    EXPECT_TRUE(service->updateDepartment(updateDep));

    auto check = service->getDepartmentByID(updateDep.getDepartmentID());
    ASSERT_TRUE(check.has_value());
    EXPECT_EQ(check->getName(), "NewName");
}

TEST_F(DepartmentServiceTest, UpdateDepartment_DuplicateName_Fail) {
    service->createDepartment("DeptA", "Desc", "Loc", "0901111111");
    auto depB = service->createDepartment("DeptB", "Desc", "Loc", "0902222222");

    Model::Department updateB = depB.value();
    updateB.setName("DeptA");

    EXPECT_FALSE(service->updateDepartment(updateB));
}

TEST_F(DepartmentServiceTest, AssignDoctor_NewAssignment_Success) {
    auto dep = service->createDepartment("DeptA", "Desc", "Loc", "0901111111");
    docRepo->add(makeDoc("DOC1", "Doctor One"));

    EXPECT_TRUE(service->assignDoctor(dep->getDepartmentID(), "DOC1"));
    EXPECT_TRUE(service->isDoctorInDepartment(dep->getDepartmentID(), "DOC1"));
}

TEST_F(DepartmentServiceTest, AssignDoctor_MoveDepartment_Success) {
    auto depA = service->createDepartment("DeptA", "Desc", "Loc", "0901111111");
    auto depB = service->createDepartment("DeptB", "Desc", "Loc", "0902222222");
    docRepo->add(makeDoc("DOC1", "Doctor One"));

    service->assignDoctor(depA->getDepartmentID(), "DOC1");
    EXPECT_TRUE(service->isDoctorInDepartment(depA->getDepartmentID(), "DOC1"));

    EXPECT_TRUE(service->assignDoctor(depB->getDepartmentID(), "DOC1"));

    EXPECT_FALSE(service->isDoctorInDepartment(depA->getDepartmentID(), "DOC1"));
    EXPECT_TRUE(service->isDoctorInDepartment(depB->getDepartmentID(), "DOC1"));
}

TEST_F(DepartmentServiceTest, UnassignDoctor_Success) {
    auto dep = service->createDepartment("DeptA", "Desc", "Loc", "0901111111");
    docRepo->add(makeDoc("DOC1", "Doctor One"));
    service->assignDoctor(dep->getDepartmentID(), "DOC1");

    EXPECT_TRUE(service->unassignDoctor(dep->getDepartmentID(), "DOC1"));
    EXPECT_FALSE(service->isDoctorInDepartment(dep->getDepartmentID(), "DOC1"));
}

TEST_F(DepartmentServiceTest, SetHeadDoctor_AutoAssign_Success) {
    auto dep = service->createDepartment("DeptA", "Desc", "Loc", "0901111111");
    docRepo->add(makeDoc("DOC1", "Head Candidate"));

    EXPECT_TRUE(service->setDepartmentHead(dep->getDepartmentID(), "DOC1"));

    auto checkDep = service->getDepartmentByID(dep->getDepartmentID());
    ASSERT_TRUE(checkDep.has_value());
    EXPECT_EQ(checkDep->getHeadDoctorID(), "DOC1");
    EXPECT_TRUE(service->isDoctorInDepartment(dep->getDepartmentID(), "DOC1"));
}

TEST_F(DepartmentServiceTest, GetDoctorsInDepartment) {
    auto dep = service->createDepartment("DeptA", "Desc", "Loc", "0901111111");
    docRepo->add(makeDoc("D1", "A"));
    docRepo->add(makeDoc("D2", "B"));

    service->assignDoctor(dep->getDepartmentID(), "D1");
    service->assignDoctor(dep->getDepartmentID(), "D2");

    auto list = service->getDoctorsInDepartment(dep->getDepartmentID());
    EXPECT_EQ(list.size(), 2);
}

TEST_F(DepartmentServiceTest, GetUnassignedDoctors) {
    docRepo->add(makeDoc("D1", "Assigned"));
    docRepo->add(makeDoc("D2", "Free"));

    auto dep = service->createDepartment("DeptA", "Desc", "Loc", "0901111111");
    service->assignDoctor(dep->getDepartmentID(), "D1");

    auto freeDocs = service->getUnassignedDoctors();
    ASSERT_EQ(freeDocs.size(), 1);
    EXPECT_EQ(freeDocs[0].getID(), "D2");
}

TEST_F(DepartmentServiceTest, GetDepartmentStats_RevenueCalculation) {
    auto dep = service->createDepartment("RevenueDept", "Desc", "Loc", "0901111111");
    std::string depId = dep->getDepartmentID();

    docRepo->add(makeDoc("D1", "Earner"));
    service->assignDoctor(depId, "D1");

    appRepo->add(makeAppt("A1", "D1", 100.0, AppointmentStatus::COMPLETED));
    appRepo->add(makeAppt("A2", "D1", 50.0, AppointmentStatus::COMPLETED));
    appRepo->add(makeAppt("A3", "D1", 200.0, AppointmentStatus::CANCELLED));
    appRepo->add(makeAppt("A4", "D99", 500.0, AppointmentStatus::COMPLETED));

    DepartmentStats stats = service->getDepartmentStats(depId);

    EXPECT_EQ(stats.doctorCount, 1);
    EXPECT_EQ(stats.appointmentCount, 2);
    EXPECT_DOUBLE_EQ(stats.totalRevenue, 150.0);
}

TEST_F(DepartmentServiceTest, GetAllDepartmentStats) {
    auto d1 = service->createDepartment("D1", "Desc", "Loc", "0901111111");
    auto d2 = service->createDepartment("D2", "Desc", "Loc", "0902222222");

    auto statsMap = service->getAllDepartmentStats();
    EXPECT_EQ(statsMap.size(), 2);
    EXPECT_TRUE(statsMap.count(d1->getDepartmentID()));
    EXPECT_TRUE(statsMap.count(d2->getDepartmentID()));
}