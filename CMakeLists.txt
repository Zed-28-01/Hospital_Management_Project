cmake_minimum_required(VERSION 3.10)
project(HospitalManagement)

# Cấu hình C++ 23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Chỉ định folder header
include_directories(include)

# Tự động tìm tất cả file .cpp trong src
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Tạo file chạy tên là HospitalApp
add_executable(HospitalApp ${SOURCES})

# ===== PHẦN GTEST =====
# Bật testing
enable_testing()
find_package(GTest REQUIRED)

# Tìm tất cả file test
file(GLOB_RECURSE TEST_SOURCES "test/*.cpp")

# Nếu có file test thì tạo executable cho test
if(TEST_SOURCES)
    add_executable(HospitalTests ${TEST_SOURCES})
    target_link_libraries(HospitalTests GTest::GTest GTest::Main pthread)
    add_test(NAME HospitalTests COMMAND HospitalTests)
endif()