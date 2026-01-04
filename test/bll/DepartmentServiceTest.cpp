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

// ==================== Test Fixture ====================
class DepartmentServiceTest : public ::testing::Test
{
protected:
    DepartmentService *service;
    DepartmentRepository *deptRepo;
    DoctorRepository *docRepo;
    AppointmentRepository *appRepo;

    std::string testDir;
    std::string deptFilePath;
    std::string docFilePath;
    std::string appFilePath;

    void SetUp() override
    {
        // Setup test file paths
        testDir = "test_data/dept_service/";
        deptFilePath = testDir + "departments.txt";
        docFilePath = testDir + "doctors.txt";
        appFilePath = testDir + "appointments.txt";

        // Ensure test directory exists
        std::filesystem::create_directories(testDir);

        // Reset all singletons first to ensure clean state
        DepartmentService::resetInstance();
        DepartmentRepository::resetInstance();
        DoctorRepository::resetInstance();
        AppointmentRepository::resetInstance();

        // Get repository instances and set file paths
        deptRepo = DepartmentRepository::getInstance();
        docRepo = DoctorRepository::getInstance();
        appRepo = AppointmentRepository::getInstance();

        deptRepo->setFilePath(deptFilePath);
        docRepo->setFilePath(docFilePath);
        appRepo->setFilePath(appFilePath);

        // Clear any existing data
        deptRepo->clear();
        docRepo->clear();
        appRepo->clear();

        // Get service instance
        service = DepartmentService::getInstance();
    }

    void TearDown() override
    {
        // Clear repositories
        deptRepo->clear();
        docRepo->clear();
        appRepo->clear();

        // Reset singletons
        DepartmentService::resetInstance();
        DepartmentRepository::resetInstance();
        DoctorRepository::resetInstance();
        AppointmentRepository::resetInstance();

        // Clean up test files including backups
        cleanupTestFile(deptFilePath);
        cleanupTestFile(docFilePath);
        cleanupTestFile(appFilePath);

        // Remove test directory
        if (std::filesystem::exists(testDir))
        {
            std::filesystem::remove_all(testDir);
        }
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

    // Helper to create test department
    Department createTestDepartment(
        const std::string &id = "DEP001",
        const std::string &name = "Cardiology",
        const std::string &description = "Heart and cardiovascular care")
    {
        Department dept(id, name, description, "");
        dept.setLocation("Building A, Floor 2");
        dept.setPhone("0123456789");
        return dept;
    }

    // Helper to create test doctor
    Doctor createTestDoctor(
        const std::string &id = "DOC001",
        const std::string &name = "Dr. Test")
    {
        return Doctor(id, "user_" + id, name, "0987654321", Gender::MALE,
                      "1980-01-01", "General Medicine",
                      500000.0);
    }

    // Helper to create test appointment
    Appointment createTestAppointment(
        const std::string &id,
        const std::string &doctorId,
        double price,
        AppointmentStatus status)
    {
        return Appointment(id, "patient_user", doctorId, "2023-01-01", "10:00",
                           "General checkup", price, true, status, "Note");
    }
};

// ==================== Singleton Tests ====================

TEST_F(DepartmentServiceTest, GetInstance_ReturnsSameInstance)
{
    DepartmentService *instance1 = DepartmentService::getInstance();
    DepartmentService *instance2 = DepartmentService::getInstance();

    EXPECT_EQ(instance1, instance2);
    EXPECT_NE(instance1, nullptr);
}

TEST_F(DepartmentServiceTest, ResetInstance_CreatesFreshInstance)
{
    DepartmentService *instance1 = DepartmentService::getInstance();
    EXPECT_NE(instance1, nullptr);

    DepartmentService::resetInstance();
    DepartmentService *instance2 = DepartmentService::getInstance();

    EXPECT_NE(instance2, nullptr);
}

// ==================== CRUD - Create Tests ====================

TEST_F(DepartmentServiceTest, CreateDepartment_ValidDepartment_Success)
{
    Department dept = createTestDepartment("DEP001");

    bool result = service->createDepartment(dept);

    EXPECT_TRUE(result);
    EXPECT_EQ(service->getDepartmentCount(), 1u);
}

TEST_F(DepartmentServiceTest, CreateDepartment_WithParams_Success)
{
    auto result = service->createDepartment(
        "Cardiology", "Heart care", "Building A", "0901234567");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getName(), "Cardiology");
    EXPECT_EQ(result->getDescription(), "Heart care");
    EXPECT_EQ(result->getLocation(), "Building A");
    EXPECT_EQ(service->getDepartmentCount(), 1u);
}

TEST_F(DepartmentServiceTest, CreateDepartment_DuplicateID_Fail)
{
    Department dept1 = createTestDepartment("DEP001", "Cardiology");
    Department dept2 = createTestDepartment("DEP001", "Neurology");

    service->createDepartment(dept1);
    bool result = service->createDepartment(dept2);

    EXPECT_FALSE(result);
    EXPECT_EQ(service->getDepartmentCount(), 1u);
}

TEST_F(DepartmentServiceTest, CreateDepartment_DuplicateName_Fail)
{
    Department dept1 = createTestDepartment("DEP001", "Cardiology");
    Department dept2 = createTestDepartment("DEP002", "Cardiology");

    service->createDepartment(dept1);
    bool result = service->createDepartment(dept2);

    EXPECT_FALSE(result);
    EXPECT_EQ(service->getDepartmentCount(), 1u);
}

TEST_F(DepartmentServiceTest, CreateDepartment_DuplicateNameCaseInsensitive_Fail)
{
    service->createDepartment("Cardiology", "Desc", "Loc", "0901234567");
    auto result = service->createDepartment("CARDIOLOGY", "Desc2", "Loc2", "0902345678");

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(service->getDepartmentCount(), 1u);
}

TEST_F(DepartmentServiceTest, CreateDepartment_DuplicateNameWithWhitespace_Fail)
{
    service->createDepartment("Cardiology", "Desc", "Loc", "0901234567");
    auto result = service->createDepartment("  Cardiology  ", "Desc2", "Loc2", "0902345678");

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(service->getDepartmentCount(), 1u);
}

TEST_F(DepartmentServiceTest, CreateDepartment_EmptyName_Fail)
{
    auto result = service->createDepartment("", "Description", "Location", "0901234567");

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(service->getDepartmentCount(), 0u);
}

TEST_F(DepartmentServiceTest, CreateDepartment_InvalidPhone_Fail)
{
    auto result = service->createDepartment("Neurology", "Brain care", "Loc", "INVALID");

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(service->getDepartmentCount(), 0u);
}

TEST_F(DepartmentServiceTest, CreateDepartment_EmptyPhoneAllowed_Success)
{
    auto result = service->createDepartment("Neurology", "Brain care", "Loc", "");

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(service->getDepartmentCount(), 1u);
}

// ==================== CRUD - Read Tests ====================

TEST_F(DepartmentServiceTest, GetDepartmentByID_Exists_ReturnsDepartment)
{
    Department dept = createTestDepartment("DEP001", "Cardiology");
    service->createDepartment(dept);

    auto result = service->getDepartmentByID("DEP001");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getDepartmentID(), "DEP001");
    EXPECT_EQ(result->getName(), "Cardiology");
}

TEST_F(DepartmentServiceTest, GetDepartmentByID_NotExists_ReturnsNullopt)
{
    auto result = service->getDepartmentByID("NONEXISTENT");

    EXPECT_FALSE(result.has_value());
}

TEST_F(DepartmentServiceTest, GetDepartmentByName_Exists_ReturnsDepartment)
{
    service->createDepartment(createTestDepartment("DEP001", "Cardiology"));

    auto result = service->getDepartmentByName("Cardiology");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getName(), "Cardiology");
}

TEST_F(DepartmentServiceTest, GetDepartmentByName_NotExists_ReturnsNullopt)
{
    auto result = service->getDepartmentByName("NonExistent");

    EXPECT_FALSE(result.has_value());
}

TEST_F(DepartmentServiceTest, GetAllDepartments_Empty_ReturnsEmptyList)
{
    auto departments = service->getAllDepartments();

    EXPECT_TRUE(departments.empty());
}

TEST_F(DepartmentServiceTest, GetAllDepartments_Multiple_ReturnsAll)
{
    service->createDepartment(createTestDepartment("DEP001", "Cardiology"));
    service->createDepartment(createTestDepartment("DEP002", "Neurology"));
    service->createDepartment(createTestDepartment("DEP003", "Oncology"));

    auto departments = service->getAllDepartments();

    EXPECT_EQ(departments.size(), 3u);
}

TEST_F(DepartmentServiceTest, SearchDepartments_MatchingKeyword_ReturnsMatches)
{
    service->createDepartment(createTestDepartment("DEP001", "Cardiology"));
    service->createDepartment(createTestDepartment("DEP002", "Neurology"));

    auto results = service->searchDepartments("Card");

    EXPECT_EQ(results.size(), 1u);
    EXPECT_EQ(results[0].getName(), "Cardiology");
}

TEST_F(DepartmentServiceTest, SearchDepartments_EmptyKeyword_ReturnsAll)
{
    service->createDepartment(createTestDepartment("DEP001", "Cardiology"));
    service->createDepartment(createTestDepartment("DEP002", "Neurology"));

    auto results = service->searchDepartments("");

    EXPECT_EQ(results.size(), 2u);
}

TEST_F(DepartmentServiceTest, SearchDepartments_NoMatch_ReturnsEmpty)
{
    service->createDepartment(createTestDepartment("DEP001", "Cardiology"));

    auto results = service->searchDepartments("XYZ");

    EXPECT_TRUE(results.empty());
}

TEST_F(DepartmentServiceTest, GetDepartmentCount_ReturnsCorrectCount)
{
    EXPECT_EQ(service->getDepartmentCount(), 0u);

    service->createDepartment(createTestDepartment("DEP001", "Cardiology"));
    EXPECT_EQ(service->getDepartmentCount(), 1u);

    service->createDepartment(createTestDepartment("DEP002", "Neurology"));
    EXPECT_EQ(service->getDepartmentCount(), 2u);

    service->deleteDepartment("DEP001");
    EXPECT_EQ(service->getDepartmentCount(), 1u);
}

TEST_F(DepartmentServiceTest, GetAllDepartmentNames_ReturnsNames)
{
    service->createDepartment(createTestDepartment("DEP001", "Cardiology"));
    service->createDepartment(createTestDepartment("DEP002", "Neurology"));

    auto names = service->getAllDepartmentNames();

    EXPECT_EQ(names.size(), 2u);
}

// ==================== CRUD - Update Tests ====================

TEST_F(DepartmentServiceTest, UpdateDepartment_ValidUpdate_Success)
{
    Department dept = createTestDepartment("DEP001", "OldName");
    service->createDepartment(dept);

    dept.setName("NewName");
    dept.setDescription("Updated description");
    bool result = service->updateDepartment(dept);

    EXPECT_TRUE(result);

    auto updated = service->getDepartmentByID("DEP001");
    ASSERT_TRUE(updated.has_value());
    EXPECT_EQ(updated->getName(), "NewName");
    EXPECT_EQ(updated->getDescription(), "Updated description");
}

TEST_F(DepartmentServiceTest, UpdateDepartment_NonExistent_Fail)
{
    Department dept = createTestDepartment("NONEXISTENT");

    bool result = service->updateDepartment(dept);

    EXPECT_FALSE(result);
}

TEST_F(DepartmentServiceTest, UpdateDepartment_DuplicateName_Fail)
{
    service->createDepartment(createTestDepartment("DEP001", "DeptA"));
    auto deptB = service->createDepartment("DeptB", "Desc", "Loc", "0902222222");

    Department updateB = deptB.value();
    updateB.setName("DeptA");

    EXPECT_FALSE(service->updateDepartment(updateB));

    // Original name should be preserved
    auto check = service->getDepartmentByID(deptB->getDepartmentID());
    EXPECT_EQ(check->getName(), "DeptB");
}

TEST_F(DepartmentServiceTest, UpdateDepartment_SameNameSameID_Success)
{
    auto dept = service->createDepartment("Cardiology", "Desc", "Loc", "0901234567");

    Department update = dept.value();
    update.setDescription("Updated description");
    // Name stays the same

    EXPECT_TRUE(service->updateDepartment(update));
}

// ==================== CRUD - Delete Tests ====================

TEST_F(DepartmentServiceTest, DeleteDepartment_Exists_Success)
{
    service->createDepartment(createTestDepartment("DEP001"));

    bool result = service->deleteDepartment("DEP001");

    EXPECT_TRUE(result);
    EXPECT_EQ(service->getDepartmentCount(), 0u);
    EXPECT_FALSE(service->getDepartmentByID("DEP001").has_value());
}

TEST_F(DepartmentServiceTest, DeleteDepartment_NonExistent_Fail)
{
    bool result = service->deleteDepartment("NONEXISTENT");

    EXPECT_FALSE(result);
}

TEST_F(DepartmentServiceTest, DeleteDepartment_EmptyID_Fail)
{
    bool result = service->deleteDepartment("");

    EXPECT_FALSE(result);
}

// ==================== Doctor Assignment Tests ====================

TEST_F(DepartmentServiceTest, AssignDoctor_ValidAssignment_Success)
{
    auto dept = service->createDepartment("Cardiology", "Desc", "Loc", "0901234567");
    docRepo->add(createTestDoctor("DOC001", "Dr. One"));

    bool result = service->assignDoctor(dept->getDepartmentID(), "DOC001");

    EXPECT_TRUE(result);
    EXPECT_TRUE(service->isDoctorInDepartment(dept->getDepartmentID(), "DOC001"));
}

TEST_F(DepartmentServiceTest, AssignDoctor_UpdatesDoctorSpecialization)
{
    // Bug fix: Verify that doctor's specialization is updated when assigned to department
    auto dept = service->createDepartment("Cardiology", "Desc", "Loc", "0901234567");
    Doctor doc = createTestDoctor("DOC001", "Dr. One");
    doc.setSpecialization("General Medicine"); // Original specialization
    docRepo->add(doc);

    bool result = service->assignDoctor(dept->getDepartmentID(), "DOC001");

    EXPECT_TRUE(result);

    // Verify doctor's specialization is now updated to match the department name
    auto updatedDoc = docRepo->getById("DOC001");
    ASSERT_TRUE(updatedDoc.has_value());
    EXPECT_EQ(updatedDoc->getSpecialization(), "Cardiology");
}

TEST_F(DepartmentServiceTest, AssignDoctor_AlreadyAssigned_ReturnsTrue)
{
    // Test idempotent behavior - assigning same doctor again returns true
    auto dept = service->createDepartment("Cardiology", "Desc", "Loc", "0901234567");
    docRepo->add(createTestDoctor("DOC001", "Dr. One"));

    service->assignDoctor(dept->getDepartmentID(), "DOC001");
    bool result = service->assignDoctor(dept->getDepartmentID(), "DOC001");

    EXPECT_TRUE(result); // Idempotent: returns true if already assigned
    EXPECT_TRUE(service->isDoctorInDepartment(dept->getDepartmentID(), "DOC001"));
}

TEST_F(DepartmentServiceTest, AssignDoctor_CrossDepartmentTransfer_Success)
{
    auto deptA = service->createDepartment("DeptA", "Desc", "Loc", "0901111111");
    auto deptB = service->createDepartment("DeptB", "Desc", "Loc", "0902222222");
    docRepo->add(createTestDoctor("DOC001", "Dr. One"));

    // Assign to DeptA first
    service->assignDoctor(deptA->getDepartmentID(), "DOC001");
    EXPECT_TRUE(service->isDoctorInDepartment(deptA->getDepartmentID(), "DOC001"));

    // Verify specialization is updated to DeptA
    auto docAfterFirstAssign = docRepo->getById("DOC001");
    EXPECT_EQ(docAfterFirstAssign->getSpecialization(), "DeptA");

    // Now assign to DeptB - should auto-transfer
    bool result = service->assignDoctor(deptB->getDepartmentID(), "DOC001");

    EXPECT_TRUE(result);
    EXPECT_FALSE(service->isDoctorInDepartment(deptA->getDepartmentID(), "DOC001"));
    EXPECT_TRUE(service->isDoctorInDepartment(deptB->getDepartmentID(), "DOC001"));

    // Verify specialization is updated to DeptB
    auto docAfterTransfer = docRepo->getById("DOC001");
    EXPECT_EQ(docAfterTransfer->getSpecialization(), "DeptB");
}

TEST_F(DepartmentServiceTest, AssignDoctor_NonExistentDepartment_Fail)
{
    docRepo->add(createTestDoctor("DOC001"));

    bool result = service->assignDoctor("NONEXISTENT", "DOC001");

    EXPECT_FALSE(result);
}

TEST_F(DepartmentServiceTest, AssignDoctor_NonExistentDoctor_Fail)
{
    service->createDepartment(createTestDepartment("DEP001"));

    bool result = service->assignDoctor("DEP001", "NONEXISTENT");

    EXPECT_FALSE(result);
}

TEST_F(DepartmentServiceTest, UnassignDoctor_ValidUnassignment_Success)
{
    auto dept = service->createDepartment("Cardiology", "Desc", "Loc", "0901234567");
    docRepo->add(createTestDoctor("DOC001"));
    service->assignDoctor(dept->getDepartmentID(), "DOC001");

    bool result = service->unassignDoctor(dept->getDepartmentID(), "DOC001");

    EXPECT_TRUE(result);
    EXPECT_FALSE(service->isDoctorInDepartment(dept->getDepartmentID(), "DOC001"));
}

TEST_F(DepartmentServiceTest, UnassignDoctor_NotAssigned_Fail)
{
    service->createDepartment(createTestDepartment("DEP001"));
    docRepo->add(createTestDoctor("DOC001"));

    bool result = service->unassignDoctor("DEP001", "DOC001");

    EXPECT_FALSE(result);
}

TEST_F(DepartmentServiceTest, UnassignDoctor_ClearsHeadIfUnassigningHead)
{
    service->createDepartment(createTestDepartment("DEP001"));
    docRepo->add(createTestDoctor("DOC001"));
    service->assignDoctor("DEP001", "DOC001");
    service->setDepartmentHead("DEP001", "DOC001");

    service->unassignDoctor("DEP001", "DOC001");

    auto dept = service->getDepartmentByID("DEP001");
    EXPECT_TRUE(dept->getHeadDoctorID().empty());
}

TEST_F(DepartmentServiceTest, UnassignDoctor_NonExistentDepartment_Fail)
{
    bool result = service->unassignDoctor("NONEXISTENT", "DOC001");

    EXPECT_FALSE(result);
}

// ==================== Department Head Tests ====================

TEST_F(DepartmentServiceTest, SetDepartmentHead_AlreadyAssigned_Success)
{
    service->createDepartment(createTestDepartment("DEP001"));
    docRepo->add(createTestDoctor("DOC001"));
    service->assignDoctor("DEP001", "DOC001");

    bool result = service->setDepartmentHead("DEP001", "DOC001");

    EXPECT_TRUE(result);

    auto head = service->getDepartmentHead("DEP001");
    ASSERT_TRUE(head.has_value());
    EXPECT_EQ(head->getID(), "DOC001");
}

TEST_F(DepartmentServiceTest, SetDepartmentHead_NotAssigned_AutoAssigns)
{
    auto dept = service->createDepartment("Cardiology", "Desc", "Loc", "0901234567");
    docRepo->add(createTestDoctor("DOC001", "Head Candidate"));

    bool result = service->setDepartmentHead(dept->getDepartmentID(), "DOC001");

    EXPECT_TRUE(result);

    auto checkDept = service->getDepartmentByID(dept->getDepartmentID());
    ASSERT_TRUE(checkDept.has_value());
    EXPECT_EQ(checkDept->getHeadDoctorID(), "DOC001");
    EXPECT_TRUE(service->isDoctorInDepartment(dept->getDepartmentID(), "DOC001"));
}

TEST_F(DepartmentServiceTest, SetDepartmentHead_EmptyID_ClearsHead)
{
    service->createDepartment(createTestDepartment("DEP001"));
    docRepo->add(createTestDoctor("DOC001"));
    service->assignDoctor("DEP001", "DOC001");
    service->setDepartmentHead("DEP001", "DOC001");

    bool result = service->setDepartmentHead("DEP001", "");

    EXPECT_TRUE(result);
    EXPECT_FALSE(service->getDepartmentHead("DEP001").has_value());
}

TEST_F(DepartmentServiceTest, SetDepartmentHead_NonExistentDoctor_Fail)
{
    service->createDepartment(createTestDepartment("DEP001"));

    bool result = service->setDepartmentHead("DEP001", "NONEXISTENT");

    EXPECT_FALSE(result);
}

TEST_F(DepartmentServiceTest, SetDepartmentHead_NonExistentDepartment_Fail)
{
    docRepo->add(createTestDoctor("DOC001"));

    bool result = service->setDepartmentHead("NONEXISTENT", "DOC001");

    EXPECT_FALSE(result);
}

TEST_F(DepartmentServiceTest, GetDepartmentHead_NoHead_ReturnsNullopt)
{
    service->createDepartment(createTestDepartment("DEP001"));

    auto result = service->getDepartmentHead("DEP001");

    EXPECT_FALSE(result.has_value());
}

TEST_F(DepartmentServiceTest, GetDepartmentHead_NonExistentDepartment_ReturnsNullopt)
{
    auto result = service->getDepartmentHead("NONEXISTENT");

    EXPECT_FALSE(result.has_value());
}

TEST_F(DepartmentServiceTest, IsDoctorInDepartment_InDepartment_ReturnsTrue)
{
    service->createDepartment(createTestDepartment("DEP001"));
    docRepo->add(createTestDoctor("DOC001"));
    service->assignDoctor("DEP001", "DOC001");

    EXPECT_TRUE(service->isDoctorInDepartment("DEP001", "DOC001"));
}

TEST_F(DepartmentServiceTest, IsDoctorInDepartment_NotInDepartment_ReturnsFalse)
{
    service->createDepartment(createTestDepartment("DEP001"));
    docRepo->add(createTestDoctor("DOC001"));

    EXPECT_FALSE(service->isDoctorInDepartment("DEP001", "DOC001"));
}

TEST_F(DepartmentServiceTest, IsDoctorInDepartment_NonExistentDepartment_ReturnsFalse)
{
    EXPECT_FALSE(service->isDoctorInDepartment("NONEXISTENT", "DOC001"));
}

// ==================== Doctor Query Tests ====================

TEST_F(DepartmentServiceTest, GetDoctorsInDepartment_ReturnsDoctors)
{
    auto dept = service->createDepartment("Cardiology", "Desc", "Loc", "0901234567");
    docRepo->add(createTestDoctor("DOC001", "Dr. One"));
    docRepo->add(createTestDoctor("DOC002", "Dr. Two"));
    service->assignDoctor(dept->getDepartmentID(), "DOC001");
    service->assignDoctor(dept->getDepartmentID(), "DOC002");

    auto doctors = service->getDoctorsInDepartment(dept->getDepartmentID());

    EXPECT_EQ(doctors.size(), 2u);
}

TEST_F(DepartmentServiceTest, GetDoctorsInDepartment_EmptyDepartment_ReturnsEmpty)
{
    service->createDepartment(createTestDepartment("DEP001"));

    auto doctors = service->getDoctorsInDepartment("DEP001");

    EXPECT_TRUE(doctors.empty());
}

TEST_F(DepartmentServiceTest, GetDoctorsInDepartment_NonExistentDepartment_ReturnsEmpty)
{
    auto doctors = service->getDoctorsInDepartment("NONEXISTENT");

    EXPECT_TRUE(doctors.empty());
}

TEST_F(DepartmentServiceTest, GetDoctorDepartment_DoctorInDepartment_ReturnsDepartment)
{
    service->createDepartment(createTestDepartment("DEP001", "Cardiology"));
    docRepo->add(createTestDoctor("DOC001"));
    service->assignDoctor("DEP001", "DOC001");

    auto result = service->getDoctorDepartment("DOC001");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getName(), "Cardiology");
}

TEST_F(DepartmentServiceTest, GetDoctorDepartment_NotInAnyDepartment_ReturnsNullopt)
{
    docRepo->add(createTestDoctor("DOC001"));

    auto result = service->getDoctorDepartment("DOC001");

    EXPECT_FALSE(result.has_value());
}

TEST_F(DepartmentServiceTest, GetUnassignedDoctors_ReturnsUnassignedOnly)
{
    docRepo->add(createTestDoctor("DOC001", "Assigned"));
    docRepo->add(createTestDoctor("DOC002", "Free"));

    auto dept = service->createDepartment("Cardiology", "Desc", "Loc", "0901234567");
    service->assignDoctor(dept->getDepartmentID(), "DOC001");

    auto freeDocs = service->getUnassignedDoctors();

    ASSERT_EQ(freeDocs.size(), 1u);
    EXPECT_EQ(freeDocs[0].getID(), "DOC002");
}

TEST_F(DepartmentServiceTest, GetUnassignedDoctors_AllAssigned_ReturnsEmpty)
{
    docRepo->add(createTestDoctor("DOC001"));
    auto dept = service->createDepartment("Cardiology", "Desc", "Loc", "0901234567");
    service->assignDoctor(dept->getDepartmentID(), "DOC001");

    auto freeDocs = service->getUnassignedDoctors();

    EXPECT_TRUE(freeDocs.empty());
}

TEST_F(DepartmentServiceTest, GetUnassignedDoctors_NoDoctors_ReturnsEmpty)
{
    auto freeDocs = service->getUnassignedDoctors();

    EXPECT_TRUE(freeDocs.empty());
}

// ==================== Statistics Tests ====================

TEST_F(DepartmentServiceTest, GetDepartmentStats_BasicStats)
{
    service->createDepartment(createTestDepartment("DEP001", "Cardiology"));
    docRepo->add(createTestDoctor("DOC001", "Dr. One"));
    docRepo->add(createTestDoctor("DOC002", "Dr. Two"));
    service->assignDoctor("DEP001", "DOC001");
    service->assignDoctor("DEP001", "DOC002");

    auto stats = service->getDepartmentStats("DEP001");

    EXPECT_EQ(stats.departmentID, "DEP001");
    EXPECT_EQ(stats.departmentName, "Cardiology");
    EXPECT_EQ(stats.doctorCount, 2);
    EXPECT_EQ(stats.doctorNames.size(), 2u);
}

TEST_F(DepartmentServiceTest, GetDepartmentStats_WithAppointments_CalculatesRevenue)
{
    auto dept = service->createDepartment("RevenueDept", "Desc", "Loc", "0901234567");
    std::string depId = dept->getDepartmentID();

    docRepo->add(createTestDoctor("DOC001", "Earner"));
    service->assignDoctor(depId, "DOC001");

    // Add appointments - only completed ones should count
    appRepo->add(createTestAppointment("A1", "DOC001", 100.0, AppointmentStatus::COMPLETED));
    appRepo->add(createTestAppointment("A2", "DOC001", 50.0, AppointmentStatus::COMPLETED));
    appRepo->add(createTestAppointment("A3", "DOC001", 200.0, AppointmentStatus::CANCELLED));
    appRepo->add(createTestAppointment("A4", "DOC099", 500.0, AppointmentStatus::COMPLETED)); // Different doctor

    DepartmentStats stats = service->getDepartmentStats(depId);

    EXPECT_EQ(stats.doctorCount, 1);
    EXPECT_EQ(stats.appointmentCount, 2);
    EXPECT_DOUBLE_EQ(stats.totalRevenue, 150.0);
}

TEST_F(DepartmentServiceTest, GetDepartmentStats_NonExistentDepartment_ReturnsEmptyStats)
{
    auto stats = service->getDepartmentStats("NONEXISTENT");

    EXPECT_EQ(stats.departmentID, "NONEXISTENT");
    EXPECT_EQ(stats.departmentName, "");
    EXPECT_EQ(stats.doctorCount, 0);
    EXPECT_EQ(stats.appointmentCount, 0);
    EXPECT_DOUBLE_EQ(stats.totalRevenue, 0.0);
}

TEST_F(DepartmentServiceTest, GetAllDepartmentStats_ReturnsAllStats)
{
    auto d1 = service->createDepartment("D1", "Desc", "Loc", "0901111111");
    auto d2 = service->createDepartment("D2", "Desc", "Loc", "0902222222");

    auto statsMap = service->getAllDepartmentStats();

    EXPECT_EQ(statsMap.size(), 2u);
    EXPECT_TRUE(statsMap.count(d1->getDepartmentID()));
    EXPECT_TRUE(statsMap.count(d2->getDepartmentID()));
}

TEST_F(DepartmentServiceTest, GetDoctorCountByDepartment_ReturnsCounts)
{
    service->createDepartment(createTestDepartment("DEP001", "Cardiology"));
    service->createDepartment(createTestDepartment("DEP002", "Neurology"));
    docRepo->add(createTestDoctor("DOC001"));
    docRepo->add(createTestDoctor("DOC002"));
    service->assignDoctor("DEP001", "DOC001");
    service->assignDoctor("DEP001", "DOC002");

    auto counts = service->getDoctorCountByDepartment();

    EXPECT_EQ(counts["DEP001"], 2);
    EXPECT_EQ(counts["DEP002"], 0);
}

// ==================== Validation Tests ====================

TEST_F(DepartmentServiceTest, ValidateDepartment_ValidDepartment_ReturnsTrue)
{
    Department dept = createTestDepartment("DEP001");

    EXPECT_TRUE(service->validateDepartment(dept));
}

TEST_F(DepartmentServiceTest, ValidateDepartment_EmptyID_ReturnsFalse)
{
    Department dept("", "Name", "Desc", "");
    dept.setPhone("0123456789");

    EXPECT_FALSE(service->validateDepartment(dept));
}

TEST_F(DepartmentServiceTest, ValidateDepartment_EmptyName_ReturnsFalse)
{
    Department dept("DEP001", "", "Desc", "");
    dept.setPhone("0123456789");

    EXPECT_FALSE(service->validateDepartment(dept));
}

TEST_F(DepartmentServiceTest, ValidateDepartment_InvalidPhone_ReturnsFalse)
{
    // Model is a data container - validation is done at BLL layer
    Department dept = createTestDepartment("DEP001");
    dept.setPhone("invalid");

    // Service validates and rejects invalid phone
    EXPECT_FALSE(service->validateDepartment(dept));
}

TEST_F(DepartmentServiceTest, ValidateDepartment_EmptyPhoneAllowed_ReturnsTrue)
{
    Department dept("DEP001", "Name", "Desc", "");
    dept.setPhone("");

    EXPECT_TRUE(service->validateDepartment(dept));
}

TEST_F(DepartmentServiceTest, ValidateDepartment_InvalidHeadDoctor_ReturnsFalse)
{
    Department dept = createTestDepartment("DEP001");
    dept.setHeadDoctorID("NONEXISTENT");

    EXPECT_FALSE(service->validateDepartment(dept));
}

TEST_F(DepartmentServiceTest, ValidateDepartment_ValidHeadDoctor_ReturnsTrue)
{
    docRepo->add(createTestDoctor("DOC001"));

    Department dept = createTestDepartment("DEP001");
    dept.setHeadDoctorID("DOC001");

    EXPECT_TRUE(service->validateDepartment(dept));
}

TEST_F(DepartmentServiceTest, DepartmentExists_Exists_ReturnsTrue)
{
    service->createDepartment(createTestDepartment("DEP001"));

    EXPECT_TRUE(service->departmentExists("DEP001"));
}

TEST_F(DepartmentServiceTest, DepartmentExists_NotExists_ReturnsFalse)
{
    EXPECT_FALSE(service->departmentExists("NONEXISTENT"));
}

TEST_F(DepartmentServiceTest, DepartmentNameExists_Exists_ReturnsTrue)
{
    service->createDepartment(createTestDepartment("DEP001", "Cardiology"));

    EXPECT_TRUE(service->departmentNameExists("Cardiology"));
}

TEST_F(DepartmentServiceTest, DepartmentNameExists_CaseInsensitive_ReturnsTrue)
{
    service->createDepartment(createTestDepartment("DEP001", "Cardiology"));

    EXPECT_TRUE(service->departmentNameExists("cardiology"));
    EXPECT_TRUE(service->departmentNameExists("CARDIOLOGY"));
}

TEST_F(DepartmentServiceTest, DepartmentNameExists_WithWhitespace_ReturnsTrue)
{
    service->createDepartment(createTestDepartment("DEP001", "Cardiology"));

    EXPECT_TRUE(service->departmentNameExists("  Cardiology  "));
    EXPECT_TRUE(service->departmentNameExists("Cardiology "));
    EXPECT_TRUE(service->departmentNameExists(" Cardiology"));
}

TEST_F(DepartmentServiceTest, DepartmentNameExists_WithExcludeID_ExcludesCorrectly)
{
    service->createDepartment(createTestDepartment("DEP001", "Cardiology"));

    // Same ID excluded - should return false
    EXPECT_FALSE(service->departmentNameExists("Cardiology", "DEP001"));
    // Different ID not excluded - should return true
    EXPECT_TRUE(service->departmentNameExists("Cardiology", "DEP002"));
}

TEST_F(DepartmentServiceTest, DepartmentNameExists_NotExists_ReturnsFalse)
{
    EXPECT_FALSE(service->departmentNameExists("NonExistent"));
}

// ==================== Data Persistence Tests ====================

TEST_F(DepartmentServiceTest, SaveData_Success)
{
    service->createDepartment(createTestDepartment("DEP001"));

    bool result = service->saveData();

    EXPECT_TRUE(result);
    EXPECT_TRUE(std::filesystem::exists(deptFilePath));
}

TEST_F(DepartmentServiceTest, LoadData_Success)
{
    bool result = service->loadData();

    EXPECT_TRUE(result);
}

TEST_F(DepartmentServiceTest, SaveAndLoad_DataPersists)
{
    Department dept = createTestDepartment("DEP001", "Cardiology");
    dept.setDescription("Heart care");
    dept.setLocation("Building A");
    service->createDepartment(dept);
    service->saveData();

    // Reset and reload
    DepartmentService::resetInstance();
    DepartmentRepository::resetInstance();

    deptRepo = DepartmentRepository::getInstance();
    deptRepo->setFilePath(deptFilePath);

    service = DepartmentService::getInstance();
    service->loadData();

    auto loaded = service->getDepartmentByID("DEP001");

    ASSERT_TRUE(loaded.has_value());
    EXPECT_EQ(loaded->getName(), "Cardiology");
    EXPECT_EQ(loaded->getDescription(), "Heart care");
    EXPECT_EQ(loaded->getLocation(), "Building A");
}

// ==================== Integration Tests ====================

TEST_F(DepartmentServiceTest, IntegrationTest_CompleteWorkflow)
{
    // 1. Create departments
    auto cardio = service->createDepartment("Cardiology", "Heart care", "Building A", "0901111111");
    auto neuro = service->createDepartment("Neurology", "Brain care", "Building B", "0902222222");

    ASSERT_TRUE(cardio.has_value());
    ASSERT_TRUE(neuro.has_value());
    EXPECT_EQ(service->getDepartmentCount(), 2u);

    // 2. Add doctors
    docRepo->add(createTestDoctor("DOC001", "Dr. Heart"));
    docRepo->add(createTestDoctor("DOC002", "Dr. Brain"));
    docRepo->add(createTestDoctor("DOC003", "Dr. Free"));

    // 3. Assign doctors
    EXPECT_TRUE(service->assignDoctor(cardio->getDepartmentID(), "DOC001"));
    EXPECT_TRUE(service->assignDoctor(neuro->getDepartmentID(), "DOC002"));

    // 4. Verify assignments
    EXPECT_EQ(service->getDoctorsInDepartment(cardio->getDepartmentID()).size(), 1u);
    EXPECT_EQ(service->getUnassignedDoctors().size(), 1u);

    // 5. Set department heads
    EXPECT_TRUE(service->setDepartmentHead(cardio->getDepartmentID(), "DOC001"));

    auto head = service->getDepartmentHead(cardio->getDepartmentID());
    ASSERT_TRUE(head.has_value());
    EXPECT_EQ(head->getID(), "DOC001");

    // 6. Transfer doctor
    EXPECT_TRUE(service->assignDoctor(neuro->getDepartmentID(), "DOC001"));
    EXPECT_FALSE(service->isDoctorInDepartment(cardio->getDepartmentID(), "DOC001"));
    EXPECT_TRUE(service->isDoctorInDepartment(neuro->getDepartmentID(), "DOC001"));

    // 7. Get statistics
    auto stats = service->getDepartmentStats(neuro->getDepartmentID());
    EXPECT_EQ(stats.doctorCount, 2); // DOC001 and DOC002

    // 8. Delete department
    EXPECT_TRUE(service->deleteDepartment(cardio->getDepartmentID()));
    EXPECT_EQ(service->getDepartmentCount(), 1u);
}

TEST_F(DepartmentServiceTest, IntegrationTest_RevenueCalculation)
{
    // Setup department with doctor
    auto dept = service->createDepartment("RevenueDept", "Desc", "Loc", "0901234567");
    docRepo->add(createTestDoctor("DOC001", "Dr. Money"));
    service->assignDoctor(dept->getDepartmentID(), "DOC001");

    // Add various appointments - default isPaid=true
    appRepo->add(createTestAppointment("A1", "DOC001", 100.0, AppointmentStatus::COMPLETED));
    appRepo->add(createTestAppointment("A2", "DOC001", 200.0, AppointmentStatus::COMPLETED));
    appRepo->add(createTestAppointment("A3", "DOC001", 150.0, AppointmentStatus::SCHEDULED));
    appRepo->add(createTestAppointment("A4", "DOC001", 50.0, AppointmentStatus::CANCELLED));

    auto stats = service->getDepartmentStats(dept->getDepartmentID());

    // Only completed AND paid appointments count
    EXPECT_EQ(stats.appointmentCount, 2);
    EXPECT_DOUBLE_EQ(stats.totalRevenue, 300.0);
}

TEST_F(DepartmentServiceTest, GetDepartmentStats_OnlyCountsPaidRevenue)
{
    // Bug fix test: Verify revenue only counts completed AND paid appointments
    auto dept = service->createDepartment("RevenueDept", "Desc", "Loc", "0901234567");
    docRepo->add(createTestDoctor("DOC001", "Dr. Money"));
    service->assignDoctor(dept->getDepartmentID(), "DOC001");

    // Add appointments with mixed payment status
    // Paid and completed - should count
    Appointment paid1("A1", "patient_user", "DOC001", "2023-01-01", "10:00",
                      "Checkup", 100.0, true, AppointmentStatus::COMPLETED, "Note");
    Appointment paid2("A2", "patient_user", "DOC001", "2023-01-02", "11:00",
                      "Checkup", 200.0, true, AppointmentStatus::COMPLETED, "Note");

    // Unpaid but completed - should NOT count in revenue
    Appointment unpaid1("A3", "patient_user", "DOC001", "2023-01-03", "12:00",
                        "Checkup", 150.0, false, AppointmentStatus::COMPLETED, "Note");
    Appointment unpaid2("A4", "patient_user", "DOC001", "2023-01-04", "13:00",
                        "Checkup", 50.0, false, AppointmentStatus::COMPLETED, "Note");

    // Scheduled/cancelled - should not count regardless of payment status
    Appointment scheduled("A5", "patient_user", "DOC001", "2023-01-05", "14:00",
                          "Checkup", 75.0, true, AppointmentStatus::SCHEDULED, "Note");

    appRepo->add(paid1);
    appRepo->add(paid2);
    appRepo->add(unpaid1);
    appRepo->add(unpaid2);
    appRepo->add(scheduled);

    auto stats = service->getDepartmentStats(dept->getDepartmentID());

    // Only paid + completed appointments count (A1 and A2)
    EXPECT_EQ(stats.appointmentCount, 2);
    EXPECT_DOUBLE_EQ(stats.totalRevenue, 300.0);  // 100 + 200, excludes unpaid
}

/*
Build and run tests:
cd build && cmake --build . && ./HospitalTests --gtest_filter="DepartmentServiceTest.*"
*/
