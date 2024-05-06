// File: my_functions.h
#ifndef MY_FUNCTIONS_H
#define MY_FUNCTIONS_H

void myFunction(); // Khai báo hàm

#endif

// File: main.cpp
#include "my_functions.h"

int main() {
    myFunction(); // Gọi hàm từ tệp khác
    return 0;
}

// File: my_functions.cpp
#include <iostream>
void myFunction() {
    std::cout << "Hello from myFunction!" << std::endl;
}

// File: main.cpp
#include <iostream>
#include "my_functions.h" // Định nghĩa khai báo hàm

int main() {
    myFunction(); // Gọi hàm từ thư viện động
    return 0;
}


// File: my_functions.cpp
#include <iostream>
namespace MyNamespace {
    void myFunction() {
        std::cout << "Hello from myFunction!" << std::endl;
    }
}

// File: main.cpp
#include <iostream>
#include "my_functions.h" // Định nghĩa khai báo hàm

int main() {
    MyNamespace::myFunction(); // Gọi hàm từ namespace
    return 0;
}