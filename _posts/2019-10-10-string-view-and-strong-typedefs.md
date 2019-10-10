---
layout: post
title: 'An unexpected brush with "strong typedefs"'
date: 2019-10-10 00:01:00 +0000
tags:
  library-design
  parameter-only-types
  standard-library-trivia
---

Today I failed to refactor a piece of code. Simplified:

    int extract_port(const std::string& hostport)
    {
        auto pos = hostport.find(':');
        if (pos != std::string::npos) {
            return std::stoi(hostport.substr(pos+1));
        }
        return -1;
    }

The original code didn't use `std::stoi`, mind you.
It used a different, third-party library API that expected
to take `const std::string&`.

I tried to apply the typical C++17 `const string& -> string_view` refactoring.

    int extract_port(std::string_view hostport)
    {
        auto pos = hostport.find(':');
        if (pos != std::string::npos) {
            return std::stoi(hostport.substr(pos+1));
        }
        return -1;
    }

[And it failed to compile.](https://godbolt.org/z/kIa3d8)
Why? Doesn't `std::string_view` provide all the same API as `std::string`?
The compiler is perfectly happy with the references to `hostport.find(':')`
and `hostport.substr(pos+1)`. What could be going wrong?

----

One of the incidental inefficiencies in the original code is that
 `std::string::substr` returns a _string_.
Since `substr` has `str` right there in the name, we shouldn't be too surprised
that it returns a `string`. We may wish `std::string` had a `subview` method
(`substr_view`? `slice`?), but it doesn't.

Our refactoring quietly changed the code from calling `string::find` to
calling `string_view::find`, and from calling `string::substr` to
calling `string_view::substr`. The former is a drop-in replacement: both
versions of `find` return `size_t`. But the latter is surprisingly _not_
a drop-in replacement! When the Committee standardized `string_view`, they
did a drive-by fix for this inefficiency: `std::string_view::substr` returns
`string_view`, not `string`.

> Incidentally, this means that the way to spell our hypothetical `mystring.subview(pos, len)`
> in standard C++17 is `std::string_view(mystring).substr(pos, len)`. Not too bad,
> as long as you trust your readers to know the trick.

So, the type of `hostport.substr(pos+1)` has changed from `string` to `string_view`.
And `std::stoi` doesn't accept `string_view`. (Why?
[Because it's a thin wrapper around `strtol`, and `strtol` requires null-termination.](/blog/2018/06/12/perennial-impossibilities/#string_view-versions-of-many-utility-functions)
It's a sordid story. Anyway, remember, my actual code used a different API there.
This was not the fault of `std::stoi` in particular.)

----

It occurred to me that this story relates directly to the
[perennial impossibility](/blog/2018/06/12/perennial-impossibilities/#strong-typedefs)
of "strong typedefs." What we had in the STL, pre-C++17, was a single type shaped roughly
like this:

     struct S {
         size_t find(char);
         S substr(size_t);
     };

And then in C++17, we did the _manual equivalent_ of

    strong_typedef SV = S;

Which produced a brand-new struct type `SV` shaped roughly like this:

     struct SV {
         size_t find(char);
         SV substr(size_t);
     };

But is that what the user wanted? Or did the user want

     struct SV {
         size_t find(char);
         S substr(size_t);
     };

instead? In this particular case, it seems that some users wanted
the one thing, and some other users wanted the other. It really depends
on how the type is going to be used.

In this case, both options were defensible. But,
given that `string_view`'s raison d'être was to be a mechanical
drop-in replacement for parameters of type `const std::string&`,
I think we might have been better off with a
`string_view::substr` that returned `string`.

----

Furthermore, I think that C++2a should be delayed.
(Previously on this blog: [1](/blog/2019/07/10/ways-to-get-dangling-references-with-coroutines/#under-construction),
[2](/blog/2019/07/04/strong-structural-equality-is-broken/#the-release-of-c2a-should-be-delayed-past-2020),
[3](/blog/2019/06/26/pro-p1485/#the-release-of-c2a-should-be-delayed-past-2020))
