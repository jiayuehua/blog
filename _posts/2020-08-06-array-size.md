---
layout: post
title: 'The "array size constant" antipattern'
date: 2020-08-06 00:01:00 +0000
tags:
  antipatterns
  c++-style
---

Here's a bad pattern I see frequently in student code on Code Review StackExchange,
and even in a fair amount of (other people's) teaching material.
They'll have an array of, let's say, 5 elements; and then because magic numbers
are bad, they'll introduce a named constant to refer to the count of elements "5".

    void example()
    {
        constexpr int myArraySize = 5;
        int myArray[myArraySize] = {2, 7, 1, 8, 2};
        ...

But this is the wrong way around! In the code above, the number five is _repeated_:
first in the value of `myArraySize = 5` and then again when you
actually write down the elements of `myArray`. The code above is just as
horrible, maintenance-wise, as:

    constexpr int messageLength = 45;
    const char message[messageLength] =
        "Invalid input. Please enter a valid number.\n";

—which surely none of us would ever write.


## Code that repeats itself is fragile code

Notice that in both of the above
code snippets, any time you change the contents of the array or the wording
of the message, you must update _two_ lines of code instead of just one.
Here's an example of how a maintainer might update this code _incorrectly:_

        constexpr int myArraySize = 5;
    -   int myArray[myArraySize] = {2, 7, 1, 8, 2};
    +   int myArray[myArraySize] = {3, 1, 4};

The patch above _looks_ like it's changing the contents of the array from
`2,7,1,8,2` to `3,1,4`, but it's not! It's actually changing it to `3,1,4,0,0`
— padded with zeroes — because the maintainer forgot to change
`myArraySize` in sync with `myArray`.


## The robust approach

Computers are really good at counting things. Let the computer do the counting!

    int myArray[] = {2, 7, 1, 8, 2};
    constexpr int myArraySize = std::size(myArray);

Now you can change the array's contents — say, from `2,7,1,8,2` to `3,1,4` — by
changing only a single line of code. There's no second place to update.

In fact, real-world code will typically use range-based `for` loops and/or iterator-range
algorithms to manipulate `myArray`, so it won't need a named variable to hold
the size of the array at all.

    for (int elt : myArray) {
        use(elt);
    }
    std::sort(myArray.begin(), myArray.end());
    std::ranges::sort(myArray);

    // Warning: Unused variable 'myArraySize'

In the "bad" version of this code, `myArraySize` is always used (by the
declaration of `myArray`) and so the programmer is unlikely to see that it
can be eliminated. In the "good" version, it's trivial for the compiler to
detect that `myArraySize` is unused.


## How do we do this with `std::array`?

Sometimes the programmer goes one step farther toward the Dark Side, and writes:

    constexpr int myArraySize = 5;
    std::array<int, myArraySize> myArray = {2, 7, 1, 8, 2};

This should at least be rewritten as:

    std::array<int, 5> myArray = {2, 7, 1, 8, 2};
    constexpr int myArraySize = myArray.size();  // or std::size(myArray)

However, there's no simple way to eliminate the manual counting in the first line.
C++17's CTAD does permit us to write

    std::array myArray = {2, 7, 1, 8, 2};

but that works only if you want an array of `int` — it wouldn't work if you wanted
an array of `short`, for example, or an array of `uint32_t`.

C++20 gives us [`std::to_array`](https://en.cppreference.com/w/cpp/container/array/to_array),
which permits us to write

    auto myArray = std::to_array<int>({2, 7, 1, 8, 2});
    constexpr int myArraySize = myArray.size();

Notice that this makes a C array and then _move-constructs_ its elements into the `std::array`.
All our previous examples initialized `myArray` with a braced initializer list,
which triggered aggregate initialization and constructed the `array`'s elements directly in-place.

Anyway, _all_ of these options result in a lot of extra template instantiations,
compared to plain old C-style arrays (which require zero template instantiations).
Therefore I strongly prefer `T[]` over `std::array<T, N>`.

In C++11 and C++14, `std::array` did have the ergonomic benefit of being able to say `arr.size()`;
but that benefit evaporated when C++17 gave us [`std::size(arr)`](https://en.cppreference.com/w/cpp/iterator/size)
for built-in arrays too.
There's no _ergonomic_ benefit to `std::array` anymore. Use it if you need its whole-object
value semantics (pass a whole array to a function! return an array from a function!
assign between arrays with `=`! compare arrays with `==`!) but otherwise I recommend
to avoid `std::array`.

> Similarly, I recommend to avoid `std::list` unless you need its
> iterator stability, fast splicing, sorting without swapping elements, and so on.
> I'm not saying these types have no place in C++; I'm just saying that they have
> a "very particular set of skills," and if you're not using those skills,
> you're probably overpaying.

Conclusions: Don't put the cart before the horse. In fact, you might not need a cart.
And if you must use a zebra to do a horse's job, you shouldn't put the cart before the zebra, either.
