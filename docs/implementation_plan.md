# Advanced Features Implementation Plan - Hospital Management System

## Goal
Implement the Advanced Features phase (Department, Medicine, Prescription, ReportGenerator) following the existing 3-tier architecture pattern to enhance the Hospital Management System with organizational structure, pharmacy management, and comprehensive reporting capabilities.

---

## Context Analysis Summary

After analyzing your codebase, here's what I found:

### What Already Exists

| Component | Location | Status |
|-----------|----------|--------|
| `Department.h` | `include/advance/` | Header with class declaration, no `.cpp` |
| `Medicine.h` | `include/advance/` | Header with class declaration, no `.cpp` |
| `Prescription.h` | `include/advance/` | Header with class + PrescriptionItem struct, no `.cpp` |
| `ReportGenerator.h` | `include/advance/` | Header with enums (ReportType, ExportFormat), Report struct, class declaration |

### What Needs to Be Created

| Layer | New Files | Count |
|-------|-----------|-------|
| **Model** | `src/model/Department.cpp`, `Medicine.cpp`, `Prescription.cpp` | 3 |
| **DAL Headers** | `include/dal/DepartmentRepository.h`, `MedicineRepository.h`, `PrescriptionRepository.h` | 3 |
| **DAL Impl** | `src/dal/DepartmentRepository.cpp`, `MedicineRepository.cpp`, `PrescriptionRepository.cpp` | 3 |
| **BLL Headers** | `include/bll/DepartmentService.h`, `MedicineService.h`, `PrescriptionService.h` | 3 |
| **BLL Impl** | `src/bll/DepartmentService.cpp`, `MedicineService.cpp`, `PrescriptionService.cpp`, `ReportGenerator.cpp` | 4 |
| **Tests** | 10 test files across model/dal/bll | 10 |
| **Data Files** | 3 data files + 3 sample files + 3 test fixtures | 9 |
| **Total** | | **35 files** |

---

## 1. Structural Recommendations

### 1.1 Directory Structure (No Changes Needed)

Your existing structure perfectly supports the new features. The `include/advance/` already contains the model headers:

```
Hospital_Management_Project/
├── include/
│   ├── advance/                    # ✅ EXISTS - Model headers here
│   │   ├── Department.h            # ✅ EXISTS
│   │   ├── Medicine.h              # ✅ EXISTS
│   │   ├── Prescription.h          # ✅ EXISTS
│   │   └── ReportGenerator.h       # ✅ EXISTS (BLL, but header is here)
│   │
│   ├── dal/                        # Add repository headers here
│   │   ├── DepartmentRepository.h  # 🆕 NEW
│   │   ├── MedicineRepository.h    # 🆕 NEW
│   │   └── PrescriptionRepository.h # 🆕 NEW
│   │
│   └── bll/                        # Add service headers here
│       ├── DepartmentService.h     # 🆕 NEW
│       ├── MedicineService.h       # 🆕 NEW
│       └── PrescriptionService.h   # 🆕 NEW
│
├── src/
│   ├── model/                      # Add model implementations here
│   │   ├── Department.cpp          # 🆕 NEW
│   │   ├── Medicine.cpp            # 🆕 NEW
│   │   └── Prescription.cpp        # 🆕 NEW
│   │
│   ├── dal/                        # Add repository implementations here
│   │   ├── DepartmentRepository.cpp   # 🆕 NEW
│   │   ├── MedicineRepository.cpp     # 🆕 NEW
│   │   └── PrescriptionRepository.cpp # 🆕 NEW
│   │
│   └── bll/                        # Add service implementations here
│       ├── DepartmentService.cpp      # 🆕 NEW
│       ├── MedicineService.cpp        # 🆕 NEW
│       ├── PrescriptionService.cpp    # 🆕 NEW
│       └── ReportGenerator.cpp        # 🆕 NEW
│
├── data/
│   ├── Department.txt              # 🆕 NEW
│   ├── Medicine.txt                # 🆕 NEW
│   ├── Prescription.txt            # 🆕 NEW
│   ├── reports/                    # 🆕 NEW directory
│   └── sample/
│       ├── Department_sample.txt   # 🆕 NEW
│       ├── Medicine_sample.txt     # 🆕 NEW
│       └── Prescription_sample.txt # 🆕 NEW
│
└── test/
    ├── model/
    │   ├── DepartmentTest.cpp      # 🆕 NEW
    │   ├── MedicineTest.cpp        # 🆕 NEW
    │   └── PrescriptionTest.cpp    # 🆕 NEW
    │
    ├── dal/
    │   ├── DepartmentRepositoryTest.cpp   # 🆕 NEW
    │   ├── MedicineRepositoryTest.cpp     # 🆕 NEW
    │   └── PrescriptionRepositoryTest.cpp # 🆕 NEW
    │
    ├── bll/
    │   ├── DepartmentServiceTest.cpp     # 🆕 NEW
    │   ├── MedicineServiceTest.cpp       # 🆕 NEW
    │   ├── PrescriptionServiceTest.cpp   # 🆕 NEW
    │   └── ReportGeneratorTest.cpp       # 🆕 NEW
    │
    └── fixtures/
        ├── Department_test.txt     # 🆕 NEW
        ├── Medicine_test.txt       # 🆕 NEW
        └── Prescription_test.txt   # 🆕 NEW
```

### 1.2 Naming Conventions (Follow Existing Patterns)

Based on your codebase analysis:

| Element | Convention | Example |
|---------|------------|---------|
| Classes | PascalCase | `MedicineRepository`, `DepartmentService` |
| Methods | camelCase | `getMedicineByID`, `assignDoctor` |
| Member variables | m_ prefix | `m_medicines`, `m_filePath` |
| Static members | s_ prefix | `s_instance`, `s_mutex` |
| Constants | UPPER_SNAKE_CASE | `MEDICINE_FILE`, `DEPARTMENT_ID_PREFIX` |
| ID format | Prefix + 3 digits | `DEP001`, `MED001`, `PRE001` |

---

## 2. Impact on Existing Code

### 2.1 Files That Need Modification

| File | Changes Required | Priority |
|------|------------------|----------|
| `include/common/Constants.h` | Add file paths, ID prefixes, medicine constants | **Required** |
| `include/common/Types.h` | Add type aliases (DepartmentID, MedicineID, PrescriptionID) | **Optional but recommended** |
| `docs/ARCHITECTURE.md` | Document new components | After implementation |
| `docs/ARCHITECTURE_VI.md` | Vietnamese version update | After implementation |
| `docs/diagrams/architecture-overview.md` | Add new services to diagram | After implementation |
| `docs/diagrams/architecture-detailed.md` | Add new class diagrams | After implementation |

### 2.2 Files That Do NOT Need Modification

| File | Reason |
|------|--------|
| `CMakeLists.txt` | Uses `file(GLOB ...)` - auto-discovers new `.cpp` files |
| Existing Services | New services are independent, no changes needed |
| Existing Repositories | New repositories are independent |
| `HMSFacade.h/.cpp` | Not needed until UI integration phase |
| `ConsoleUI.h/.cpp` | UI layer - leave for last phase |

### 2.3 Cross-Layer Dependencies

```
┌─────────────────────────────────────────────────────────────────┐
│                    NEW COMPONENTS                                │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│  PrescriptionService ──────┬──► AppointmentRepository (EXISTING) │
│         │                  │                                     │
│         ├──► PrescriptionRepository (NEW)                        │
│         │                  │                                     │
│         └──► MedicineRepository (NEW) ◄─── MedicineService (NEW) │
│                                                                  │
│  DepartmentService ────────┬──► DepartmentRepository (NEW)       │
│         │                  │                                     │
│         └──► DoctorRepository (EXISTING)                         │
│                                                                  │
│  ReportGenerator ──────────┬──► PatientService (EXISTING)        │
│         │                  ├──► DoctorService (EXISTING)         │
│         │                  ├──► AppointmentService (EXISTING)    │
│         │                  ├──► DepartmentService (NEW)          │
│         │                  ├──► MedicineService (NEW)            │
│         └──────────────────└──► PrescriptionService (NEW)        │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

### 2.4 Potential Integration Points

1. **PrescriptionService → MedicineRepository**
   - When dispensing a prescription, must update medicine inventory
   - Need to check stock availability before dispensing

2. **PrescriptionService → AppointmentRepository**
   - Prescriptions are linked to appointments
   - Need to validate appointment exists

3. **DepartmentService → DoctorRepository**
   - When assigning doctors to departments
   - Need to validate doctor exists

4. **ReportGenerator → All Services**
   - Aggregates data from all services for reports
   - Read-only access, no modifications

---

## 3. Detailed Implementation Specifications

### 3.1 Model Layer Implementations

#### Medicine.cpp Implementation

```cpp
// src/model/Medicine.cpp
#include "advance/Medicine.h"
#include "common/Utils.h"
#include "common/Constants.h"
#include <sstream>
#include <iostream>
#include <iomanip>

namespace HMS {
namespace Model {

// Constructor implementations
Medicine::Medicine(const std::string& medicineID,
                   const std::string& name,
                   const std::string& category,
                   double unitPrice,
                   int quantityInStock)
    : m_medicineID(medicineID)
    , m_name(name)
    , m_category(category)
    , m_unitPrice(unitPrice)
    , m_quantityInStock(quantityInStock)
    , m_reorderLevel(Constants::DEFAULT_REORDER_LEVEL)
{
}

// Inventory Operations
void Medicine::addStock(int quantity) {
    if (quantity > 0) {
        m_quantityInStock += quantity;
    }
}

bool Medicine::removeStock(int quantity) {
    if (quantity <= 0 || quantity > m_quantityInStock) {
        return false;
    }
    m_quantityInStock -= quantity;
    return true;
}

bool Medicine::isLowStock() const {
    return m_quantityInStock <= m_reorderLevel;
}

bool Medicine::isExpired() const {
    if (m_expiryDate.empty()) return false;
    return Utils::compareDates(m_expiryDate, Utils::getCurrentDate()) < 0;
}

bool Medicine::isExpiringSoon(int daysThreshold) const {
    if (m_expiryDate.empty()) return false;
    // Calculate threshold date
    // Implementation using date arithmetic
    return Utils::compareDates(m_expiryDate, Utils::getCurrentDate()) <= daysThreshold;
}

// Serialization
std::string Medicine::serialize() const {
    std::ostringstream oss;
    oss << m_medicineID << Constants::FIELD_DELIMITER
        << m_name << Constants::FIELD_DELIMITER
        << m_genericName << Constants::FIELD_DELIMITER
        << m_category << Constants::FIELD_DELIMITER
        << m_manufacturer << Constants::FIELD_DELIMITER
        << m_description << Constants::FIELD_DELIMITER
        << std::fixed << std::setprecision(0) << m_unitPrice << Constants::FIELD_DELIMITER
        << m_quantityInStock << Constants::FIELD_DELIMITER
        << m_reorderLevel << Constants::FIELD_DELIMITER
        << m_expiryDate << Constants::FIELD_DELIMITER
        << m_dosageForm << Constants::FIELD_DELIMITER
        << m_strength;
    return oss.str();
}

Result<Medicine> Medicine::deserialize(const std::string& line) {
    auto parts = Utils::split(line, Constants::FIELD_DELIMITER);
    if (parts.size() < 12) {
        return std::nullopt;
    }

    Medicine med;
    med.m_medicineID = parts[0];
    med.m_name = parts[1];
    med.m_genericName = parts[2];
    med.m_category = parts[3];
    med.m_manufacturer = parts[4];
    med.m_description = parts[5];
    med.m_unitPrice = std::stod(parts[6]);
    med.m_quantityInStock = std::stoi(parts[7]);
    med.m_reorderLevel = std::stoi(parts[8]);
    med.m_expiryDate = parts[9];
    med.m_dosageForm = parts[10];
    med.m_strength = parts[11];

    return med;
}

void Medicine::displayInfo() const {
    std::cout << "Medicine ID: " << m_medicineID << "\n"
              << "Name: " << m_name << " (" << m_genericName << ")\n"
              << "Category: " << m_category << "\n"
              << "Price: " << Utils::formatMoney(m_unitPrice) << "\n"
              << "Stock: " << m_quantityInStock
              << (isLowStock() ? " [LOW STOCK]" : "") << "\n"
              << "Expiry: " << m_expiryDate
              << (isExpired() ? " [EXPIRED]" : "") << "\n";
}

// All getters/setters...

} // namespace Model
} // namespace HMS
```

#### Department.cpp Key Points

```cpp
// Handle doctorIDs vector serialization
std::string Department::serialize() const {
    std::ostringstream oss;
    oss << m_departmentID << '|'
        << m_name << '|'
        << m_description << '|'
        << m_headDoctorID << '|';

    // Join doctorIDs with comma
    for (size_t i = 0; i < m_doctorIDs.size(); ++i) {
        if (i > 0) oss << ',';
        oss << m_doctorIDs[i];
    }

    oss << '|' << m_location << '|' << m_phone;
    return oss.str();
}

// Deserialize doctorIDs from comma-separated
Result<Department> Department::deserialize(const std::string& line) {
    auto parts = Utils::split(line, '|');
    if (parts.size() < 7) return std::nullopt;

    Department dept;
    // ... set other fields ...

    // Parse doctorIDs
    if (!parts[4].empty()) {
        dept.m_doctorIDs = Utils::split(parts[4], ',');
    }

    return dept;
}
```

#### Prescription.cpp Key Points

```cpp
// Complex items serialization format:
// medicineID:quantity:dosage:duration:instructions;...

std::string serializeItems(const std::vector<PrescriptionItem>& items) {
    std::ostringstream oss;
    for (size_t i = 0; i < items.size(); ++i) {
        if (i > 0) oss << ';';
        const auto& item = items[i];
        oss << item.medicineID << ':'
            << item.quantity << ':'
            << item.dosage << ':'
            << item.duration << ':'
            << item.instructions;
    }
    return oss.str();
}

std::vector<PrescriptionItem> deserializeItems(const std::string& itemsStr) {
    std::vector<PrescriptionItem> items;
    if (itemsStr.empty()) return items;

    auto itemStrings = Utils::split(itemsStr, ';');
    for (const auto& itemStr : itemStrings) {
        auto parts = Utils::split(itemStr, ':');
        if (parts.size() >= 5) {
            PrescriptionItem item;
            item.medicineID = parts[0];
            item.quantity = std::stoi(parts[1]);
            item.dosage = parts[2];
            item.duration = parts[3];
            item.instructions = parts[4];
            items.push_back(item);
        }
    }
    return items;
}
```

### 3.2 Repository Layer Pattern

All new repositories should follow this exact pattern (based on your existing PatientRepository):

```cpp
// include/dal/MedicineRepository.h
#pragma once

#include "IRepository.h"
#include "advance/Medicine.h"
#include <vector>
#include <optional>
#include <mutex>
#include <memory>

namespace HMS {
namespace DAL {

class MedicineRepository : public IRepository<Model::Medicine> {
private:
    static std::unique_ptr<MedicineRepository> s_instance;
    static std::mutex s_mutex;

    std::vector<Model::Medicine> m_medicines;
    std::string m_filePath;
    bool m_isLoaded;

    MedicineRepository();

public:
    // Singleton
    static MedicineRepository* getInstance();
    static void resetInstance();

    // Delete copy/assignment
    MedicineRepository(const MedicineRepository&) = delete;
    MedicineRepository& operator=(const MedicineRepository&) = delete;

    ~MedicineRepository();

    // IRepository implementation
    std::vector<Model::Medicine> getAll() override;
    std::optional<Model::Medicine> getById(const std::string& id) override;
    bool add(const Model::Medicine& medicine) override;
    bool update(const Model::Medicine& medicine) override;
    bool remove(const std::string& id) override;
    bool save() override;
    bool load() override;
    size_t count() const override;
    bool exists(const std::string& id) const override;
    bool clear() override;

    // Specialized queries
    std::vector<Model::Medicine> getByCategory(const std::string& category);
    std::vector<Model::Medicine> getLowStock();
    std::vector<Model::Medicine> getExpiringSoon(int days = 30);
    std::vector<Model::Medicine> searchByName(const std::string& name);
    std::vector<std::string> getAllCategories();

    // ID generation
    std::string getNextId();

    // File path management (for testing)
    void setFilePath(const std::string& path);
    std::string getFilePath() const;
};

} // namespace DAL
} // namespace HMS
```

### 3.3 Service Layer Pattern

```cpp
// include/bll/MedicineService.h
#pragma once

#include "dal/MedicineRepository.h"
#include "advance/Medicine.h"
#include <vector>
#include <optional>
#include <mutex>
#include <memory>

namespace HMS {
namespace BLL {

class MedicineService {
private:
    static std::unique_ptr<MedicineService> s_instance;
    static std::mutex s_mutex;

    DAL::MedicineRepository* m_medicineRepo;

    MedicineService();

public:
    // Singleton
    static MedicineService* getInstance();
    static void resetInstance();

    MedicineService(const MedicineService&) = delete;
    MedicineService& operator=(const MedicineService&) = delete;

    ~MedicineService();

    // CRUD with validation
    bool createMedicine(const Model::Medicine& medicine);
    std::optional<Model::Medicine> createMedicine(
        const std::string& name,
        const std::string& category,
        double unitPrice,
        int quantity);
    bool updateMedicine(const Model::Medicine& medicine);
    bool deleteMedicine(const std::string& medicineID);

    // Queries
    std::optional<Model::Medicine> getMedicineByID(const std::string& id);
    std::vector<Model::Medicine> getAllMedicines();
    std::vector<Model::Medicine> getMedicinesByCategory(const std::string& category);
    std::vector<Model::Medicine> searchMedicines(const std::string& keyword);
    std::vector<std::string> getAllCategories();
    size_t getMedicineCount() const;

    // Stock management
    bool addStock(const std::string& medicineID, int quantity);
    bool removeStock(const std::string& medicineID, int quantity);
    bool hasEnoughStock(const std::string& medicineID, int quantity) const;

    // Alerts
    std::vector<Model::Medicine> getLowStockAlerts();
    std::vector<Model::Medicine> getExpiryAlerts(int daysThreshold = 30);

    // Validation
    bool validateMedicine(const Model::Medicine& medicine) const;
    bool medicineExists(const std::string& medicineID) const;

    // Persistence
    bool saveData();
    bool loadData();
};

} // namespace BLL
} // namespace HMS
```

---

## 4. Data File Formats

### 4.1 Department.txt

```
# Hospital Management System - Department Data
# Format: departmentID|name|description|headDoctorID|doctorIDs|location|phone
# doctorIDs is comma-separated list
DEP001|Cardiology|Heart and cardiovascular care|D001|D001,D005,D012|Building A, Floor 2|0281234567
DEP002|Pediatrics|Child healthcare|D002|D002,D008|Building B, Floor 1|0281234568
DEP003|Neurology|Brain and nervous system|D003|D003,D010|Building A, Floor 3|0281234569
```

### 4.2 Medicine.txt

```
# Hospital Management System - Medicine Data
# Format: medicineID|name|genericName|category|manufacturer|description|unitPrice|quantity|reorderLevel|expiryDate|dosageForm|strength
MED001|Paracetamol 500mg|Acetaminophen|Pain Relief|PharmaCorp|For pain and fever|5000|500|100|2025-12-31|Tablet|500mg
MED002|Amoxicillin 250mg|Amoxicillin|Antibiotic|MedPharm|Bacterial infections|15000|200|50|2025-06-30|Capsule|250mg
MED003|Omeprazole 20mg|Omeprazole|Gastrointestinal|GastroCare|Acid reflux treatment|12000|300|75|2025-09-15|Capsule|20mg
```

### 4.3 Prescription.txt

```
# Hospital Management System - Prescription Data
# Format: prescriptionID|appointmentID|patientUsername|doctorID|date|diagnosis|notes|isDispensed|items
# Items format: medicineID:quantity:dosage:duration:instructions;...
PRE001|APT001|patient001|D001|2024-03-15|Viral Fever|Rest recommended|1|MED001:20:2 tablets 3 times daily:5 days:After meals;MED003:10:1 capsule daily:7 days:Before breakfast
PRE002|APT002|patient002|D002|2024-03-16|Common Cold|Plenty of fluids|0|MED001:10:1 tablet twice daily:3 days:As needed
```

---

## 5. Constants.h Additions

```cpp
// Add to include/common/Constants.h

// ==================== Advanced Feature File Paths ====================
constexpr const char* DEPARTMENT_FILE = "data/Department.txt";
constexpr const char* MEDICINE_FILE = "data/Medicine.txt";
constexpr const char* PRESCRIPTION_FILE = "data/Prescription.txt";
constexpr const char* REPORTS_DIR = "data/reports/";

// ==================== Advanced Feature ID Prefixes ====================
constexpr const char* DEPARTMENT_ID_PREFIX = "DEP";
constexpr const char* MEDICINE_ID_PREFIX = "MED";
constexpr const char* PRESCRIPTION_ID_PREFIX = "PRE";

// ==================== Medicine Constants ====================
constexpr int DEFAULT_REORDER_LEVEL = 10;
constexpr int EXPIRY_WARNING_DAYS = 30;
constexpr int LOW_STOCK_THRESHOLD = 10;

// ==================== Menu Options - Advanced Features ====================
namespace Menu {
    namespace Pharmacy {
        constexpr int VIEW_MEDICINES = 1;
        constexpr int ADD_MEDICINE = 2;
        constexpr int UPDATE_STOCK = 3;
        constexpr int VIEW_LOW_STOCK = 4;
        constexpr int VIEW_EXPIRING = 5;
        constexpr int BACK = 6;
    }

    namespace Reports {
        constexpr int DAILY_REPORT = 1;
        constexpr int WEEKLY_REPORT = 2;
        constexpr int MONTHLY_REPORT = 3;
        constexpr int REVENUE_REPORT = 4;
        constexpr int EXPORT_REPORT = 5;
        constexpr int BACK = 6;
    }
}
```

---

## 6. Types.h Additions

```cpp
// Add to include/common/Types.h

// ==================== Advanced Feature Type Aliases ====================
using DepartmentID = std::string;
using MedicineID = std::string;
using PrescriptionID = std::string;

// Note: ReportType and ExportFormat enums are already in ReportGenerator.h
```

---

## 7. Verification Plan

### 7.1 Build Verification Commands

```bash
# Navigate to project root
cd /workspaces/Hospital_Management_Project

# Configure (first time or after CMake changes)
cmake -B build

# Build library only (fast check for compilation errors)
cmake --build build --target HospitalLib

# Build everything including tests
cmake --build build

# Run all tests
./build/HospitalTests

# Run specific test suites
./build/HospitalTests --gtest_filter="MedicineTest.*"
./build/HospitalTests --gtest_filter="DepartmentTest.*"
./build/HospitalTests --gtest_filter="PrescriptionTest.*"
./build/HospitalTests --gtest_filter="*Repository*"
./build/HospitalTests --gtest_filter="*Service*"
./build/HospitalTests --gtest_filter="ReportGenerator*"

# Run all new feature tests
./build/HospitalTests --gtest_filter="*Medicine*:*Department*:*Prescription*:*Report*"
```

### 7.2 Test Coverage Matrix

| Component | Test File | Key Test Cases |
|-----------|-----------|----------------|
| Medicine Model | `MedicineTest.cpp` | Constructors, stock operations, expiry checks, serialize/deserialize |
| Department Model | `DepartmentTest.cpp` | Constructors, doctor management, serialize/deserialize |
| Prescription Model | `PrescriptionTest.cpp` | Constructors, item management, cost calculation, complex serialization |
| MedicineRepository | `MedicineRepositoryTest.cpp` | CRUD, category queries, low stock, expiring queries, file persistence |
| DepartmentRepository | `DepartmentRepositoryTest.cpp` | CRUD, doctor lookup, file persistence with vector |
| PrescriptionRepository | `PrescriptionRepositoryTest.cpp` | CRUD, patient/doctor queries, complex item persistence |
| MedicineService | `MedicineServiceTest.cpp` | Validation, stock management, alerts |
| DepartmentService | `DepartmentServiceTest.cpp` | Doctor assignment validation, head doctor rules |
| PrescriptionService | `PrescriptionServiceTest.cpp` | Appointment validation, dispensing with inventory update |
| ReportGenerator | `ReportGeneratorTest.cpp` | Report generation, export formats |

---

## 8. Implementation Order (Recommended)

### Phase 1: Foundation (Independent Models)
```
1. Medicine.cpp          ←── Start here (completely independent)
2. Department.cpp        ←── Independent
3. Verify build
```

### Phase 2: Repositories
```
4. MedicineRepository.h/.cpp
5. DepartmentRepository.h/.cpp
6. PrescriptionRepository.h/.cpp   ←── Needs Medicine for cost calculation
7. Verify build
```

### Phase 3: Prescription Model
```
8. Prescription.cpp      ←── Can reference MedicineRepository now
9. Verify build
```

### Phase 4: Services
```
10. MedicineService.h/.cpp
11. DepartmentService.h/.cpp
12. PrescriptionService.h/.cpp
13. ReportGenerator.cpp
14. Verify build
```

### Phase 5: Integration & Testing
```
15. Update Constants.h
16. Update Types.h (optional)
17. Create data files
18. Write all test files
19. Run tests
```

### Phase 6: Documentation
```
20. Update architecture documentation
```

---

## 9. Risk Mitigation

### 9.1 Potential Issues

| Risk | Mitigation |
|------|------------|
| Circular dependencies | Repositories only depend on models; Services depend on repositories |
| Thread safety | Use same mutex pattern as existing code |
| File format breaking | Add version header to data files for future compatibility |
| Memory leaks | Use `std::unique_ptr` for singleton instances (existing pattern) |

### 9.2 Rollback Strategy

Since all new code is in new files:
- Model implementations: Delete `.cpp` files in `src/model/`
- Repository/Service files: Delete from `include/dal/`, `include/bll/`, `src/dal/`, `src/bll/`
- Data files: Delete from `data/`
- Revert `Constants.h` and `Types.h` changes

---

## 10. Summary

| Category | Count |
|----------|-------|
| New Model Implementations | 3 files |
| New Repository Headers | 3 files |
| New Repository Implementations | 3 files |
| New Service Headers | 3 files |
| New Service Implementations | 4 files |
| Modified Common Files | 2 files |
| New Data Files | 3 files |
| New Sample Data Files | 3 files |
| New Test Files | 10 files |
| New Test Fixtures | 3 files |
| **Total Files** | **37 files** |

---

## Appendix: Quick Reference

### Include Statements for New Files

```cpp
// For model implementations
#include "advance/Medicine.h"      // or Department.h, Prescription.h
#include "common/Utils.h"
#include "common/Constants.h"
#include "common/Types.h"

// For repository implementations
#include "dal/MedicineRepository.h"  // or Department, Prescription
#include "dal/FileHelper.h"
#include "advance/Medicine.h"

// For service implementations
#include "bll/MedicineService.h"
#include "dal/MedicineRepository.h"

// For ReportGenerator
#include "advance/ReportGenerator.h"
#include "bll/PatientService.h"
#include "bll/DoctorService.h"
#include "bll/AppointmentService.h"
#include "bll/AdminService.h"
// Plus new services when implemented
```

### Singleton Static Member Initialization

```cpp
// In each .cpp file
std::unique_ptr<ClassName> ClassName::s_instance = nullptr;
std::mutex ClassName::s_mutex;
```
