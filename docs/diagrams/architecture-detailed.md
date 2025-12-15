# Sơ Đồ Kiến Trúc Chi Tiết

> **Mục đích:** Hiểu đầy đủ relationships, dependencies, và class structure
> **Đối tượng:** Developers implementing code, architecture review

---

## 🔍 Sơ Đồ Dependencies Chi Tiết

```mermaid
graph TB
    subgraph UI["PRESENTATION LAYER"]
        ConsoleUI[ConsoleUI]
        Facade[HMSFacade]
        ConsoleUI --> Facade
    end

    subgraph BLL["BUSINESS LOGIC LAYER"]
        direction LR

        subgraph AuthFlow["🔐 Auth Flow"]
            AuthService[AuthService]
            AccountRepo1[AccountRepository]
            AuthService --> AccountRepo1
        end

        subgraph PatientFlow["👤 Patient Flow"]
            PatientService[PatientService]
            PatientRepo1[PatientRepository]
            AppointmentRepo1[AppointmentRepository]
            PatientService --> PatientRepo1
            PatientService --> AppointmentRepo1
        end

        subgraph DoctorFlow["👨‍⚕️ Doctor Flow"]
            DoctorService[DoctorService]
            DoctorRepo1[DoctorRepository]
            AppointmentRepo2[AppointmentRepository]
            DoctorService --> DoctorRepo1
            DoctorService --> AppointmentRepo2
        end

        subgraph AppointmentFlow["📅 Appointment Flow"]
            AppointmentService[AppointmentService]
            AppointmentRepo3[AppointmentRepository]
            PatientRepo2[PatientRepository]
            DoctorRepo2[DoctorRepository]
            AppointmentService --> AppointmentRepo3
            AppointmentService --> PatientRepo2
            AppointmentService --> DoctorRepo2
        end

        subgraph AdminFlow["⚡ Admin Flow"]
            AdminService[AdminService]
            AdminService -.aggregates.-> PatientService
            AdminService -.aggregates.-> DoctorService
            AdminService -.aggregates.-> AppointmentService
        end
    end

    subgraph DAL["DATA ACCESS LAYER"]
        AccountRepo[AccountRepository]
        PatientRepo[PatientRepository]
        DoctorRepo[DoctorRepository]
        AppointmentRepo[AppointmentRepository]
        FileHelper[FileHelper]

        AccountRepo --> FileHelper
        PatientRepo --> FileHelper
        DoctorRepo --> FileHelper
        AppointmentRepo --> FileHelper
    end

    subgraph Files["DATA FILES"]
        AccountTxt[(Account.txt)]
        PatientTxt[(Patient.txt)]
        DoctorTxt[(Doctor.txt)]
        AppointmentTxt[(Appointment.txt)]
    end

    Facade --> AuthService
    Facade --> PatientService
    Facade --> DoctorService
    Facade --> AppointmentService
    Facade --> AdminService

    AccountRepo1 -.same instance.-> AccountRepo
    PatientRepo1 -.same instance.-> PatientRepo
    PatientRepo2 -.same instance.-> PatientRepo
    DoctorRepo1 -.same instance.-> DoctorRepo
    DoctorRepo2 -.same instance.-> DoctorRepo
    AppointmentRepo1 -.same instance.-> AppointmentRepo
    AppointmentRepo2 -.same instance.-> AppointmentRepo
    AppointmentRepo3 -.same instance.-> AppointmentRepo

    FileHelper --> AccountTxt
    FileHelper --> PatientTxt
    FileHelper --> DoctorTxt
    FileHelper --> AppointmentTxt

    style UI fill:#e3f2fd
    style BLL fill:#fff8e1
    style DAL fill:#f3e5f5
    style Files fill:#e8f5e9
```

---

## 📋 Bảng Dependencies Chi Tiết

### Services → Repositories

| Service | Uses Repositories | Lý Do |
|---------|------------------|-------|
| **AuthService** | • AccountRepository | Login/logout<br>Quản lý account |
| **PatientService** | • PatientRepository<br>• AppointmentRepository | CRUD patients<br>Xem lịch hẹn của patient |
| **DoctorService** | • DoctorRepository<br>• AppointmentRepository | CRUD doctors<br>Xem lịch làm việc của doctor |
| **AppointmentService** | • AppointmentRepository<br>• PatientRepository<br>• DoctorRepository | CRUD appointments<br>Validate patient exists<br>Get doctor's consultation fee |
| **AdminService** | • PatientService<br>• DoctorService<br>• AppointmentService | Tổng hợp thống kê<br>Không trực tiếp dùng Repos |

### Repositories → Files

| Repository | File | Operations |
|-----------|------|-----------|
| AccountRepository | Account.txt | Read/Write accounts |
| PatientRepository | Patient.txt | Read/Write patients |
| DoctorRepository | Doctor.txt | Read/Write doctors |
| AppointmentRepository | Appointment.txt | Read/Write appointments |

**Tất cả repositories đều sử dụng FileHelper cho I/O operations.**

---

## 🏗️ Class Diagram Đầy Đủ

### Model Layer (Entities)

```mermaid
classDiagram
    class Person {
        <<abstract>>
        #string name
        #string phone
        #string gender
        #string dateOfBirth
        +Person(name, phone, gender, dob)
        +getName() string
        +getPhone() string
        +displayInfo()* void
    }

    class Patient {
        -string patientID
        -string username
        -string address
        -string medicalHistory
        +Patient(...)
        +getPatientID() string
        +getUsername() string
        +displayInfo() void
    }

    class Doctor {
        -string doctorID
        -string username
        -string specialization
        -string schedule
        -double consultationFee
        +Doctor(...)
        +getDoctorID() string
        +getConsultationFee() double
        +displayInfo() void
    }

    class Admin {
        -string adminID
        -string username
        +Admin(...)
        +getAdminID() string
        +displayInfo() void
    }

    class Account {
        -string username
        -string passwordHash
        -string role
        -bool isActive
        -string createdDate
        +Account(...)
        +getUsername() string
        +getPasswordHash() string
        +getRole() string
    }

    class Appointment {
        -string appointmentID
        -string patientUsername
        -string doctorID
        -string date
        -string time
        -string disease
        -double price
        -bool isPaid
        -string status
        +Appointment(...)
        +getAppointmentID() string
        +getPatientUsername() string
        +getDoctorID() string
        +setStatus(string) void
        +setPaid(bool) void
    }

    Person <|-- Patient
    Person <|-- Doctor
    Person <|-- Admin
```

### Data Access Layer

```mermaid
classDiagram
    class IRepository~T~ {
        <<interface>>
        +getAll() vector~T~
        +getById(id) optional~T~
        +add(T) bool
        +update(T) bool
        +remove(id) bool
        +save() bool
        +load() bool
    }

    class AccountRepository {
        <<Singleton>>
        -static AccountRepository* instance
        -vector~Account~ accounts
        -string filePath
        -AccountRepository()
        +getInstance() AccountRepository*
        +getAll() vector~Account~
        +getByUsername(username) optional~Account~
        +add(Account) bool
        +update(Account) bool
        +save() bool
        +load() bool
    }

    class PatientRepository {
        <<Singleton>>
        -static PatientRepository* instance
        -vector~Patient~ patients
        -string filePath
        -PatientRepository()
        +getInstance() PatientRepository*
        +getAll() vector~Patient~
        +getByUsername(username) optional~Patient~
        +add(Patient) bool
        +update(Patient) bool
        +save() bool
        +load() bool
    }

    class DoctorRepository {
        <<Singleton>>
        -static DoctorRepository* instance
        -vector~Doctor~ doctors
        -string filePath
        -DoctorRepository()
        +getInstance() DoctorRepository*
        +getBySpecialization(spec) vector~Doctor~
        +save() bool
        +load() bool
    }

    class AppointmentRepository {
        <<Singleton>>
        -static AppointmentRepository* instance
        -vector~Appointment~ appointments
        -string filePath
        -AppointmentRepository()
        +getInstance() AppointmentRepository*
        +getByPatient(username) vector~Appointment~
        +getByDoctor(doctorID) vector~Appointment~
        +getByDate(date) vector~Appointment~
        +save() bool
        +load() bool
    }

    class FileHelper {
        <<utility>>
        +readLines(filePath) vector~string~
        +writeLines(filePath, lines) bool
        +appendLine(filePath, line) bool
        +fileExists(filePath) bool
        +createBackup(filePath) bool
    }

    IRepository <|.. AccountRepository
    IRepository <|.. PatientRepository
    IRepository <|.. DoctorRepository
    IRepository <|.. AppointmentRepository

    AccountRepository ..> FileHelper
    PatientRepository ..> FileHelper
    DoctorRepository ..> FileHelper
    AppointmentRepository ..> FileHelper
```

### Business Logic Layer

```mermaid
classDiagram
    class AuthService {
        <<Singleton>>
        -static AuthService* instance
        -AccountRepository* accountRepo
        -string currentUsername
        -string currentRole
        -AuthService()
        +getInstance() AuthService*
        +login(username, password) bool
        +logout() void
        +registerAccount(...) bool
        +changePassword(...) bool
        +getCurrentUsername() string
        +isLoggedIn() bool
    }

    class PatientService {
        <<Singleton>>
        -static PatientService* instance
        -PatientRepository* patientRepo
        -AppointmentRepository* appointmentRepo
        -PatientService()
        +getInstance() PatientService*
        +createPatient(Patient) bool
        +updatePatient(Patient) bool
        +deletePatient(patientID) bool
        +getPatientByID(patientID) optional~Patient~
        +getPatientAppointmentHistory(username) vector~Appointment~
        +calculateTotalBill(username) double
    }

    class DoctorService {
        <<Singleton>>
        -static DoctorService* instance
        -DoctorRepository* doctorRepo
        -AppointmentRepository* appointmentRepo
        -DoctorService()
        +getInstance() DoctorService*
        +createDoctor(Doctor) bool
        +updateDoctor(Doctor) bool
        +getDoctorsBySpecialization(spec) vector~Doctor~
        +getDoctorSchedule(doctorID, date) vector~Appointment~
    }

    class AppointmentService {
        <<Singleton>>
        -static AppointmentService* instance
        -AppointmentRepository* appointmentRepo
        -PatientRepository* patientRepo
        -DoctorRepository* doctorRepo
        -AppointmentService()
        +getInstance() AppointmentService*
        +bookAppointment(...) bool
        +cancelAppointment(appointmentID) bool
        +markAsCompleted(appointmentID) bool
        +isSlotAvailable(doctorID, date, time) bool
    }

    class AdminService {
        <<Singleton>>
        -static AdminService* instance
        -PatientService* patientService
        -DoctorService* doctorService
        -AppointmentService* appointmentService
        -AdminService()
        +getInstance() AdminService*
        +getStatistics() Statistics
        +getTotalRevenue() double
    }

    class Statistics {
        +int totalPatients
        +int totalDoctors
        +int totalAppointments
        +double totalRevenue
    }

    AuthService --> AccountRepository
    PatientService --> PatientRepository
    PatientService --> AppointmentRepository
    DoctorService --> DoctorRepository
    DoctorService --> AppointmentRepository
    AppointmentService --> AppointmentRepository
    AppointmentService --> PatientRepository
    AppointmentService --> DoctorRepository
    AdminService --> PatientService
    AdminService --> DoctorService
    AdminService --> AppointmentService
    AdminService ..> Statistics
```

### Presentation Layer

```mermaid
classDiagram
    class HMSFacade {
        <<Singleton, Facade>>
        -static HMSFacade* instance
        -AuthService* authService
        -PatientService* patientService
        -DoctorService* doctorService
        -AppointmentService* appointmentService
        -AdminService* adminService
        -HMSFacade()
        +getInstance() HMSFacade*
        +login(username, password) bool
        +logout() void
        +bookAppointment(...) bool
        +getAllDoctors() vector~Doctor~
        +getStatistics() Statistics
    }

    class ConsoleUI {
        -HMSFacade* facade
        -bool isRunning
        +ConsoleUI()
        +run() void
        -showMainMenu() void
        -showLoginScreen() void
        -showPatientMenu() void
        -showDoctorMenu() void
        -showAdminMenu() void
    }

    class InputValidator {
        <<utility>>
        +validateUsername(username) bool
        +validatePassword(password) bool
        +validatePhone(phone) bool
        +validateDate(date) bool
    }

    class DisplayHelper {
        <<utility>>
        +clearScreen() void
        +printHeader(title) void
        +printTable(headers, rows) void
        +printPatientInfo(Patient) void
        +printDoctorInfo(Doctor) void
    }

    ConsoleUI --> HMSFacade
    ConsoleUI ..> InputValidator
    ConsoleUI ..> DisplayHelper
    HMSFacade --> AuthService
    HMSFacade --> PatientService
    HMSFacade --> DoctorService
    HMSFacade --> AppointmentService
    HMSFacade --> AdminService
```

---

## 🔄 Use Case: Book Appointment

### Flow Chi Tiết

```
1. User input → ConsoleUI.showPatientMenu()
   ↓
2. ConsoleUI → facade->bookAppointment(doctorID, date, time, disease)
   ↓
3. Facade → appointmentService->bookAppointment(...)
   ↓
4. AppointmentService:
   4.1. Validate patient → patientRepo->getByUsername(currentUsername)
   4.2. Validate doctor → doctorRepo->getById(doctorID)
   4.3. Get fee → doctor.getConsultationFee()
   4.4. Check slot → isSlotAvailable(doctorID, date, time)
   4.5. Create appointment → appointmentRepo->add(appointment)
   4.6. Save to file → appointmentRepo->save()
   ↓
5. Facade → return success/failure
   ↓
6. ConsoleUI → DisplayHelper.printSuccess("Đặt lịch thành công!")
```

---

## 📊 Method Responsibilities

### AppointmentService Methods

| Method | Uses Repositories | Purpose |
|--------|------------------|---------|
| `bookAppointment()` | PatientRepo, DoctorRepo, AppointmentRepo | Validate và tạo appointment mới |
| `isSlotAvailable()` | AppointmentRepo | Check xem time slot còn trống không |
| `cancelAppointment()` | AppointmentRepo | Cập nhật status thành "cancelled" |
| `markAsCompleted()` | AppointmentRepo | Cập nhật status thành "completed" |
| `markAsPaid()` | AppointmentRepo | Cập nhật isPaid = true |

### PatientService Methods

| Method | Uses Repositories | Purpose |
|--------|------------------|---------|
| `createPatient()` | PatientRepo | Thêm patient mới |
| `getPatientAppointmentHistory()` | AppointmentRepo | Lấy tất cả appointments của patient |
| `getPatientUpcomingAppointments()` | AppointmentRepo | Lấy appointments chưa hoàn thành |
| `calculateTotalBill()` | AppointmentRepo | Tính tổng tiền các appointments |

### DoctorService Methods

| Method | Uses Repositories | Purpose |
|--------|------------------|---------|
| `createDoctor()` | DoctorRepo | Thêm doctor mới |
| `getDoctorSchedule()` | AppointmentRepo | Lấy lịch làm việc của doctor |
| `getDoctorsBySpecialization()` | DoctorRepo | Tìm doctors theo chuyên khoa |

---

## 🎯 Singleton Pattern Implementation

### Tại Sao Dùng Singleton?

✅ **Repositories:** Đảm bảo chỉ 1 instance truy cập file, tránh conflict khi ghi
✅ **Services:** Đảm bảo state nhất quán (VD: AuthService.currentUsername)
✅ **HMSFacade:** Điểm truy cập duy nhất cho UI

### Template Code

```cpp
class AccountRepository {
private:
    static AccountRepository* instance;
    AccountRepository() = default;  // Private constructor

public:
    // Delete copy & assignment
    AccountRepository(const AccountRepository&) = delete;
    AccountRepository& operator=(const AccountRepository&) = delete;

    static AccountRepository* getInstance() {
        if (instance == nullptr) {
            instance = new AccountRepository();
        }
        return instance;
    }
};

// Khởi tạo static member
AccountRepository* AccountRepository::instance = nullptr;
```

---

## 📚 Đọc Thêm

- **Overview đơn giản:** Xem [architecture-overview.md](architecture-overview.md)
- **Full documentation:** Xem [../ARCHITECTURE_vi.md](../ARCHITECTURE_vi.md)
- **Build guide:** Xem [../BUILD_vi.md](../BUILD_vi.md)

---

**💡 TIP:** Dùng sơ đồ này khi implement code để biết service nào cần repository nào!