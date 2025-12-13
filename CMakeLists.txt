cmake_minimum_required(VERSION 3.10)
project(HospitalManagement)

# Cấu hình C++ 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Chỉ định folder header
include_directories(include)

# Tự động tìm tất cả file .cpp trong src
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Tạo file chạy tên là HospitalApp
add_executable(HospitalApp ${SOURCES})