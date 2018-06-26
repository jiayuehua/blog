---
layout: post
title: 'shields.io'
date: 2018-06-26 00:02:00 +0000
tags:
  hyperrogue
  sre
  today-i-learned
  web
---

Today I learned that [shields.io](https://shields.io) is a thing.

Adding badges to your repo's README has never been easier!

shields.io is a very cool thing, but it doesn't solve the two problems I immediately ran into
with TravisCI badges:

- They must be hard-coded to a GitHub username, which means they're fragile w.r.t. forking.
  See [#779](https://github.com/travis-ci/travis-ci/issues/779), [#974](https://github.com/travis-ci/travis-ci/issues/974),
  [#1892](https://github.com/travis-ci/travis-ci/issues/1892).

- They don't support badges "per job" (read: per-OS; read: per-person-who-maintains-this-use-case),
  only "per branch." See [#4623](https://github.com/travis-ci/travis-ci/issues/4623),
  and homebrewed solutions such as [exogen/badge-matrix](https://github.com/exogen/badge-matrix).

[exogen/badge-matrix](https://github.com/exogen/badge-matrix) is actually even awesomer than
shields.io, in the sense that it supports "per job" badges. So for example we can generate
this array of badges for my fork of HyperRogue:

[![OSX, Makefile.mac](https://badges.herokuapp.com/travis/Quuxplusone/hyperrogue?branch=travis-ci&env=TRAVIS_BUILD_SYSTEM=Makefile.mac&label=OSX,%20Makefile.mac)](https://travis-ci.org/Quuxplusone/hyperrogue/builds)
[![OSX, autotools](https://badges.herokuapp.com/travis/Quuxplusone/hyperrogue?branch=travis-ci&env=TRAVIS_OS_NAME=osx.*TRAVIS_BUILD_SYSTEM=autotools&label=OSX,%20autotools)](https://travis-ci.org/Quuxplusone/hyperrogue/builds)
[![Linux, autotools](https://badges.herokuapp.com/travis/Quuxplusone/hyperrogue?branch=travis-ci&env=TRAVIS_OS_NAME=linux.*TRAVIS_BUILD_SYSTEM=autotools&label=Linux,%20autotools)](https://travis-ci.org/Quuxplusone/hyperrogue/builds)
