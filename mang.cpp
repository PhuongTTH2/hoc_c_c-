// Vector:
// Ưu điểm:
// Cho phép truy cập ngẫu nhiên với độ phức tạp O(1).
// Hỗ trợ thêm, xóa ở cuối mảng với độ phức tạp trung bình là O(1).
// Dữ liệu được lưu trữ một cách liên tục trong bộ nhớ, dễ dàng để truy cập và duyệt qua.
// Nhược điểm:
// Thêm, xóa ở đầu hoặc giữa mảng có thể tốn độ phức tạp O(n) do việc phải dịch chuyển các phần tử.
// Khi cần thêm hoặc xóa phần tử giữa mảng một cách thường xuyên, vector có thể không phải là sự lựa chọn tốt nhất.
// Array:
// Ưu điểm:
// Cấp phát bộ nhớ tĩnh, cho phép truy cập ngẫu nhiên với độ phức tạp O(1).
// Kích thước cố định, hiệu suất cao hơn vector trong một số trường hợp.
// Nhược điểm:
// Kích thước cố định, không thể thay đổi kích thước sau khi được khởi tạo.
// Không cung cấp các phương thức cho việc thêm, xóa phần tử một cách linh hoạt như vector.
// List:
// Ưu điểm:
// Hỗ trợ thêm, xóa ở bất kỳ vị trí nào trong danh sách với độ phức tạp O(1).
// Không tốn chi phí dịch chuyển phần tử khi thêm, xóa.
// Nhược điểm:
// Truy cập ngẫu nhiên chậm hơn với độ phức tạp O(n).
// Sử dụng nhiều bộ nhớ hơn so với vector vì mỗi phần tử cần một liên kết đến phần tử tiếp theo.
// Map:
// Ưu điểm:
// Cung cấp cặp key-value, cho phép truy cập nhanh chóng theo key với độ phức tạp trung bình O(log n).
// Dữ liệu được tổ chức dưới dạng cây cân bằng, giữ cho thời gian tìm kiếm ổn định.
// Nhược điểm:
// Dùng nhiều bộ nhớ hơn so với các cấu trúc dữ liệu không cần sắp xếp như unordered_map.
// Thao tác thêm, xóa có thể tốn kém hơn so với unordered_map.
// Set:
// Ưu điểm:
// Lưu trữ các giá trị duy nhất, không cho phép các phần tử trùng lặp.
// Hỗ trợ các phép toán tập hợp như giao, hợp, hiệu, ...
// Nhược điểm:
// Thao tác thêm, xóa có thể tốn độ phức tạp O(log n) tùy thuộc vào cài đặt cụ thể.
// Dùng nhiều bộ nhớ hơn so với các cấu trúc dữ liệu không cần sắp xếp như unordered_set.

// vector sử dụng bộ nhớ liên tục, tức là các phần tử được lưu trữ dọc theo một vùng bộ nhớ duy nhất. Điều này cho phép truy cập ngẫu nhiên nhanh chóng và hiệu quả.
// list sử dụng bộ nhớ không liên tục, mỗi phần tử được lưu trữ trong một vùng bộ nhớ riêng biệt và có một con trỏ chỉ đến phần tử tiếp theo. Điều này cho phép thêm và xóa phần tử ở bất kỳ vị trí nào trong danh sách một cách hiệu quả.

#include <iostream>
#include <vector>
#include <array>
#include <list>
#include <map>
#include <set>

int main() {
    // Ví dụ về sử dụng vector
    std::vector<int> vec = {1, 2, 3, 4, 5};
    vec.push_back(6); // Thêm phần tử vào cuối vector
    vec.pop_back();   // Xóa phần tử cuối cùng của vector

    // Ví dụ về sử dụng array
    std::array<int, 5> arr = {1, 2, 3, 4, 5};
    // Kích thước của array cố định là 5, không thể thay đổi

    // Ví dụ về sử dụng list
    std::list<int> li = {1, 2, 3, 4, 5};
    li.push_front(0); // Thêm phần tử vào đầu danh sách
    li.insert(++li.begin(), 6); // Thêm phần tử vào vị trí thứ hai

    // Ví dụ về sử dụng map
    std::map<std::string, int> mp;
    mp["apple"] = 5;
    mp["banana"] = 10;
    mp["orange"] = 7;

    // Ví dụ về sử dụng set
    std::set<int> st = {3, 1, 4, 1, 5, 9, 2};
    st.insert(6); // Thêm phần tử vào tập hợp
    st.erase(4);  // Xóa phần tử khỏi tập hợp

    // In ra các phần tử của vector
    std::cout << "Vector: ";
    for (const auto& elem : vec) {
        std::cout << elem << " ";
    }
    std::cout << std::endl;

    // In ra các phần tử của list
    std::cout << "List: ";
    for (const auto& elem : li) {
        std::cout << elem << " ";
    }
    std::cout << std::endl;

    // In ra các phần tử của map
    std::cout << "Map: ";
    for (const auto& pair : mp) {
        std::cout << "(" << pair.first << ", " << pair.second << ") ";
    }
    std::cout << std::endl;

    // In ra các phần tử của set
    std::cout << "Set: ";
    for (const auto& elem : st) {
        std::cout << elem << " ";
    }
    std::cout << std::endl;

    return 0;
}