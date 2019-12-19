---
layout: post
title: 'How to build LLVM from source, monorepo version'
date: 2019-11-09 00:01:00 +0000
tags:
  how-to
  llvm
---

This is an update of my previous post
["How to build LLVM from source"](/blog/2018/04/16/building-llvm-from-source) (2018-04-16),
which is now out-of-date. LLVM has moved to a "monorepo" design instead of a collection
of smaller tool-specific repositories. So this new post has fewer steps!

----

The LLVM codebase's official home is [`https://github.com/llvm/llvm-project`](https://github.com/llvm/llvm-project).

(However, to submit patches to LLVM projects, you must use
[the official Phabricator](https://reviews.llvm.org/differential/diff/create/);
don't submit GitHub pull requests against `llvm/llvm-project`!
At least not as of November 2019.)


## Step 1: Fork!

Go to your GitHub account and fork the following repository:

- [llvm/llvm-project](https://github.com/llvm/llvm-project)


## Step 2: Get the code!

Locally clone the repo to the right place.

    cd $ROOT
    git clone git@github.com:llvm/llvm-project

This is a good time to set up the `.git/config` for the repo
you just cloned (for example, `$ROOT/llvm-project/.git/config`).
I set it up this way:

    [remote "origin"]
        url = git@github.com:Quuxplusone/llvm-project.git
        fetch = +refs/heads/*:refs/remotes/origin/*
    [remote "upstream"]
        url = git@github.com:llvm/llvm-project.git
        fetch = +refs/heads/*:refs/remotes/upstream/*
    [branch "master"]
        remote = upstream
        merge = refs/heads/master

This gives me two remotes: one named `upstream` whence I can pull,
and one named `origin` whither I can push. My local `master` and `origin/master`
will both track `upstream/master`. Anything I do in my local repo, I will do in a
feature branch; my feature branches will track `origin`.


## Step 3: Build!

Unfortunately, if you're on Mac OSX like me, then this step is extra complicated.
I don't really understand why, but, to build Clang on OSX you'll need to
[apply this patch](/blog/code/2019-11-09-Jens-Jorgensen-clang-patch.diff)
(hat tip to Jens Jorgensen for writing it) and pass an extra parameter to CMake.

I've also updated this step to use `-G Ninja` instead of `-G 'Unix Makefiles'`,
just because all the cool kids seem to be using Ninja these days.

    cd $ROOT/llvm-project
    patch -p1 < 2019-11-09-Jens-Jorgensen-clang-patch.diff

    mkdir $ROOT/llvm-project/build
    cd $ROOT/llvm-project/build
    cmake -G 'Ninja' \
        -DCLANG_XCODE_TOOLCHAIN_ROOT=/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain \
        -DLLVM_ENABLE_PROJECTS="clang" \
        -DCMAKE_BUILD_TYPE=Release ../llvm
    ninja clang

Making `clang` will build both `clang` and `clang++`.

If you omit `-DCMAKE_BUILD_TYPE=Release` (or at least `-DCMAKE_BUILD_TYPE=RelWithDebInfo`),
this first part will still work, but you'll produce
a "debug-build" version of `clang` that is super slow, and then the "bootstrap" step below will
take days instead of minutes. So watch out for that.

On my laptop, `cmake` takes about 42 seconds;
`ninja clang` takes about 96 minutes.


### Troubleshooting step 3

If something goes wrong, you can usually recover via

    rm $ROOT/llvm-project/build/CMakeCache.txt

and, absolute worst case, you can `rm -rf $ROOT/llvm-project/build` and start over.

Notice that I'm building only Clang, not libc++. Ever since libc++ moved to an explicit
list of exported linker symbols, it can't be built with any compiler that doesn't support
both `__int128` and `__float128` — and OSX's system compiler doesn't qualify. So in order to
build libc++ we'll need to build Clang first, and then bootstrap libc++ (see below). If you're
getting linker errors like the following, that's what's going on:

    Undefined symbols for architecture x86_64:
      "__ZTIDu", referenced from:
         -exported_symbol[s_list] command line option
      "typeinfo for __float128", referenced from:
         -exported_symbol[s_list] command line option
      "typeinfo for __int128 const*", referenced from:
         -exported_symbol[s_list] command line option

Furthermore, I'm not building `check-clang` yet, because, ironically enough,
it uses features from my own paper
[P1155 "More Implicit Moves"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1155r3.html)
which post-date Xcode's compiler release. If you get compiler errors like the following, that's
what's going on:

    $ROOT/llvm-project/clang/lib/DirectoryWatcher/mac/DirectoryWatcher-mac.cpp:246:10:
    error: no viable conversion from returned value of type 'std::unique_ptr<DirectoryWatcher>'
    to function return type 'llvm::Expected<std::unique_ptr<DirectoryWatcher> >'

Finally, remember that the Clang that we built in this first step will use Xcode's own C++ headers
by default. When you compile a .cpp file with this Clang, it will not have access to newer headers
such as  `<optional>` which don't exist in Xcode yet.


## Step 4: Bootstrap `check-clang` and libc++.

Here we will instruct CMake to build Clang again, using the Clang we just built.
There is apparently [an official way to bootstrap Clang](https://llvm.org/docs/AdvancedBuilds.html)
(probably out-of-date). However, I use an approach inspired by
[the CMake FAQ](https://gitlab.kitware.com/cmake/community/wikis/FAQ#how-do-i-use-a-different-compiler).
Note that we will *not* be installing Clang over top of the system compiler; that would be super dangerous
and you should never do it!

First, make sure you have <b>reverted</b> Jens Jorgensen's patch from Step 3.
During the bootstrap step, we don't want that `CLANG_XCODE_TOOLCHAIN_ROOT` stuff to exist anymore.

    cd $ROOT/llvm-project
    patch -R -p1 < 2019-11-09-Jens-Jorgensen-clang-patch.diff

    mkdir $ROOT/llvm-project/build2
    cd $ROOT/llvm-project/build2
    CXX="$ROOT/llvm-project/build/bin/clang++" \
    cmake -G 'Ninja' \
        -DLLVM_ENABLE_PROJECTS="clang;libcxx;libcxxabi" \
        -DCMAKE_BUILD_TYPE=RelWithDebInfo ../llvm
    ninja clang cxx
    ninja check-clang check-cxx

Notice that we're enabling more projects this time: `clang;libcxx;libcxxabi`.
(Unfortunately, `compiler-rt` will not build on OSX.)

Now you have built two versions of `clang++`: `$ROOT/llvm-project/build/bin/clang++` is the version
built with your system compiler, and `$ROOT/llvm-project/build2/bin/clang++` is the version built with
_that_ version. You can extend this to `build3`, `build4`, etc.

Making `cxx` will build `libc++.dylib`, `libc++.a`, and `libc++abi.dylib`.
Making `cxxabi` will build `libc++abi.a`.
Making `check-cxx` will build `libc++experimental.a`.

This time, `cmake` takes about 58 seconds; `ninja clang` takes about 153 minutes.
`ninja check-clang` takes another 53 minutes:
36 minutes to build a bunch of additional tools, and then 17 minutes to run the actual tests.
`ninja cxx` takes about 84 seconds.
`ninja check-cxx` takes about 50 minutes (but see the caveat below about `cxx_under_test`).

## Step 5: Run specific tests.

Running a specific test or directory-of-tests for any product is easy:

    cd $ROOT/llvm-project/build2
    ./bin/llvm-lit -sv ../llvm/test/Analysis
    ./bin/llvm-lit -sv ../clang/test/ARCMT
    ./bin/llvm-lit -sv ../libcxx/test/std/re

However, before you can successfully run one of these lines,
you must have run the corresponding one of `make check-{llvm,clang,cxx}`
at least once, to initialize the right stuff under the `build2` directory.

(Thanks to [Brian Cain](http://lists.llvm.org/pipermail/llvm-dev/2018-May/123049.html)
for documenting this recipe.)

But watch out — both `make check-cxx` and `llvm-lit` will by default use your *system compiler*
to run the libc++ tests! This is not what you want! Tell `llvm-lit` to use your newly built Clang
by passing the `cxx_under_test` parameter, like this:

    ./bin/llvm-lit -sv --param cxx_under_test=`pwd`/bin/clang ../libcxx/test/

On my laptop, this command line again takes about 50 minutes to run all the libc++ tests,
but this time it correctly uses the bootstrapped compiler.
