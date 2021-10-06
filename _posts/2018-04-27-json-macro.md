---
layout: post
title: 'Embedding quotes in strings'
date: 2018-04-27 00:01:00 +0000
tags:
  c++-style
  pearls
  preprocessor
  war-stories
excerpt: |
  Yesterday I saw this (anonymized) in our codebase's unit tests:

      test_json_thing("{\"key\": 42, \"otherkey\": \"value\"}");

  And then I looked *elsewhere* in our unit tests, and I saw this:

      test_json_thing(R"( {"key": 42, "otherkey": "value"} )");

  Frankly, these alternatives are both awful and I hate them.
---

Yesterday I saw this (anonymized) in our codebase's unit tests:

    test_json_thing("{\"key\": 42, \"otherkey\": \"value\"}");

And then I looked *elsewhere* in our unit tests, and I saw this:

    test_json_thing(R"( {"key": 42, "otherkey": "value"} )");

Frankly, these alternatives are both awful and I hate them.
So when I wrote *my* unit test, I wrote this:

    #define TEST_JSON_THING(...) test_json_thing(#__VA_ARGS__)

    TEST_JSON_THING( {"key": 42, "otherkey": "value"} );

[obxkcd.](https://xkcd.com/927/)
