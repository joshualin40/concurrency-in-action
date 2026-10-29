#include <gtest/gtest.h>
#include <memory> 

void helper1() {
    int* p = new int{3};
    (*p) *= 2;
    delete p;  
}

void helper2() {
    std::unique_ptr<int> p = std::make_unique<int>(4);
    (*p) *= 2;
}

struct MyUniquePtr {
    int* p;
    MyUniquePtr(const int& val) {
        p = new int{val};
    } 
    ~MyUniquePtr() {
        delete p;
    }
};

void helper3() {
    MyUniquePtr p(4); 
    (*(p.p)) *= 2;
}

std::shared_ptr<int> helper4() {
    std::shared_ptr<int> p2 = std::make_shared<int>(4); 
    (*p2) *= 2;
    return p2; 
}

TEST(smart_pointer_practice, test_unique_pointer) {
    auto x = helper4();
    ASSERT_EQ(*x, 8); 
}