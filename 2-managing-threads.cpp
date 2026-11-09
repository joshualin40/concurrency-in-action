#include <gtest/gtest.h>

#include <exception>
#include <print>
#include <stdexcept>
#include <thread>
#include <numeric> 

// ============================================================================
// Chapter 2: Managing threads
// ============================================================================

void do_something()
{
    std::print("Hello");
}

class background_task
{
public:
    void operator()() const
    {
        do_something();
    }
};

// ----------------------------------------------------------------------------
// Launching threads
// ----------------------------------------------------------------------------
TEST(chapter_2, test_1)
{
    // Point: std::thread works with any callable type
    background_task f;
    std::thread my_thread(f);
    my_thread.join();

    // Point: be careful of most-vexing-parse: when compiler interprets object
    // instantiation as function declaration
    //
    //   std::thread my_thread(background_task());  // ambiguous, can be interpreted as function declaration
    std::thread my_thread2{background_task()}; // use {}
    my_thread2.join();
    // lambda expressions can avoid this problem
}

/*
 * ----------------------------------------------------------------------------
 * Notes: join vs. detach
 * ----------------------------------------------------------------------------
 * - Must make decision to correctly join/detach thread before object is destroyed.
 *     - If you detach and thread is still running, it may continue after the
 *       object is destroyed and only stops when function returns.
 *     - The Detach Exception: if you explicitly call .detach() before the object
 *       is destroyed, the OS thread will survive and keep running in the background.
 *
 * - join() waits for thread to finish. detach() completely forgets about it.
 *
 * - If we detach(), we must make sure that the data accessed by the thread is valid.
 *     - It's a bad idea to create a thread within a function that has access to
 *       the local variables in that function, unless the thread is guaranteed to
 *       finish before the function exits.
 *
 * - Thread can only be joined once, make sure to check joinable().
 *
 * - If the function in thread throws exception, then the program can terminate
 *   before thread is joined, so make sure to catch exceptions and join in that case.
 *
 * - If we want to make sure thread is finished at the end of function scope, we
 *   can use a thread_guard which utilizes the RAII idiom.
 *
 * - Detached threads are called daemon threads. Ownership and control are passed
 *   over to the C++ runtime library.
 *
 * ----------------------------------------------------------------------------
 * Notes: passing arguments
 * ----------------------------------------------------------------------------
 * - std::thread constructor copies each argument into the new thread's internal
 *   storage first, and then passes them to f.
 * - std::thread passes copied arguments as rvalues. When you have non const ref
 *   in thread function, use std::ref.
 *
 * ----------------------------------------------------------------------------
 * Notes: ownership
 * ----------------------------------------------------------------------------
 * - std::thread can be moved to transfer ownership.
 * - Semantics: a std::thread is not the same as a thread. A std::thread is a
 *   wrapper that manages an OS-level thread. The lifetime of a std::thread
 *   object is not necessarily tied to the thread it's managing.
 */

// ----------------------------------------------------------------------------
// Transferring ownership
// ----------------------------------------------------------------------------
void some_function() {}
void some_other_function() {}

TEST(chapter_2, test_2)
{
    std::thread t1(some_function);
    std::thread t2 = std::move(t1);
    t1 = std::thread(some_other_function); // moving from temporaries is automatic and implicit

    std::thread t3;
    t3 = std::move(t2);

    // t1 = std::move(t3);  // tries to move thread associated with t3 to t1, which
    //                      // already has an associated thread. std::terminate is called

    t3.join();
    t1.join();
}

// Ownership can readily be transferred out of a function, eg.
namespace ex1
{

    void some_function() {}

    std::thread f()
    {
        // passes the responsibility of managing its lifecycle (joining or detaching)
        // to its caller
        return std::thread(some_function);
    }

    void some_other_function(int a) {}

    std::thread g()
    {
        std::thread t(some_other_function, 42);
        return t;
    }

    void foo()
    {
        auto x = g(); // return value of g is moved into x
        x.join();
    }

} // namespace ex1

// Functions can also accept std::thread as parameters.
/*
 * Aside: remember
 *   Passing a Temporary:      f(std::thread(some_function));  // no need to call move as we are passing in an rvalue temp
 *   Passing a Named Variable: f(t);  (without std::move)      // does not compile if t is a move-only type
 */

// ----------------------------------------------------------------------------
// thread_guard: uses RAII idiom to ensure that threads are joined before they
// go out of scope
//
//   note: thread_guard doesn't own the thread
//   note: takes a reference to thread
//   why?  eg. if program throws exception, the t.join(); line below where the
//         exception was may not execute, leaving a dangling thread
// ----------------------------------------------------------------------------
class thread_guard
{
public:
    explicit thread_guard(std::thread &t) : m_t(t) {}

    ~thread_guard()
    {
        if (m_t.joinable())
            m_t.join();
    }

    thread_guard(const thread_guard &other) = delete;
    thread_guard &operator=(const thread_guard &other) = delete;

private:
    std::thread &m_t;
};

// ----------------------------------------------------------------------------
// scoped_thread: now owns the thread
//
// Difference between scoped_thread and thread_guard:
//   "avoids any unpleasant consequences should the thread_guard object outlive
//    the thread it was referencing, and it also means that no one else can join
//    or detach the thread once ownership has been transferred into the object."
// ----------------------------------------------------------------------------
class scoped_thread
{
public:
    explicit scoped_thread(std::thread t_) : t(std::move(t_))
    {
        if (!t.joinable())
            throw std::logic_error("No thread");
    }

    ~scoped_thread()
    {
        // attention: we don't have to check if joinable because scoped_thread
        // completely encapsulates the thread it owns
        t.join();
    }

    scoped_thread(scoped_thread const &) = delete;
    scoped_thread &operator=(scoped_thread const &) = delete;

private:
    std::thread t;
};

// Note: we have std::jthread, which automatically joins at the end of its lifetime.

// Aside: emplace_back vs push_back and its significance

namespace ex2
{
    void do_work(unsigned id) {}
    void f()
    {
        std::vector<std::thread> threads;
        for (unsigned i = 0; i < 20; ++i)
        {
            threads.emplace_back(do_work, i);
        }
        for (auto &entry : threads)
            entry.join();
    }

    // push_back: expects already-constructed object
    // compiler creates temporary on the stack and moves it
    // emplace_back: constructs object directly inside the vector's memory allocatino
    // slightly more performant, as is preferred in modern cpp

    // note: push_back immediately fails with immovable types, but emplace_back likely fails later when
    // vector expands its capacity and must find a bigger block of contiguous memory

    // vector is also a move-aware container, meaning that instead of deep copying when resizing it uses move semantics
    

} // namespace ex2

TEST(chapter_2, test_3)
{
    std::cout << std::thread::hardware_concurrency() << '\n';
}

/* Aside: what is std::accumulate?
    std::accumulate takes your init value and
    sequentially combines it with every single element in the range from start to finish.
*/

namespace ex3
{
    template <typename Iterator, typename T>
    struct accumulate_block
    {
        void operator()(Iterator first, Iterator last, T &result)
        {
            result = std::accumulate(first, last, result);
        }
    };

    template <typename Iterator, typename T>
    T parallel_accumulate(Iterator first, Iterator last, T init)
    {
        unsigned long const length = std::distance(first, last);
        if (!length)
            return init;
        unsigned long const min_per_thread = 25;
        unsigned long const max_threads =
            (length + min_per_thread - 1) / min_per_thread;
        unsigned long const hardware_threads =
            std::thread::hardware_concurrency();
        unsigned long const num_threads =
            std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads);
        unsigned long const block_size = length / num_threads;
        std::vector<T> results(num_threads);
        std::vector<std::thread> threads(num_threads - 1);
        Iterator block_start = first;
        for (unsigned long i = 0; i < (num_threads - 1); ++i)
        {
            Iterator block_end = block_start;
            std::advance(block_end, block_size);
            threads[i] = std::thread(
                accumulate_block<Iterator, T>(),
                block_start, block_end, std::ref(results[i]));
            block_start = block_end;
        }
        accumulate_block<Iterator, T>()(
            block_start, last, results[num_threads - 1]);
        for (auto &entry : threads)
            entry.join();
        return std::accumulate(results.begin(), results.end(), init);
    }
} // namespace ex3
TEST(chapter_2, test_4)
{
    std::cout << std::this_thread::get_id(); 
    // can use virtually any comparison between std::thread::id's
    // can be used as hash keys
    // can associate behaviour/data with certain threads through identifier (eg. "used as keys into associative containers where spe-
    // cific data needs to be associated with a thread")
}
