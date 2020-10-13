---
layout: post
title: 'Precedence of a proposed `|>` operator'
date: 2020-04-10 00:01:00 +0000
tags:
  coroutines
  language-design
  proposal
  ranges
  wg21
---

Colby Pike and Barry Revzin's [P2011R0 "A pipeline-rewrite operator"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p2011r0.html)
(January 2020) proposes for C++ a "pizza operator" similar to
[the one proposed for JavaScript](https://github.com/tc39/proposal-pipeline-operator/wiki).
This came out of a blog post by Colby Pike:
["Eliminating the Static Overhead of Ranges"](https://vector-of-bool.github.io/2019/10/21/rngs-static-ovr.html) (October 2019).

The idea of the "pizza operator" `|>` is that it provides syntactic sugar, sort of related to
[UFCS](/blog/2019/08/02/the-tough-guide-to-cpp-acronyms/#ufcs).
Suppose you want to write a chain of method calls like

    // Ideal-for-this-purpose class-based API
    auto x = a.convolve(gauss5x5).clamp(0, 1).sharpen();
    auto y = b.each_as<fs::path>().transform(get_basename)
              .to<std::vector<std::string>>();

except that your API doesn't _do_ methods, it does free functions; so you are currently forced to write
these two lines as more like

    // Unfortunate free-function-based API
    auto x = sharpen(clamp(convolve(a, gauss5x5), 0, 1));
    auto y = to<std::vector<std::string>>(transform(each_as<fs::path>(b), get_basename));

In a hypothetical future C++ with a pizza operator, you'd simply write

    // Still using the free-function-based API!
    auto x = a |> convolve(gauss5x5) |> clamp(0, 1) |> sharpen();
    auto y = b |> each_as<fs::path>() |> transform(get_basename)
               |> to<std::vector<std::string>>();

The proposed `|>` isn't an operator like `operator|`; it doesn't follow the traditional model of
"evaluate LHS, evaluate RHS, combine the resulting values." Instead, it does "evaluate LHS, _parse_
RHS, insert evaluated LHS into RHS's parse tree, _now_ evaluate RHS." `x |> y(z)` is syntactic sugar
for `y(x, z)`.

The Ranges library overloads `operator|`, and provides lots of machinery including multiple overloaded
call operators in all of its range adaptors, basically to simulate what's happening above. If C++ were
to provide `|>` out of the box, a huge swath of Ranges machinery could go away (except for backward
compatibility).


## No placeholders in C++

In the JavaScript proposal, you're allowed to write things like

    let x = a |> (# - 1);
    let y = a |> (1 - #);

where the former means the same as `(a - 1)` and the latter means the same as `(1 - a)`.
The scripting language [Hack](https://docs.hhvm.com/hack/expressions-and-operators/pipe) does the same thing,
using `$$` as the placeholder.

P2011 doesn't propose any kind of placeholder. It says, when you pipeline `x` into `f(1,2,3)`, `x` always gets
inserted as the first argument, because that's the most common idiom in C++. If you want it to get inserted
as the second argument, you'll have to do something like

    x |> (std::bind(f,_2,_1,_3,_4))(1,2,3)

Why those extra parentheses? Read on.


## P2011R0's precedence for `|>` leads to weirdness

The current proposal, P2011R0, proposes that `|>` should be a new postfix operator on the same level
as `.` and `->`. This makes sense if you're just looking at the example above: In the member-function
API we write `x.y(z)`, in the free-function API we write `x|>y(z)`, so `|>` replaces `.` one-for-one,
so it should have the same precedence.

However, P2011R0's rule leads to surprising behavior. For example, the `.` operator binds tighter
than the unary `-` operator, so if we bind `|>` as tightly as `.`, then `-x|>f()` means `-f(x)`:

    assert(-1 |> std::abs() == -1);  // wat?

Or again,

    ++i |> v.at();  // means the same as ++v.at(i)

The essential issue here is that P2011R0's chosen precedence didn't match the visual symbolism of `|>`.
The `|>` operator looks just like the `|` operator; it's used with pipelines in the same way; and
`|` has one of the lowest precedences in C++. In fact we've been subtly indicating "Low precedence, low precedence!"
via our use of whitespace. You've seen C++ code that indicates precedence by whitespace, even if
you didn't consciously realize it.

    if (x+1 == y)    // + binds tighter than ==
    y->z + 1         // -> binds tighter than +

So if `|>` is to have a super high precedence, we should really be writing

    - 1|>std::abs()
    ++ i|>v.at()

But anecdotal evidence (from P2011's own sample code!) is that people don't want to write `x|>y`; the
association of `|>` with `|` pipelines is just too strong. We should go with the flow and lower the
precedence of `|>` until it matches our visual intuition.

P2011R1 is coming, and it seems likely that this time it will propose a suitably low precedence.


## My proposal

For the record, here's how I would do it. (This is _not_ a description of P2011R0 anymore!)

Step one, the precedence of `|>` should be exactly the same as the precedence of `|`, and of course
it should be left-associative.

Step two, consider an [operator-precedence parser](https://en.wikipedia.org/wiki/Operator-precedence_parser)
— which admittedly is not how most compilers work in practice, but I'll blithely assume
that they can figure out how to translate this algorithm into their own terms.

*If* you are currently parsing the right-hand side of `x |>` — that is, if there's a `|>` on the top
of the operator stack — and you have just finished parsing
a function-call expression `f(y)`, then _do not do overload resolution on `f(y)` yet._ First, look at the next
operator in the expression. If it is something with tighter (higher) precedence than `|>`, then
do overload resolution on `f(y)`, and proceed. But if
it's something with lower precedence (or another `|>` or the end of the expression),
then completely resolve the `x |> f(y)` to your left before proceeding
(which means doing overload resolution on `f(x,y)`, not on `f(y)`).

Finally, if you are currently parsing the right-hand side of `x |>`, and you get to an operator
with lower precedence than `|>` (or another `|>` or the end of the expression), that's an error!
You should have been able to resolve that first `|>` operator by now. If it's still on the stack,
it means you've got some bogus expression like `x |> y` where there's no function call at all.
That's a syntax error.

Now, there are some gray areas.

- If `A` is a class type, should we treat `x |> A(y)` the same as `x |> f(y)`? (I think yes. And this also applies
    in the CTAD case where `A` is a template-id.)

- What about `x |> A{y}`? (I think no.)

- What about parenthesized call-expressions like `x |> (f(y))`? (My algorithm says no, which I think is fine.)

- What about similar-looking postfix-expressions like `x |> typeid()` or `x |> static_cast<int>()`?
    (I think no, and nobody should ever write these seriously.)

- What about similar-looking unary-expressions like `x |> sizeof()`?
    (I think no, and nobody should ever write this seriously.)


## The table

Here is a table of expressions extracted from the paper P2011R0 and from a long reflector thread
on the subject. For each expression, I provide two interpretations: the P2011R0 interpretation
(where `|>` has the precedence of `.` and must be followed by a primary-expression)
and the interpretation implied by the proposal above (where `|>` has the precedence of `|`).

In many realistic cases, the two proposals give the same interpretation, so some of these rows have the
same thing in both columns. At the same time, bear in mind that this table is heavily weighted toward
the corner cases. Personally I would hope that any compiler would diagnose `x + y |> f()` and suggest
parentheses — either `(x + y) |> f()` or `x + (y |> f())` — the same way
[GCC (but sadly only GCC) diagnoses](https://godbolt.org/z/2abc7i) `x + y | f()` today.

|  Expression                        | P2011R0                            | This post                             |
|:-----------------------------------|:-----------------------------------|:--------------------------------------|
| `r |> filter(f) |> transform(g)`   | `transform(filter(r,f),g)`         | `transform(filter(r,f),g)`            |
| `x |> f() |> g<0>(0)`              | `g<0>(f(x), 0)`                    | `g<0>(f(x), 0)`                       |
| `x |> A::staticmethod()`           | `A::staticmethod(x)`               | `A::staticmethod(x)`                  |
| `x |> A{}.staticmethod()`          | Syntax error                       | `A{}.staticmethod(x)`                 |
| `x |> A().staticmethod()`          | Syntax error                       | `A().staticmethod(x)`                 |
| `x + y |> f()`                     | `x + f(y)`                         | `f(x + y)`                            |
| `x |> c.f()`                       | Syntax error                       | `c.f(x)`                              |
| `x |> (f()).g()`                   | Syntax error                       | `f().g(x)`                            |
| `x |> f().g()`                     | `f(x).g()`                         | `f().g(x)`                            |
| `x |> always(y)(z)`                | `always(x,y)(z)`                   | `always(y)(x,z)`                      |
| `x |> always(y)() |> split()`      | `split(always(x,y)())`             | `split(always(y)(x))`                 |
| `x |> get()++`                     | `get(x)++`                         | Syntax error                          |
| `x |> ++get()`                     | Syntax error                       | Syntax error                          |
| `++x |> get()`                     | `++get(x)`                         | `get(++x)`                            |
| `x |> (y |> z())`                  | Syntax error                       | Syntax error                          |
| `x |> f().g<0>(0)`                 | `f(x).g<0>(0)`                     | `f().g<0>(x,0)`                       |
| `-3 |> std::abs()`                 | `-std::abs(3)`                     | `std::abs(-3)`                        |
| `co_await x |> via(e)`             | `co_await via(x, e)`               | `via(co_await x, e)`                  |
| `co_yield x |> via(e)`             | `co_yield via(x, e)`               | `co_yield via(x, e)`                  |
| `throw x |> via(e)`                | `throw via(x, e)`                  | `throw via(x, e)`                     |
| `return x |> via(e)`               | `return via(x, e)`                 | `return via(x, e)`                    |
| `s |> rev() |> find_if(a).base()`  | `find_if(rev(s), a).base()`        | `find_if(a).base(rev(s))`             |
| `x |> (f())`                       | Syntax error                       | Syntax error                          |
| `ctr |> size() == max()`           | `size(ctr) == max()`               | Syntax error                          |
| `(ctr |> size()) == max()`         | `size(ctr) == max()`               | `size(ctr) == max()`                  |
| `x |> f() + g()`                   | `f(x) + g()`                       | Syntax error                          |
| `x |> f() + 3`                     | `f(x) + 3`                         | Syntax error                          |
| `(x |> f()) + 3`                   | `f(x) + 3`                         | `f(x) + 3`                            |
| `x |> get()[i]`                    | `get(x)[i]`                        | Syntax error                          |
| `x |> v[i]()`                      | Syntax error                       | `v[i](x)`                             |
| `x |> v[i]()()`                    | Syntax error                       | `v[i]()(x)`                           |
| `(x |> v[i]())()`                  | Syntax error                       | `v[i](x)()`                           |
| `x |> (v[i])()()`                  | `v[i](x)()`                        | `v[i]()(x)`                           |
| `"hi"sv |> ra::count('o') == 0`    | `ra::count("hi"sv, 'o') == 0`      | Syntax error                          |
| `("hi"sv |> ra::count('o')) == 0`  | `ra::count("hi"sv, 'o') == 0`      | `ra::count("hi"sv, 'o') == 0`         |
| `v |> filter(2) |> size() == 1`    | `size(filter(v, 2)) == 0`          | Syntax error                          |
| `(v |> filter(2) |> size()) == 1`  | `size(filter(v, 2)) == 0`          | `size(filter(v, 2)) == 0`             |
| `x |> y.operator+()`               | Syntax error                       | `y.operator+(x)`                      |
| `x |> +y`                          | Syntax error                       | Syntax error                          |
| `a |> b() - c |> d()`              | `b(a) - d(c)`                      | Syntax error                          |
| `a |> b() | c() |> d()`            | `b(a) | d(c())`                    | `d(b(a) | c())`                       |
| `r |> filter(f) | transform(g)`    | `filter(r,f) | transform(g)`       | `filter(r,f) | transform(g)`          |
| `r | filter(f) |> transform(g)`    | `r | transform(filter(f), g)`      | `transform(r | filter(f), g)`         |
| `x + y |> f() + g() |> a.h()`      | Syntax error                       | Syntax error                          |
| `x |> getA().member()`             | `getA(x).member()`                 | `getA().member(x)`                    |
| `x |> getA().*getMemptr()`         | `getA(x).*getMemptr()`             | Syntax error                          |
| `c ? left : right |> split('/')`   | `c ? left : split(right, '/')`     | `c ? left : split(right, '/')`        |
| `(c ? left : right) |> split('/')` | `split(c ? left : right)`          | `split(c ? left : right)`             |
| `c ? left |> split('/') : right`   | `c ? split('/', left) : right`     | `c ? split('/', left) : right`        |
| `x |> f() |> std::make_pair(y)`    | `std::make_pair(f(x), y)`          | `std::make_pair(f(x), y)`             |
| `x |> f() |> std::pair(y)`         | Syntax error                       | `std::pair(x, y)` in the gray area    |
| `x |> f() |> std::pair<X,Y>(y)`    | Syntax error                       | `std::pair<X,Y>(x, y)` in the gray area |
| `x |> new T()`                     | Syntax error                       | Syntax error                          |
| `x |> [](int x, int y){ return x+y; }(1)` | `[](int x, int y){ return x+y; }(x,1)` | `[](int x, int y){ return x+y; }(x,1)` |
| `x |> f() |> std::plus{}(1)`       | Syntax error                       | `std::plus{}(f(x),1)`                 |
{:.smaller}

Since that table is a lot to read, I'll pull out the four most important differences here:

|  Expression                        | P2011R0                            | This post                             |
|:-----------------------------------|:-----------------------------------|:--------------------------------------|
| `co_await f() |> via(e)`           | `co_await via(f,e)`                | `via(co_await f(), e)`                |
| `v |> filter(2) |> size() == 1`    | `size(filter(v, 2)) == 0`          | Syntax error                          |
| `r | filter(f) |> transform(g)`    | `r | transform(filter(f), g)`      | `transform(r | filter(f), g)`         |
| `-3 |> std::abs()`                 | `-std::abs(3)`                     | `std::abs(-3)`                        |
{:.smaller}

The reason `co_await` has this difference, but not any of the other "word-like" unary operations (`co_yield`,
`co_return`, `return`, `throw`) is a "bug" in the grammar of C++20. Notice that `co_yield(1) + 2` means `co_yield 3`,
but `co_await(1) + 2` means `(co_await 1) + 2`. Lewis Baker tells me he's opening an issue for something along
these lines, although his main focus is on how `!co_yield(x)` shouldn't be a syntax error.

Finally, Tony Van Eerd provides food for thought: What if some future version of C++ _were_ to provide
a placeholder, such as `#`? If it did, then we'd have

|  Expression                            | P2011R0 (hypothetically)   | This post (hypothetically)    |
|:---------------------------------------|:---------------------------|:------------------------------|
| `x |> f(#) |> # + 3 |> g(#)`           | `f(x) + g(3)`              | `g(f(x) + 3)`                 |
| `r = ints |> # + 3 |> # * 2 |> vec(#)` | `ints + 3 * vec(2)`        | `vec((ints + 3) * 2)`         |
{:.smaller}
