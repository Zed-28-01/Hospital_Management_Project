# Hệ Thống Quản Lý Bệnh Viện - Hướng Dẫn Build

## ⚠️ LƯU Ý QUAN TRỌNG VỀ MÚI GIỜ ⚠️

**Ứng dụng YÊU CẦU múi giờ Việt Nam (Asia/Ho_Chi_Minh) để các tính năng đặt lịch hẹn hoạt động chính xác!**

### Thiết Lập Múi Giờ (BẮT BUỘC)

```bash
# KHUYẾN NGHỊ: Thiết lập múi giờ vĩnh viễn
export TZ='Asia/Ho_Chi_Minh'
echo "export TZ='Asia/Ho_Chi_Minh'" >> ~/.bashrc
source ~/.bashrc
```

**Nếu không thiết lập múi giờ, các chức năng đặt lịch, kiểm tra thời gian sẽ không hoạt động đúng!**

---

## Yêu Cầu Hệ Thống

- **CMake** >= 3.10
- **GCC** >= 14 (hỗ trợ C++23)
- **Google Test** (libgtest-dev)

Cấu hình devcontainer đã bao gồm tất cả các yêu cầu này.

---

## Bắt Đầu Nhanh

```bash
# Bước 1: Thiết lập múi giờ (BẮT BUỘC - chỉ làm 1 lần)
export TZ='Asia/Ho_Chi_Minh'
echo "export TZ='Asia/Ho_Chi_Minh'" >> ~/.bashrc
source ~/.bashrc

# Bước 2: Build ứng dụng
cd /workspaces/Hospital_Management_Project
mkdir -p build && cd build
cmake ..
make HospitalApp

# Bước 3: Chạy ứng dụng từ thư mục GỐC của project
cd ..
./build/HospitalApp
```

**⚠️ LƯU Ý:** Phải chạy `./build/HospitalApp` từ thư mục gốc của project, KHÔNG phải từ thư mục `build/`

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

# Build nhanh với nhiều CPU cores
make -j$(nproc)

# Xóa các build artifacts
make clean
```

### Chạy Ứng Dụng

```bash
# ⚠️ QUAN TRỌNG: Chạy từ thư mục GỐC của project, KHÔNG phải từ build/
cd /workspaces/Hospital_Management_Project
./build/HospitalApp
```

**Tại sao phải chạy từ thư mục gốc?**
- Ứng dụng sử dụng đường dẫn `data/` để lưu trữ dữ liệu
- Nếu chạy từ `build/`, ứng dụng sẽ không tìm thấy các file dữ liệu

### Chạy Tests

```bash
# Chạy từ thư mục build
cd /workspaces/Hospital_Management_Project/build

# Chạy tất cả tests
./HospitalTests

# Chạy tests với output chi tiết
./HospitalTests --gtest_output=xml:test_results.xml

# Chạy test suite cụ thể
./HospitalTests --gtest_filter=PatientTest.*

# Chạy với CTest
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

### Rebuild Sau Khi Thay Đổi Code

```bash
cd /workspaces/Hospital_Management_Project/build
make HospitalApp
cd ..
./build/HospitalApp
```

### Clean Build (Khi Gặp Lỗi)

```bash
cd /workspaces/Hospital_Management_Project
rm -rf build
mkdir build && cd build
cmake ..
make HospitalApp
cd ..
./build/HospitalApp
```

### Cấu Trúc Thư Mục Source Files

```
src/
├── model/              # Triển khai Entity
│   ├── Person.cpp
│   ├── Patient.cpp
│   ├── Doctor.cpp
│   ├── Admin.cpp
│   ├── Account.cpp
│   ├── Appointment.cpp
│   ├── Medicine.cpp
│   ├── Department.cpp
│   ├── Prescription.cpp
│   └── Report.cpp
│
├── dal/                # Tầng Data Access
│   ├── AccountRepository.cpp
│   ├── PatientRepository.cpp
│   ├── DoctorRepository.cpp
│   ├── AppointmentRepository.cpp
│   ├── MedicineRepository.cpp
│   ├── DepartmentRepository.cpp
│   ├── PrescriptionRepository.cpp
│   └── FileHelper.cpp
│
├── bll/                # Tầng Business Logic
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
├── ui/                 # Tầng Presentation
│   ├── HMSFacade.cpp
│   ├── ConsoleUI.cpp
│   ├── InputValidator.cpp
│   └── DisplayHelper.cpp
│
├── common/             # Utilities
│   └── Utils.cpp
│
└── main.cpp            # Entry point
```

### Cấu Trúc Thư Mục Data Files

```
data/
├── Account.txt          # Tài khoản người dùng
├── Patient.txt          # Thông tin bệnh nhân
├── Doctor.txt           # Thông tin bác sĩ
├── Appointment.txt      # Lịch hẹn khám
├── Medicine.txt         # Danh mục thuốc
├── Department.txt       # Danh mục khoa
├── Prescription.txt     # Đơn thuốc
└── Report.txt           # Báo cáo hệ thống
```

**Lưu ý:** CMake không còn copy data files vào thư mục `build/` nữa. Ứng dụng truy cập trực tiếp vào thư mục `data/` ở thư mục gốc của project để đảm bảo dữ liệu được lưu trữ vĩnh viễn.

---

## Các Vấn Đề Thường Gặp

### 1. Lỗi "Could not open file" khi chạy ứng dụng

**Nguyên nhân:** Chạy ứng dụng từ thư mục sai.

**Giải pháp:** Phải chạy từ thư mục gốc của project:
```bash
cd /workspaces/Hospital_Management_Project
./build/HospitalApp
```

### 2. Các tính năng đặt lịch không hoạt động đúng

**Nguyên nhân:** Chưa thiết lập múi giờ Việt Nam.

**Giải pháp:**
```bash
export TZ='Asia/Ho_Chi_Minh'
echo "export TZ='Asia/Ho_Chi_Minh'" >> ~/.bashrc
source ~/.bashrc
```

### 3. Lỗi "CMake Error: The source directory ... is a file"

**Nguyên nhân:** Sử dụng lệnh `cmake ./HospitalApp` thay vì `make HospitalApp`.

**Giải pháp:**
```bash
cd build
cmake ..          # Chỉ chạy 1 lần để configure
make HospitalApp  # Dùng lệnh này để build
```

### 4. Build bị lỗi sau khi thay đổi code

**Giải pháp:** Clean build lại:
```bash
cd /workspaces/Hospital_Management_Project
rm -rf build
mkdir build && cd build
cmake ..
make HospitalApp
```

---

## Quick Reference - Các Lệnh Thường Dùng

### Setup Ban Đầu (Chỉ làm 1 lần)
```bash
# Thiết lập múi giờ vĩnh viễn
export TZ='Asia/Ho_Chi_Minh'
echo "export TZ='Asia/Ho_Chi_Minh'" >> ~/.bashrc
source ~/.bashrc

# Build lần đầu
cd /workspaces/Hospital_Management_Project
mkdir -p build && cd build
cmake ..
make HospitalApp
```

### Chạy Ứng Dụng
```bash
# Luôn chạy từ thư mục gốc của project
cd /workspaces/Hospital_Management_Project
./build/HospitalApp
```

### Rebuild Sau Khi Sửa Code
```bash
cd /workspaces/Hospital_Management_Project/build
make HospitalApp
cd ..
./build/HospitalApp
```

### Chạy Tests
```bash
cd /workspaces/Hospital_Management_Project/build
./HospitalTests
```

### Clean Build (Khi có lỗi)
```bash
cd /workspaces/Hospital_Management_Project
rm -rf build
mkdir build && cd build
cmake ..
make HospitalApp
cd ..
./build/HospitalApp
```

### Build Song Song (Nhanh Hơn)
```bash
# Tự động sử dụng tất cả CPU cores
make -j$(nproc)
```

---

## Kiểm Tra Nhanh Hệ Thống

```bash
# Kiểm tra múi giờ
echo $TZ  # Kết quả phải là: Asia/Ho_Chi_Minh

# Kiểm tra compiler
g++ --version  # Phải >= 14

# Kiểm tra CMake
cmake --version  # Phải >= 3.10

# Kiểm tra Google Test
dpkg -l | grep libgtest
```

---

**Để biết thêm thông tin về kiến trúc và thiết kế hệ thống, hãy xem [ARCHITECTURE_vi.md](ARCHITECTURE_vi.md)**
