#define CATCH_CONFIG_MAIN
#include "catch.hpp"

// Khai báo hàm cần test
int add(int a, int b);

TEST_CASE("add function works", "[add]") {
    REQUIRE(add(2, 3) == 5);
    REQUIRE(add(-1, 1) == 0);
    REQUIRE(add(0, 0) == 0);
}