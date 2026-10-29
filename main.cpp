#include <print> 
#include <gtest/gtest.h>


int* helper() {
    int* p = new int{42}; 
    return p; // ownership of p is helper()
}

TEST(test_group_name, test_case_name) {
    // EXPECT_EQ(1,2);
    // ASSERT_EQ(1,2);
    int* x = helper();
    ASSERT_EQ(*x, 43); 
}