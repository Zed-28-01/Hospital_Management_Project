#include <gtest/gtest.h>
#include "dal/DepartmentRepository.h"
#include "advance/Department.h"
#include "common/Constants.h"

#include <fstream>
#include <filesystem>

using namespace HMS;
using namespace HMS::DAL;
using namespace HMS::Model;

namespace {
    const std::string TEST_DIR = "test/fixtures/";
    const std::string TEST_FILE = "test/fixtures/Department_test.txt";
}

class DepartmentRepositoryTest : public ::testing::Test {
protected:
    DepartmentRepository* repo;

    void SetUp() override {
        std::filesystem::create_directories(TEST_DIR);

        DepartmentRepository::resetInstance();
        repo = DepartmentRepository::getInstance();
        repo->setFilePath(TEST_FILE);

        std::ofstream ofs(TEST_FILE, std::ios::trunc);
        ofs.close();

        repo->clear();
    }

    void TearDown() override {
        repo->clear();
        DepartmentRepository::resetInstance();
    }

    Department createDepartment(
        const std::string& id = "D001",
        const std::string& name = "Cardiology",
        const std::vector<std::string>& doctors = {}
    ) {
        Department d(id, name);
        for (const auto& doc : doctors) {
            d.addDoctor(doc);
        }
        return d;
    }
};

/* ===================== SINGLETON ===================== */

TEST_F(DepartmentRepositoryTest, GetInstance_ReturnsSameInstance) {
    EXPECT_EQ(DepartmentRepository::getInstance(),
              DepartmentRepository::getInstance());
}

TEST_F(DepartmentRepositoryTest, ResetInstance_CreatesNewInstance) {
    auto* oldInstance = DepartmentRepository::getInstance();
    DepartmentRepository::resetInstance();
    auto* newInstance = DepartmentRepository::getInstance();

    EXPECT_NE(oldInstance, newInstance);
}

/* ===================== ADD ===================== */

TEST_F(DepartmentRepositoryTest, Add_ValidDepartment_Succeeds) {
    EXPECT_TRUE(repo->add(createDepartment()));
    EXPECT_EQ(repo->count(), 1u);
}

TEST_F(DepartmentRepositoryTest, Add_DuplicateID_Fails) {
    EXPECT_TRUE(repo->add(createDepartment("D001")));
    EXPECT_FALSE(repo->add(createDepartment("D001", "Other")));
}

TEST_F(DepartmentRepositoryTest, Add_MultipleDepartments) {
    repo->add(createDepartment("D001", "A"));
    repo->add(createDepartment("D002", "B"));
    repo->add(createDepartment("D003", "C"));
    EXPECT_EQ(repo->count(), 3u);
}

/* ===================== GET ===================== */

TEST_F(DepartmentRepositoryTest, GetById_Existing_ReturnsDepartment) {
    repo->add(createDepartment("D001", "Cardio"));
    auto dep = repo->getById("D001");

    ASSERT_TRUE(dep.has_value());
    EXPECT_EQ(dep->getName(), "Cardio");
}

TEST_F(DepartmentRepositoryTest, GetById_NotFound_ReturnsNullopt) {
    EXPECT_FALSE(repo->getById("D999").has_value());
}

TEST_F(DepartmentRepositoryTest, GetByName_CaseInsensitive) {
    repo->add(createDepartment("D001", "Neurology"));
    EXPECT_TRUE(repo->getByName("neurology").has_value());
}

TEST_F(DepartmentRepositoryTest, GetByName_NotFound) {
    EXPECT_FALSE(repo->getByName("Unknown").has_value());
}

/* ===================== UPDATE ===================== */

TEST_F(DepartmentRepositoryTest, Update_ExistingDepartment) {
    repo->add(createDepartment("D001", "Old"));

    Department updated("D001", "New");
    EXPECT_TRUE(repo->update(updated));

    auto dep = repo->getById("D001");
    ASSERT_TRUE(dep.has_value());
    EXPECT_EQ(dep->getName(), "New");
}

TEST_F(DepartmentRepositoryTest, Update_NonExisting_Fails) {
    EXPECT_FALSE(repo->update(createDepartment("D999")));
}

/* ===================== REMOVE ===================== */

TEST_F(DepartmentRepositoryTest, Remove_ExistingDepartment) {
    repo->add(createDepartment("D001"));
    EXPECT_TRUE(repo->remove("D001"));
    EXPECT_FALSE(repo->exists("D001"));
}

TEST_F(DepartmentRepositoryTest, Remove_NonExisting_Fails) {
    EXPECT_FALSE(repo->remove("D999"));
}

/* ===================== EXISTS / COUNT ===================== */

TEST_F(DepartmentRepositoryTest, Exists_WorksCorrectly) {
    repo->add(createDepartment("D001"));
    EXPECT_TRUE(repo->exists("D001"));
    EXPECT_FALSE(repo->exists("D002"));
}

TEST_F(DepartmentRepositoryTest, Count_AfterOperations) {
    repo->add(createDepartment("D001"));
    repo->add(createDepartment("D002"));
    repo->remove("D001");
    EXPECT_EQ(repo->count(), 1u);
}

/* ===================== CLEAR ===================== */

TEST_F(DepartmentRepositoryTest, Clear_RemovesAll) {
    repo->add(createDepartment("D001"));
    repo->add(createDepartment("D002"));
    repo->clear();
    EXPECT_EQ(repo->count(), 0u);
}

/* ===================== SEARCH ===================== */

TEST_F(DepartmentRepositoryTest, SearchByName_PartialMatch) {
    repo->add(createDepartment("D001", "Cardiology"));
    repo->add(createDepartment("D002", "Cardio Rehab"));
    repo->add(createDepartment("D003", "Neurology"));

    auto result = repo->searchByName("cardio");
    EXPECT_EQ(result.size(), 2u);
}

TEST_F(DepartmentRepositoryTest, SearchByName_Empty_ReturnsAll) {
    repo->add(createDepartment("D001"));
    repo->add(createDepartment("D002"));
    EXPECT_EQ(repo->searchByName("").size(), 2u);
}

/* ===================== GET ALL ===================== */

TEST_F(DepartmentRepositoryTest, GetAll_ReturnsInInsertOrder) {
    repo->add(createDepartment("D001", "A"));
    repo->add(createDepartment("D002", "B"));
    repo->add(createDepartment("D003", "C"));

    auto all = repo->getAll();
    ASSERT_EQ(all.size(), 3u);
    EXPECT_EQ(all[0].getName(), "A");
    EXPECT_EQ(all[1].getName(), "B");
    EXPECT_EQ(all[2].getName(), "C");
}

TEST_F(DepartmentRepositoryTest, GetAllNames_ReturnsCorrectList) {
    repo->add(createDepartment("D001", "A"));
    repo->add(createDepartment("D002", "B"));

    auto names = repo->getAllNames();
    EXPECT_EQ(names.size(), 2u);
    EXPECT_EQ(names[0], "A");
    EXPECT_EQ(names[1], "B");
}

/* ===================== DOCTOR QUERIES ===================== */

TEST_F(DepartmentRepositoryTest, GetDepartmentByDoctor_ReturnsCorrect) {
    repo->add(createDepartment("D001", "A", {"DOC1"}));
    repo->add(createDepartment("D002", "B", {"DOC2"}));

    auto dep = repo->getDepartmentByDoctor("DOC1");
    ASSERT_TRUE(dep.has_value());
    EXPECT_EQ(dep->getDepartmentID(), "D001");
}

TEST_F(DepartmentRepositoryTest, GetDepartmentsByDoctor_ReturnsMultiple) {
    repo->add(createDepartment("D001", "A", {"DOC1"}));
    repo->add(createDepartment("D002", "B", {"DOC1"}));
    repo->add(createDepartment("D003", "C", {"DOC2"}));

    auto deps = repo->getDepartmentsByDoctor("DOC1");
    EXPECT_EQ(deps.size(), 2u);
}

TEST_F(DepartmentRepositoryTest, GetDepartmentByDoctor_NotFound) {
    EXPECT_FALSE(repo->getDepartmentByDoctor("UNKNOWN").has_value());
}

/* ===================== GET NEXT ID ===================== */

TEST_F(DepartmentRepositoryTest, GetNextId_EmptyRepo) {
    EXPECT_EQ(repo->getNextId(), "D001");
}

TEST_F(DepartmentRepositoryTest, GetNextId_WithGaps) {
    repo->add(createDepartment("D001"));
    repo->add(createDepartment("D005"));
    repo->add(createDepartment("D003"));

    EXPECT_EQ(repo->getNextId(), "D006");
}

TEST_F(DepartmentRepositoryTest, GetNextId_IgnoresInvalidIDs) {
    repo->add(createDepartment("D001"));
    repo->add(createDepartment("XYZ"));
    repo->add(createDepartment("D002"));

    EXPECT_EQ(repo->getNextId(), "D003");
}

/* ===================== PERSISTENCE ===================== */

TEST_F(DepartmentRepositoryTest, SaveAndLoad_PersistsData) {
    repo->add(createDepartment("D001", "Cardio", {"DOC1"}));
    repo->add(createDepartment("D002", "Neuro", {"DOC2"}));
    repo->save();

    DepartmentRepository::resetInstance();
    repo = DepartmentRepository::getInstance();
    repo->setFilePath(TEST_FILE);
    repo->load();

    EXPECT_EQ(repo->count(), 2u);
    EXPECT_TRUE(repo->exists("D001"));
    EXPECT_TRUE(repo->exists("D002"));
}

TEST_F(DepartmentRepositoryTest, LazyLoad_LoadsOnAccess) {
    repo->add(createDepartment("D001"));
    repo->save();

    DepartmentRepository::resetInstance();
    repo = DepartmentRepository::getInstance();
    repo->setFilePath(TEST_FILE);

    EXPECT_TRUE(repo->exists("D001"));
}

/* ===================== FILE PATH ===================== */

TEST_F(DepartmentRepositoryTest, SetFilePath_ForcesReload) {
    repo->add(createDepartment("D001"));
    repo->save();

    std::string secondFile = TEST_DIR + "Department_test_2.txt";
    {
        std::ofstream out(secondFile);
        out << "# Department\n";
        out << createDepartment("D002", "Second").serialize() << "\n";
    }

    repo->setFilePath(secondFile);
    auto all = repo->getAll();

    EXPECT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].getDepartmentID(), "D002");

    std::filesystem::remove(secondFile);
}
