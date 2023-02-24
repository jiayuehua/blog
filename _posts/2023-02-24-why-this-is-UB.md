---
layout: post
title:  "为什么++i+++i是UB"
date:   2023-02-24 09:24:58 +0800
categories: jekyll update
tags:
  Undefined Behavior
---
看embrace modern c++ safely时候，lambda一章提到:

    #define twice(E) ((E)+(E))
    int test(int value)
    {
      return twice(++value);//UB
    }

是UB，为什么呢？在slack上发问，得到了arthur专业回答，记录一下。

  > Sure, \++value + \++value is UB because it reads value twice and writes value twice, all within the same "breath" (what C++ used to call "with no intervening sequence point"). You can't tell if it'll do "load value, load value, add them, store value+1, store value+1" or "load value, store value+1, load value+1, add them, store value+2", or anything even weirder than that.   
 Hypothetically, the mental model is that it can do both sides of the + at once, e.g. by spawning a left-hand thread and a right-hand thread. Obviously nobody does that, but I believe it's legally possible.   
  Just to confuse matters, C++17 made some operators' order of evaluation well-defined; e.g. ++a << \++b always does \++a first and then \++b afterward .   
  And sometime (C\++17 also?) made function call arguments unspecified rather than undefined; e.g. f(++a, ++b) will do either ++a then ++b, or ++b then ++a, but definitely not overlap/race them, as ++a + ++b is allowed to do.
    
    
