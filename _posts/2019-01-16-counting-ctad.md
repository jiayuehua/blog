---
layout: post
title: "Measuring adoption of C++17 and CTAD in real codebases"
date: 2019-01-16 00:01:00 +0000
tags:
  class-template-argument-deduction
  llvm
  science
---

I looked at [cppreference's list of open-source C++ libraries](https://en.cppreference.com/w/cpp/links/libs),
Ctrl+F'ed for "17", and took the three hits: yomm2, nytl, and Yato.

## yomm2

    brew install cmake
    git clone https://github.com/jll63/yomm2
    cd yomm2
    mkdir build
    cd build
    CXX=$ROOT/llvm/build/bin/clang++ CXXFLAGS='-std=c++17 -Wc++14-compat' cmake -G 'Unix Makefiles' ..
    make 2>&1 | grep 'warning:' | sort | uniq -c

This produced 20 warnings of the form

    static_assert with no message is incompatible with C++ standards before C++17 [-Wc++98-c++11-c++14-compat]

and no diagnostics about CTAD. Just to be sure, I built it again with my `-Wctad` diagnostic:

    cd .. ; rm -rf build ; mkdir build ; cd build
    CXXFLAGS='-std=c++17 -Wctad' CXX=$ROOT/llvm/build/bin/clang++ cmake -G 'Unix Makefiles' ..
    make

It built clean.


## Yato

Yato kind of supports C++17, but its CMakeLists.txt defaults to `-std=c++14`. So I used
`sed` to change those `14`s to `17`s. Then I noticed that "yato/aligning_allocator.h"
[is buggy](https://bitbucket.org/alexey_gruzdev/yato/issues/4/) in C++17 mode,
so I used `sed` to fix that. Also, Yato uses `-Werror` in its makefile: totally reasonable,
but I need to count warnings, so, `sed` again.

    brew install cmake
    git clone https://bitbucket.org/alexey_gruzdev/yato.git
    cd yato
    git grep -l 'c++14' | xargs sed -i -e 's/c++14/c++17/g'
    sed -i -e 's/YATO_CXX17/DUMMY/' include/yato/aligning_allocator.h
    git grep -l 'Werror' | xargs sed -i -e 's/Werror/Wno-error/g'
    mkdir build
    cd build
    CXX=$ROOT/llvm/build/bin/clang++ CXXFLAGS='-std=c++17 -Wc++14-compat' cmake -G 'Unix Makefiles' -DGTEST_DOWNLOAD=yes ..
    make 2>&1 | grep 'warning:' | sort | uniq -c

This produced 10 warnings of the form

    warning: inline variables are incompatible with C++ standards before C++17 [-Wc++98-c++11-c++14-compat]

and no diagnostics about CTAD. Just to be sure, I built it again with my `-Wctad` diagnostic:

    cd .. ; rm -rf build ; mkdir build ; cd build
    CXX=$ROOT/llvm/build/bin/clang++ CXXFLAGS='-std=c++17 -Wctad' cmake -G 'Unix Makefiles' -DGTEST_DOWNLOAD=yes ..
    make

It built clean.


## nytl

This was my first time using the [Meson](https://mesonbuild.com/Quick-guide.html) build system.
I'm surprised that Meson continues to use the CMake model — run the "build system" command
(`cmake` or `meson`) once to create a makefile, and then run a different command (`make` or `ninja`)
to actually build the project. I always assumed that a more "modern" model would unify
every aspect of the build under the same high-level command (like, `meson setup` and then `meson build`,
or something).

The one change I had to make is to remove its use of
[`std::uncaught_exceptions`](https://en.cppreference.com/w/cpp/error/uncaught_exception),
which is new in C++17 and mandatory in C++2a (shades of
[`std::random_shuffle`](https://stackoverflow.com/questions/27791474/what-are-best-practices-for-simple-random-shuffling-in-code-thats-both-c03-an)!) —
but which does not exist on MacOS prior to 10.12 Sierra. So I just flipped it back to
the old familiar `std::uncaught_exception` using — what else? — `sed`.

    brew install meson
    git clone https://github.com/nyorain/nytl
    cd nytl
    git grep -l uncaught_exceptions | xargs sed -i -e 's/uncaught_exceptions/uncaught_exception/g'
    mkdir build
    cd build
    CXX=$ROOT/llvm/build/bin/clang++ CXXFLAGS='-std=c++17 -Wc++14-compat' meson setup -Dtests=true -Dwerror=false . ..
    ninja 2>&1 | grep 'warning:' | sort | uniq -c

By comparison to these other libraries, nytl is *insanely* C++17-heavy!
The build produced 67 compatibility warnings, which break down as follows:

|:-------------------------------------------- | ----:|
| class template argument deduction            |   30 |
| static_assert with no message                |   29 |
| template template parameter using 'typename' |    4 |
| constexpr if                                 |    2 |
| nested namespace definition                  |    1 |
| decomposition declarations                   |    1 |

nytl's uses of CTAD — which are all intentional — break down like this:

|:-------------------------------------------- | ----:|
| `nytl::span` tests                           |    4 |
| `nytl::Vec` tests                            |   12 |
| `nytl::Rect` tests                           |    3 |
| `nytl::ScopeGuard` tests                     |    4 |
| `nytl::SuccessGuard` tests                   |    3 |
| `nytl::ExceptionGuard` tests                 |    3 |
| `nytl::ScopeGuard` used in library code      |    1 |

Nineteen uses are testing library types that in my opinion "don't really need" CTAD.
They use CTAD basically as a party trick — observe [the test](https://github.com/nyorain/nytl/blob/39397a41d1ddd/docs/tests/vec.cpp#L73-L74)
which verifies that `Vec{1.f, 2, 3, 4.}` deduces as `Vec<4, double>`.

But the other 11 uses are for a scope-guard class which "really needs" CTAD in order to be useful,
and which is actually used (once) within the library itself.
(Previously on this blog (2018-08-11): [I don't always use ad-hoc scope guards...](/blog/2018/08/11/the-auto-macro))

`span`, `Vec`, and `Rect` all have explicit deduction guides.
[`SuccessGuard` and `ExceptionGuard` also have deduction
guides](https://github.com/nyorain/nytl/blob/39397a41d1ddd/nytl/scope.hpp#L77-L78) — I confess I
don't understand why. `ScopeGuard` itself does not have any deduction guides.
