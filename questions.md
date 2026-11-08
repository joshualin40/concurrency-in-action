1. Difference between scoped_thread and thread_guard?
- key difference is ownership, both classes ensure thread is joined before exiting the scope 
- for thread_guard, since it takes a reference the thread is still accessible outside the guard
- scoped_thread takes complete ownership, thread is not accessible outside the scoped_thread

Explain Listing 2.9 (page 32) 