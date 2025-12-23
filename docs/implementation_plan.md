# Advanced Features Implementation Plan - Hospital Management System

## Goal
Implement the Advanced Features phase (Department, Medicine, Prescription, ReportGenerator) following the existing 3-tier architecture pattern to enhance the Hospital Management System with organizational structure, pharmacy management, and comprehensive reporting capabilities.

---

## Context Analysis Summary

After analyzing your codebase, here's what I found:

### ✅ What Already Exists
- **Placeholder Headers** in `include/advance/`:
  - `Department.h` - Hospital department with doctor assignments
  - `Medicine.h` - Medicine inventory management
  - `Prescription.h` - Links appointments with prescribed medicines
  - `ReportGenerator.h` - Report generation singleton (lives in BLL)

### ❌ What Needs to Be Created
- **Model Implementations** (`.cpp` files)
- **Repository Layer** (headers + implementations)
- **Service Layer** (headers + implementations)
- **Data Files** (`.txt` files)
- **Unit Tests**

---

## Proposed Changes

### Model Layer (`src/model/`)

#### [NEW] [Department.cpp]
Implement the Department class declared in `include/advance/Department.h`:
- Constructor implementation
- All getters/setters
- Doctor management methods (`addDoctor`, `removeDoctor`, `hasDoctor`)
- `serialize()` and `deserialize()` for file persistence
- `displayInfo()` for console output

#### [NEW] [Medicine.cpp]
Implement the Medicine class:
- Constructor implementation
- All getters/setters
- Inventory operations (`addStock`, `removeStock`, `isLowStock`)
- Expiry tracking methods (`isExpired`, `isExpiringSoon`)
- `serialize()`/`deserialize()` methods

#### [NEW] [Prescription.cpp]
Implement the Prescription class:
- Constructor implementation
- All getters/setters
- Item management (`addItem`, `removeItem`, `clearItems`)
- `calculateTotalCost()` method
- `serialize()`/`deserialize()` with complex item format
- `toPrintFormat()` for prescription printing

---

### Data Access Layer

#### [NEW] [DepartmentRepository.h]+ [DepartmentRepository.cpp]
Singleton repository following `IRepository<T>` pattern:
- CRUD operations for departments
- Query methods: `getByName()`, `getByHeadDoctor()`, `getDepartmentByDoctor()`
- File persistence to `data/Department.txt`

#### [NEW] [MedicineRepository.h] + [MedicineRepository.cpp]
Singleton repository:
- CRUD operations for medicines
- Query methods: `getByCategory()`, `getLowStock()`, `getExpiringSoon()`, `searchByName()`
- File persistence to `data/Medicine.txt`

#### [NEW] [PrescriptionRepository.h]+ [PrescriptionRepository.cpp]
Singleton repository:
- CRUD operations for prescriptions
- Query methods: `getByPatient()`, `getByDoctor()`, `getByAppointment()`, `getUndispensed()`
- File persistence to `data/Prescription.txt`

---

### Business Logic Layer

#### [NEW] [DepartmentService.h] + [DepartmentService.cpp]
Singleton service for department business logic:
- Department CRUD with validation
- Assign/unassign doctors to departments
- Set department head (must be doctor in that department)
- Get department statistics
- Cross-check with DoctorRepository for validation

#### [NEW] [MedicineService.h] + [MedicineService.cpp]
Singleton service for pharmacy/inventory:
- Medicine CRUD with validation
- Stock management (add/remove stock)
- Low stock alerts
- Expiry tracking and alerts
- Category-based listing

#### [NEW] [PrescriptionService.h] + [PrescriptionService.cpp]
Singleton service for prescriptions:
- Create prescription linked to appointment
- Add/remove prescription items
- Calculate total cost
- Mark as dispensed (updates medicine inventory)
- Cross-validation with AppointmentRepository, MedicineRepository

#### [NEW] [ReportGenerator.cpp]
Implement the ReportGenerator singleton (header already exists):
- Generate daily/weekly/monthly reports
- Revenue reports
- Patient/Doctor statistics
- Export to TEXT, CSV, HTML formats
- Uses existing services for data aggregation

---

### Common Layer

#### [MODIFY] [Constants.h]
Add new file path constants:
```cpp
// Advanced feature file paths
constexpr const char* DEPARTMENT_FILE = "data/Department.txt";
constexpr const char* MEDICINE_FILE = "data/Medicine.txt";
constexpr const char* PRESCRIPTION_FILE = "data/Prescription.txt";

// Medicine categories
constexpr const char* DEFAULT_MEDICINE_CATEGORIES[] = {...};

// Report file paths
constexpr const char* REPORTS_DIR = "data/reports/";
```

#### [MODIFY] [Types.h]
Add new type aliases if needed:
```cpp
using DepartmentID = std::string;
using MedicineID = std::string;
using PrescriptionID = std::string;
```

---

### Data Files

#### [NEW] Department.txt
```
# Format: departmentID|name|description|headDoctorID|doctorIDs|location|phone
```

#### [NEW] Medicine.txt
```
# Format: medicineID|name|genericName|category|manufacturer|description|unitPrice|quantity|reorderLevel|expiryDate|dosageForm|strength
```

#### [NEW] Prescription.txt
```
# Format: prescriptionID|appointmentID|patientUsername|doctorID|date|diagnosis|notes|isDispensed|items
# Items: medicineID:quantity:dosage:duration:instructions;...
```

---

## Structural Recommendations

Based on your existing architecture, here's the **recommended directory structure**:

```
Hospital_Management_Project/
├── include/
│   ├── advance/                    # ✅ Already exists with headers
│   │   ├── Department.h            # ✅ Exists
│   │   ├── Medicine.h              # ✅ Exists
│   │   ├── Prescription.h          # ✅ Exists
│   │   └── ReportGenerator.h       # ✅ Exists
│   │
│   ├── dal/
│   │   ├── DepartmentRepository.h  # 🆕 NEW
│   │   ├── MedicineRepository.h    # 🆕 NEW
│   │   └── PrescriptionRepository.h # 🆕 NEW
│   │
│   └── bll/
│       ├── DepartmentService.h     # 🆕 NEW
│       ├── MedicineService.h       # 🆕 NEW
│       └── PrescriptionService.h   # 🆕 NEW
│
├── src/
│   ├── model/
│   │   ├── Department.cpp          # 🆕 NEW
│   │   ├── Medicine.cpp            # 🆕 NEW
│   │   └── Prescription.cpp        # 🆕 NEW
│   │
│   ├── dal/
│   │   ├── DepartmentRepository.cpp   # 🆕 NEW
│   │   ├── MedicineRepository.cpp     # 🆕 NEW
│   │   └── PrescriptionRepository.cpp # 🆕 NEW
│   │
│   └── bll/
│       ├── DepartmentService.cpp      # 🆕 NEW
│       ├── MedicineService.cpp        # 🆕 NEW
│       ├── PrescriptionService.cpp    # 🆕 NEW
│       └── ReportGenerator.cpp        # 🆕 NEW
│
├── data/
│   ├── Department.txt              # 🆕 NEW
│   ├── Medicine.txt                # 🆕 NEW
│   ├── Prescription.txt            # 🆕 NEW
│   └── reports/                    # 🆕 NEW directory for generated reports
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

---

## User Review Required

> [!IMPORTANT]
> **Implementation Scope Decision**
>
> The advanced features are interconnected. I recommend implementing in this order:
> 1. **Medicine** (independent) → Foundation for prescriptions
> 2. **Department** (independent) → Organizational structure
> 3. **Prescription** (depends on Medicine, Appointment) → Links them together
> 4. **ReportGenerator** (uses all services) → Aggregates everything
>
> Would you like me to:
> - **Option A:** Implement ALL features in one go
> - **Option B:** Implement one feature at a time (start with Medicine)

> [!WARNING]
> **CMakeLists.txt Auto-Discovery**
>
> Your `CMakeLists.txt` uses `file(GLOB ...)` which automatically picks up new `.cpp` files in the source directories. **No CMake changes are required** for the new implementations. However, you may want to verify the build after adding new files.

---

## Verification Plan

### Automated Tests

The project uses **Google Test (GTest)** for unit testing.

**Build and Run Commands:**
```bash
# Navigate to project root
cd c:\Users\DELL\Downloads\Hospital_Management_Project-main\Hospital_Management_Project-main

# Create/enter build directory
mkdir -p build && cd build

# Configure with CMake
cmake ..

# Build all (including tests)
make all
# OR on Windows with MSVC:
# cmake --build . --config Debug

# Run all tests
./HospitalTests
# OR on Windows:
# .\HospitalTests.exe
```

### Test Coverage Plan

For each new component, I will create tests following the existing patterns in `test/`:

| Component | Test File | Key Test Cases |
|-----------|-----------|----------------|
| Department Model | `test/model/DepartmentTest.cpp` | Constructors, getters/setters, doctor management, serialize/deserialize |
| Medicine Model | `test/model/MedicineTest.cpp` | Constructors, inventory operations, expiry checks, serialize/deserialize |
| Prescription Model | `test/model/PrescriptionTest.cpp` | Constructors, item management, cost calculation, serialize/deserialize |
| DepartmentRepository | `test/dal/DepartmentRepositoryTest.cpp` | CRUD operations, queries, file persistence |
| MedicineRepository | `test/dal/MedicineRepositoryTest.cpp` | CRUD, category queries, low stock queries |
| PrescriptionRepository | `test/dal/PrescriptionRepositoryTest.cpp` | CRUD, patient/doctor queries |
| DepartmentService | `test/bll/DepartmentServiceTest.cpp` | Business logic, validation, doctor assignments |
| MedicineService | `test/bll/MedicineServiceTest.cpp` | Stock management, alerts |
| PrescriptionService | `test/bll/PrescriptionServiceTest.cpp` | Prescription creation, dispensing |
| ReportGenerator | `test/bll/ReportGeneratorTest.cpp` | Report generation, export formats |

### Manual Verification

After implementation, you can manually verify by:

1. **Build Success Check:**
   ```bash
   cd build && cmake .. && make HospitalApp
   ```
   - Expected: No compilation errors

2. **Test Execution:**
   ```bash
   ./HospitalTests --gtest_filter="*Department*:*Medicine*:*Prescription*:*Report*"
   ```
   - Expected: All new tests pass

3. **Data File Creation:**
   - Verify `data/Department.txt`, `data/Medicine.txt`, `data/Prescription.txt` are created correctly

---

## Implementation Order (Suggested)

1. **Phase 1 - Foundation Models** (serialize/deserialize critical first)
   - Medicine.cpp
   - Department.cpp
   - Prescription.cpp

2. **Phase 2 - Repositories**
   - MedicineRepository.h/.cpp
   - DepartmentRepository.h/.cpp
   - PrescriptionRepository.h/.cpp

3. **Phase 3 - Services**
   - MedicineService.h/.cpp
   - DepartmentService.h/.cpp
   - PrescriptionService.h/.cpp
   - ReportGenerator.cpp

4. **Phase 4 - Integration**
   - Update Constants.h
   - Create data files
   - Write tests

---

## Summary

| Category | Count |
|----------|-------|
| New Headers | 6 files |
| New Implementations | 10 files |
| Modified Files | 2 files |
| New Test Files | 10 files |
| New Data Files | 3 files |
| **Total New/Modified** | **31 files** |

This is a significant implementation. Please review and let me know:
1. Which implementation scope you prefer (all at once vs. incremental)
2. Any specific features to prioritize
3. Any modifications to the proposed structure
