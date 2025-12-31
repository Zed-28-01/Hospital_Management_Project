# Hệ Thống Quản Lý Bệnh Viện - Hướng Dẫn Build

## Yêu Cầu Hệ Thống

- **CMake** >= 3.10
- **GCC** >= 14 (hỗ trợ C++23)
- **Google Test** (libgtest-dev)

Cấu hình devcontainer đã bao gồm tất cả các yêu cầu này.

---

## Bắt Đầu Nhanh

```bash
# Từ thư mục gốc của project
mkdir -p build && cd build
cmake ..
make HospitalApp
./HospitalApp
```

---

## Các Lệnh Build

### Cấu Hình Project

```bash
# Tạo thư mục build
mkdir -p build
cd build

# Cấu hình với CMake (chế độ Debug - mặc định)
cmake ..

# Cấu hình với chế độ Release
cmake -DCMAKE_BUILD_TYPE=Release ..
```

### Build Targets

```bash
# Build chỉ ứng dụng chính
make HospitalApp

# Build static library (được dùng bởi tests)
make HospitalLib

# Build chỉ tests
make HospitalTests

# Build tất cả
make all

# Xóa các build artifacts
make clean
```

### Chạy Ứng Dụng

```bash
# Từ thư mục build
./HospitalApp
```

### Chạy Tests

```bash
# Chạy tất cả tests
./HospitalTests

# Chạy tests với output chi tiết
./HospitalTests --gtest_output=xml:test_results.xml

# Chạy test suite cụ thể
./HospitalTests --gtest_filter=PatientTest.*

# Chạy với CTest
ctest

# Chạy với CTest chi tiết
ctest --verbose
```

---

## Tóm Tắt Cấu Hình Build

Sau khi chạy `cmake ..`, bạn sẽ thấy:

```
=== Hospital Management System Build Configuration ===
CMake version:     <version>
C++ Standard:      23
Build type:        Debug
Compiler:          GNU <version>
Project version:   1.0.0

Source directories:
  Model:           src/model/
  DAL:             src/dal/
  BLL:             src/bll/
  UI:              src/ui/
  Common:          src/common/

Build targets:
  HospitalApp      - Main application
  HospitalLib      - Static library
  HospitalTests    - Unit tests (GTest)
======================================================
```

---

## Quy Trình Phát Triển

### Thêm File Source Mới

1. Thêm header file vào thư mục `include/<layer>/` phù hợp
2. Thêm source file vào thư mục `src/<layer>/` tương ứng
3. Chạy lại `cmake ..` (hoặc CMake sẽ tự động phát hiện file mới)
4. Build với `make`

### Thêm Tests Mới

1. Tạo file test trong thư mục `test/<layer>/` phù hợp
2. Tuân theo quy ước đặt tên của Google Test
3. Rebuild tests với `make HospitalTests`

### Cấu Trúc Thư Mục Source Files

```
src/
├── model/          # Triển khai Entity
│   ├── Person.cpp
│   ├── Patient.cpp
│   ├── Doctor.cpp
│   ├── Admin.cpp
│   ├── Account.cpp
│   └── Appointment.cpp
│
├── dal/            # Tầng Data Access
│   ├── AccountRepository.cpp
│   ├── PatientRepository.cpp
│   ├── DoctorRepository.cpp
│   ├── AppointmentRepository.cpp
│   └── FileHelper.cpp
│
├── bll/            # Tầng Business Logic
│   ├── AuthService.cpp
│   ├── PatientService.cpp
│   ├── DoctorService.cpp
│   ├── AppointmentService.cpp
│   ├── AdminService.cpp
│   ├── MedicineService.cpp
│   ├── DepartmentService.cpp
│   ├── PrescriptionService.cpp
│   └── ReportGenerator.cpp
│
├── ui/             # Tầng Presentation
│   ├── HMSFacade.cpp
│   ├── ConsoleUI.cpp
│   ├── InputValidator.cpp
│   └── DisplayHelper.cpp
│
├── common/         # Utilities
│   └── Utils.cpp
│
└── main.cpp        # Entry point
```

---

## Các Vấn Đề Thường Gặp

### 1. Lỗi phiên bản CMake

```
CMake Error at CMakeLists.txt:1 (cmake_minimum_required):
  CMake 3.10 or higher is required.
```

**Giải pháp:** Cập nhật CMake hoặc sử dụng devcontainer.

### 2. Không tìm thấy GTest

```
CMake Error: Could not find GTest
```

**Giải pháp:** Cài đặt Google Test:
```bash
sudo apt-get install libgtest-dev
```

### 3. Tính năng C++23 không được hỗ trợ

```
error: 'expected' is not a member of 'std'
```

**Giải pháp:** Sử dụng GCC 14 trở lên. Devcontainer đã được cấu hình sẵn với GCC 14.

### 4. Lỗi linker cho undefined references

Nếu bạn thấy lỗi undefined reference sau khi thêm source files mới:

```bash
# Clean và rebuild
cd build
rm -rf *
cmake ..
make all
```

---

## Tích Hợp VS Code

Nếu sử dụng VS Code với devcontainer:

1. Mở Command Palette (Ctrl+Shift+P)
2. Chọn "CMake: Configure"
3. Chọn "CMake: Build" hoặc nhấn F7
4. Chạy với F5 (Debug) hoặc Ctrl+F5 (Run)

---

## Release Build

Để build bản release/production:

```bash
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make all

# Executable sẽ được tối ưu hóa
./HospitalApp
```

Sự khác biệt giữa Debug và Release:

| Chế Độ | Compiler Flags | Kích Thước | Tốc Độ | Debug Info |
|--------|---------------|-----------|---------|-----------|
| Debug | `-g -O0 -Wall -Wextra` | Lớn hơn | Chậm hơn | Có |
| Release | `-O3 -DNDEBUG` | Nhỏ hơn | Nhanh hơn | Không |

---

## Cài Đặt (Tùy Chọn)

```bash
# Cài đặt vào hệ thống (mặc định: /usr/local/bin)
sudo make install

# Cài đặt vào thư mục tùy chỉnh
cmake -DCMAKE_INSTALL_PREFIX=/custom/path ..
make install
```

---

## Debug và Testing

### Debug với GDB

```bash
# Build với debug symbols
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make HospitalApp

# Chạy với GDB
gdb ./HospitalApp

# Trong GDB:
(gdb) break main           # Đặt breakpoint tại main
(gdb) run                  # Chạy chương trình
(gdb) next                 # Chạy dòng tiếp theo
(gdb) print variable_name  # In giá trị biến
(gdb) continue             # Tiếp tục chạy
(gdb) quit                 # Thoát GDB
```

### Memory Check với Valgrind

```bash
# Kiểm tra memory leaks
valgrind --leak-check=full ./HospitalApp

# Kiểm tra với tests
valgrind --leak-check=full ./HospitalTests
```

### Coverage Testing

```bash
# Build với coverage flags
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="--coverage" ..
make all

# Chạy tests
./HospitalTests

# Tạo coverage report
gcov *.cpp
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_html
```

---

## Tối Ưu Hóa Build

### Build Song Song

```bash
# Build với nhiều cores
make -j4  # Sử dụng 4 cores

# Hoặc tự động phát hiện số cores
make -j$(nproc)
```

### Incremental Build

CMake hỗ trợ incremental build - chỉ compile các file đã thay đổi:

```bash
# Chỉ cần chạy make, không cần clean
cd build
make
```

### Clean Build

Khi cần build lại từ đầu:

```bash
# Cách 1: Clean trong thư mục build
cd build
make clean
cmake ..
make all

# Cách 2: Xóa toàn bộ thư mục build
cd ..
rm -rf build
mkdir build
cd build
cmake ..
make all
```

---

## Các Target CMake Nâng Cao

### Xem Tất Cả Targets

```bash
make help
```

### Build Cụ Thể

```bash
# Build chỉ một source file
make src/model/Patient.cpp.o

# Build và xem verbose output
make VERBOSE=1
```

### Install và Package

```bash
# Install vào hệ thống
sudo make install

# Gỡ cài đặt
sudo make uninstall  # (nếu có target này)

# Tạo package (cần cấu hình CPack)
make package
```

---

## Troubleshooting Nâng Cao

### Kiểm Tra Compiler

```bash
# Kiểm tra version GCC
g++ --version

# Kiểm tra C++23 features
g++ -std=c++23 -E -dM - < /dev/null | grep __cplusplus
```

### Kiểm Tra CMake Cache

```bash
# Xem các biến CMake
cmake -L
cmake -LA  # Xem tất cả biến kể cả advanced

# Xóa cache và reconfigure
rm CMakeCache.txt
cmake ..
```

### Debug CMake

```bash
# Chạy CMake với debug output
cmake --debug-output ..

# Hoặc với trace
cmake --trace ..
```

---

## Best Practices

### 1. Quy Trình Build Hàng Ngày

```bash
# Sáng: Pull code mới
git pull origin main

# Build và test
cd build
cmake ..
make all
./HospitalTests

# Nếu có lỗi, clean build
make clean
cmake ..
make all
```

### 2. Trước Khi Commit

```bash
# Chạy tất cả tests
./HospitalTests

# Kiểm tra memory leaks
valgrind --leak-check=full ./HospitalTests

# Chạy với sanitizers
cmake -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined" ..
make all
./HospitalTests
```

### 3. Khi Gặp Lỗi Lạ

```bash
# 1. Clean build hoàn toàn
cd ..
rm -rf build
mkdir build
cd build
cmake ..
make all

# 2. Kiểm tra compiler và flags
cmake -LA | grep CMAKE_CXX

# 3. Build từng target riêng
make HospitalLib
make HospitalApp
make HospitalTests
```

---

## Cấu Trúc Build Output

Sau khi build thành công:

```
build/
├── CMakeCache.txt
├── CMakeFiles/
├── Makefile
├── HospitalApp           # Executable chính
├── HospitalTests         # Executable tests
├── libHospitalLib.a      # Static library
├── data/                 # Data files (được copy)
│   ├── Account.txt
│   ├── Patient.txt
│   ├── Doctor.txt
│   └── Appointment.txt
└── test/
    └── fixtures/         # Test data (được copy)
```

---

## Tham Khảo

### CMake Documentation
- [CMake Tutorial](https://cmake.org/cmake/help/latest/guide/tutorial/index.html)
- [CMake Commands](https://cmake.org/cmake/help/latest/manual/cmake-commands.7.html)

### Google Test Documentation
- [Google Test Primer](https://google.github.io/googletest/primer.html)
- [Advanced Testing](https://google.github.io/googletest/advanced.html)

### C++23 Resources
- [C++23 Features](https://en.cppreference.com/w/cpp/23)
- [GCC C++23 Support](https://gcc.gnu.org/projects/cxx-status.html#cxx23)

---

## Quick Reference

### Lệnh Thường Dùng

```bash
# Build nhanh
cd build && cmake .. && make -j$(nproc)

# Clean và rebuild
cd build && make clean && cmake .. && make all

# Test nhanh
cd build && make HospitalTests && ./HospitalTests

# Run ứng dụng
cd build && make HospitalApp && ./HospitalApp

# Debug với GDB
cd build && gdb ./HospitalApp
```

### Kiểm Tra Nhanh

```bash
# Kiểm tra compiler
g++ --version

# Kiểm tra CMake
cmake --version

# Kiểm tra Google Test
dpkg -l | grep libgtest

# Kiểm tra C++ standard
echo | g++ -std=c++23 -x c++ -E -dM - | grep __cplusplus
```

---

**Để biết thêm thông tin về kiến trúc và thiết kế hệ thống, hãy xem ARCHITECTURE_vi.md**