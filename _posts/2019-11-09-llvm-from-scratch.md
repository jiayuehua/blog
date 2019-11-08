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

    mkdir $ROOT/llvm-project/build
    cd $ROOT/llvm-project/build
    cmake -G 'Unix Makefiles' \
        -DLLVM_ENABLE_PROJECTS="clang;libcxx;libcxxabi" \
        -DCMAKE_BUILD_TYPE=RelWithDebInfo ../llvm
    make -j5 clang
    make -j5 check-clang

If you omit `-DCMAKE_BUILD_TYPE=RelWithDebInfo`, this first part will still work, but you'll produce
a "debug-build" version of `clang` that is super slow, and then the "bootstrap" step below will
take days instead of minutes. So watch out for that.

`make -j5 clang` takes about 96 minutes on my laptop.
`make -j5 check-clang` takes another 38 minutes
(24 minutes to build a bunch of additional tools, and then another 14 minutes to run the actual tests).

Making `clang` will build both `clang` and `clang++`.
Making `cxx` will build `libc++.dylib`, `libc++.a`, and `libc++abi.dylib`.
Making `cxxabi` will build `libc++abi.a`.
Making `check-cxx` will build `libc++experimental.a`.

However, I have found that `make cxx` doesn't work with the system compiler on OS X 10.13.
I get errors like this (where `__ZTIDu` is the mangled name of `typeinfo for char8_t`):

    [ 60%] Linking CXX shared library ../../../lib/libc++abi.dylib
    Undefined symbols for architecture x86_64:
      "__ZTIDu", referenced from:
         -exported_symbol[s_list] command line option

If you get these same errors with `make cxx`, then you should just `make clang` and then follow the
instructions under ["Bootstrap," below](#step-5-bootstrap), to build `cxx` with your bootstrapped
Clang instead of the system compiler.

If something goes wrong, you can usually recover via

    rm $ROOT/llvm-project/build/CMakeCache.txt

and, absolute worst case, you can blow away `$ROOT/llvm-project/build` and start over.


## Step 4: Run specific tests.

Running a specific test or directory-of-tests for any product is easy:

    cd $ROOT/llvm-project/build
    ./bin/llvm-lit -sv ../llvm/test/Analysis
    ./bin/llvm-lit -sv ../clang/test/ARCMT
    ./bin/llvm-lit -sv ../libcxx/test/std/re

However, it looks like before you can successfully run one of these lines,
you must have run the corresponding one of `make check-{llvm,clang,cxx}`
at least once, to initialize the right stuff in the `build` directory.

(Thanks to [Brian Cain](http://lists.llvm.org/pipermail/llvm-dev/2018-May/123049.html)
for documenting this recipe.)

But watch out! Both `make check-cxx` and `llvm-lit` will by default use your *system compiler*
to run the libc++ tests! This is not what you want! Tell `llvm-lit` to use your newly built Clang
by passing the `cxx_under_test` parameter, like this:

    make -j5 clang
    make -j5 cxx
    ./bin/llvm-lit -sv --param cxx_under_test=`pwd`/bin/clang ../libcxx/test/


## Step 5: Bootstrap!

Here we will *not* be installing Clang over top of the system compiler
(super dangerous!); but we *will* instruct CMake to build Clang using
the previously built Clang.

There is apparently [an official way to bootstrap Clang](https://llvm.org/docs/AdvancedBuilds.html)
(probably out-of-date).

However, when I bootstrap Clang, I use this crude approach inspired by
[the CMake FAQ](https://gitlab.kitware.com/cmake/community/wikis/FAQ#how-do-i-use-a-different-compiler):

    mkdir $ROOT/llvm-project/build2
    cd $ROOT/llvm-project/build2
    CXX="$ROOT/llvm-project/build/bin/clang++" \
    cmake -G 'Unix Makefiles' \
        -DLLVM_ENABLE_PROJECTS="clang;libcxx;libcxxabi" \
        -DCMAKE_BUILD_TYPE=RelWithDebInfo ../llvm
    make -j5 clang VERBOSE=1

(This takes the same 96 minutes as the original `make -j5 clang` did.)

Now you have built two versions of `clang++`: `$ROOT/llvm-project/build/bin/clang++` is the version
built with your system compiler, and `$ROOT/llvm-project/build2/bin/clang++` is the version built with
_that_ version. You can extend this to `build3`, `build4`, etc.
