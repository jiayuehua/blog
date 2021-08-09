---
layout: post
title: "What is the virtual table table?"
date: 2019-09-30 00:01:00 +0000
tags:
  classical-polymorphism
  constructors
---

The other day on Slack, I learned a new acronym for my [C++ acronym glossary](/blog/2019/08/02/the-tough-guide-to-cpp-acronyms/):
"VTT." [Godbolt](https://godbolt.org/z/3Pu26_):

    test.o: In function `MyClass':
    test.cpp:3: undefined reference to `VTT for MyClass'

"VTT" in this context stands for <b>virtual table table</b>.
It's an auxiliary data structure used (in the Itanium C++ ABI) during the construction of some
base classes which themselves
have virtual bases. It follows the same placement rules as the class's vtable and typeinfo; so if
you're getting the above error, you can simply imagine that it said "vtable" instead of "VTT" and
debug accordingly. (Most likely, you left the class's
[key function](https://stackoverflow.com/questions/45850063/what-is-a-c-key-function-as-described-by-gold)
undefined.)
To see why the VTT — or something like it — is needed, let's start with the basics.


## Order of construction for non-virtual bases

When we have an inheritance hierarchy, base classes are constructed
[from the basest subobjects downward](https://godbolt.org/z/FZwO65). To construct a `Charlie`,
we must first construct his parent classes `MrsBucket` and `MrBucket`; recursively, to construct
`MrBucket`, we must first construct his parent classes `GrandmaJosephine` and `GrandpaJoe`.

That is:

    struct A {};
    struct B : A {};
    struct C {};
    struct D : C {};
    struct E : B, D {};

    // Constructor bodies run in the order
    // A B C D E


## Order of construction for virtual bases

But virtual bases mess up the order a little bit! With virtual bases, we might have a diamond
hierarchy, where two different parent classes _share_ a single instance of a grandparent class.

    struct G {};
    struct M : virtual G {};
    struct F : virtual G {};
    struct E : M, F {};

    // Constructor bodies run in the order
    // G M F E

In the previous section, each constructor was responsible for calling the constructors of
its own base class subobjects.
But now that we're doing virtual inheritance, the constructors of `M` and `F`
must somehow know _not_ to construct their subobject of type `G`, because that subobject is shared.
If `M` and `F` were responsible for constructing
their own virtual base subobjects, we'd end up constructing that shared subobject twice in this case, and that
would be bad.

So, to deal with virtual base subobjects, the Itanium C++ ABI splits each constructor into two
parts: a _base object constructor_ and a _complete object constructor_. The base object constructor
is responsible for constructing all of the object's non-virtual base subobjects (and
its member subobjects, and setting its vptr to point to its vtable; and running whatever code
is inside the curly braces in your C++ code). The complete object constructor, which is called
whenever you create a complete C++ object, is responsible for constructing
all of the most derived object's virtual base subobjects
and then doing all the rest of that stuff too.

Observe the difference between our `A B C D E` example from the previous section and the following example:

    struct A {};
    struct B : virtual A {};
    struct C {};
    struct D : virtual C {};
    struct E : B, D {};

    // Constructor bodies run in the order
    // A C B D E

The complete object constructor for `E` _first_ calls the base object constructors of virtual subobjects
`A` and `C`; _then_ it calls the base object constructors of non-virtual bases `B` and `D`. `B` and `D`
are no longer responsible for constructing `A` and `C` respectively.


## Construction vtables

Suppose we have a class with some virtual methods, like this ([Godbolt](https://godbolt.org/z/2uiCsY)):

    struct Cat {
        Cat() { poke(); }
        virtual void poke() { puts("meow"); }
    };

    struct Lion : Cat {
        std::string roar = "roar";
        Lion() { poke(); }
        void poke() override {
            roar += '!';
            puts(roar.c_str());
        }
    };

When we are constructing a `Lion`, we start by constructing its `Cat` base subobject.
The `Cat` constructor calls `poke()`. At this point we have only a `Cat` object — we have not
yet initialized the data members that are necessary to make it a `Lion`. If `Cat`'s constructor
called `Lion::poke()`, it would try to modify an uninitialized `std::string roar` and we'd get UB.
So the C++ standard mandates that inside `Cat`'s constructor, calling a virtual method such as
`poke()` must call `Cat::poke()`, _not_ `Lion::poke()`!

This is no problem. The compiler simply makes sure that `Cat::Cat()` (both the base-object and
complete-object versions) starts by setting the object's vptr to point to `vtable for Cat`.
`Lion::Lion()` will call `Cat::Cat()`, and then _reset_ the vptr to point to `vtable for Cat-in-Lion`
before running the code in the curly braces. No problem at all!


## Virtual base offsets

Suppose `Cat` has a virtual base `Animal`. Then `vtable for Cat`
holds not just function pointers to `Cat`'s virtual member functions, but also the offset of
`Cat`'s virtual `Animal` subobject. ([Godbolt.](https://godbolt.org/z/_6xFgX))

    struct Animal {
        const char *data = "hi";
    };

    struct Cat : virtual Animal {
        Cat() { puts(data); }
    };

    struct Nermal : Cat {};

    struct Garfield : Cat {
        int padding;
    };

`Cat`'s constructor asks for this `Cat`'s `Animal::data` member.
If this `Cat` object is a base subobject of a
`Nermal` object, then its `data` member lives at offset 8, just beyond the vptr. But if this
`Cat` object is a base subobject of a `Garfield` object, then its `data` member lives at offset
16 — beyond the vptr and `Garfield::padding`. To deal with this situation, the Itanium ABI stores
_virtual base offsets_ in the vtable for `Cat`. The vtable for `Cat-in-Nermal` stores the fact that
the `Cat`'s base `Animal` subobject is located at offset 8; the vtable for `Cat-in-Garfield` stores
the fact that the `Cat`'s base `Animal` subobject is located at offset 16.

Now combine this with the previous section. The compiler must make sure that `Cat::Cat()`
(both the base-object and complete-object versions) starts by setting the object's vptr to
point to `vtable for Cat-in-Nermal` or `vtable for Cat-in-Garfield`, depending on the type
of the most derived object! How on earth does this work?

Well, the most derived object's complete object constructor must precompute which vtable it
wants the base subobject's vptr to be pointed at during construction, and then the MDO's COC
must pass that information down into the base subobject's base object constructor as a hidden
parameter! Look at the codegen for `Cat::Cat()` now ([Godbolt](https://godbolt.org/z/RaLIff)):

    _ZN3CatC1Ev: # complete object constructor for Cat
      movq $_ZTV3Cat+24, (%rdi)  # this->vptr = &vtable-for-Cat;
      retq

    _ZN3CatC2Ev: # base object constructor for Cat
      movq (%rsi), %rax  # fetch a value from rsi
      movq %rax, (%rdi)  # this->vptr = *rsi;
      retq

The base object constructor takes not just a hidden `this` parameter in `%rdi`, but _also_
a hidden "VTT" parameter in `%rsi`! The base object constructor loads an address from `(%rsi)`
and stores that address into the vtable of the `Cat` object.

Whoever calls the base object constructor of `Cat` is responsible for precomputing what address
`Cat::Cat()` ought to store in its vptr, and setting `(%rsi)` to point to that address.


## Why the extra indirection?

Look at `Nermal`'s  complete object constructor.

    _ZN3CatC2Ev: # base object constructor for Cat
      movq (%rsi), %rax  # fetch a value from rsi
      movq %rax, (%rdi)  # this->vptr = *rsi;
      retq
    _ZN6NermalC1Ev: # complete object constructor for Nermal
      pushq %rbx
      movq %rdi, %rbx
      movl $_ZTT6Nermal+8, %esi    # %rsi = &VTT-for-Nermal
      callq _ZN3CatC2Ev            # call Cat's base object constructor
      movq $_ZTV6Nermal+24, (%rbx) # this->vptr = &vtable-for-Nermal
      popq %rbx
      retq
    _ZTT6Nermal:
      .quad _ZTV6Nermal+24         # vtable-for-Nermal
      .quad _ZTC6Nermal0_3Cat+24   # construction-vtable-for-Cat-in-Nermal

Why does it "spill" `_ZTC6Nermal0_3Cat+24` into the data section and pass its _address_ in
`%rsi`, instead of simply passing `_ZTC6Nermal0_3Cat+24` directly?

    # Why not just this?

    _ZN3CatC2Ev: # base object constructor for Cat
      movq %rsi, (%rdi)  # this->vptr = rsi;
      retq
    _ZN6NermalC1Ev: # complete object constructor for Nermal
      pushq %rbx
      movq %rdi, %rbx
      movl $_ZTC6Nermal0_3Cat+24, %esi # %rsi = &construction-vtable-for-Cat-in-Nermal
      callq _ZN3CatC2Ev                # call Cat's base object constructor
      movq $_ZTV6Nermal+24, (%rbx)     # this->vptr = &vtable-for-Nermal
      popq %rbx
      retq

Well, that's because we might have several levels of inheritance here!
At each level of inheritance, the base object constructor needs to set the vptr to
_something_, and then possibly relay instructions further up the chain so that the
more-base constructors can set their own vptrs to _something(s) else_. This implies
a whole list or table of vtable pointers.

Here's a concrete example ([Godbolt](https://godbolt.org/z/8JW8Ch)):

    struct VB {
        int member_of_vb = 42;
    };
    struct Grandparent : virtual VB {
        Grandparent() {}
    };
    struct Parent : Grandparent {
        Parent() {}
    };

    struct Gretel : Parent {
        Gretel() : VB{1000} {}
    };
    struct Hansel : Parent {
        int padding;
        Hansel() : VB{2000} {}
    };

The base object constructor of `Grandparent` must set its vptr to point to
<code>Grandparent-in-</code><i>whatever the most derived class is</i>.
The base object constructor of `Parent` must first call `Grandparent::Grandparent()`
with a suitable `%rsi`, and then set _its_ vptr to point to
<code>Parent-in-</code><i>whatever the most derived class is</i>.
So the way `Gretel` implements this is:

    Gretel::Gretel() [complete object constructor]:
      pushq %rbx
      movq %rdi, %rbx
      movl $1000, 8(%rdi) # imm = 0x3E8
      movl $VTT for Gretel+8, %esi
      callq Parent::Parent() [base object constructor]
      movq $vtable for Gretel+24, (%rbx)
      popq %rbx
      retq
    VTT for Gretel:
      .quad vtable for Gretel+24
      .quad construction vtable for Parent-in-Gretel+24
      .quad construction vtable for Grandparent-in-Gretel+24

You can see in the Godbolt that `Parent`'s base object constructor
first calls `Grandparent::Grandparent()` with `%rsi+8`, and then sets
its own vptr to `(%rsi)`. So it's actually using the fact that `Gretel`
has carefully laid out a trail of breadcrumbs, so to speak, which can be
followed by all her base classes during construction.

The same VTT is used during destruction ([Godbolt](https://godbolt.org/z/ndTUqO)).

As far as I know, the zeroth entry in the VTT is never needed. `Gretel`'s
constructor does load `vtable for Gretel+24` into the vptr, but it knows that
address statically; it never needs to load it from the VTT. I think the zeroth
entry is probably just an accident of history. (And of course the compiler can't
just omit the zeroth entry these days, because that would violate the Itanium ABI
and prevent linking against older, Itanium-ABI-following code.)

There you have it: the scoop on the virtual table table, or VTT.


## For more information

You can find further information about the VTT in these places:

- [StackOverflow: "What is the VTT for a class?"](https://stackoverflow.com/questions/6258559/what-is-the-vtt-for-a-class)
- ["VTable Notes on Multiple Inheritance in GCC C++ Compiler v4.0.1"](http://web.archive.org/web/20190930143149/https://ww2.ii.uj.edu.pl/~kapela/pn/cpp_vtable.html) (Morgan Deters, 2005)
- [The Itanium C++ ABI](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#vtable-ctor-vtt), section "VTT Order"

Finally, I should repeat that the VTT is a feature of the _Itanium_ C++ ABI, as used on Linux,
OSX, etc. The MSVC ABI used on Windows doesn't have anything called the "VTT" as far as I'm
aware, and uses a fairly different mechanism for virtual bases. I know next to nothing about
the MSVC ABI (so far), but maybe one day I'll figure it out and write a blog post about it!
