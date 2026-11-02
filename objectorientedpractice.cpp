#include <gtest/gtest.h>
#include <print> 
#include <iostream> 

// protected access level: only derived classes have access 
class Order { // interface
    public: // any user of this class has access
        Order(int id, int quantity) : m_id(id), m_quantity(quantity) {}
        virtual void execute() = 0; // pure virtual function, must be overriden

        virtual void printResult() {
            std::println("{} {}", m_id, m_quantity); 
        }

        virtual ~Order() {
            std::println("Order Destructor"); 
        }

    protected: // only current class has access
        int m_id;
        int m_quantity;
};

// derived class has all functionality and members that base class has 
class Market: public Order {
    public:
        Market(int id, int quantity) : Order(id, quantity) {}
        ~Market() {
            std::println("Market destructor"); 
        }
        void execute() override {
            std::println("Market Implementation"); 
        }

        void printResult() override {
            std::println("Market implementation");
            Order::printResult(); 
        }

        void function1() {}


}; 

class Limit: public Order {
    public: 
        Limit(int id, int quantity) : Order(id, quantity) {}

        ~Limit() {
            std::println("Limit destructor"); 
        }
        void execute() override {
            std::println("Limit Implementation"); 
        }

        void function2() {}
};

TEST(object_oriented_practice, test_1) {
    // Order o(1, 100); cannot create instance of class with pure virtual function
    Market m(1, 100);
    Limit l(2, 100);
    m.execute(); 
    l.execute(); 
    m.printResult(); 
    l.printResult(); 
}

TEST(object_oriented_practice, test_2) {
    Order* p1 = new Market(1, 100);
    Order* p2 = new Limit(2, 100); 
    delete p1; // first derived class is destroyed, then the base class
    delete p2; 
}

void processOrder(Order* order) {
    order->execute();
    order->printResult();  
}

TEST(object_oriented_practice, test_3) {
    Market* p1 = new Market(1, 100);
    Limit* p2 = new Limit(2, 100); 
    processOrder(p1);
    processOrder(p2); 
    delete p1; // first derived class is destroyed, then the base class
    delete p2; 
}

// --------------------- 
#pragma pack(push, 4)
struct Base {
    uint32_t val;

    virtual void print() { 
        std::cout << val;  
    }
    // functions do not change the size of classes/structs
    // when compiled, functions are converted to assembly, and are stored in the executable

    // when you have a virtual function, compiler creates a member called vtable
};
#pragma pack(pop)
/*
__declspec(align(4)) struct Base2 { only available in msvc compilers
    uint32_t val;

    virtual void print() { 
        std::cout << val;  
    }
    // functions do not change the size of classes/structs
    // when compiled, functions are converted to assembly, and are stored in the executable

    // when you have a virtual function, compiler creates a member called vtable
};
*/


TEST(object_oriented_practice, test_4) {
    EXPECT_EQ(sizeof(Base), 12); 
}

// ----------------------

/* 
Vehicle Fleet with Multiple Inheritance
• Objective: Multiple inheritance and the diamond problem resolution. 
• Task: Create a base class Vehicle with a fuelLevel property. 
Derive two classes: ElectricPowered (with batteryCapacity) and CombustionPowered (with tankSize). 
• Requirements: • Create a class HybridCar that inherits from both ElectricPowered and CombustionPowered. 
• Use virtual inheritance (virtual public) to avoid the diamond problem. 
• Write a method to display the total energy/fuel metrics.
*/

class Vehicle {
    public: 
        Vehicle(int fuelLevel): m_fuelLevel(fuelLevel) {}
        virtual void print() const = 0;
    protected: 
        int m_fuelLevel; 
};
class ElectricPowered: public Vehicle {
    public: 
        ElectricPowered(int batteryCapacity = 0) : Vehicle(batteryCapacity) {}
        virtual int getCapacity() const {
            return m_fuelLevel; 
        }
        void print() const override {
            std::cout << "Battery Capacity: " << m_fuelLevel << '\n'; 
        }
};
class CombustionPowered: public Vehicle {
    public: 
        CombustionPowered(int tankSize = 0) : Vehicle(tankSize) {}
        virtual int getTankSize() const {
            return m_fuelLevel; 
        }
        void print() const override {
            std::cout << "Tank Size: " << m_fuelLevel << '\n'; 
        }
}; 

class HybridCar: public ElectricPowered, public CombustionPowered {
    public: 
    HybridCar(int batteryCapacity, int tankSize) : ElectricPowered(batteryCapacity), CombustionPowered(tankSize) {}
    void print() const override {
        std::cout << "Battery Capacity: " << ElectricPowered::m_fuelLevel << '\n'; 
        std::cout << "Tank Size: " << CombustionPowered::m_fuelLevel << '\n'; 
    }
};

TEST(object_oriented_practice, test_5) {
    ElectricPowered electric1(4);
    EXPECT_EQ(electric1.getCapacity(), 4);  
    electric1.print(); 

    CombustionPowered combustion1(3); 
    EXPECT_EQ(combustion1.getTankSize(), 3); 
    combustion1.print(); 

    HybridCar hybrid1(7,6); 
    EXPECT_EQ(hybrid1.getCapacity(), 7); 
    EXPECT_EQ(hybrid1.getTankSize(), 6); 
    hybrid1.print(); 
}