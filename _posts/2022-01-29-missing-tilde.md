---
layout: post
title: "A minimally interesting typo-bug"
date: 2022-01-29 00:01:00 +0000
tags:
  compiler-diagnostics
excerpt: |
  I just ran into some code like this in a test suite:

      struct MoveOnlyWidget {
          MoveOnlyWidget(int);
          MoveOnlyWidget(const MoveOnlyWidget&) = delete;
          MoveOnlyWidget(MoveOnlyWidget&&) = default;
          MoveOnlyWidget& operator=(const MoveOnlyWidget&) = delete;
          MoveOnlyWidget& operator=(MoveOnlyWidget&&) = default;
          MoveOnlyWidget() = default;
      };
---

I just ran into some code like this in a test suite:

    struct MoveOnlyWidget {
        MoveOnlyWidget(int);
        MoveOnlyWidget(const MoveOnlyWidget&) = delete;
        MoveOnlyWidget(MoveOnlyWidget&&) = default;
        MoveOnlyWidget& operator=(const MoveOnlyWidget&) = delete;
        MoveOnlyWidget& operator=(MoveOnlyWidget&&) = default;
        MoveOnlyWidget() = default;
    };

I suspect that the original author intended that last line
to have a `~` in front of `MoveOnlyWidget` — this type ends up
being default-constructible when maybe it wasn't intended to be.

It occurs to me that it would be neat if something
like [PVS-Studio](https://pvs-studio.com/en/blog/examples/)
gave a warning for a defaulted default constructor that wasn't
at the top of the class — or maybe _any_ constructor that
wasn't grouped with the other constructors of the class.
(Maybe PVS-Studio already does! I don't know.)

---

Anyway, if it really mattered that `MoveOnlyWidget` shouldn't
be default-constructible, the original author could have
enforced that by adding any one of these lines after the class's
closing brace:

    static_assert(!std::is_constructible_v<MoveOnlyWidget>);
    static_assert(!std::is_default_constructible_v<MoveOnlyWidget>);
    static_assert(!std::default_initializable<MoveOnlyWidget>);
