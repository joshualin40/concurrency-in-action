# Chapter 2: Managing Threads

### Difference between `scoped_thread` and `thread_guard`?

- The key difference is **ownership**. Both classes ensure the thread is joined before exiting the scope.
- `thread_guard` takes a *reference*, so the thread is still accessible outside the guard.
- `scoped_thread` takes *complete ownership*, so the thread is not accessible outside the `scoped_thread`.

### Explain Listing 2.9 (page 32)

### Why does the `std::thread` destructor call `std::terminate()` instead of joining or detaching automatically?

- What problems would each of those defaults cause?
- How does `std::jthread` change this?

### When is `detach()` actually a good idea?

- What does a detached thread need to be safe (ownership of data, lifetime)?

### Why does `std::thread` copy its arguments by default?

`std::thread` copies its arguments into internal storage. Why did the designers choose copying by default instead of references? What would go wrong otherwise?

### Why is `std::thread` movable but not copyable?

- What would it even mean to copy a thread?

### How would you write a function that returns a `std::thread`, or a container that holds many threads?

- What does ownership look like in each case?

### What does `std::thread::hardware_concurrency()` tell you?

- Why might it be a bad idea to spawn exactly that many threads?

### If you split work across N threads (like the parallel accumulate example), how do you choose N?

- What overhead does each thread add?
- At what data size does threading stop paying off?

### How do exceptions behave inside a thread?

- If a thread function throws, what happens?
- How could you get that error back to the caller?

### What are `std::thread::id` values useful for?

- Where might you use them in real code, such as logging or tracking which thread owns a resource?

### Compare `thread_guard` and `scoped_thread`

- Why does taking ownership of the thread (by move) make `scoped_thread` safer?

### What happens if you pass a member function to `std::thread`?

- How does the object get passed, and what are the lifetime risks?

### For your matching engine: which parts could run on separate threads?

- Which data would those threads need to share?
