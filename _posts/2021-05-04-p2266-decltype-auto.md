---
layout: post
title: "P2266's interaction with `decltype(auto)`"
date: 2021-05-04 00:01:00 +0000
tags:
  cppnow
  implicit-move
  proposal
excerpt: |
  C++Now 2021 is happening this week. Normally it's in Aspen, Colorado, but this year it's
  completely online. I presented two talks:
  ["When Should You Give Two Things the Same Name?"](https://www.youtube.com/watch?v=OQgFEkgKx2s)
  and ["The Complete Guide to `return x`."](https://www.youtube.com/watch?v=OGKAJD7bmr8)

  In my `return x` talk, slide 77 showed a table illustrating how
  P2266's value-category changes affected five subtly different functions. One of the
  entries was incorrect; and, as I fixed it, I realized that for completeness the table
  should have included _eight_ subtly different functions!

  Here is a revised and I hope fully correct version of that table.
---

C++Now 2021 is happening this week. Normally it's in Aspen, Colorado, but this year it's
completely online. I presented two talks:

* ["When Should You Give Two Things the Same Name?,"](https://www.youtube.com/watch?v=OQgFEkgKx2s)
    based on [this blog post of mine](/blog/2020/10/09/when-to-derive-and-overload/) from October 2020

* ["The Complete Guide to `return x`,"](https://www.youtube.com/watch?v=OGKAJD7bmr8)
    based on my committee papers
    [P1155 "More Implicit Move"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1155r3.html)
    (adopted into C++20) and
    [P2266 "Simpler Implicit Move"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2266r1.html)
    (proposed for C++23).

In the latter talk, my slide 77 showed a table illustrating how
P2266's value-category changes affected five subtly different functions. One of the
entries was incorrect; and, as I fixed it, I realized that for completeness the table
should have included _eight_ subtly different functions!

Here is a revised and I hope fully correct version of that table.
This revised version appears in the slide deck I submitted after the talk.

|---------------------------------------------------|:-----------------------:|:-----------------------:|
|                                                   | C++14/17/20             | P2266 (C++23?)          |
|---------------------------------------------------|:-----------------------:|:-----------------------:|
| `auto a(T x) -> decltype(x) { return x; }`        | `T` <sup>1</sup>        | `T` <sup>1</sup>        |
| `auto b(T x) -> decltype((x)) { return (x); }`    | `T&`                    | `T&`                    |
| `auto c(T x) -> decltype(auto) { return x; }`     | `T` <sup>1</sup>        | `T` <sup>1</sup>        |
| `auto d(T x) -> decltype(auto) { return (x); }`   | `T&`                    | `T&&`                   |
| `auto e(T&& x) -> decltype(x) { return x; }`      | ill-formed <sup>2</sup> | `T&&`                   |
| `auto f(T&& x) -> decltype((x)) { return (x); }`  | `T&`                    | ill-formed <sup>3</sup> |
| `auto g(T&& x) -> decltype(auto) { return x; }`   | ill-formed <sup>2</sup> | `T&&`                   |
| `auto h(T&& x) -> decltype(auto) { return (x); }` | `T&`                    | `T&&` <sup>4</sup>      |
|---------------------------------------------------|-------------------------|-------------------------|
{:.smaller}

1) Implicit move: the returned `T` object is move-constructed from `x`.

2) The return type is deduced as `T&&`, but lvalue `x` cannot be returned as `T&&` without a cast.
(C++20's change to permit implicit-moving from rvalue reference variables is irrelevant.
Implicit move is not considered, because this is not a copy-initialization context.)

3) The return type is `T&`; but xvalue `x` cannot be returned as `T&` without a cast.

4) In C++20, the return type is deduced as `T&` because the returned expression `(x)` is an lvalue.
After P2266, the return type is deduced as `T&&` because the returned expression `(x)` is an xvalue.

Observe that in C++20, functions `f` and `h` both do "rvalue laundering,"
accepting an rvalue reference and returning an lvalue reference, with no
visible cast. After [P2266](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2266r1.html),
neither of them works anymore to launder rvalues: `f` becomes ill-formed,
and `h` becomes the identity function.

And just for _total_ completeness:

|---------------------------------------------------|:-----------:|:--------------:|
|                                                   | C++14/17/20 | P2266 (C++23?) |
|---------------------------------------------------|:-----------:|:--------------:|
| `auto t(T& x) -> decltype(x) { return x; }`       | `T&`        | `T&`           |
| `auto u(T& x) -> decltype((x)) { return (x); }`   | `T&`        | `T&`           |
| `auto v(T& x) -> decltype(auto) { return x; }`    | `T&`        | `T&`           |
| `auto w(T& x) -> decltype(auto) { return (x); }`  | `T&`        | `T&`           |
|---------------------------------------------------|-------------|----------------|
{:.smaller}
