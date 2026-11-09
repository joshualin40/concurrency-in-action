#include <gtest/gtest.h>

#include <exception>
#include <print>
#include <stdexcept>
#include <thread>
#include <mutex>
#include <numeric>

// in the case that data is not read only and that multiple threads are modifying the same data, we must
// take care to ensure that everything works out OK

// invariants: statements that are always true about a particular data structure

// race condition: outcome depends on which thread gets there first.
// anything where the outcome depends on the relative ordering of execution of operations on two or more threads
// often hard to find and duplicate: window of opporunity is small
// can often disappear when run with a debugger as the debugger affects timing of program

// simplest way to deal with invairants:
/*
 wrap your data structure with a protection mechanism to ensure that only the
thread performing a modification can see the intermediate states where the invariants
are broken. (ie. a mutex, locks)
*/

// another way: modify design of data structure and invariants such that modifications are done in a series of
// indivisible changes: lock-free programming

// mutual exclusion: only one thread and access shared resource/critical section of code

// lock_guard: implements RAII idiom for mutex, locks and unlocks mutex upon construction and destruction of object

// C++17 has template argument deduction:
namespace ex1
{
    std::mutex mu;
    void foo()
    {
        {
            std::lock_guard<std::mutex> lock(mu);
        }
        {
            std::lock_guard lock2(mu); // equivalent (CTAD deduces std::mutex)
        }
        // note: locking mu twice in the same scope would deadlock (std::mutex is not recursive)
    }
}

// protecting data with mutex requires careful interface design. eg if the function we are protecting wiht a mutex
// returns a pointer or a reference to the protected data, it is accessible without locking the mutex

namespace ex2
{
    int protected_int{};
    std::mutex mu;

    int *f()
    {
        std::lock_guard lock(mu);
        // do something to the int here that is protected
        ++protected_int;
        return &protected_int; // the protected int is now accessible outside of this function
    }

    void bad()
    {
        int *p = f(); // lock released when f returns
        *p = 42;      // unprotected write: race with any other thread calling f()
    }
}

// it’s also important to check that they don’t pass these pointers or
// references in to functions they call that aren’t under your control.

// "dont pass pointers and references to protected data outside the scope of a lock, whether by returning them
// from a function, storing them in externally visible memory, or passing them as arguments to user supplied functions

// spotting race conditions inherent in interfaces

#include <exception>
#include <memory>
#include <mutex>
#include <stack>
namespace ex3
{
    struct empty_stack : std::exception
    {
        const char *what() const noexcept; // custom exception overriding standard what() function of std::exception
    };
    template <typename T>
    class threadsafe_stack
    {
    private:
        std::stack<T> data;
        mutable std::mutex m;

    public:
        threadsafe_stack() {}
        threadsafe_stack(const threadsafe_stack &other)
        {
            std::lock_guard lock(other.m);
            data = other.data;
        }
        threadsafe_stack &operator=(const threadsafe_stack &) = delete; // delete assingment
        void push(T new_value)
        {
            std::lock_guard lock(m);
            data.push(std::move(new_value));
        }
        std::shared_ptr<T> pop()
        {
            std::lock_guard lock(m);
            if (data.empty())
                throw empty_stack();
            std::shared_ptr<T> const res(std::make_shared<T>(data.top()));
            data.pop();
            return res;
        }
        void pop(T &value)
        {
            std::lock_guard lock{m};
            if (data.empty())
                throw empty_stack();
            value = data.top(); // is this line even needed?
            data.pop();
        }
        bool empty() const
        {
            std::lock_guard lock(m);
            return data.empty();
        }
    };
}

// deadlock: each of a pair of threads needs to lock both of a pair of
// mutexes to perform some operation, and each thread has one mutex and is waiting
// for the other. just think about the two-children drum/drumstick analogy
// common advice to avoid is to always lock A before B
// however can backfire

namespace ex4
{
    struct X
    {
    };
    void exchange(X &lhs, X &rhs) {} // say we have some exchange function that always locks lhs before rhs
    /*
    1. Thread A calls exchange(Object 1, Object 2);
    • Following the rule, Thread A successfully locks the mutex for Object 1.
    2. Thread B calls exchange(Object 2, Object 1);
    • Following the rule, Thread B successfully locks the mutex for Object 2.

    Deadlock!
    */
}

// std::lock locks multiple lockable's using a deadlock avoidance algorithm
namespace ex5
{
    std::mutex m1;
    std::mutex m2;
    void foo()
    {
        // ⚠️ RISKY IF EXCEPTIONS OCCUR BETWEEN LINES
        std::lock(m1, m2);

        // If an exception or early return happens exactly here,
        // lhs.m and rhs.m stay locked forever!

        std::unique_lock<std::mutex> lock1(m1, std::adopt_lock);
        std::unique_lock<std::mutex> lock2(m2, std::adopt_lock);
    }
}
namespace ex6
{
    std::mutex m1;
    std::mutex m2;
    void foo()
    {
        //  EXCEPTION-SAFE (C++11)
        std::unique_lock<std::mutex> lock1(m1, std::defer_lock);
        std::unique_lock<std::mutex> lock2(m2, std::defer_lock);

        // std::lock can safely lock unique_locks.
        // If an exception is thrown inside here, RAII ensures everything is cleaned up.
        std::lock(lock1, lock2);
    }
}

namespace ex6
{
    class some_big_object
    {
    };
    void swap(some_big_object &lhs, some_big_object &rhs) {}
    class X
    {
    private:
        some_big_object some_detail;
        std::mutex m;

    public:
        X(some_big_object const &sd) : some_detail(sd) {}
        friend void swap(X &lhs, X &rhs)
        {
            if (&lhs == &rhs)
                return;
            std::lock(lhs.m, rhs.m);
            std::lock_guard<std::mutex> lock_a(lhs.m, std::adopt_lock);
            std::lock_guard<std::mutex> lock_b(rhs.m, std::adopt_lock);
            swap(lhs.some_detail, rhs.some_detail);
        }
    };
}

// deadlocks are nasty problems!

// deadlocks can also occur with threads in the absence of locks
// don't wait for a thread if there's a chance it's waiting for you
// prevent deadlocks:
// avoid nested locks
// avoid calling user-generated code after locking/holding a lock

// acquire locks in fixed order
// think about stack

// think about swap operation
// the fixed order can lead to deadlock, luckily scoped_lock acquiring both at once solves this

// think about DLL
// if theres a mutex per node, threads traversing the list can get stuck in a classic deadlock
// scenario rather easily
// same with deleting a Node: thread deleting node can deadlock with other tread traversing list
// one way to prevent deadlock is to define an order of traversal

// deadlock hierarchy
// define with hierarchical_mutex type
// in book example we see a three hiearchies of functions and their related mutexes

// std::lock_guard<> template can also work with user defined mutex types as long as the type
// has lock(), unlock(), and try.lock() methods

// use cases for lock_guard vs unique_lock
// unique_lock: not necessarily owning
// can defer locking with std::defer_lock
// can take already locked mutexes with std::adopt_lock.
// has internal bool flag indicate whether it owns the lock it wraps
/*
How the Flag Changes:
• On Construction:
    • If you use std::defer_lock, the flag is set to false.
    • If you lock normally or use std::adopt_lock, the flag is set to true.
• During Execution:
    • If you call lock.unlock(), the flag drops to false.
    • If you call lock.lock(), the flag flips to true.
• On Destruction:
    • The destructor checks this flag: if (owns_lock_) { mtx->unlock(); }.
    If the flag is false, the destructor does absolutely nothing to the mutex.
*/

// lock_guard: strictly owning
// must unlock lock in destructor
// less overhead

// -------Transferring mutex ownership between scopes ------------- //

// function can transfer lock mutex and transfer ownership of lock to caller

namespace ex7
{
    // gateway pattern: takes some mutex and returns lock to caller
    std::unique_lock<std::mutex> get_lock()
    {
        extern std::mutex some_mutex;
        std::unique_lock<std::mutex> lk(some_mutex);
        // prepare data
        return lk;
    }

    // gateway pattern:
}

// A fine-grained lock protects a small amount of data, and a coarse-grained lock
// protects a large amount of data.
// granularity isn't just about the amount of data, it is also about how long the lock is held
// and what operations are performed. it is a performance concept

// however it is not ONly a performance concept:
// too fine a granularity can : An operation that should happen as one atomic step gets broken into
// pieces, and other threads can change things in between, as in the operator== example.

// what if shared data is read-only? then it only needs protection during initialization.
// in this case, locking a mutex during reads is an unnecessary performance hit
// c++ provides a mechanicsm for protecting data only during initialization
//

// std::call_once calls a function once and indicates to all other threads through the once_flag
// that it has already been called. Other threads wait until it finishes and are guaranteed
// to see its results.

namespace
{
    namespace with_mutex
    {

        std::unique_ptr<std::ofstream> log_file;
        std::mutex log_mutex;

        void log(const std::string &msg)
        {
            std::unique_lock<std::mutex> lk(log_mutex); // every call locks
            if (!log_file)
            {
                log_file = std::make_unique<std::ofstream>("mutex.log");
            }
            lk.unlock();
            // write msg (needs its own sync if multiple threads write)
        }

    } // namespace with_mutex

    namespace with_call_once
    {

        std::unique_ptr<std::ofstream> log_file;
        std::once_flag log_flag;

        void open_log()
        {
            log_file = std::make_unique<std::ofstream>("call_once.log");
        }

        void log(const std::string &msg)
        {
            std::call_once(log_flag, open_log); // opens once, cheap check after
            // write msg (needs its own sync if multiple threads write)
        }

    } // namespace with_call_once

} // namespace

// page 66