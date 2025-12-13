#include <iostream>
#include <print>
#include <expected>
#include <string>
#include <vector>
#include <ranges>

// 1. std::expected - xử lý lỗi kiểu mới
std::expected<int, std::string> divide(int a, int b) {
    if (b == 0) 
        return std::unexpected("Khong the chia cho 0!");
    return a / b;
}

// 2. Deducing this
struct Counter {
    int value = 0;
    
    // C++23: explicit object parameter
    auto& increment(this auto& self) {
        ++self.value;
        return self;
    }
};

int main() {
    // Test std::print và std::println (C++23)
    std::println("=== TEST C++23 voi GCC 14 ===");
    std::println("");
    
    // Test std::expected
    std::println("1. Test std::expected:");
    auto result1 = divide(10, 2);
    if (result1) {
        std::println("   10 / 2 = {}", *result1);
    }
    
    auto result2 = divide(10, 0);
    if (!result2) {
        std::println("   Loi: {}", result2.error());
    }
    
    // Test std::print (không xuống dòng)
    std::println("");
    std::println("2. Test std::print:");
    std::print("   Hello ");
    std::print("World ");
    std::println("!!!");
    
    // Test ranges với C++23
    std::println("");
    std::println("3. Test ranges:");
    std::vector<int> nums = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::print("   So chan: ");
    for (int n : nums | std::views::filter([](int x) { return x % 2 == 0; })) {
        std::print("{} ", n);
    }
    std::println("");
    
    // Test size_t literal (C++23)
    std::println("");
    std::println("4. Test size_t literal:");
    auto idx = 42uz;
    std::println("   idx = {}, sizeof = {} bytes", idx, sizeof(idx));
    
    // Test deducing this
    std::println("");
    std::println("5. Test deducing this:");
    Counter c;
    c.increment().increment().increment();
    std::println("   Counter value = {}", c.value);
    
    // Test std::format trong print
    std::println("");
    std::println("6. Test formatting:");
    double pi = 3.14159265359;
    std::println("   Pi = {:.4f}", pi);
    std::println("   Hex 255 = {:#x}", 255);
    std::println("   Binary 10 = {:#b}", 10);
    
    std::println("");
    std::println("=== TAT CA TINH NANG C++23 HOAT DONG TOT! ===");
    
    return 0;
}

/*
Go cau lenh sau day vao terminal bash de xem lieu co cai thanh cong g++14 va c++23 hay chua

g++ -std=c++23 test/test_cpp23_full.cpp -o test_cpp23_full && ./test_cpp23_full
*/