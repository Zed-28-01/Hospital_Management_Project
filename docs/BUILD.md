# Hospital Management System - Build Guide

## Prerequisites

- **CMake** >= 3.10
- **GCC** >= 14 (with C++23 support)
- **Google Test** (libgtest-dev)

The devcontainer configuration includes all prerequisites.

---

## Quick Start

```bash
# From project root directory
mkdir -p build && cd build
cmake ..
make HospitalApp
./HospitalApp
```

---

## Build Commands

### Configure Project

```bash
# Create build directory
mkdir -p build
cd build

# Configure with CMake (Debug mode - default)
cmake ..

# Configure with Release mode
cmake -DCMAKE_BUILD_TYPE=Release ..
```

### Build Targets

```bash
# Build main application only
make HospitalApp

# Build the static library (used by tests)
make HospitalLib

# Build tests only
make HospitalTests

# Build everything
make all

# Clean build artifacts
make clean
```

### Run Application

```bash
# From build directory
./HospitalApp
```

**Timezone Note:** The application uses system time to validate appointment time slots. If the server runs in UTC timezone, you need to set Vietnam timezone:

```bash
# Option 1: Run with Vietnam timezone (each time)
TZ='Asia/Ho_Chi_Minh' ./HospitalApp

# Option 2: Set permanently in shell (recommended)
echo "export TZ='Asia/Ho_Chi_Minh'" >> ~/.bashrc
source ~/.bashrc
./HospitalApp
```

### Run Tests

```bash
# Run all tests
./HospitalTests

# Run tests with verbose output
./HospitalTests --gtest_output=xml:test_results.xml

# Run specific test suite
./HospitalTests --gtest_filter=PatientTest.*

# Run with CTest
ctest

# Run with verbose CTest
ctest --verbose
```

---

## Build Configuration Summary

After running `cmake ..`, you'll see:

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

## Development Workflow

### Adding New Source Files

1. Add header file to appropriate `include/<layer>/` directory
2. Add source file to corresponding `src/<layer>/` directory
3. Run `cmake ..` again (or CMake will auto-detect new files)
4. Build with `make`

### Adding New Tests

1. Create test file in appropriate `test/<layer>/` directory
2. Follow Google Test naming conventions
3. Rebuild tests with `make HospitalTests`

### Directory Structure for Source Files

```
src/
├── model/          # Entity implementations
│   ├── Person.cpp
│   ├── Patient.cpp
│   ├── Doctor.cpp
│   ├── Admin.cpp
│   ├── Account.cpp
│   └── Appointment.cpp
│
├── dal/            # Data Access Layer
│   ├── AccountRepository.cpp
│   ├── PatientRepository.cpp
│   ├── DoctorRepository.cpp
│   ├── AppointmentRepository.cpp
│   └── FileHelper.cpp
│
├── bll/            # Business Logic Layer
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
├── ui/             # Presentation Layer
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

## Common Issues

### 1. CMake version error

```
CMake Error at CMakeLists.txt:1 (cmake_minimum_required):
  CMake 3.10 or higher is required.
```

**Solution:** Update CMake or use the devcontainer.

### 2. GTest not found

```
CMake Error: Could not find GTest
```

**Solution:** Install Google Test:
```bash
sudo apt-get install libgtest-dev
```

### 3. C++23 features not supported

```
error: 'expected' is not a member of 'std'
```

**Solution:** Use GCC 14 or higher. The devcontainer is pre-configured with GCC 14.

### 4. Linker errors for undefined references

If you see undefined reference errors after adding new source files:

```bash
# Clean and rebuild
cd build
rm -rf *
cmake ..
make all
```

---

## VS Code Integration

If using VS Code with the devcontainer:

1. Open Command Palette (Ctrl+Shift+P)
2. Select "CMake: Configure"
3. Select "CMake: Build" or press F7
4. Run with F5 (Debug) or Ctrl+F5 (Run)

---

## Release Build

For production/release builds:

```bash
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make all

# The executable will be optimized
./HospitalApp
```

---

## Installation (Optional)

```bash
# Install to system (default: /usr/local/bin)
sudo make install

# Install to custom location
cmake -DCMAKE_INSTALL_PREFIX=/custom/path ..
make install
```
