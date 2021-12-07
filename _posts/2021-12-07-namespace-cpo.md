---
layout: post
title: "A note on `namespace __cpo`"
date: 2021-12-07 00:01:00 +0000
tags:
  hidden-friend-idiom
  library-design
  llvm
  ranges
  war-stories
excerpt: |
  Eventually I'll write up a full "libc++ beginner's guide to writing niebloids."
  For now, this is just a quick note to explain one particular quirk.
  The shape of a libc++ [niebloid and/or CPO](/blog/2019/08/02/the-tough-guide-to-cpp-acronyms/#cpo) is:

      namespace std::ranges {
        namespace __iter_swap {
          struct __fn {
            auto operator()(~~~) const { ~~~ }
          };
        }
        inline namespace __cpo {
          inline constexpr auto iter_swap = __iter_swap::__fn{};
        }
      }

  Why do we have that extra `inline namespace __cpo`?
---

Eventually I'll write up a full "libc++ beginner's guide to writing niebloids."
For now, this is just a quick note to explain one particular quirk.
The shape of a libc++ [niebloid and/or CPO](/blog/2019/08/02/the-tough-guide-to-cpp-acronyms/#cpo) is:

    namespace std::ranges {
      namespace __iter_swap {
        struct __fn {
          auto operator()(~~~) const { ~~~ }
        };
      }
      inline namespace __cpo {
        inline constexpr auto iter_swap = __iter_swap::__fn{};
      }
    }

Why do we have that extra `inline namespace __cpo`? Why not simply do this?

    namespace std::ranges {
      namespace __iter_swap {
        ~~~
      }
      inline constexpr auto iter_swap = __iter_swap::__fn{};
    }

Well, you see, in another part of the forest we have `counted_iterator`,
which has an ADL `iter_swap` defined using the hidden friend idiom:

    namespace std::ranges {
      template<~~~>
      struct counted_iterator {
        friend void iter_swap(counted_iterator, counted_iterator) { ~~~ }
      };
    }

Hidden friends are hidden from qualified name lookup, but they still
possess qualified names, according to their associated namespace.
Here we're telling the compiler that for each instantiation of `counted_iterator`
there exists a function `std::ranges::iter_swap` (which btw is hidden from ordinary
qualified lookup). There might be many such functions; that's okay
because functions can be overloaded.

But the compiler will complain loudly if we tell it that `std::ranges::iter_swap`
is _both_ a function _and_ a variable! Variables can't be "overloaded" with
functions in that way.

    int f(int); int f(double);  // OK

    int f(int); int f;  // Error

So any time any `class std::ranges::Foo` has a hidden friend named `bar`,
we can never define `std::ranges::bar` as a variable.

But we _can_ define `std::ranges::__cpo::bar` as a variable! And we can mark
`__cpo` as an inline namespace, so that `std::ranges::__cpo::bar` will be
found by ordinary qualified lookup of `std::ranges::bar`. (Unambiguously,
because all the functions named `std::ranges::bar` that might compete with it
are defined as hidden friends.)

So, for certain identifiers such as `iter_swap`, `swap`, and `iter_move`,
some little quirk like `namespace __cpo` is simply mandatory. For other identifiers,
such as `take` and `equal` and (AFAIK) `cbegin`, the extra namespace
is not required today — but libc++ (as of this writing) does it anyway, for
two reasons:

- Consistency is the best policy. It's easier to just be consistent, than
    to have to worry about "Should I create the inline namespace for this
    particular niebloid or not? What are our criteria?" The criterion is
    simple: Just do it.

- Future-proofing. Today, `std::ranges::cbegin` doesn't technically require the
    inline namespace because no type in the `std::ranges` namespace provides
    a hidden-friend `cbegin` function. But that's no guarantee it won't happen
    in the future! (There is precedent for types with ADL `begin` functions;
    [`std::valarray`](https://en.cppreference.com/w/cpp/numeric/valarray/begin2) and
    [`std::filesystem::directory_iterator`](https://en.cppreference.com/w/cpp/filesystem/directory_iterator/begin)
    are two examples.) Likewise, `std::views::take` doesn't require the inline
    namespace today, but there is already a WG21 proposal (Giuseppe D'Angelo's
    [P2226 "An idiom to move from an object and reset it to its default-constructed state"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p2226r0.html),
    October 2020) to add an ADL customization point named `take`, which raises
    the terrifying possibility of some standard type acquiring a hidden-friend `take`
    at some point in the future. If this ever happened, we might have to go add
    an inline namespace around `std::ranges::cbegin`, or around `std::views::take`.
    Which would be an ABI break. So let's just do it right the first time.

> "But there are no types in the `std::views` namespace!"
> Fair enough. But there might be in the future. Who knows?
> And what about names like `copy` and `equal`, which are directly
> in the `std::ranges` namespace?

Microsoft STL appears to do the same inline-namespace trick
[for `iter_swap`](https://github.com/microsoft/STL/blob/e89128e/stl/inc/xutility#L1070-L1072)
and [all the other CPOs](https://github.com/microsoft/STL/search?q=_Cpos)
(`cbegin`, `data`, `strong_order`,...) but stops there: it does not
extend the franchise to all niebloids (`take`, `copy`, `equal`,...)
as a general rule. Its inline namespace is `namespace _Cpos`.

Likewise, GNU libstdc++ appears to do the trick
[for most CPOs](https://github.com/gcc-mirror/gcc/search?q=__cust),
but notably not for `strong_order` etc., and not for all niebloids (`take`, `copy`, `equal`...)
as a general rule. Its inline namespace is `namespace __cust`.
