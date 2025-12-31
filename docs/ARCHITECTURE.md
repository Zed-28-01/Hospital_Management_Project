# Hospital Management System - Architecture Design Document

## Table of Contents
1. [Class Diagram Review & Optimization](#1-class-diagram-review--optimization)
2. [3-Tier Architecture Design](#2-3-tier-architecture-design)
3. [Design Patterns Implementation](#3-design-patterns-implementation)
4. [Directory Structure](#4-directory-structure)
5. [File Responsibilities](#5-file-responsibilities)
6. [Data File Formats](#6-data-file-formats)
7. [Build System](#7-build-system)

---

## 1. Class Diagram Review & Optimization

### Issues Found in Original Design POC

#### Problem 1: Violation of Single Responsibility Principle (SRP)
**Original:** `Patient`, `Doctor`, `Admin` classes contain both data AND business logic methods.
- `Patient.bookAppointment()`, `Doctor.addPatient()`, `Admin.addDoctor()`

**Solution:** Separate data models (entities) from business logic (services).

#### Problem 2: Violation of Interface Segregation Principle (ISP)
**Original:** `HospitalManagement` is a "God Class" with too many responsibilities.

**Solution:** Split into specialized manager/service classes.

#### Problem 3: Tight Coupling
**Original:** Classes directly perform file operations and business logic.

**Solution:** Introduce Data Access Objects (DAO) for file operations.

#### Problem 4: Missing Abstraction for File Operations
**Original:** No interface for data persistence.

**Solution:** Create `IRepository` interfaces for each entity type.

#### Problem 5: Account Embedded in Person
**Original:** `Person "1" -- "1" Account` couples identity with authentication.

**Solution:** Keep `Account` separate, linked by username. This allows role-based access without tight coupling.


### Key Changes Summary

| Original | Optimized | Reason |
|----------|-----------|--------|
| Methods in entities (e.g., `Patient.bookAppointment()`) | Methods moved to Services | SRP: Entities only hold data |
| `HospitalManagement` god class | Split into specialized Services | SRP + ISP |
| Direct file operations | Repository pattern with `IRepository` interface | DIP + OCP |
| No clear layer separation | 3-Tier with clear boundaries | Architecture clarity |
| Missing Facade | `HMSFacade` coordinates all services | Facade Pattern |
| No Singleton | Repositories and Services use Singleton | Singleton Pattern |

---

## 2. 3-Tier Architecture Design

### Architecture Diagrams

For visual representation of the architecture:
- **[Overview Diagram](diagrams/architecture-overview.md)** - Simple, high-level view (recommended for README.md)
- **[Detailed Diagram](diagrams/architecture-detailed.md)** - Complete dependencies and class diagrams


### Layer Responsibilities

| Layer | Responsibility | Components |
|-------|---------------|------------|
| **Presentation** | User interaction, input/output | ConsoleUI, DisplayHelper, InputValidator, HMSFacade |
| **Business Logic** | Business rules, validation, orchestration | AuthService, PatientService, DoctorService, AppointmentService, AdminService, **MedicineService**, **DepartmentService**, **PrescriptionService**, **ReportGenerator** |
| **Data Access** | Data persistence, CRUD operations | AccountRepository, PatientRepository, DoctorRepository, AppointmentRepository, **MedicineRepository**, **DepartmentRepository**, **PrescriptionRepository**, FileHelper |

### Data Flow

```
User Input → ConsoleUI → HMSFacade → Service → Repository → File
                                        ↓
User Output ← ConsoleUI ← HMSFacade ← Service ← Repository ← File
```

---

## 3. Design Patterns Implementation

### 3.1 Singleton Pattern

Applied to:
- All Repository classes (ensure single point of data access)
- All Service classes (ensure single instance of business logic)
- HMSFacade (single entry point)

```cpp
// Example: Singleton implementation
class AccountRepository {
private:
    static AccountRepository* instance;
    AccountRepository() = default;  // Private constructor

public:
    // Delete copy constructor and assignment operator
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

`HMSFacade` provides a simplified interface to the complex subsystem:

```cpp
// Client code (ConsoleUI) only interacts with Facade
class ConsoleUI {
    HMSFacade* facade;

    void bookAppointment() {
        // Simple call - Facade handles all complexity
        facade->bookAppointment(doctorID, date, time, disease);
        // Instead of:
        // appointmentService->validateSlot()
        // patientService->getPatient()
        // doctorService->getDoctor()
        // appointmentService->create()
        // etc.
    }
};
```

### 3.3 Repository Pattern

Abstracts data access with consistent interface:

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

## 4. Directory Structure

```
Hospital_Management_Project/
│
├── .devcontainer/                  # Development environment (existing)
│   ├── Dockerfile
│   ├── devcontainer.json
│   └── reinstall-cmake.sh
│
├── .github/                        # GitHub configuration (existing)
│   └── dependabot.yml
│
├── .vscode/                        # VS Code settings (existing)
│   └── settings.json
│
├── docs/                           # Documentation (NEW)
│   ├── ARCHITECTURE.md             # This document
│   ├── CLASS_DIAGRAM.md            # Mermaid class diagrams
│   └── API.md                      # Internal API documentation
│
├── data/                           # Data files
│   ├── Account.txt                 # User accounts
│   ├── Patient.txt                 # Patient records
│   ├── Doctor.txt                  # Doctor records
│   ├── Appointment.txt             # Appointment records
│   ├── Department.txt              # Department records
│   ├── Medicine.txt                # Medicine/inventory records
│   ├── Prescription.txt            # Prescription records
│   ├── backup/                     # Backup directory
│   ├── reports/                    # Generated reports directory
│   └── sample/                     # Sample data for testing
│       ├── Account_sample.txt
│       ├── Patient_sample.txt
│       ├── Doctor_sample.txt
│       ├── Appointment_sample.txt
│       ├── Department_sample.txt
│       ├── Medicine_sample.txt
│       └── Prescription_sample.txt
│
├── include/                        # Header files (RESTRUCTURED)
│   │
│   ├── model/                      # Entity classes (Data structures)
│   │   ├── Person.h
│   │   ├── Patient.h
│   │   ├── Doctor.h
│   │   ├── Admin.h
│   │   ├── Account.h
│   │   ├── Appointment.h
│   │   ├── Statistics.h
│   │   ├── Department.h            # Department entity
│   │   ├── Medicine.h              # Medicine/inventory entity
│   │   └── Prescription.h          # Prescription entity
│   │
│   ├── dal/                        # Data Access Layer
│   │   ├── IRepository.h           # Repository interface (template)
│   │   ├── AccountRepository.h
│   │   ├── PatientRepository.h
│   │   ├── DoctorRepository.h
│   │   ├── AppointmentRepository.h
│   │   ├── DepartmentRepository.h  # Department data access
│   │   ├── MedicineRepository.h    # Medicine data access
│   │   ├── PrescriptionRepository.h # Prescription data access
│   │   └── FileHelper.h
│   │
│   ├── bll/                        # Business Logic Layer
│   │   ├── AuthService.h
│   │   ├── PatientService.h
│   │   ├── DoctorService.h
│   │   ├── AppointmentService.h
│   │   ├── AdminService.h
│   │   ├── DepartmentService.h     # Department management
│   │   ├── MedicineService.h       # Medicine/inventory management
│   │   └── PrescriptionService.h   # Prescription management
│   │
│   ├── ui/                         # Presentation Layer
│   │   ├── HMSFacade.h             # Facade pattern
│   │   ├── ConsoleUI.h
│   │   ├── InputValidator.h
│   │   └── DisplayHelper.h
│   │
│   ├── common/                     # Shared utilities
│   │   ├── Constants.h             # Global constants
│   │   ├── Types.h                 # Type aliases and enums
│   │   └── Utils.h                 # Utility functions
│   │
│   └── advance/                     # Advanced features
│       └── ReportGenerator.h       # Report generation utilities
│
├── src/                            # Source files (RESTRUCTURED)
│   │
│   ├── model/                      # Entity implementations
│   │   ├── Person.cpp
│   │   ├── Patient.cpp
│   │   ├── Doctor.cpp
│   │   ├── Admin.cpp
│   │   ├── Account.cpp
│   │   ├── Appointment.cpp
│   │   ├── Department.cpp
│   │   ├── Medicine.cpp
│   │   └── Prescription.cpp
│   │
│   ├── dal/                        # Data Access implementations
│   │   ├── AccountRepository.cpp
│   │   ├── PatientRepository.cpp
│   │   ├── DoctorRepository.cpp
│   │   ├── AppointmentRepository.cpp
│   │   ├── DepartmentRepository.cpp
│   │   ├── MedicineRepository.cpp
│   │   ├── PrescriptionRepository.cpp
│   │   └── FileHelper.cpp
│   │
│   ├── bll/                        # Business Logic implementations
│   │   ├── AuthService.cpp
│   │   ├── PatientService.cpp
│   │   ├── DoctorService.cpp
│   │   ├── AppointmentService.cpp
│   │   ├── AdminService.cpp
│   │   ├── DepartmentService.cpp
│   │   ├── MedicineService.cpp
│   │   ├── PrescriptionService.cpp
│   │   └── ReportGenerator.cpp
│   │
│   ├── ui/                         # Presentation implementations
│   │   ├── HMSFacade.cpp
│   │   ├── ConsoleUI.cpp
│   │   ├── InputValidator.cpp
│   │   └── DisplayHelper.cpp
│   │
│   ├── common/                     # Utility implementations
│   │   └── Utils.cpp
│   │
│   └── main.cpp                    # Application entry point
│
├── test/                           # Test files (RESTRUCTURED)
│   │
│   ├── model/                      # Model unit tests
│   │   ├── PersonTest.cpp
│   │   ├── PatientTest.cpp
│   │   ├── DoctorTest.cpp
│   │   └── AppointmentTest.cpp
│   │
│   ├── dal/                        # DAL unit tests
│   │   ├── AccountRepositoryTest.cpp
│   │   ├── PatientRepositoryTest.cpp
│   │   ├── DoctorRepositoryTest.cpp
│   │   ├── AppointmentRepositoryTest.cpp
│   │   └── FileHelperTest.cpp
│   │
│   ├── bll/                        # BLL unit tests
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
│   ├── fixtures/                   # Test data
│   │   ├── Account_test.txt
│   │   ├── Patient_test.txt
│   │   ├── Doctor_test.txt
│   │   └── Appointment_test.txt
│   │
│   └── test_main.cpp               # Test runner entry point
│
├── build/                          # Build output (git ignored)
│
├── CMakeLists.txt                  # Build configuration (UPDATED)
├── README.md                       # Project README (existing)
└── .gitignore                      # Git ignore rules (existing)
```

---

## 5. File Responsibilities

### 5.1 Model Layer (`include/model/`, `src/model/`)

| File | Responsibility |
|------|----------------|
| `Person.h/cpp` | Abstract base class with common person attributes |
| `Patient.h/cpp` | Patient entity with medical history |
| `Doctor.h/cpp` | Doctor entity with specialization and schedule |
| `Admin.h/cpp` | Admin entity |
| `Account.h/cpp` | User account with authentication data |
| `Appointment.h/cpp` | Appointment entity with booking details |
| `Statistics.h` | Statistics data structure (no .cpp needed) |
| `Department.h/cpp` | Department entity with doctor assignments |
| `Medicine.h/cpp` | Medicine entity with stock management |
| `Prescription.h/cpp` | Prescription entity with items and dispensing |

### 5.2 Data Access Layer (`include/dal/`, `src/dal/`)

| File | Responsibility |
|------|----------------|
| `IRepository.h` | Generic repository interface template |
| `AccountRepository.h/cpp` | Account CRUD operations + file persistence |
| `PatientRepository.h/cpp` | Patient CRUD operations + file persistence |
| `DoctorRepository.h/cpp` | Doctor CRUD operations + file persistence |
| `AppointmentRepository.h/cpp` | Appointment CRUD + queries by patient/doctor |
| `DepartmentRepository.h/cpp` | Department CRUD + doctor assignment queries |
| `MedicineRepository.h/cpp` | Medicine CRUD + stock queries, expiry alerts |
| `PrescriptionRepository.h/cpp` | Prescription CRUD + patient/doctor queries |
| `FileHelper.h/cpp` | Low-level file I/O utilities |

### 5.3 Business Logic Layer (`include/bll/`, `src/bll/`)

| File | Responsibility |
|------|----------------|
| `AuthService.h/cpp` | Authentication, login/logout, password management |
| `PatientService.h/cpp` | Patient business logic, search, bill calculation |
| `DoctorService.h/cpp` | Doctor business logic, schedule management |
| `AppointmentService.h/cpp` | Booking logic, slot availability, status management |
| `AdminService.h/cpp` | Admin operations, statistics generation |
| `DepartmentService.h/cpp` | Department management, doctor assignments |
| `MedicineService.h/cpp` | Medicine CRUD, stock management, alerts |
| `PrescriptionService.h/cpp` | Prescription creation, dispensing, inventory updates |
| `ReportGenerator.h/cpp` | Generate daily/weekly/monthly reports, export to various formats |

### 5.4 Presentation Layer (`include/ui/`, `src/ui/`)

| File | Responsibility |
|------|----------------|
| `HMSFacade.h/cpp` | Facade pattern - single entry point for UI |
| `ConsoleUI.h/cpp` | Console menu system, user interaction loop |
| `InputValidator.h/cpp` | Input validation utilities |
| `DisplayHelper.h/cpp` | Output formatting, table display |

### 5.5 Common Utilities (`include/common/`, `src/common/`)

| File | Responsibility |
|------|----------------|
| `Constants.h` | File paths, menu options, validation rules |
| `Types.h` | Enums (Role, AppointmentStatus), type aliases |
| `Utils.h/cpp` | Date utilities, string helpers, ID generation |

---

## 6. Data File Formats

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
P001|patient001|Nguyen Van A|0901234567|Male|1990-05-15|123 Le Loi, HCM|Hypertension
P002|patient002|Tran Thi B|0912345678|Female|1985-08-20|456 Nguyen Hue, HCM|Diabetes
```

### 6.3 Doctor.txt
```
# Format: doctorID|username|name|phone|gender|dateOfBirth|specialization|schedule|consultationFee
D001|doctor001|Dr. Pham Van C|0923456789|Male|1975-03-10|Cardiology|Mon-Fri 08:00-17:00|500000
D002|doctor002|Dr. Le Thi D|0934567890|Female|1980-11-25|Pediatrics|Mon-Sat 09:00-16:00|400000
```

### 6.4 Appointment.txt
```
# Format: appointmentID|patientUsername|doctorID|date|time|disease|price|isPaid|status|notes
A001|patient001|D001|2024-03-15|09:00|Chest pain|500000|1|completed|Regular checkup
A002|patient002|D002|2024-03-16|10:30|Fever|400000|0|scheduled|Follow-up visit
```

### 6.5 Department.txt
```
# Format: departmentID|name|headDoctorID|phone|location|description|doctorIDs
# doctorIDs is comma-separated list
DEP001|Cardiology|D001|0901234567|Building A, Floor 2|Heart and cardiovascular care|D001,D003
DEP002|Pediatrics|D002|0912345678|Building B, Floor 1|Child healthcare|D002,D004
```

### 6.6 Medicine.txt
```
# Format: medicineID|name|category|manufacturer|unitPrice|stockQuantity|reorderLevel|expiryDate|description
MED001|Paracetamol|Pain Relief|PharmaCorp|15000|500|50|2025-12-31|Pain and fever relief
MED002|Amoxicillin|Antibiotic|MediLab|25000|200|30|2025-06-30|Bacterial infection treatment
```

### 6.7 Prescription.txt
```
# Format: prescriptionID|appointmentID|patientID|doctorID|diagnosis|notes|isDispensed|createdDate|items
# items format: medicineID:quantity:dosage;medicineID:quantity:dosage
PRE001|APT001|P001|D001|Hypertension|Take with food|1|2024-03-15|MED001:30:2 tablets daily;MED003:15:1 tablet daily
PRE002|APT002|P002|D002|Common Cold|Rest recommended|0|2024-03-16|MED001:10:1 tablet as needed
```

### 6.8 Status Values
- `scheduled` - Appointment booked, not yet occurred
- `completed` - Appointment finished
- `cancelled` - Appointment cancelled by patient or doctor
- `no_show` - Patient did not attend

---

## 7. Build System

See `CMakeLists.txt` for complete build configuration.

### Build Commands

```bash
# Create and enter build directory
mkdir -p build && cd build

# Configure with CMake
cmake ..

# Build main application
make HospitalApp

# Build tests
make HospitalTests

# Build everything
make all

# Run application
./HospitalApp

# Run tests
./HospitalTests

# Run tests with verbose output
./HospitalTests --gtest_output=xml:test_results.xml

# Clean build
make clean
```

### CMake Targets

| Target | Description |
|--------|-------------|
| `HospitalApp` | Main application executable |
| `HospitalTests` | Test executable with Google Test |
| `all` | Build all targets |
| `clean` | Remove build artifacts |

---

## Appendix A: SOLID Principles Application

| Principle | Application |
|-----------|-------------|
| **S**ingle Responsibility | Each class has one job: Models hold data, Repositories handle persistence, Services contain business logic, UI handles display |
| **O**pen/Closed | Repository interface allows new data sources without modifying existing code |
| **L**iskov Substitution | Patient, Doctor, Admin can substitute Person in any context |
| **I**nterface Segregation | IRepository provides focused interface; Services have specific methods |
| **D**ependency Inversion | Services depend on Repository interfaces, not concrete implementations |

## Appendix B: Advanced Features (IMPLEMENTED ✅)

The following advanced features have been fully implemented:

### Department Management
- **Model**: `Department.h/cpp` - Department entity with doctor assignments
- **DAL**: `DepartmentRepository.h/cpp` - CRUD operations, doctor lookup
- **BLL**: `DepartmentService.h/cpp` - Department management, statistics

### Medicine & Inventory
- **Model**: `Medicine.h/cpp` - Medicine entity with stock tracking, expiry
- **DAL**: `MedicineRepository.h/cpp` - CRUD, low stock alerts, expiry queries
- **BLL**: `MedicineService.h/cpp` - Stock management, alerts, validation

### Prescription System
- **Model**: `Prescription.h/cpp` - Prescription with items, dosage
- **DAL**: `PrescriptionRepository.h/cpp` - CRUD, patient/doctor queries
- **BLL**: `PrescriptionService.h/cpp` - Creation, dispensing, inventory integration

### Report Generation
- **BLL**: `ReportGenerator.h/cpp` - Daily/weekly/monthly reports
- **Features**: Export to TXT, CSV, HTML formats
- **Reports**: Revenue, patient statistics, doctor performance, appointment analysis

The architecture successfully supported these additions without modifying existing core code (Open/Closed Principle).
