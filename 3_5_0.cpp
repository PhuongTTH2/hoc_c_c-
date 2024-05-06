// Rule of Three: một lớp String cơ bản: hàm tạo, hàm hủy, hàm tạo sao chép và toán tử gán. Điều này đảm bảo rằng lớp String có thể được sao chép và gán một cách an toàn.
#include <cstring>

class String {
private:
    char* buffer;
public:
    // Constructor
    String(const char* str) {
        buffer = new char[strlen(str) + 1];
        strcpy(buffer, str);
    }

    // Destructor
    ~String() {
        delete[] buffer;
    }

    // Copy constructor
    String(const String& other) {
        buffer = new char[strlen(other.buffer) + 1];
        strcpy(buffer, other.buffer);
    }

    // Assignment operator
    String& operator=(const String& other) {
        if (this != &other) {
            delete[] buffer;
            buffer = new char[strlen(other.buffer) + 1];
            strcpy(buffer, other.buffer);
        }
        return *this;
    }
};
//Rule of Five: hàm tạo chuyển đổi và toán tử gán chuyển đổi.

#include <iostream>

class Resource {
private:
    int* data;
public:
    // Constructor
    Resource(int value) : data(new int(value)) {}

    // Destructor
    ~Resource() {
        delete data;
    }

    // Copy constructor
    Resource(const Resource& other) : data(new int(*other.data)) {}

    // Move constructor
    Resource(Resource&& other) noexcept : data(other.data) {
        other.data = nullptr;
    }

    // Assignment operator
    Resource& operator=(const Resource& other) {
        if (this != &other) {
            delete data;
            data = new int(*other.data);
        }
        return *this;
    }

    // Move assignment operator
    Resource& operator=(Resource&& other) noexcept {
        if (this != &other) {
            delete data;
            data = other.data;
            other.data = nullptr;
        }
        return *this;
    }

    // Getter   
    int getValue() const {
        return *data;
    }
};

int main() {
    Resource res1(5);
    Resource res2 = res1; // Copy constructor
    Resource res3(10);
    res3 = std::move(res1); // Move assignment operator

    std::cout << res2.getValue() << std::endl; // 5
    std::cout << res3.getValue() << std::endl; // 5 (after moving from res1)

    return 0;
}

//Rule of Zero
#include <iostream>
#include <vector>

class MyVector {
private:
    std::vector<int> data;
public:
    // Không cần khai báo bất kỳ hàm tạo, hàm hủy, hàm tạo sao chép hoặc toán tử gán

    // Phương thức thêm phần tử vào vector
    void add(int value) {
        data.push_back(value);
    }

    // Phương thức lấy kích thước của vector
    size_t size() const {
        return data.size();
    }
};

int main() {
    MyVector v;
    v.add(1);
    v.add(2);
    v.add(3);

    std::cout << "Size of v: " << v.size() << std::endl; // 3

    return 0;
}