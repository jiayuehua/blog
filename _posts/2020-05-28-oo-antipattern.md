---
layout: post
title: 'The "OO" Antipattern'
date: 2020-05-28 00:01:00 +0000
tags:
  antipatterns
  blog-roundup
  c++-learner-track
  c++-style
  slogans
  war-stories
---

For those who only read above the fold: I _don't_ say that all object-orientation is bad!
OOP, especially classical polymorphic OOP, has a well-deserved place in real code.
I'm going to talk about a very specific antipattern that I see with some frequency:
the use of `class` for things that should be simple free functions.

I originally wrote about this specific piece of code
[on Code Review StackExchange](https://codereview.stackexchange.com/questions/188300/counting-the-number-of-domino-tilings-for-a-rectangular-grid)
(February 2018). I gave an unrecorded lightning talk based on the same material in April 2018.

If this kind of thing appeals to you, I recommend with the highest possible recommendation
that you get yourself a copy of Kernighan and Plauger's [_The Elements of Programming Style_](https://amzn.to/2X5W1H0)
(1974, reissued 1978).


## The original code

Students who've learned C++ in a particular kind of academic setting often approach
it with the "everything is an object" mindset. Suppose they're given a task to compute
some value; then their first step is to create a `ValueComputer` object with a
member function `vc.computeResult()`.

For example: A student is asked to use [dynamic programming](https://en.wikipedia.org/wiki/Dynamic_programming)
to count the number of distinct [domino tilings](https://en.wikipedia.org/wiki/Domino_tiling)
of a $$w\times h$$ rectangle. The student writes:

    int main()
    {
        DominoTilingCounter tc(4, 7);  // in a 4x7 rectangle
        std::cout << tc.count() << '\n';
    }

Having framed the problem, they go on to implement the `DominoTilingCounter` class.
The clever student even adds memoization, so that the `count()` member function won't be
so slow the second time it's called:

    class DominoTilingCounter {
        int height, width;
        bool done = false;
        int tilingCount;
        int computeCount(int h, int w, std::string_view prevRow, int rowIdx) {
            [...recursive solution omitted...]
        }
    public:
        explicit DominoTilingCounter(int h, int w) : height(h), width(w) {
            if (h == 0 || w == 0 || (w*h) % 2 != 0) {
                tilingCount = 0;
                done = true;
            }
        }
        int count() {
            if (!done) {
                tilingCount = computeCount(height, width, "", 0);
                done = true;
            }
            return tilingCount;
        }
    };

(Experienced programmers may be cringing, here. That's the point.)

Unfortunately, this code fails the [const-correctness](/blog/2019/01/03/const-is-a-contract/) test:
the `count()` member function sounds like it should be non-modifying, but in fact
it needs to update member data and thus cannot be const.

    std::cout << DominoTilingCounter(4, 7).count() << '\n';
        // Fails to compile!

(Hacker News points out that this _does_ compile; class prvalues are not const
the way scalar prvalues are. Oops.)
Anyway, we can fix this issue incidentally, just by applying a little more logic.


## The logical leap

Look: when you construct a `DominoTilingCounter` object `tc`,
it is _specifically for the purpose_ of computing `tc.count()`, right?
There's no other purpose that `tc` could serve?

Again: When you <b>construct</b> a `DominoTilingCounter` object, it is specifically
for the purpose of <b>computing</b> `tc.count()`.

So put the <b>computation</b> in the <b>constructor!</b>

    class DominoTilingCounter {
        int height, width;
        int tilingCount;
        int computeCount(int h, int w, std::string_view prevRow, int rowIdx) {
            [...recursive solution omitted...]
        }
    public:
        explicit DominoTilingCounter(int h, int w) : height(h), width(w) {
            if (h == 0 || w == 0 || (w*h) % 2 != 0) {
                tilingCount = 0;
            } else {
                tilingCount = computeCount(height, width, "", 0);
            }
        }
        int count() const {
            return tilingCount;
        }
    };

This refactoring does several things:

- It permits `count()` to be a const member function.

- It eliminates the "empty, uncomputed" state from your program.

- It eliminates the data member `done`, whose whole purpose was to track that empty state.
    (In C++17 we might use a `std::optional` for that purpose; but look, now we don't have to!)

In fact, the private data members `height` and `width` are now unused as well. It turns out
that we were using them only to pass data from the constructor to the computation in
the `count()` method; and now that the computation is taking place in the constructor, we
don't need those data members anymore. Our code shrinks drastically.


## The final leap

In the original code, the student's `computeCount` member function happened to take `w` and `h`
as function parameters, rather than reading them from the `height` and `width` data members.
That was a fortunate accident: `computeCount` doesn't use any of the
data members of the `DominoTilingCounter` object, and so we can mark it `static`.
Our code is now something like this:

    class DominoTilingCounter {
        int tilingCount;
        static int computeCount(int h, int w, std::string_view prevRow, int rowIdx) {
            if (h == 0 || w == 0 || (w*h) % 2 != 0) {
                return 0;
            }
            [...recursive solution omitted...]
        }
    public:
        explicit DominoTilingCounter(int h, int w) {
            tilingCount = computeCount(h, w, "", 0);
        }
        int count() const {
            return tilingCount;
        }
    };

The final leap is to observe that this entire class does nothing but wrap an assignment to an `int`!

> Avoid classes that are tantamount to `int`.

What we _should_ write is a simple free function `countDominoTilings(w, h)`:

    int countDominoTilingsImpl(int h, int w, std::string_view prevRow, int rowIdx) {
        if (h == 0 || w == 0 || (w*h) % 2 != 0) {
            return 0;
        }
        [...recursive solution omitted...]
    }

    int countDominoTilings(int h, int w) {
        return countDominoTilingsImpl(h, w, "", 0);
    }

    int main() {
        int tc = countDominoTilings(4, 7);  // in a 4x7 rectangle
        std::cout << tc << '\n';
    }

No more `class`, no more worrying about const, no more worrying about memoization (it becomes the
caller's problem, for better or worse). Our original `DominoTilingCounter` object wasn't thread-safe,
but now we don't have to worry about that, either. And our code is about a dozen lines shorter.

----

Again, this is not to say that all classes are bad! In fact, the antipattern discussed here is
very close to the [Builder pattern](https://en.wikipedia.org/wiki/Builder_pattern),
and there's nothing wrong with the Builder pattern — when it's needed, that is.
All I'm saying is:

When you have to compute a value, don't write a `ValueComputer` class.  
Write a `compute_value` function instead.

----

For other takes on this topic, see:

- ["Execution in the Kingdom of Nouns"](http://steve-yegge.blogspot.com/2006/03/execution-in-kingdom-of-nouns.html) (Steve Yegge, March 2006)

- ["Stop Writing Classes"](https://www.youtube.com/watch?v=o9pEzgHorH0) (Jack Diederich, PyCon 2012)
