---
layout: post
title: 'Field-testing "Down with lifetime extension!"'
date: 2020-03-04 00:02:00 +0000
tags:
  copy-elision
  initializer-list
  lifetime-extension
  llvm
  proposal
excerpt: |
  I hacked my local Clang to produce a warning every time [`Sema::checkInitializerLifetime`](https://github.com/llvm/llvm-project/blob/86565c13094236e022d2238f5653641aaca7d31f/clang/lib/Sema/SemaInit.cpp#L7341)
  detected that lifetime extension was necessary.

      test.cpp:31:16: warning: binding temporary of type 'int' to a reference
      relies on lifetime extension [-Wlifetime-extension]
          const int& i2 = 42;
                          ^~

  Then I ran it over the LLVM codebase by rebuilding Clang with itself
  (["How to build LLVM from source, monorepo version"](/blog/2019/11/09/llvm-from-scratch/) (2019-11-09)).
  Here's what I found.
---

I hacked my local Clang to produce a warning every time [`Sema::checkInitializerLifetime`](https://github.com/llvm/llvm-project/blob/86565c13094236e022d2238f5653641aaca7d31f/clang/lib/Sema/SemaInit.cpp#L7341)
detected that lifetime extension was necessary.
(My patch is on GitHub [here](https://github.com/Quuxplusone/llvm-project/commit/wlifetime-extension).)

    test.cpp:31:16: warning: binding temporary of type 'int' to a reference
    relies on lifetime extension [-Wlifetime-extension]
        const int& i2 = 42;
                        ^~

Then I ran it over the LLVM codebase by rebuilding Clang with itself
(["How to build LLVM from source, monorepo version"](/blog/2019/11/09/llvm-from-scratch/) (2019-11-09)).
Here's what I found.


## False positive #1: `for` loops lower to lifetime-extension

The first thing I had to do was insert a special case to prevent the diagnostic from firing
on ranged `for` loops.

    std::vector<int> getVector();
    for (int i : getVector()) { ... }

is lowered by the front-end into the moral equivalent of

    auto&& __range1 = getVector();
    auto __begin1 = __range1.begin();
    auto __end1 = __range1.end();
    for ( ; __begin1 != __end1; ++__begin1) { ... }

Since copy elision exists, I cannot think of any reason for the lowering to use `auto&&`
instead of `decltype(auto)`. But the language standard says `auto&&`, so that's what we get.
(EDIT: Mathias Stearn points out that if you wanted to respecify the feature using `decltype(auto)`,
you'd also have to wrap the range expression in an extra pair of parens. `decltype(auto) r1 = a;`
and `decltype(auto) r1 = (a);` have different semantics, and we want the latter.)

In this example, the lifetime of the temporary returned by `getVector()` is extended to match
the lifetime of `__range1`. But this happens "behind the scenes"; we don't want to discourage
the use of ranged `for`-loops in general. So we suppress the diagnostic in this case.

----

Incidentally, I had hoped that I could detect this case by looking at

    VarDecl *VD = dyn_cast<VarDecl>(ExtendingEntity->getDecl());
    if (VD && VD->isCXXForRangeDecl()) { ... }

but nope. I don't know what Clang uses that bit for, but apparently it's not for marking
lowered variables like `__range1`. I ended up just looking for
`VD->getNameAsString().substr(0,7) == "__range"`.


## False positive #2: `std::initializer_list` uses lifetime extension

LLVM's [AMDGPUISelLowering.cpp](https://github.com/llvm/llvm-project/blob/e440f9991/llvm/lib/Target/AMDGPU/AMDGPUISelLowering.cpp#L693)
contains a lot of code like this:

    SDValue Ops[2];
    SDValue Ops[2] = { Div, Rem };
    SDValue Ops[] = {Join, DAG.getNode(ISD::TokenFactor, SL, MVT::Other,
                                     LoLoad.getValue(1), HiLoad.getValue(1))};

And then one instance like this:

    auto Ops = {DAG.getConstant(0, SDLoc(), Op.getValueType()), Op.getOperand(0)};

My diagnostic fired on that line. The line turns out to be okay... but can you see why Clang thinks
lifetime extension is happening here?

It's because `auto` applied to a braced initializer-list
[deduces `std::initializer_list`](https://mariusbancila.ro/blog/2017/04/13/cpp17-new-rules-for-auto-deduction-from-braced-init-list/).
What we have here is the equivalent of

    std::initializer_list<SDValue> Ops = { sd1, sd2 };

That lowers into something that's impossible to spell in C++. The `initializer_list` object named `Ops`
is just a pair of pointers. Those pointers point into a backing array of type `const SDValue[2]`...
but that array is actually a temporary. (It cannot be static data, because the initial values
of `sd1` and `sd2` may vary at runtime.) The temporary array undergoes lifetime extension so that its
lifetime will match the lifetime of `Ops`.

I consider that line of code in AMDGPUISelLowering.cpp to be poorly written; it would be better if it
said `SDValue Ops[2] = ...` instead of `auto Ops = ...`. I suspect that it is almost
always unintended for anyone ever to create a local variable of type `std::initializer_list`.
But that's a job for a different diagnostic.


## False positive #3: Clang's GSL diagnostics interfere with lifetime-extension bookkeeping

I observed that a line like

    std::reverse_iterator<int*> rit = std::reverse_iterator<int*>();

would trigger my diagnostic, if and only if the `<array>` header was included. I eventually tracked
down the cause to [`Sema::inferGslPointerAttribute`](https://github.com/llvm/llvm-project/blob/23092ca9/clang/lib/Sema/SemaAttr.cpp#L100-L137),
which adds some kind of GSL-related ownership-tracking attribute to this class if and only if it's
known to be the aliasee of `std::array<int, 1>::reverse_iterator`. And then that attribute gets used
somehow in [`Sema::checkInitializerLifetime`](https://github.com/llvm/llvm-project/blob/23092ca9/clang/lib/Sema/SemaInit.cpp#L7048).
Anyway, I hacked around that by removing all the code in `Sema::inferGslPointerAttribute`.


## True positive #1: Redundant lifetime extension on iterators

LLVM contains _lots_ of instances of [this pattern](https://github.com/llvm/llvm-project/blob/c8cd58fa2/llvm/lib/Target/Mips/MipsSEISelLowering.cpp#L2683-L2684):

    const auto &Begin = Indices.begin();
    const auto &End = Indices.end();

I think where this comes from is that people see

    for (const auto &Elt : Indices)

and then when they need to break it out into a more complicated loop for some algorithmic reason,
they propagate the `const&`-ness of the "loop variables" without thinking about it.

Essentially, I call this _fuzzy thinking_ about _iterators versus references._ C++'s lifetime extension
means that you can often use these two constructs interchangeably:

    const auto &it = vec.front();
    use(it);

    const auto &it = vec.begin();  // lifetime-extended
    use(*it);

But _should_ you use them interchangeably? I say no.

I count this one tentatively as a success story for `-Wlifetime-extension`: it found silly code that was technically
correct but could be made more readable by eliminating the use of lifetime extension.


## True positive #2: Redundant lifetime extension on `APInt`

This is an interesting one, because it is a true positive that nevertheless can't be
improved by refactoring. LLVM contains lots of instances of this pattern:

    const APInt &StartInt = StartC->getAPInt();
    const APInt &StartRem = StartInt.urem(StepInt);

    const APInt &C = SC->getAPInt();
    const APInt &D = extractConstantWithoutWrapping(*this, C, Step);

In each of these cases, the first variable is initialized with `SC->getAPInt()`, which returns
`const APInt&`. The second variable is initialized with the result of some function that returns
a new `APInt` by value. So the one of these declarations involves lifetime extension, and the
other doesn't. Yet they _look_ parallel.

My kneejerk reaction is that this is more of a win than a lose. It is nice that we can make
both declarations look the same, because we're going to use them pretty much the same way.
The compiler takes care of the details in the most efficient way possible. This is good.

As in the for-loop case, we _could_ rewrite it to use `decltype(auto)` and stay just as efficient
and just as parallel:

    decltype(auto) StartInt = StartC->getAPInt();  // by reference
    decltype(auto) StartRem = StartInt.urem(StepInt);  // by value

But this syntax hides the fact that both of them are `APInt`s, so it would never fly
with LLVM coding style, nor with me.

<b>I count this as a good use of lifetime extension.</b>
I can't see how avoiding lifetime extension could possibly improve this code.


## True positive #3: Conditionally redundant lifetime extension in template code

In [SetOperations.h](https://github.com/llvm/llvm-project/blob/56eb15a1c/llvm/include/llvm/ADT/SetOperations.h#L41)
we find the following template:

    template <class S1Ty, class S2Ty>
    void set_intersect(S1Ty &S1, const S2Ty &S2) {
       for (typename S1Ty::iterator I = S1.begin(); I != S1.end();) {
         const auto &E = *I;
         ++I;
         if (!S2.count(E)) S1.erase(E);   // Erase element if not in S2
       }
    }

If we're intersecting `std::set<std::string>`, it certainly makes sense to take a reference to
`*I`, instead of copying it. But LLVM also provides sets like
[`llvm::SmallPtrSet<llvm::Value*, 4>`](https://github.com/llvm/llvm-project/blob/56eb15a1c/llvm/include/llvm/ADT/SmallPtrSet.h#L35-L47)
which are optimized for storing small trivially copyable elements while avoiding template bloat,
and in those cases, `*I` actually returns by value!

    const PtrTy operator*() const {
        [...]
        return PtrTraits::getFromVoidPointer(const_cast<void*>(*Bucket));
    }

When `*I` returns a value, that temporary value is bound to `E` and lifetime-extended.

> The spurious `const` at the front of that declaration is leftover cruft from
> [a July 2007 commit](https://github.com/llvm/llvm-project/commit/49f037ac291eec84bc6a8049c8bcba3dbcbe07c6)
> that sprinkled `const` randomly, in utter mental confusion between `T const*` and `T *const`.
> For guidelines about when to use `const`, see ["`const` is a contract"](/blog/2019/01/03/const-is-a-contract/) (2019-01-03).
> This digression is completely irrelevant to lifetime extension.

We could avoid lifetime extension in `set_intersect` by using `decltype(auto) E` instead of `const auto &E`.
However, `const auto&` gives the reader the additional information that we do not plan to modify `E`.
There is no such thing as `decltype(const auto)`.

Also, the line `const auto& E = *I;` is completely idiomatic C++, and it would be silly to change it
(A) just to avoid lifetime extension (B) in the presence of wacky iterator types.
If any line of code is at fault here, it's `SmallPtrSetIterator` for having a `reference` typedef
that's not a reference type.

<b>I count this as an unavoidable use of lifetime extension,</b>
although I would be moderately sympathetic to a compiler warning that fired in this case
and forced someone to grapple with the arguably unidiomatic design of `llvm::SmallPtrSetIterator`.


## True positive #4: Refactoring-induced lifetime extension on `std::string`

LLVM contains many instances of [this pattern](https://github.com/llvm/llvm-project/blob/adcd0268/clang/lib/Driver/ToolChains/Linux.cpp#L323-L324):

    const std::string &LibPath =
        std::string(GCCInstallation.getParentLibPath());

This looked silly enough that I went searching for the reason. It turns out that this line
used to look saner:

    const std::string &LibPath =
        GCCInstallation.getParentLibPath();

Prior to November 2011, `GCCInstallation.getParentLibPath()` actually returned `const std::string&`, and so
this line did exactly what it looked like, and it was efficient.
But in November 2011, [`getParentLibPath()` was changed](https://github.com/llvm/llvm-project/commit/4be70dd963#diff-430f5388221a0d46cb3e0a1cde8af98a)
to return `llvm::StringRef` instead of `const std::string&`. Since `StringRef` was implicitly convertible to
`std::string`, this line still compiled. But now it was quietly making a copy of the referenced string
and lifetime-extending that temporary copy.

Then, in January 2020, they decided that having an implicit conversion from `StringRef` to `string` was a bad idea.
(`std::string_view` doesn't have such a conversion, either.) So
[a machine-generated refactoring](https://github.com/llvm/llvm-project/commit/adcd0268#diff-f18bb9123f2f6377fd9a263acc894dcc)
added explicit casts to `std::string` around all the places where a `StringRef` was being implicitly converted.
And thus we end up with the silly-looking line above, which gets a `StringRef`, copies its contents into a
temporary `std::string`, and then lifetime-extends the temporary by binding it to a reference variable.
What we actually _want_ here is simply

    StringRef LibPath =
        GCCInstallation.getParentLibPath();

I count this one as an unqualified success story for `-Wlifetime-extension`: it found silly code that was technically
correct but could be made more readable and more efficient by eliminating the use of lifetime extension.


## True positive #5: Examples of utterly bogus lifetime extension

I don't want to leave you with the impression that every usage of lifetime extension in LLVM is "interesting."
Here are some snippets where using lifetime extension is (technically correct but) obviously wrong:

[SLPVectorizer.cpp](https://github.com/llvm/llvm-project/blob/36e6096a0/llvm/lib/Transforms/Vectorize/SLPVectorizer.cpp#L5107-L5109):

    auto &&ExtraVectorization = [this](Instruction *I, BoUpSLP &R) -> bool {
        return tryToVectorize(I, R);
    };

[AliasAnalysis.h](https://github.com/llvm/llvm-project/blob/967e7966f/llvm/include/llvm/Analysis/AliasAnalysis.h#L529).
This was caused by refactoring; `Loc` used to be a by-reference parameter.

    ModRefInfo getModRefInfo(const Instruction *I,
                             const Optional<MemoryLocation> &OptLoc) {
        const MemoryLocation &Loc = OptLoc.getValueOr(MemoryLocation());

[Lint.cpp](https://github.com/llvm/llvm-project/blob/6e60297ee/llvm/lib/Analysis/Lint.cpp#L291).
Prior to [a February 2013 refactoring](https://github.com/llvm/llvm-project/commit/89ade9287/#diff-9df8097baaeb9360d04d53413bf0da7e)
that made `getAttributes()` return by value instead of by const reference, this code correctly avoided a copy.
After the refactoring, this code relies on lifetime extension:

    const AttributeList &PAL = CI->getAttributes();


## Conclusion

If `-Wlifetime-extension` existed as a clang-tidy diagnostic, I would use it. Lifetime extension is _usually_
unintentional, and therefore a diagnostic that calls it out will likely call out poorly understood areas of the
code (as in our false positive #2 and true positives #1, #4, and #5). But I have seen some good uses of
lifetime extension too (as in true positives #2, #3, and arguably by enabling #4).

There are certainly a lot of cases of lifetime extension in the wild.
