#include "ui/DisplayHelper.h"
#include "model/Patient.h"
#include "model/Doctor.h"
#include "advance/Medicine.h"
#include "common/Types.h"
#include <vector>
#include <iostream>

using namespace HMS;
using namespace HMS::UI;
using namespace HMS::Model;

int main() {
    std::cout << "=== DEMO TINH NANG PHAN TRANG ===\n\n";

    // Create 100 sample patients to demonstrate pagination
    std::vector<Patient> patients;
    for (int i = 1; i <= 100; ++i) {
        std::string id = "P" + std::to_string(1000 + i);
        std::string username = "patient" + std::to_string(i);
        std::string name = "Nguyen Van " + std::to_string(i);
        std::string phone = "09" + std::to_string(10000000 + i);
        std::string dob = "1990-01-" + (i % 28 == 0 ? "28" : (i % 28 < 10 ? "0" + std::to_string(i % 28) : std::to_string(i % 28)));

        patients.emplace_back(id, username, name, phone,
                             (i % 3 == 0) ? Gender::FEMALE : Gender::MALE,
                             dob, "Ha Noi", "");
    }

    std::cout << "Da tao " << patients.size() << " benh nhan mau.\n";
    std::cout << "Tính nang phan trang se hien thi 40 benh nhan moi trang.\n";
    std::cout << "Ban co the dieu huong bang [P]revious, [N]ext, [G]oto, [Q]uit\n\n";
    DisplayHelper::pause();

    // Display with pagination
    DisplayHelper::printPatientTable(patients);

    std::cout << "\n=== DEMO HOAN TAT ===\n";
    return 0;
}
