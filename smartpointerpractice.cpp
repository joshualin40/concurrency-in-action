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

// Rule of 5: if you define one, you must define all 5 
    // copy constructor
    // copy assignment
    // move constructor 
    // move assignment
// practically, when defining classes you want to follow the rule of zero: don't define any of them 
struct MyUniquePtr { // TODO: complete implementation
    int* p;
    MyUniquePtr(const int& val) {
        p = new int{val};
    }
    ~MyUniquePtr() {
        delete p;
    }
    // MyUniquePtr cannot be copied. 
    MyUniquePtr(const MyUniquePtr& other) = delete; 
    MyUniquePtr operator=(MyUniquePtr other) = delete; 

    MyUniquePtr(MyUniquePtr&& other) { // MyUniquePtr&& & -> MyUniquePtr& 
        p = other.p; 
        other.p = nullptr; 
    }

    MyUniquePtr operator=(MyUniquePtr&& other) {
        auto val = *(other.p); 
        other.p = nullptr; 
        return MyUniquePtr(val); 
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

MyUniquePtr helper5() {
    MyUniquePtr a(4); 
    (*(a.p)) *= 2;

    MyUniquePtr b(std::move(a));
    return b; 
}

TEST(smart_pointer_practice, test_unique_pointer) {
    auto y = helper5(); 
    ASSERT_EQ(*(y.p), 8); 
}