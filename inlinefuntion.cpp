// một hàm được gọi
// 1, CPU sẽ lưu địa chỉ bộ nhớ của dòng lệnh hiện tại mà nó đang thực thi
// 2, sao chép các đối số của hàm trên ngăn xếp (stack)
// 3, chuyển hướng điều khiển sang hàm đã chỉ định.
// CPU sau đó thực thi mã bên trong hàm, lưu trữ giá trị trả về của hàm trong một vùng nhớ/thanh ghi và trả lại quyền điều khiển cho vị trí lời gọi hàm.

// Lợi ít

// Đối với các hàm lớn hoặc các tác vụ phức tạp, tổng chi phí của lệnh gọi hàm thường không đáng kể so với lượng thời gian mà hàm mất để chạy.
// Tuy nhiên, đối với các hàm nhỏ, thường được sử dụng, thời gian cần thiết để thực hiện lệnh gọi hàm thường nhiều hơn rất nhiều so với thời gian cần
// thiết để thực thi mã của hàm

#include <iostream>
using namespace std;
inline int max(int a, int b)
{
    return a > b ? a : b;
}
int main()
{
    cout << max(3, 6) << '\n';
    cout << max(6, 3) << '\n';
    return 0;
}
// =>

int main()
{
    cout << (3 > 6 ? 3 : 6) << '\n';
    cout << (6 > 3 ? 6 : 3) << '\n';
    return 0;
}

// Trình biên dịch có thể không thực hiện nội tuyến trong các trường hợp như:
// Hàm chứa vòng lặp (for, while, do-while).
// Hàm chứa các biến tĩnh.
// Hàm đệ quy.
// Hàm chứa câu lệnh switch hoặc goto.