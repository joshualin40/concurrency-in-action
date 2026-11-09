# Chapter 3: Sharing Data Between Threads

## My Questions

### Why shouldn't you pass pointers/references outside the scope of a lock?

### How could you get race conditions in a stack container?

- Since stack operations such as `push()` and `pop()` are non-atomic, several threads using the same stack can hit a failure.
- E.g. if Thread A and Thread B both call `pop()` on a 1-item stack, this can happen:
  1. Thread A checks if the stack is empty. It is not.
  2. Thread B checks if the stack is empty. It is not.
  3. Thread A pops the last item of the stack.
  4. Thread B tries to pop an empty stack, and bad things happen.

**Why is this a race condition?** Recall the definition: the outcome depends on which thread gets there first. If Thread A pops before Thread B checks `empty()`, then it is fine. But whether Thread A gets there first is often indeterminate.

### What are possible solutions to the non-thread-safe interface of `stack` for designing a thread-safe stack?

- **Combine `pop` and `top`.** Drawbacks?
  - If we copy-construct a new variable from the popped element and the copy constructor throws, the data is popped but the copy was unsuccessful.
  - This is why `pop` and `top` are separated in the design of `std::stack`, but that separation can cause a race condition.
- **Pass in a reference**
  - Pass a reference to the variable you want to receive the popped value.
- **Require a no-throw copy constructor / move constructor**
  - As described above, the problem only occurs if the copy constructor throws, so one option is to restrict the stack to types whose copy constructors don't throw.
    - `std::is_nothrow_copy_constructible` and `std::is_nothrow_move_constructible`
- **Return a pointer to the popped item**
  - Managing a pointer can be more expensive than the actual type for simple types such as `int`s.

### What does `mutable` mean?

- If we have a `const` stack object with an internal `mutable` mutex, `mutable` lets the mutex be locked and unlocked within the stack's methods.
- Other usages of `mutable`?
  - Allows differentiation of *bitwise const* and *logical const*:
    - **Logical const:** the object cannot change from the perspective of all public APIs.
    - **Bitwise const:** no member variable of the object can be modified.

### Why wrap the returned pointer in a `shared_ptr` in the thread-safe stack's `pop()`?

- This allows multiple ownership of the object.
- Prevents race conditions and avoids resource leaks.

### In what scenarios would we need more than two mutexes?

### What does it mean to lock at a small/large granularity, and what are the implications of this on the Linux kernel?

### What does `std::lock` do?

### `std::lock` vs `std::scoped_lock`?

- `scoped_lock` handles RAII and locking in one step.
- After calling `std::lock`, you have to wrap the mutexes in a lock again and pass `std::adopt_lock` as the second parameter to indicate they were already locked by `std::lock`.

### In the implementation of `hierarchical_mutex`, why does each thread need a `thread_local` hierarchy value?

> "allows the code to check the behavior of each thread separately, and the code for each mutex can check whether or not the current thread is allowed to lock that mutex."

### What are the benefits of `hierarchical_mutex`?

### Compare all of the locking-related APIs and explain what they do and when to use them

- `std::lock`
- `std::unique_lock`
- `std::scoped_lock`
- `std::lock_guard`

---

## More Questions (by Chapter 3 Section)

### 3.1 Problems with sharing data

1. What is an *invariant*? Why are broken invariants at the root of most race conditions?
   - Give an example using deleting a node from a doubly linked list.
2. What is the difference between a "benign" race condition and a "problematic" race condition? What is a *data race*, and why is it undefined behavior?
3. Besides mutexes, what other two broad approaches to avoiding race conditions does the book mention? *(Hint: lock-free programming, and transactions / STM.)* Why is each one harder or less common?

### 3.2.1–3.2.2 Using mutexes & structuring code

1. Why is calling `lock()`/`unlock()` on a `std::mutex` directly discouraged? What happens if an exception is thrown between them?
2. What does `std::lock_guard` do, and what is class template argument deduction (CTAD) letting you write in C++17?
3. Why is it common to group the mutex and the data it protects into the same class as private members?
4. Follow-up to your first question: write a small example where a member function hands a pointer/reference to the protected data to a *user-supplied function*. Why can't the mutex protect you here?
   - What guideline does the book give about this?

### 3.2.3 Race conditions inherent in interfaces

1. Even if every method of `std::stack` is protected internally by a mutex, why is this still broken?
   ```cpp
   if (!s.empty()) { auto v = s.top(); s.pop(); }
   ```
2. Walk through the interleaving where two threads both call `top()` then `pop()`. What goes wrong that *doesn't* cause a crash or error? *(Hint: one value gets processed twice, one is never processed.)*
3. Why does the book's `threadsafe_stack` delete the assignment operator, and why does it lack `swap()`?
4. Why does the copy constructor of `threadsafe_stack` lock the *other* stack's mutex in the body instead of using the member initializer list?
5. Revisit your `shared_ptr` answer: in `pop()`, the `std::make_shared<T>(data.top())` allocation can throw. Why is that still safe? Where exactly is the "point of no return" relative to `data.pop()`?
   - Why is returning a `shared_ptr` safe when returning a `T` by value was not? *(Think: can copying a `shared_ptr` throw?)*
6. In the `pop(T& value)` overload, what requirements does this place on `T`? (Default constructible? Assignable?)

### 3.2.4–3.2.5 Deadlock

1. Define deadlock. What are the conditions that make it possible?
2. Why can deadlock happen with *no mutexes at all*? *(Hint: two threads calling `join()` on each other.)*
3. In the `swap(X& lhs, X& rhs)` example, why does the code check `if (&lhs == &rhs) return;` before calling `std::lock`? What would happen otherwise?
4. If `std::lock` acquires the first mutex and then an exception is thrown trying to lock the second, what happens to the first?
5. List the book's guidelines for avoiding deadlock:
   - Avoid nested locks
   - Avoid calling user-supplied code while holding a lock
   - Acquire locks in a fixed order
   - Use a lock hierarchy
   - Extend these guidelines beyond locks

   For each, can you write or describe a small example of breaking the rule?
6. Why is "always lock in the same order" not enough in the swap example? *(Hint: `swap(a, b)` vs `swap(b, a)`.)*
7. For the `hierarchical_mutex`: trace what happens when a thread holding a mutex of level 10000 tries to lock a level 5000 mutex, then vice versa. What does `previous_hierarchy_value` exist for?
8. Why is `hierarchical_mutex` a *runtime* check rather than a compile-time one, and why is that still useful?
9. Can you hold two locks at the same hierarchy level? What problem does this create for something like hand-over-hand locking a linked list?

### 3.2.6–3.2.7 `std::unique_lock` & transferring ownership

1. What can `std::unique_lock` do that `std::lock_guard` cannot?
2. What do `std::defer_lock`, `std::try_to_lock`, and `std::adopt_lock` each mean?
3. What is the cost of `unique_lock` compared to `lock_guard`? *(Hint: it has to store whether it owns the mutex.)*
4. Why is `unique_lock` movable but not copyable? Give an example where a function locks a mutex, prepares data, and returns the lock to its caller ("gateway" pattern).
   - It is not copyable because a `unique_lock` object represents ownership of a lock, and a lock can only be owned by one object.
5. When would you call `unlock()` on a `unique_lock` before it goes out of scope?
   - When you're done with the data but the function still has more (unrelated) work to do. Unlocking early avoids blocking other threads, since you're done with the protected data anyway.
   - Mainly for performance.

### 3.2.8 Lock granularity

1. What does it mean for a lock to be held "for the minimum possible time"? Why should you never hold a lock during file I/O?
2. In the book's `operator==` example, the two values are copied out under separate locks, then compared. Why can the result be meaningless even though each read was protected?
   - What does this tell you about the tradeoff between fine-grained locking and correctness?

### 3.3 Alternative facilities

1. Why is the "double-checked locking" pattern broken without proper synchronization? What data race does it introduce?
2. What do `std::call_once` and `std::once_flag` do? Why is `std::once_flag` neither copyable nor movable?
3. How can `call_once` be used for lazy initialization of a class member (e.g., a connection that's opened on first use)?
4. Since C++11, what guarantee does the language give for initializing a `static` local variable? How does this give you a thread-safe singleton?
5. What is a reader-writer mutex? When does `std::shared_mutex` help, and when could it be *slower* than `std::mutex`?
6. `std::shared_lock` vs `std::unique_lock` on a `shared_mutex`: which one do readers use, and which do writers use?
7. What is `std::recursive_mutex`? Why does the book say needing one is usually a sign of a design problem?
   - How would you refactor a class whose public method calls another public method that also locks the mutex?

### Putting it together

1. Write a threadsafe queue (`push`, `try_pop`, `empty`) using what you learned. Which interface-race issues from the stack apply?
2. For each tool — `lock_guard`, `scoped_lock`, `unique_lock`, `shared_lock`, `call_once`, `recursive_mutex` — write one sentence on when you'd reach for it.
