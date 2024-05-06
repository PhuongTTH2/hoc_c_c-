1. const int a;
2. int const a;
3. const int *a;
4. int * const a;
5. int const * a const;

1 “a” là một số nguyên không đổi.
2 Tương tự như thứ nhất, “a” là một số nguyên không đổi.
3 Ở đây “a” là con trỏ tới một số nguyên const, giá trị của số nguyên không thể sửa đổi được nhưng con trỏ có thể sửa đổi được.
4 Ở đây “a” là một con trỏ hằng tới một số nguyên, giá trị của số nguyên nhọn có thể sửa đổi được, nhưng con trỏ thì không thể sửa đổi được.
5 Ở đây “a” là một con trỏ const tới một số nguyên const, có nghĩa là cả giá trị của số nguyên nhọn và con trỏ đều không thể sửa đổi được.


