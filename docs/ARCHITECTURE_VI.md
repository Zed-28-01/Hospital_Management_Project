# Hệ Thống Quản Lý Bệnh Viện - Tài Liệu Thiết Kế Kiến Trúc

## Mục Lục
1. [Xem Xét & Tối Ưu Class Diagram](#1-xem-xét--tối-ưu-class-diagram)
2. [Thiết Kế Kiến Trúc 3 Tầng](#2-thiết-kế-kiến-trúc-3-tầng)
3. [Áp Dụng Design Patterns](#3-áp-dụng-design-patterns)
4. [Cấu Trúc Thư Mục](#4-cấu-trúc-thư-mục)
5. [Trách Nhiệm Các File](#5-trách-nhiệm-các-file)
6. [Định Dạng File Dữ Liệu](#6-định-dạng-file-dữ-liệu)
7. [Hệ Thống Build](#7-hệ-thống-build)

---

## 1. Xem Xét & Tối Ưu Class Diagram

### Các Vấn Đề Trong Thiết Kế Ban Đầu POC của nhóm

#### Vấn Đề 1: Vi Phạm Nguyên Tắc Single Responsibility (SRP)
**Ban đầu:** Các class `Patient`, `Doctor`, `Admin` chứa cả DỮ LIỆU VÀ business logic.
- `Patient.bookAppointment()`, `Doctor.addPatient()`, `Admin.addDoctor()`

**Giải pháp:** Tách riêng data models (entities) khỏi business logic (services).

#### Vấn Đề 2: Vi Phạm Nguyên Tắc Interface Segregation (ISP)
**Ban đầu:** `HospitalManagement` là một "God Class" với quá nhiều trách nhiệm.

**Giải pháp:** Chia nhỏ thành các manager/service classes chuyên biệt.

#### Vấn Đề 3: Coupling Chặt Chẽ
**Ban đầu:** Các class trực tiếp thực hiện file operations và business logic.

**Giải pháp:** Giới thiệu Data Access Objects (DAO) cho các thao tác file.

#### Vấn Đề 4: Thiếu Abstraction Cho File Operations
**Ban đầu:** Không có interface cho data persistence.

**Giải pháp:** Tạo `IRepository` interfaces cho mỗi loại entity.

#### Vấn Đề 5: Account Gắn Với Person
**Ban đầu:** `Person "1" -- "1" Account` kết nối chặt identity với authentication.

**Giải pháp:** Giữ `Account` riêng biệt, liên kết qua username. Điều này cho phép role-based access mà không bị coupling chặt.

### Tóm Tắt Những Thay Đổi Chính

| Ban Đầu | Đã Tối Ưu | Lý Do |
|---------|-----------|-------|
| Methods trong entities (VD: `Patient.bookAppointment()`) | Methods chuyển sang Services | SRP: Entities chỉ chứa dữ liệu |
| `HospitalManagement` god class | Chia thành các Services chuyên biệt | SRP + ISP |
| Thao tác file trực tiếp | Repository pattern với `IRepository` interface | DIP + OCP |
| Không có phân tầng rõ ràng | 3-Tier với ranh giới rõ ràng | Rõ ràng về kiến trúc |
| Thiếu Facade | `HMSFacade` điều phối tất cả services | Facade Pattern |
| Không có Singleton | Repositories và Services dùng Singleton | Singleton Pattern |

---

## 2. Thiết Kế Kiến Trúc 3 Tầng

### Trách Nhiệm Các Tầng

| Tầng | Trách Nhiệm | Các Thành Phần |
|------|-------------|----------------|
| **Presentation** | Tương tác người dùng, input/output | ConsoleUI, DisplayHelper, InputValidator, HMSFacade |
| **Business Logic** | Quy tắc nghiệp vụ, validation, điều phối | AuthService, PatientService, DoctorService, AppointmentService, AdminService |
| **Data Access** | Lưu trữ dữ liệu, thao tác CRUD | Các Repository classes, FileHelper |

### Luồng Dữ Liệu

```
User Input → ConsoleUI → HMSFacade → Service → Repository → File
                                        ↓
User Output ← ConsoleUI ← HMSFacade ← Service ← Repository ← File
```

---

## 3. Áp Dụng Design Patterns

### 3.1 Singleton Pattern

Áp dụng cho:
- Tất cả Repository classes (đảm bảo single point of data access)
- Tất cả Service classes (đảm bảo single instance của business logic)
- HMSFacade (single entry point)

```cpp
// Ví dụ: Triển khai Singleton
class AccountRepository {
private:
    static AccountRepository* instance;
    AccountRepository() = default;  // Constructor private

public:
    // Xóa copy constructor và assignment operator
    AccountRepository(const AccountRepository&) = delete;
    AccountRepository& operator=(const AccountRepository&) = delete;

    static AccountRepository* getInstance() {
        if (instance == nullptr) {
            instance = new AccountRepository();
        }
        return instance;
    }
};
```

### 3.2 Facade Pattern

`HMSFacade` cung cấp interface đơn giản cho hệ thống phức tạp:

```cpp
// Code client (ConsoleUI) chỉ tương tác với Facade
class ConsoleUI {
    HMSFacade* facade;

    void bookAppointment() {
        // Gọi đơn giản - Facade xử lý mọi phức tạp
        facade->bookAppointment(doctorID, date, time, disease);
        // Thay vì:
        // appointmentService->validateSlot()
        // patientService->getPatient()
        // doctorService->getDoctor()
        // appointmentService->create()
        // etc.
    }
};
```

### 3.3 Repository Pattern

Trừu tượng hóa data access với interface nhất quán:

```cpp
template<typename T>
class IRepository {
public:
    virtual ~IRepository() = default;
    virtual std::vector<T> getAll() = 0;
    virtual std::optional<T> getById(const std::string& id) = 0;
    virtual bool add(const T& entity) = 0;
    virtual bool update(const T& entity) = 0;
    virtual bool remove(const std::string& id) = 0;
    virtual bool save() = 0;
    virtual bool load() = 0;
};
```

---

## 4. Cấu Trúc Thư Mục

```
Hospital_Management_Project/
│
├── .devcontainer/                  # Môi trường phát triển
│   ├── Dockerfile
│   ├── devcontainer.json
│   └── reinstall-cmake.sh
│
├── .github/                        # Cấu hình GitHub
│   └── dependabot.yml
│
├── .vscode/                        # Cài đặt VS Code
│   └── settings.json
│
├── docs/                           # Tài liệu
│   ├── ARCHITECTURE.md             # Tài liệu kiến trúc (tiếng Anh)
│   ├── ARCHITECTURE_vi.md          # Tài liệu kiến trúc (tiếng Việt)
│   ├── BUILD.md                    # Hướng dẫn build (tiếng Anh)
│   ├── BUILD_vi.md                 # Hướng dẫn build (tiếng Việt)
│   └── CLASS_DIAGRAM.md            # Class diagrams
│
├── data/                           # File dữ liệu
│   ├── Account.txt                 # Tài khoản người dùng
│   ├── Patient.txt                 # Hồ sơ bệnh nhân
│   ├── Doctor.txt                  # Hồ sơ bác sĩ
│   ├── Appointment.txt             # Hồ sơ cuộc hẹn
│   ├── backup/                     # Thư mục backup
│   └── sample/                     # Dữ liệu mẫu để test
│       ├── Account_sample.txt
│       ├── Patient_sample.txt
│       ├── Doctor_sample.txt
│       └── Appointment_sample.txt
│
├── include/                        # Header files
│   │
│   ├── model/                      # Entity classes (Cấu trúc dữ liệu)
│   │   ├── Person.h
│   │   ├── Patient.h
│   │   ├── Doctor.h
│   │   ├── Admin.h
│   │   ├── Account.h
│   │   ├── Appointment.h
│   │   └── Statistics.h
│   │
│   ├── dal/                        # Tầng Data Access
│   │   ├── IRepository.h           # Repository interface (template)
│   │   ├── AccountRepository.h
│   │   ├── PatientRepository.h
│   │   ├── DoctorRepository.h
│   │   ├── AppointmentRepository.h
│   │   └── FileHelper.h
│   │
│   ├── bll/                        # Tầng Business Logic
│   │   ├── AuthService.h
│   │   ├── PatientService.h
│   │   ├── DoctorService.h
│   │   ├── AppointmentService.h
│   │   └── AdminService.h
│   │
│   ├── ui/                         # Tầng Presentation
│   │   ├── HMSFacade.h             # Facade pattern
│   │   ├── ConsoleUI.h
│   │   ├── InputValidator.h
│   │   └── DisplayHelper.h
│   │
│   ├── common/                     # Utilities dùng chung
│   │   ├── Constants.h             # Các hằng số toàn cục
│   │   ├── Types.h                 # Type aliases và enums
│   │   └── Utils.h                 # Các hàm tiện ích
│   │
│   └── future/                     # Các tính năng tương lai
│       ├── Department.h
│       ├── Medicine.h
│       ├── Prescription.h
│       └── ReportGenerator.h
│
├── src/                            # Source files
│   │
│   ├── model/                      # Triển khai Entity
│   │   ├── Person.cpp
│   │   ├── Patient.cpp
│   │   ├── Doctor.cpp
│   │   ├── Admin.cpp
│   │   ├── Account.cpp
│   │   └── Appointment.cpp
│   │
│   ├── dal/                        # Triển khai Data Access
│   │   ├── AccountRepository.cpp
│   │   ├── PatientRepository.cpp
│   │   ├── DoctorRepository.cpp
│   │   ├── AppointmentRepository.cpp
│   │   └── FileHelper.cpp
│   │
│   ├── bll/                        # Triển khai Business Logic
│   │   ├── AuthService.cpp
│   │   ├── PatientService.cpp
│   │   ├── DoctorService.cpp
│   │   ├── AppointmentService.cpp
│   │   └── AdminService.cpp
│   │
│   ├── ui/                         # Triển khai Presentation
│   │   ├── HMSFacade.cpp
│   │   ├── ConsoleUI.cpp
│   │   ├── InputValidator.cpp
│   │   └── DisplayHelper.cpp
│   │
│   ├── common/                     # Triển khai Utility
│   │   └── Utils.cpp
│   │
│   └── main.cpp                    # Điểm khởi đầu ứng dụng
│
├── test/                           # File test
│   │
│   ├── model/                      # Unit tests cho Model
│   │   ├── PersonTest.cpp
│   │   ├── PatientTest.cpp
│   │   ├── DoctorTest.cpp
│   │   └── AppointmentTest.cpp
│   │
│   ├── dal/                        # Unit tests cho DAL
│   │   ├── AccountRepositoryTest.cpp
│   │   ├── PatientRepositoryTest.cpp
│   │   ├── DoctorRepositoryTest.cpp
│   │   ├── AppointmentRepositoryTest.cpp
│   │   └── FileHelperTest.cpp
│   │
│   ├── bll/                        # Unit tests cho BLL
│   │   ├── AuthServiceTest.cpp
│   │   ├── PatientServiceTest.cpp
│   │   ├── DoctorServiceTest.cpp
│   │   ├── AppointmentServiceTest.cpp
│   │   └── AdminServiceTest.cpp
│   │
│   ├── integration/                # Integration tests
│   │   ├── AuthFlowTest.cpp
│   │   ├── PatientFlowTest.cpp
│   │   ├── DoctorFlowTest.cpp
│   │   └── AppointmentFlowTest.cpp
│   │
│   ├── fixtures/                   # Dữ liệu test
│   │   ├── Account_test.txt
│   │   ├── Patient_test.txt
│   │   ├── Doctor_test.txt
│   │   └── Appointment_test.txt
│   │
│   └── test_main.cpp               # Test runner entry point
│
├── build/                          # Build output (git ignored)
│
├── CMakeLists.txt                  # Cấu hình build
├── README.md                       # Project README
└── .gitignore                      # Git ignore rules
```

---

## 5. Trách Nhiệm Các File

### 5.1 Tầng Model (`include/model/`, `src/model/`)

| File | Trách Nhiệm |
|------|-------------|
| `Person.h/cpp` | Abstract base class với các thuộc tính chung của con người |
| `Patient.h/cpp` | Entity bệnh nhân với lịch sử y tế |
| `Doctor.h/cpp` | Entity bác sĩ với chuyên khoa và lịch làm việc |
| `Admin.h/cpp` | Entity quản trị viên |
| `Account.h/cpp` | Tài khoản người dùng với dữ liệu xác thực |
| `Appointment.h/cpp` | Entity cuộc hẹn với chi tiết booking |
| `Statistics.h` | Cấu trúc dữ liệu thống kê (không cần .cpp) |

### 5.2 Tầng Data Access (`include/dal/`, `src/dal/`)

| File | Trách Nhiệm |
|------|-------------|
| `IRepository.h` | Generic repository interface template |
| `AccountRepository.h/cpp` | Thao tác CRUD Account + file persistence |
| `PatientRepository.h/cpp` | Thao tác CRUD Patient + file persistence |
| `DoctorRepository.h/cpp` | Thao tác CRUD Doctor + file persistence |
| `AppointmentRepository.h/cpp` | Thao tác CRUD Appointment + queries theo patient/doctor |
| `FileHelper.h/cpp` | Các tiện ích file I/O cấp thấp |

### 5.3 Tầng Business Logic (`include/bll/`, `src/bll/`)

| File | Trách Nhiệm |
|------|-------------|
| `AuthService.h/cpp` | Xác thực, login/logout, quản lý mật khẩu |
| `PatientService.h/cpp` | Business logic bệnh nhân, tìm kiếm, tính bill |
| `DoctorService.h/cpp` | Business logic bác sĩ, quản lý lịch làm việc |
| `AppointmentService.h/cpp` | Logic booking, kiểm tra slot trống, quản lý trạng thái |
| `AdminService.h/cpp` | Thao tác admin, tạo thống kê |

### 5.4 Tầng Presentation (`include/ui/`, `src/ui/`)

| File | Trách Nhiệm |
|------|-------------|
| `HMSFacade.h/cpp` | Facade pattern - điểm truy cập duy nhất cho UI |
| `ConsoleUI.h/cpp` | Hệ thống menu console, vòng lặp tương tác người dùng |
| `InputValidator.h/cpp` | Các tiện ích validation input |
| `DisplayHelper.h/cpp` | Định dạng output, hiển thị bảng |

### 5.5 Common Utilities (`include/common/`, `src/common/`)

| File | Trách Nhiệm |
|------|-------------|
| `Constants.h` | File paths, tùy chọn menu, quy tắc validation |
| `Types.h` | Enums (Role, AppointmentStatus), type aliases |
| `Utils.h/cpp` | Date utilities, string helpers, tạo ID |

---

## 6. Định Dạng File Dữ Liệu

### 6.1 Account.txt
```
# Format: username|passwordHash|role|isActive|createdDate
# Role: patient, doctor, admin
admin|5f4dcc3b5aa765d61d8327deb882cf99|admin|1|2024-01-01
doctor001|5f4dcc3b5aa765d61d8327deb882cf99|doctor|1|2024-01-15
patient001|5f4dcc3b5aa765d61d8327deb882cf99|patient|1|2024-02-01
```

### 6.2 Patient.txt
```
# Format: patientID|username|name|phone|gender|dateOfBirth|address|medicalHistory
P001|patient001|Nguyen Van A|0901234567|Male|1990-05-15|123 Le Loi, HCM|Tăng huyết áp
P002|patient002|Tran Thi B|0912345678|Female|1985-08-20|456 Nguyen Hue, HCM|Tiểu đường
```

### 6.3 Doctor.txt
```
# Format: doctorID|username|name|phone|gender|dateOfBirth|specialization|schedule|consultationFee
D001|doctor001|BS. Phạm Văn C|0923456789|Male|1975-03-10|Tim mạch|Mon-Fri 08:00-17:00|500000
D002|doctor002|BS. Lê Thị D|0934567890|Female|1980-11-25|Nhi khoa|Mon-Sat 09:00-16:00|400000
```

### 6.4 Appointment.txt
```
# Format: appointmentID|patientUsername|doctorID|date|time|disease|price|isPaid|status|notes
A001|patient001|D001|2024-03-15|09:00|Đau ngực|500000|1|completed|Khám định kỳ
A002|patient002|D002|2024-03-16|10:30|Sốt|400000|0|scheduled|Tái khám
```

### 6.5 Giá Trị Status
- `scheduled` - Cuộc hẹn đã đặt, chưa diễn ra
- `completed` - Cuộc hẹn đã hoàn thành
- `cancelled` - Cuộc hẹn đã hủy bởi bệnh nhân hoặc bác sĩ
- `no_show` - Bệnh nhân không đến

---

## 7. Hệ Thống Build

Xem `CMakeLists.txt` để biết cấu hình build đầy đủ.

### Lệnh Build

```bash
# Tạo và vào thư mục build
mkdir -p build && cd build

# Cấu hình với CMake
cmake ..

# Build ứng dụng chính
make HospitalApp

# Build tests
make HospitalTests

# Build tất cả
make all

# Chạy ứng dụng
./HospitalApp

# Chạy tests
./HospitalTests

# Chạy tests với output chi tiết
./HospitalTests --gtest_output=xml:test_results.xml

# Clean build
make clean
```

### CMake Targets

| Target | Mô Tả |
|--------|-------|
| `HospitalApp` | Executable ứng dụng chính |
| `HospitalTests` | Executable test với Google Test |
| `all` | Build tất cả targets |
| `clean` | Xóa các build artifacts |

---

## Phụ Lục A: Áp Dụng Các Nguyên Tắc SOLID

| Nguyên Tắc | Áp Dụng |
|------------|---------|
| **S**ingle Responsibility | Mỗi class có một công việc: Models chứa dữ liệu, Repositories xử lý persistence, Services chứa business logic, UI xử lý hiển thị |
| **O**pen/Closed | Repository interface cho phép thêm data sources mới mà không cần sửa code hiện có |
| **L**iskov Substitution | Patient, Doctor, Admin có thể thay thế Person trong mọi ngữ cảnh |
| **I**nterface Segregation | IRepository cung cấp interface tập trung; Services có methods cụ thể |
| **D**ependency Inversion | Services phụ thuộc vào Repository interfaces, không phải concrete implementations |

## Phụ Lục B: Tích Hợp Các Tính Năng Nâng cao (nếu có đủ thời gian)

Khi thêm tính năng mới:

1. **Department**: Thêm `Department.h/cpp` vào model/, `DepartmentRepository` vào dal/, `DepartmentService` vào bll/
2. **Medicine**: Thêm `Medicine.h/cpp`, `Prescription.h/cpp` vào model/, các repositories và services tương ứng
3. **ReportGenerator**: Thêm vào tầng bll/, sử dụng các services hiện có để tổng hợp dữ liệu

Kiến trúc hỗ trợ các bổ sung này mà không cần sửa code hiện có (Nguyên tắc Open/Closed).

---

## Ghi Chú Quan Trọng

### Các Điểm Cần Lưu Ý Khi Phát Triển

1. **Luôn bắt đầu từ tầng Model**: Triển khai các entity classes trước
2. **Sau đó đến tầng DAL**: Tạo repositories để lưu/load dữ liệu
3. **Tiếp theo là tầng BLL**: Triển khai business logic
4. **Cuối cùng là tầng UI**: Kết nối với người dùng

### Best Practices

- **Singleton**: Chỉ một instance cho mỗi Repository và Service
- **Const Correctness**: Sử dụng `const` cho methods không thay đổi state
- **Error Handling**: Sử dụng `std::optional` cho các giá trị có thể null
- **Memory Safety**: Sử dụng smart pointers thay vì raw pointers
- **Testing**: Viết tests cho mỗi component trước khi integrate

### Quy Tắc Đặt Tên

- **Classes**: PascalCase (`PatientService`, `AccountRepository`)
- **Methods**: camelCase (`getPatientByID`, `bookAppointment`)
- **Member variables**: m_ prefix (`m_patients`, `m_filePath`)
- **Static members**: s_ prefix (`s_instance`, `s_mutex`)
- **Constants**: UPPER_SNAKE_CASE (`PATIENT_ID_PREFIX`, `MAX_USERNAME_LENGTH`)

---

**Tài liệu này là hướng dẫn thiết kế cho Hospital Management System. Hãy tham khảo BUILD_vi.md để biết hướng dẫn build và chạy project.**