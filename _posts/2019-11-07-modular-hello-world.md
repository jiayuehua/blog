---
layout: post
title: "Hello World with C++2a modules"
date: 2019-11-07 00:01:00 +0000
tags:
  how-to
  llvm
  modules
---

Here's how to build a "Hello world" program using Clang's implementation of C++2a Modules,
as it currently stands as of November 2019. The compiler driver interface described here
is practically guaranteed to change over the next couple of years, but this seems to be
how it works right now in trunk.

For instructions on how to build Clang trunk in the first place,
see ["How to build LLVM from source"](/blog/2018/04/16/building-llvm-from-source/) (2018-04-16).

Thanks to Steven Cook and Steve Downey for helping me puzzle this out today on Slack!


## Step 1: Create `main.cpp`

Create your `main.cpp` with these contents:

    import Hello;
    int main() { hello(); }


## Step 2: Create `helloworld.cpp`

The naming convention for module interface units isn't established yet.
[Nathan Sidwell (2019)](https://www.youtube.com/watch?v=KVsWIEw3TTw) uses the convention "module name plus `-m.cpp`."
[Boris Kolpackov (September 2017)](https://www.youtube.com/watch?v=E8EbDcLQAoc) suggests "module name plus `.mpp`,"
but that seems confusing to me in light of Objective-C++'s established `.mm` extension.
For this example I see no reason not to just use "module name plus `.cpp`."

The naming convention for modules themselves _also_ isn't established yet. I'm choosing to name my
module `module helloworld`, all lowercase. Notice that a module is _not_ the same thing as a namespace!
This example uses only the global namespace.

So, create your `helloworld.cpp` with these contents:

    module;
    #include <stdio.h>

    export module helloworld;
    export void hello();

    module :private;
    void hello() { puts("Hello world!"); }

Notice that `#include <stdio.h>` causes the compiler to see a bunch of declarations for things like `printf`
that are _not_ supposed to be part of `module helloworld`, so it's important to place that `#include` directive
outside of `module helloworld`, in what's called the "global module fragment" or "GMF."

We want `module helloworld`'s interface to contain the declaration — but not the definition — of
`void hello()`. So we place `hello`'s definition down at the bottom, below the incantation
`module :private`, in what's called the "private module fragment" or "PMF."

> This mechanism of dividing a source file up into regions based on the placement of top-level declarations,
> rather than curly-braced lexical scopes, feels extremely "unlike C++" to me.
> I don't know what the original rationale was for doing it this way.
>
> Also, why is the latter incantation `module :private` instead of `module private`? I don't know.


## Step 3: Precompile `helloworld.cpp` into `helloworld.pcm`

Now compile `helloworld.cpp` into a [BMI](/blog/2019/08/02/the-tough-guide-to-cpp-acronyms/#bmi-cmi).
Clang's default expectation is that each BMI file will have a basename equal to the module name,
and the extension `.pcm` (for "precompiled module").
To create a BMI, you need to pass `-Xclang -emit-module-interface` to the compiler driver:

    clang++ -std=c++2a -c helloworld.cpp -Xclang -emit-module-interface -o helloworld.pcm

Remember, a BMI is *not* an object file! Functionally, it is the equivalent of a PCH file or a plain old
header file. It contains the high-level information necessary to *use* our `void hello()`;
it does not contain the actual x86-64 instructions that *implement* `void hello()`.


## Step 4: Compile `*.cpp` into `a.out`

Our final step is to compile all our `.cpp` files and link them together.

    clang++ -std=c++2a -fprebuilt-module-path=. main.cpp helloworld.cpp

We must pass `-fprebuilt-module-path=.` so that, when the compiler sees `import helloworld`, it'll know
where to look for `helloworld.pcm`. This is exactly analogous to how we would have had to pass `-I.`
so that when the compiler saw `#include "helloworld.h"` it would know where to look for `helloworld.h`.
The convention for the name of this compiler driver option is not yet standardized; I predict that
eventually Clang will pick something shorter and more `-I`-like.
(Unfortunately, [`-M` is taken](https://gcc.gnu.org/onlinedocs/gcc/Preprocessor-Options.html).)

And now we have an executable that we can run!

    ./a.out
    Hello world!

----

Notice that we collapsed a few steps into one, in that last command line there. We could have
written it all out longhand like this:

    clang++ -std=c++2a -fprebuilt-module-path=. main.cpp -c
    clang++ -std=c++2a helloworld.cpp -c
    clang++ -std=c++2a main.o helloworld.o

We didn't need to pass `-fprebuilt-module-path` to the second compilation step, because `helloworld.cpp`
doesn't `import` any BMIs and therefore doesn't need to know the search path for them.

Arguably there ought to be some way to combine Step 3 (precompiling `helloworld.cpp` into `helloworld.pcm`)
with Step 4 (compiling `helloworld.cpp` into `helloworld.o`) in just a single compiler invocation with
two file outputs.
I'm not aware that Clang supports any such feature at the moment, but it might be coming soon
and/or it might already exist and I just don't know about it.
