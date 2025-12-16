#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include "dal/AppointmentRepository.h"
#include "model/Appointment.h"

using namespace HMS::DAL;
using namespace HMS::Model;

static std::string tempPath(const std::string &fname)
{
    auto p = std::filesystem::current_path() / fname;
    return p.string();
}

TEST(AppointmentRepositoryFile, LoadSkipsMalformedLines)
{
    // Determine fixture path: prefer build/test/fixtures (ctest working dir),
    // Robust fixture discovery: search upwards from current working dir and from the
    // source file location until we either find the fixture or exhaust candidates.
    auto find_fixture = [](const std::string &name) -> std::optional<std::filesystem::path>
    {
        // Search upward from current working directory
        auto cwd = std::filesystem::current_path();
        for (auto p = cwd;; p = p.parent_path())
        {
            auto cand = p / "test" / "fixtures" / name;
            if (std::filesystem::exists(cand))
                return cand;
            if (p == p.parent_path())
                break; // reached filesystem root
        }

        // Also search upward from the source file location (covers IDE/ccache layouts)
        auto src = std::filesystem::path(__FILE__).parent_path(); // test/dal
        for (auto p = src;; p = p.parent_path())
        {
            auto cand = p / "fixtures" / name; // e.g., test/fixtures relative to a parent
            if (std::filesystem::exists(cand))
                return cand;
            if (p == p.parent_path())
                break;
        }

        return std::nullopt;
    };

    // Diagnostic: print current working directory to help debugging in CTest
    std::cerr << "DEBUG: cwd=" << std::filesystem::current_path() << std::endl;
    auto found = find_fixture("appointments_sample.txt");
    std::string path;
    if (found.has_value())
    {
        path = found->string();
    }
    else
    {
        // If fixture is not present in the repo (CI/package layouts may differ),
        // create a temporary fixture file at runtime with the expected test data.
        auto tmp = std::filesystem::temp_directory_path() / "hms_test_appointments_sample.txt";
        std::ofstream ofs(tmp);
        ASSERT_TRUE(ofs.is_open()) << "Failed to create temporary fixture file: " << tmp;
        ofs << "A1000|pfile1|D100|2025-12-20|10:00|check|120.0|0|0|note1\n";
        ofs << "A1001|pfile2|D101|2025-12-21|11:00|flu|100.0|0|0|note2\n";
        ofs << "malformed|line|too|short\n";
        ofs.close();
        path = tmp.string();
    }

    auto *repo = AppointmentRepository::getInstance();
    // Sanity-check the fixture is readable from test runtime
    std::ifstream testifs(path);
    ASSERT_TRUE(testifs.is_open()) << "Failed to open fixture file at: " << path;
    testifs.close();

    repo->setFilePath(path);

    // sanity: opening same path directly should succeed
    std::ifstream testifs2(path);
    ASSERT_TRUE(testifs2.is_open()) << "Even after setFilePath, cannot open fixture file: " << path;
    testifs2.close();

    // load should read file and skip malformed line
    EXPECT_TRUE(repo->load()) << "Repository failed to load from: " << path;
    auto all = repo->getAll();
    EXPECT_EQ(all.size(), 2u);

    auto gotA1 = repo->getById("A1000");
    ASSERT_TRUE(gotA1.has_value());
    EXPECT_EQ(gotA1->getPatientUsername(), "pfile1");

    auto gotA2 = repo->getById("A1001");
    ASSERT_TRUE(gotA2.has_value());
    EXPECT_EQ(gotA2->getPatientUsername(), "pfile2");

    // cleanup
    repo->clear();
    std::filesystem::remove(path);
}

TEST(AppointmentRepositoryFile, PersistWritesToFile)
{
    auto path = tempPath("test_appointments_persist.txt");
    std::filesystem::remove(path);

    auto *repo = AppointmentRepository::getInstance();
    repo->setFilePath(path);
    EXPECT_TRUE(repo->clear());

    Appointment a1("A2000", "pfile3", "D200", "2025-12-22", "11:00", "check", 120.0);
    EXPECT_TRUE(repo->add(a1));

    // ensure file exists and contains serialized line
    std::ifstream ifs(path);
    ASSERT_TRUE(ifs.is_open());
    std::string line;
    std::getline(ifs, line);
    ifs.close();

    EXPECT_EQ(line, a1.serialize());

    // cleanup
    repo->clear();
    std::filesystem::remove(path);
}
