#include "../include/Doctor.h" // Nối với file .h

Doctor::Doctor(std::string name) : m_name(name) {}

void Doctor::introduce() {
    std::cout << "Xin chao, toi la Bac si: " << m_name << std::endl;
}