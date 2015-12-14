# safe-map

Author: Karen Pease (meme01@eaku.net)

License: Public domain. Do as you will! No warranty either express or implied.

## Introduction

Threading is critical to many modern applications and the latest C++ standards make it easy to launch threads and control locks. Unfortunately, the STL containers themselves are very much not threadsafe. While the non-atomic nature of the individual container function calls can be worked around by keeping a mutex for the container and locking it for each operation, this unfortunately doesn't keep iterators to the map safe. Even having a map of smart pointers doesn't cut it - the iterator contains a pointer to the smart pointer but not the smart pointer itself, and thus does nothing to preserve the reference.

There are a few ways to work around this. For example, Intel's TBB contains several "threadsafe" containers. Unfortunately, you'll find that these too do nothing to preserve iterators - they only ensure atomic operations. Another option would be to skip using iterators and simply store keys. But doing that would mean doing a key lookup every time you want to use the iterator - even for simple increments. This can be slow, particularly if ordering is required (map vs. hash). There's also a number of unfortunate edge cases one can encounter with such an approach that they have to handle.

These issues inspired the creation of safe_map: a complete wrapping of std::map to provide thread safety, even for iterators.  A wide variety of template parameters are available for controlling its behavior, even allowing for circular maps.


## How it works 

In short, it works similar to smart pointers.  The second template argument to the map is wrapped into a class that contains a reference counter and an erasure-request flag. The iterators themselves are also wrapped in order to add functions to increment and decrement the references as needed.  Any std::map function that can take or return a normal iterator is provided with a wrapper - for example, find, begin, etc - which returns a wrapped iterator instead of a bare iterator. Unlike key lookups, reference counting is a fast process - the required std::mutex locking is significantly slower than the reference-counting code itself.

Most functions are straightforward wrappings... except for erasure. Rather than calling map.erase, the wrappers set the erasure flag  on the object in question. When all references to that object disappear, the object actually goes away.

The two files needed to use it are number.h (a generic number wrapping class) and safemap.h. To demonstrate usage, there's a full test suite included (map_test.cpp).  It launches multiple threads that find, insert, delete etc elements in the map at random, using every function available. It encounters no errors run under valgrind.

Because the second argument to the map gets variables added to it, it must be a class. To support use of base types as the second argument, these are doubly wrapped, first into a generic number wrapper, and then into the map wrapper.


## What does it mean to erase an element that might still be in use elsewhere? 

If an erasure flag is set on an element, it is immediately orphaned. One can think of this like removing a file inode in unix - files that already have it open can keep using it, but someone listing the directory will no longer see the file.  In the case of a map element that's been deleted while someone is using it, the element stays perfectly valid until their iterator is descoped, wherein its memory will be freed.  If one searches for a specific object that's been marked for deletion (such as with find()), they can still find it, but it's still subject to erasure when all references to it expire.


## Use cases and performance 

Oftentimes programmers need to deal with large amounts of data elements where each element represents an independent record, searches may expect to come up empty, and where many different tasks may need to be done on different parts of the data simultaneously. File systems, for example, are a non-memory data structure which is structured like this in everyday life.  In memory, caches and data stores often fall into this scenario as well. Without built-in thread safety, one has to resort to performance-hitting alternatives like locking the whole structure for the duration of long operations or having to do frequent lookups into the structure in lieu of being able to keep iterators open.

It's hard to make generalized statements about performance, as it depends highly on the scenario.  Versus the above situations, safe::map will yield higher throughput and/or reduced latency in the majority of situations, often dramatically. Versus regular one-at-a-time iterative in a std::map, however, it takes a performance hit in terms of the extra reference counting overhead and the mutex locking.  If said tasks are quick, there might be a significant slowdown by use of safe::map. However, if said tasks are at all CPU intensive, the effect of using safe::map will likely be unnoticeable.

## Implementation notes 

This class handles all of those awkward situations, such as when one thread deletes an object that another thread is working on. But this leaves some implementation details that one should be aware of.

1) Iterators are safe... but only one thread should use an iterator object at a time.

Iterators are safe.  But they're not *doubly safe*.  What an iterator points to will always be safe, but an iterator expects that only one thread will be using *the iterator object itself* at any given point of time. That is to say, if you want to have an iterator as a global variable and have multiple threads using that specific iterator *object*, you're going to need to add your own locking implementation to do so.

2) There are multiple ways incrementing and decrementing can be done - and it's your choice

The nature of safe::map incrementing and decrementing. There are two template parameters that control this:

  #3 - bool circular = false: 

If this is set, iteration past the end of the map will return to the beginning, and vice versa.

  #4 - IterationType iteration = OnlyForward

There are four possible iteration types which can be invoked by the ++ and -- operators:

 * OnlyForward: This moves along in the specified direction (++ or --), skipping over elements flagged for erasure.
 * ForwardThenBackward: This moves along in the specified direction, skipping over elements flagged for erasure. If it hits the end or beginning it starts searching in the opposite direction for an available element. It is guaranteed not to choose the same element that it started at, and to find an element so long as there is at least one non-deleted element apart from the starting element.
 * ForwardSameThenBackward: Like ForwardThenBackward, but can also land on the starting element if it doesn't find anything in the specified direction.
 * EvenErased: Like OnlyForward, but will also stop at elements flagged for erasure.

Warning: The advantage of ForwardThenBackward and FowardSameThenBackward - that they always find something to iterate to if possible - is also their weakness; their resistance to reaching the end() means that you cannot use them with the normal iterate-across-all-elements paradigm "for (auto iter = map.begin(); iter != map.end(); ++iter)". You also cannot have them select the starts and ends of ranges, as you don't know whether they've picked elements before or after the element you started at.  

Circular maps can the above paradigms, with caveats. Unless the map is empty, they too never reach end(). Hence the condition one has to check for is not whether it's reached end(), but rather whether it has looped back around to the starting element.

3) Even if you templated your map over a given type of incrementing, you can still use a different one 

While the IterationType specified in the class template will define what happens for ++ and --, the user can call whatever increment functions they want at any time:

do_minus(), do_plus(), do_minus(int), do_plus(int): like their respective operator++ / operator-- functions, except that they ignore whether an iterator is reversed or not; minus is always toward the beginning and plus is always toward the end.

decrement_active_circular, increment_active_circular: Acts like iteraton=OnlyForward, circular=true

decrement_forward, increment_forward: Acts like iteraton=OnlyForward, circular=false

decrement_forward_then_backward, increment_forward_then_backard: Acts like iteraton=ForwardThenBackward (or ForwardSameThenBackward, if iteraton##ForwardSameThenBackward), circular=false.

decrement_even_erased_linear, increment_even_erased_linear: Acts like iteration=EvenErased, circular=false

decrement_even_erased_circular, increment_even_erased_circular: Acts like iteration=EvenErased, circular=false

4) safe::map iterators are more resistant to undefined behavior than std::map iterators

Because of the need for a variety of overhead checks anyway to implement this class, as well as the fact that other threads may throw off the current thread's assumptions about its environment, safe::map iterators are more tolerant than std::map iterators.  Attempts to iterate past the end or before the beginning of the map will never lead to unexpected behavior. In the case of non-circular, forward-only iteration, such calls simply return without changing anything.

5) Reverse iterators behave the same as in std::map.... well, mostly.

A note about reverse iterators: reverse iterators posed a special implementation problem. Consider the following example with std::map:

```
#include <iostream>
#include <map>

int main(int argc, char** argv)
{
  std::map<int, int> map;
  map.emplace(1, 1);
  auto reverse_iter = map.rbegin();
  std::cout << reverse_iter->first << ", " << reverse_iter->second << std::endl;
  map.emplace(2, 2);
  std::cout << reverse_iter->first << ", " << reverse_iter->second << std::endl;

  return 0;
}
```


This prints out:

```
1, 1
2, 2
```

Nobody has touched reverse_iter, nor done anything with the element that it points to - yet nonetheless it's changed. How? Because internally reverse iterators actually point to the element *after* (from a forward-iterator perspective) the one that they seem to point to. So when a new entry is inserted, the reverse iterator suddenly starts pointing to it instead.

This is confusing in normal circumstances, but it would be ruinous for safe::map. If what an iterator points to suddenly gets changed it would throw off all of the reference counts. The basic std paradigm for reverse iterators is simply unworkable here, so I implemented them based around forward iterators, using a template parameter to the iterator wrapper class to make it behave as if it's reversed. In most cases this will be transparent to the user.  However, if you're *expecting* the wierdness as in the example above, you should be aware that it will not happen in safe::map.

In addition to standard map::clear and map::erase functions, safe::map also provides clear_fast and erase_fast functions for each of them. These functions may provide a small performance benefit at erasure time by not actually erasing the element even if there are no iterators pointing to it, always just flagging it for erasure (the "non-fast" functions will take the time to erase the element if nothing points to it). This delays the actual erasure until an iterator moves onto it and then moves off, or you call cleanup(), which iterates across the entire map and removes all elements flagged for deletion.

6) Yes, you can raise the dead.

In case you're wondering: yes, you can bring an element flagged for deletion back to life by setting _erase_when_unused to "false".   This wasn't something deliberately designed into the class, but it does work.  

7) Speed up your safe::map with NoDestructorChecks

safe::map has one more template parameter worth mentioning: "DestructorSafetyType destructor = SharedPointer".  This means that all iterators refer to the map and its lock via std::shared_ptr, such that if the safe::map itself goes out of scope, the backend std::map and associated lock will stick around until all iterators expire.  However, shared pointers do carry some overhead in terms of their reference counting. If you now that your map will not descope while there are outstanding iterators, you can instead set destructor = NoDestructorChecks. This will instead use a std::unique_ptr to hold the std::map backend, and the iterators will refer to it with a bare pointer.

## Potential future work 

This class is build around std::map. std::map however is non-atomic. An atomic version of std::map could yield higher performance by reducing the necessary number of locking events.

A class like this could be built around any stl containers whose iterators don't invalidate.  Technically it could be done around containers that invalidate their iterators as well, but one would also have to have the map refresh all existing iterators at every invalidation event, which would be very costly.

## Feedback? 

Please!  If you have any bugfixes or ideas for improvement (ideally with ready-made patches that don't cause problem in the test code, hint hint!  ;)  ), feel free to let me know (meme01@eaku.net). 
