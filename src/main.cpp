#include <iostream>
#include "../include/Doctor.h" // Gọi file header vào

// Code nay do viet de test quy trinh

int main() {
    Doctor doc("Nhat"); // Tạo bác sĩ tên Nhật
    Doctor doc2("Ket");
    doc.introduce();
    doc2.introduce();    // Yêu cầu bác sĩ giới thiệu
    return 0;
}