#include <gtest/gtest.h>
#include "dal/FileHelper.h"
#include <filesystem>
#include <fstream>

using HMS::DAL::FileHelper;
namespace fs = std::filesystem;

class FileHelperTest : public ::testing::Test {
protected:
    std::string testDir = "test_filehelper_temp";
    std::string testFile = testDir + "/test.txt";

    void SetUp() override {
        fs::create_directories(testDir);
    }

    void TearDown() override {
        fs::remove_all(testDir);
    }

    void createTestFile(const std::string& path, const std::string& content) {
        std::ofstream file(path);
        file << content;
    }
};

// ==================== Read Operations ====================

TEST_F(FileHelperTest, ReadLines_ValidFile_ReturnsLines) {
    createTestFile(testFile, "line1\nline2\nline3\n");

    auto lines = FileHelper::readLines(testFile);

    EXPECT_EQ(lines.size(), 3);
    EXPECT_EQ(lines[0], "line1");
    EXPECT_EQ(lines[1], "line2");
    EXPECT_EQ(lines[2], "line3");
}

TEST_F(FileHelperTest, ReadLines_SkipsEmptyLines) {
    createTestFile(testFile, "line1\n\nline2\n   \nline3\n");

    auto lines = FileHelper::readLines(testFile);

    EXPECT_EQ(lines.size(), 3);
    EXPECT_EQ(lines[0], "line1");
    EXPECT_EQ(lines[1], "line2");
    EXPECT_EQ(lines[2], "line3");
}

TEST_F(FileHelperTest, ReadLines_SkipsComments) {
    createTestFile(testFile, "# comment\nline1\n# another comment\nline2\n");

    auto lines = FileHelper::readLines(testFile);

    EXPECT_EQ(lines.size(), 2);
    EXPECT_EQ(lines[0], "line1");
    EXPECT_EQ(lines[1], "line2");
}

TEST_F(FileHelperTest, ReadLines_NonExistentFile_ReturnsEmpty) {
    auto lines = FileHelper::readLines("nonexistent.txt");

    EXPECT_TRUE(lines.empty());
}

TEST_F(FileHelperTest, ReadAllLines_IncludesEmptyAndComments) {
    createTestFile(testFile, "# comment\nline1\n\nline2\n");

    auto lines = FileHelper::readAllLines(testFile);

    EXPECT_EQ(lines.size(), 4);
    EXPECT_EQ(lines[0], "# comment");
    EXPECT_EQ(lines[1], "line1");
    EXPECT_EQ(lines[2], "");
    EXPECT_EQ(lines[3], "line2");
}

TEST_F(FileHelperTest, ReadFile_ValidFile_ReturnsContent) {
    std::string content = "Hello\nWorld\n";
    createTestFile(testFile, content);

    auto result = FileHelper::readFile(testFile);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), content);
}

TEST_F(FileHelperTest, ReadFile_NonExistentFile_ReturnsNullopt) {
    auto result = FileHelper::readFile("nonexistent.txt");

    EXPECT_FALSE(result.has_value());
}

// ==================== Write Operations ====================

TEST_F(FileHelperTest, WriteLines_CreatesFileWithContent) {
    std::vector<std::string> lines = {"line1", "line2", "line3"};

    bool success = FileHelper::writeLines(testFile, lines);

    EXPECT_TRUE(success);
    auto readBack = FileHelper::readLines(testFile);
    EXPECT_EQ(readBack.size(), 3);
    EXPECT_EQ(readBack[0], "line1");
}

TEST_F(FileHelperTest, WriteLines_OverwritesExistingFile) {
    createTestFile(testFile, "old content\n");
    std::vector<std::string> lines = {"new content"};

    FileHelper::writeLines(testFile, lines);

    auto readBack = FileHelper::readLines(testFile);
    EXPECT_EQ(readBack.size(), 1);
    EXPECT_EQ(readBack[0], "new content");
}

TEST_F(FileHelperTest, WriteFile_CreatesFileWithContent) {
    std::string content = "test content";

    bool success = FileHelper::writeFile(testFile, content);

    EXPECT_TRUE(success);
    auto readBack = FileHelper::readFile(testFile);
    EXPECT_EQ(readBack.value(), content);
}

TEST_F(FileHelperTest, AppendLine_AddsToExistingFile) {
    createTestFile(testFile, "line1\n");

    bool success = FileHelper::appendLine(testFile, "line2");

    EXPECT_TRUE(success);
    auto lines = FileHelper::readLines(testFile);
    EXPECT_EQ(lines.size(), 2);
    EXPECT_EQ(lines[1], "line2");
}

TEST_F(FileHelperTest, AppendLine_CreatesNewFileIfNotExists) {
    std::string newFile = testDir + "/newfile.txt";

    bool success = FileHelper::appendLine(newFile, "first line");

    EXPECT_TRUE(success);
    auto lines = FileHelper::readLines(newFile);
    EXPECT_EQ(lines.size(), 1);
    EXPECT_EQ(lines[0], "first line");
}

TEST_F(FileHelperTest, AppendLines_AddsMultipleLines) {
    createTestFile(testFile, "line1\n");
    std::vector<std::string> newLines = {"line2", "line3"};

    bool success = FileHelper::appendLines(testFile, newLines);

    EXPECT_TRUE(success);
    auto lines = FileHelper::readLines(testFile);
    EXPECT_EQ(lines.size(), 3);
}

// ==================== File Management ====================

TEST_F(FileHelperTest, FileExists_ExistingFile_ReturnsTrue) {
    createTestFile(testFile, "content");

    EXPECT_TRUE(FileHelper::fileExists(testFile));
}

TEST_F(FileHelperTest, FileExists_NonExistentFile_ReturnsFalse) {
    EXPECT_FALSE(FileHelper::fileExists("nonexistent.txt"));
}

TEST_F(FileHelperTest, CreateFileIfNotExists_CreatesNewFile) {
    std::string newFile = testDir + "/newfile.txt";

    bool success = FileHelper::createFileIfNotExists(newFile);

    EXPECT_TRUE(success);
    EXPECT_TRUE(FileHelper::fileExists(newFile));
}

TEST_F(FileHelperTest, CreateFileIfNotExists_ExistingFile_ReturnsTrue) {
    createTestFile(testFile, "content");

    bool success = FileHelper::createFileIfNotExists(testFile);

    EXPECT_TRUE(success);
}

TEST_F(FileHelperTest, CreateDirectoryIfNotExists_CreatesNewDirectory) {
    std::string newDir = testDir + "/subdir";

    bool success = FileHelper::createDirectoryIfNotExists(newDir);

    EXPECT_TRUE(success);
    EXPECT_TRUE(fs::exists(newDir));
}

TEST_F(FileHelperTest, CreateDirectoryIfNotExists_ExistingDirectory_ReturnsTrue) {
    bool success = FileHelper::createDirectoryIfNotExists(testDir);

    EXPECT_TRUE(success);
}

TEST_F(FileHelperTest, CreateDirectoryIfNotExists_NestedDirectories_CreatesAll) {
    std::string nestedDir = testDir + "/a/b/c";

    bool success = FileHelper::createDirectoryIfNotExists(nestedDir);

    EXPECT_TRUE(success);
    EXPECT_TRUE(fs::exists(nestedDir));
}

TEST_F(FileHelperTest, DeleteFile_ExistingFile_DeletesAndReturnsTrue) {
    createTestFile(testFile, "content");

    bool success = FileHelper::deleteFile(testFile);

    EXPECT_TRUE(success);
    EXPECT_FALSE(FileHelper::fileExists(testFile));
}

TEST_F(FileHelperTest, DeleteFile_NonExistentFile_ReturnsFalse) {
    bool success = FileHelper::deleteFile("nonexistent.txt");

    EXPECT_FALSE(success);
}

TEST_F(FileHelperTest, CopyFile_ValidSource_CopiesFile) {
    createTestFile(testFile, "content to copy");
    std::string destFile = testDir + "/copy.txt";

    bool success = FileHelper::copyFile(testFile, destFile);

    EXPECT_TRUE(success);
    EXPECT_TRUE(FileHelper::fileExists(destFile));
    auto content = FileHelper::readFile(destFile);
    EXPECT_EQ(content.value(), "content to copy");
}

TEST_F(FileHelperTest, CopyFile_NonExistentSource_ReturnsFalse) {
    bool success = FileHelper::copyFile("nonexistent.txt", testDir + "/dest.txt");

    EXPECT_FALSE(success);
}

TEST_F(FileHelperTest, CopyFile_OverwritesExisting) {
    createTestFile(testFile, "original");
    std::string destFile = testDir + "/dest.txt";
    createTestFile(destFile, "old content");

    FileHelper::copyFile(testFile, destFile);

    auto content = FileHelper::readFile(destFile);
    EXPECT_EQ(content.value(), "original");
}

// ==================== Utility Methods ====================

TEST_F(FileHelperTest, IsComment_CommentLine_ReturnsTrue) {
    EXPECT_TRUE(FileHelper::isComment("# this is a comment"));
    EXPECT_TRUE(FileHelper::isComment("#"));
    EXPECT_TRUE(FileHelper::isComment("# "));
}

TEST_F(FileHelperTest, IsComment_NonCommentLine_ReturnsFalse) {
    EXPECT_FALSE(FileHelper::isComment("not a comment"));
    EXPECT_FALSE(FileHelper::isComment(""));
    EXPECT_FALSE(FileHelper::isComment("data # with hash"));
}

TEST_F(FileHelperTest, IsEmpty_EmptyLine_ReturnsTrue) {
    EXPECT_TRUE(FileHelper::isEmpty(""));
    EXPECT_TRUE(FileHelper::isEmpty("   "));
    EXPECT_TRUE(FileHelper::isEmpty("\t"));
    EXPECT_TRUE(FileHelper::isEmpty("  \t  "));
}

TEST_F(FileHelperTest, IsEmpty_NonEmptyLine_ReturnsFalse) {
    EXPECT_FALSE(FileHelper::isEmpty("content"));
    EXPECT_FALSE(FileHelper::isEmpty("  content  "));
    EXPECT_FALSE(FileHelper::isEmpty("a"));
}

TEST_F(FileHelperTest, GetFileHeader_Account_ReturnsCorrectHeader) {
    std::string header = FileHelper::getFileHeader("Account");

    EXPECT_TRUE(header.find("username") != std::string::npos);
    EXPECT_TRUE(header.find("passwordHash") != std::string::npos);
    EXPECT_TRUE(header[0] == '#');
}

TEST_F(FileHelperTest, GetFileHeader_Patient_ReturnsCorrectHeader) {
    std::string header = FileHelper::getFileHeader("Patient");

    EXPECT_TRUE(header.find("patientID") != std::string::npos);
    EXPECT_TRUE(header.find("medicalHistory") != std::string::npos);
}

TEST_F(FileHelperTest, GetFileHeader_Doctor_ReturnsCorrectHeader) {
    std::string header = FileHelper::getFileHeader("Doctor");

    EXPECT_TRUE(header.find("doctorID") != std::string::npos);
    EXPECT_TRUE(header.find("specialization") != std::string::npos);
}

TEST_F(FileHelperTest, GetFileHeader_Appointment_ReturnsCorrectHeader) {
    std::string header = FileHelper::getFileHeader("Appointment");

    EXPECT_TRUE(header.find("appointmentID") != std::string::npos);
    EXPECT_TRUE(header.find("status") != std::string::npos);
}

TEST_F(FileHelperTest, GetFileHeader_Unknown_ReturnsDefaultHeader) {
    std::string header = FileHelper::getFileHeader("Unknown");

    EXPECT_EQ(header, "# Data file");
}

// ==================== Backup Operations ====================

TEST_F(FileHelperTest, GetBackupPath_ReturnsValidPath) {
    std::string backupPath = FileHelper::getBackupPath(testFile);

    EXPECT_TRUE(backupPath.find("_backup_") != std::string::npos);
    EXPECT_TRUE(backupPath.find(".txt") != std::string::npos);
}

TEST_F(FileHelperTest, CreateBackup_ExistingFile_CreatesBackup) {
    createTestFile(testFile, "backup content");

    bool success = FileHelper::createBackup(testFile);

    EXPECT_TRUE(success);
}

TEST_F(FileHelperTest, CreateBackup_NonExistentFile_ReturnsFalse) {
    bool success = FileHelper::createBackup("nonexistent.txt");

    EXPECT_FALSE(success);
}


/*
cd build && ./HospitalTests --gtest_filter="FileHelperTest.*"
*/