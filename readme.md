# HỆ THỐNG QUẢN LÝ BỆNH VIỆN (HOSPITAL MANAGEMENT SYSTEM)

## TỔNG QUAN

- **Lớp:** 24CTT1 – Nhóm 09
- **Mã lớp học phần:** CSC10003
- **Giảng viên:** Trần Duy Quang
- **Học kỳ:** HK1 2025-2026

### Thông tin nhóm

| STT | MSSV | Họ và Tên | Vai Trò |
|-----|------|-----------|---------|
| 1 | 24120209 | Vân Hoàng Nhật | **Nhóm Trưởng** |
| 2 | 24120027 | Lý Ngọc Châu | Thành Viên |
| 3 | 24120104 | Nguyễn Hữu Nghĩa | Thành Viên |
| 4 | 24120144 | Dương Ngọc Minh Thư | Thành Viên |
| 5 | 24120358 | Phùng Đức Tuấn Kiệt | Thành Viên |

---

## ĐỀ TÀI - HỆ THỐNG QUẢN LÝ BỆNH VIỆN (HOSPITAL MANAGEMENT SYSTEM)

### Mô tả

Ứng dụng **console C++23** quản lý bệnh viện, được xây dựng theo **kiến trúc 3 tầng (Presentation – Business – Data Access)**.

Hệ thống áp dụng các **Design Patterns** như **Singleton, Facade, Repository** và tuân thủ các **nguyên tắc SOLID**, nhằm đảm bảo tính mở rộng, bảo trì và chất lượng thiết kế phần mềm.

### Kiến trúc hệ thống
- **Presentation Layer**: Giao diện console, xử lý nhập/xuất dữ liệu người dùng
- **Business Logic Layer**: Xử lý nghiệp vụ, kiểm tra dữ liệu, điều phối luồng xử lý
- **Data Access Layer**: Truy cập và quản lý dữ liệu (Repository Pattern)

### Design Patterns áp dụng
- **Singleton**: Quản lý instance duy nhất cho các service/repository
- **Facade**: Đơn giản hóa giao tiếp giữa tầng giao diện và nghiệp vụ
- **Repository**: Tách biệt logic truy cập dữ liệu khỏi nghiệp vụ

### Chức năng chính:

- **Quản lý người dùng đa vai trò:** Admin, Bác sĩ, Nhân viên, Bệnh nhân
    - Đăng ký, đăng nhập, phân quyền theo role
    - Profile cá nhân, lịch sử hoạt động

- **Quản lý lịch hẹn khám bệnh:**
    - Đặt lịch, hủy lịch, sửa lịch khám
    - Xem lịch sử và lịch sắp tới

- **Quản lý khoa/phòng ban:**
    - CRUD khoa/phòng ban
    - Phân công bác sĩ vào khoa
    - Thống kê theo khoa

- **Quản lý thuốc:**
    - Quản lý kho thuốc, tồn kho
    - Cảnh báo hết hạn, cảnh báo tồn kho thấp
    - Kê đơn thuốc, xuất thuốc

- **Báo cáo và thống kê:**
    - Doanh thu theo ngày/tuần/tháng/quý/năm
    - Thống kê số lượng bệnh nhân, hiệu suất bác sĩ
    - Export báo cáo (txt, csv, html)

### Mục tiêu
- Áp dụng kiến thức **OOP, SOLID và Design Patterns** vào dự án thực tế
- Nâng cao khả năng thiết kế hệ thống và tổ chức mã nguồn C++
- Dễ dàng mở rộng và bảo trì trong tương lai

### Công nghệ sử dụng
- Ngôn ngữ: **C++23**
- Mô hình: **3-Tier Architecture**
- Paradigm: **Object-Oriented Programming**

## PHÂN CÔNG CÔNG VIỆC

### 1. Vân Hoàng Nhật - Nhóm Trưởng (20%)

**Trách nhiệm:**
- Quản lý dự án: Họp định kỳ, theo dõi tiến độ, báo cáo
- Module Doctor: `Doctor.h/cpp`, `DoctorRepository.h/cpp`, `DoctorService.h/cpp`
- Giao diện Menu: `ConsoleUI.h/cpp` - Menu tất cả roles, Login/Register
- Phân quyền: Role-based access control, điều hướng menu

### 2. Lý Ngọc Châu - Kiến Trúc (20%)

**Trách nhiệm:**
- Thiết kế kiến trúc 3-Tier, Class Diagram, xác định boundaries
- Áp dụng Design Patterns: Singleton, Facade (`HMSFacade.h/cpp`), Repository (`IRepository.h`)
- Tích hợp modules: Kết nối các module, xử lý dependencies
- Documentation: `ARCHITECTURE.md`, `ARCHITECTURE_vi.md`, diagrams
- Setup: CMake, DevContainer, coding convention

### 3. Nguyễn Hữu Nghĩa - Backend (20%)

**Trách nhiệm:**
- Data Access Layer hoàn chỉnh: `IRepository.h`, `FileHelper.h/cpp`
- Tất cả Repositories: Account, Patient, Doctor, Appointment, Department, Medicine, Prescription
- File Operations: Đọc/ghi TXT/CSV, loadFromFile(), saveToFile()
- Data queries: Search, filter, pagination, validation
- Testing: Unit tests DAL, integration tests, fixtures

### 4. Dương Ngọc Minh Thư - Features (20%)

**Trách nhiệm:**
- Appointment System: `Appointment.h/cpp`, `AppointmentRepository.h/cpp`, `AppointmentService.h/cpp`
- CRUD: Đặt/Hủy/Sửa lịch, xem lịch sử/sắp tới, validate slot
- Billing System: calculateTotalBill(), payment tracking, price management
- Department: `Department.h/cpp`, `DepartmentRepository.h/cpp`, `DepartmentService.h/cpp`
- Testing: Unit tests Appointment, test Patient flow, integration tests

### 5. Phùng Đức Tuấn Kiệt - Admin & Docs (20%)

**Trách nhiệm:**
- Admin Module: `Admin.h/cpp`, `AdminService.h/cpp` - CRUD Doctor, statistics
- Report Generator: `ReportGenerator.h/cpp` - Thống kê bệnh nhân, doanh thu, hiệu suất, export TXT/CSV/HTML
- Medicine System: `Medicine.h/cpp`, `MedicineRepository.h/cpp`, `MedicineService.h/cpp`
- Prescription System: `Prescription.h/cpp`, `PrescriptionService.h/cpp` - Kê đơn, xuất thuốc
- Documentation: `BUILD.md`, `BUILD_vi.md`, user manual, demo
- Utilities: `Constants.h`, `Types.h`, `Utils.h/cpp`

---

## TỈ LỆ ĐÓNG GÓP

| Thành Viên | Tỉ Lệ | Công Việc Chính |
|------------|-------|-----------------|
| Vân Hoàng Nhật | 20% | Nhóm trưởng, Doctor Module, Menu, Phân quyền |
| Lý Ngọc Châu | 20% | Kiến trúc 3-Tier, Design Patterns, Tích hợp, Docs |
| Nguyễn Hữu Nghĩa | 20% | Data Access Layer, All Repositories, File I/O |
| Dương Ngọc Minh Thư | 20% | Appointment, Billing, Department, Testing |
| Phùng Đức Tuấn Kiệt | 20% | Admin, Report Generator, Medicine, Prescription |
| **Tổng** | **100%** | |

---

## TỈ LỆ ĐIỂM

### Phương án: Chia đều điểm

**Lý do:**
- Công việc phân chia cân bằng về độ phức tạp và khối lượng
- Tất cả thành viên hoàn thành đầy đủ phần việc được giao
- Phối hợp chặt chẽ, code review, chất lượng đồng nhất
- Tham gia đầy đủ 6 tuần phát triển

---

## TÀI LIỆU

- [ARCHITECTURE_vi.md](docs/ARCHITECTURE_vi.md) - Kiến trúc chi tiết
- [BUILD_vi.md](docs/BUILD_vi.md) - Hướng dẫn build
- [Diagrams](docs/diagrams/) - Sơ đồ kiến trúc