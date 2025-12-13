#pragma once
#include <string>
#include <iostream>

class Doctor {
public:
    Doctor(std::string name);
    void introduce(); // Hàm giới thiệu

private:
    std::string m_name;
};