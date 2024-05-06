// tìm 3 số lớn nhất trong mảng nhưng không lập hết chỉ lập 3 lần để optimize performance.

#include <iostream>
#include <vector>

std::vector<int> findThreeLargestNumbers(std::vector<int>& nums) {
    int firstLargest = 1;
    int secondLargest = 1;
    int thirdLargest = 1;

    for (int num : nums) {
        if (num > firstLargest) {
            thirdLargest = secondLargest;
            secondLargest = firstLargest;
            firstLargest = num;
        } else if (num > secondLargest && num != firstLargest) {
            thirdLargest = secondLargest;
            secondLargest = num;
        } else if (num > thirdLargest && num != firstLargest && num != secondLargest) {
            thirdLargest = num;
        }
    }

    return {thirdLargest, secondLargest, firstLargest};
}

int main() {
    std::vector<int> nums = {5, 10, 3, 15, 20, 8, 25};
    std::vector<int> result = findThreeLargestNumbers(nums);

    std::cout << "Three largest numbers: ";
    for (int num : result) {
        std::cout << num << " ";
    }
    std::cout << std::endl;

    return 0;
}
//Tìm 3 số sao cho tổng bằng 1 số cho trước trong mảng.

#include <stdio.h>

void findThreeNumbersWithSum(int nums[], int n, int targetSum) {
    for (int i = 0; i < n - 2; ++i) {
        for (int j = i + 1; j < n - 1; ++j) {
            for (int k = j + 1; k < n; ++k) {
                if (nums[i] + nums[j] + nums[k] == targetSum) {
                    printf("Three numbers with sum %d: %d, %d, %d\n", targetSum, nums[i], nums[j], nums[k]);
                    return;
                }
            }
        }
    }
    printf("Khong tim thay ba so co tong bang %d\n", targetSum);
}

int main() {
    int nums[] = {2, 7, 11, 15, 8, 3, 1, 5};
    int n = sizeof(nums) / sizeof(nums[0]);
    int targetSum = 18;

    findThreeNumbersWithSum(nums, n, targetSum);

    return 0;
}