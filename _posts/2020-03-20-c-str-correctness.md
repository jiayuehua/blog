---
layout: post
title: '`c_str`-correctness'
date: 2020-03-20 00:01:00 +0000
tags:
  c++-style
  parameter-only-types
---

I've [already blogged about](/blog/2018/03/27/string-view-is-a-borrow-type/) one difference
between [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string) and C++17's
[`std::string_view`](http://en.cppreference.com/w/cpp/string/basic_string_view):
`string` is owning and `string_view` is non-owning (being what I call a "parameter-only type").
Today I'd like to talk about a second difference: `string` is null-terminated and `string_view`
is not.

    const char buffer[] = "abcdefghij";
    auto sv = std::string_view(buffer+2, buffer+7);
    assert(sv == "cdefg");

By design, `string_view` is not a null-terminated type. There's no null byte after that `'g'`.

Now, C library functions like `strcpy`, `fopen`, `atoi`, and `strtol` all expect null-terminated C strings;
therefore `string_view` doesn't play well with these C library functions.
And some C++ library features are built on top of the C ones.
Since you can't use `string_view` with `atoi`, you also can't use it with `std::stoi`.
Since you can't use `string_view` with `fopen`, you also can't construct a `std::fstream` with one.

When a function transitively depends on null-termination, you actually _don't_ want
to refactor its interface from `const string&` to `string_view`, because eventually
you're just going to have to copy the string back into a null-terminated buffer in order to
make use of it.

    // ACTUALLY FINE
    int parseInt(const std::string& digits) {
        return std::stoi(digits);
    }

    // STRICTLY WORSE
    int parseInt(std::string_view digits) {
        auto ntdigits = std::string(digits);
        return std::stoi(ntdigits);
    }


## `.data()` versus `.c_str()`

`std::string` has two member functions that do basically the same thing: `s.data()` and `s.c_str()`.
The difference between `s.data()` and `s.c_str()` is that `s.c_str()` _connotes null-termination._

    FILE *fp = fopen(s.c_str(), "w");
        // fopen requires a null-terminated string
    fwrite(s.data(), 1, s.size(), fp);
        // fwrite takes a buffer and a length

`std::string_view`, which is not null-terminated, deliberately provides `.data()`
but does not provide `.c_str()`.

Even if you don't use C++17 `string_view` in your codebase yet, you can prepare for C++17-ification
by following good `c_str/data` hygiene. Here's an example of _bad_ hygiene:

    void one(const std::string& fname) {
        FILE *fp = fopen(fname.data(), "w");  // BAD!
    }

The above function uses `.data()` in a context that requires null-termination. It's dangerous
because a careless maintainer might "C++17-ify" the code by changing `const std::string&` to
`std::string_view` —

    void one(std::string_view fname) {
        FILE *fp = fopen(fname.data(), "w");  // BUGGY!
    }

— and boom, now you have a bug! If the original, pre-C++17 author had used `fname.c_str()` instead
of `fname.data()`, then not only would the code have been more self-documenting, but the buggy
C++17-ification would have failed to compile, because `string_view` does not provide `.c_str()`.

Here's another example of _bad_ hygiene:

    void two(int fd, const std::string& packet) {
        write(fd, packet.c_str(), packet.length());  // BAD!
    }

This hygiene is bad because the gratuitous use of `.c_str()` for a buffer that does _not_ need to be
null-terminated needlessly thwarts the maintainer's attempt at C++17-ification:

    void two(int fd, std::string_view packet) {
        write(fd, packet.c_str(), packet.length());  // ERROR: no .c_str()
    }

The original author should have used `.data()` instead, to indicate that null-termination was
not required for correctness. (Plus, consistently using `size` instead of `length` makes it
easier to drop in `std::vector<char>` or C++20's `std::span<const char>`.)

    void two(int fd, const std::string& packet) {
        write(fd, packet.data(), packet.size());  // GOOD!
    }

    void two(int fd, std::string_view packet) {
        write(fd, packet.data(), packet.size());  // EQUALLY GOOD!
    }

So there you go. Use `.c_str()` when you mean to rely on a `string`'s null-termination, and use
`.data() + .size()` when you don't. Following this rule helps to bring your code's hidden assumptions
to light, and makes it easier to introduce `std::string_view` later.
