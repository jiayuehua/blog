---
layout: post
title: 'Bit patterns of `float`'
date: 2021-09-05 00:01:00 +0000
tags:
  how-to
  math
  operator-spaceship
  pretty-pictures
---

I've spent too many hours repeatedly trying to find this information
on the Web. Time to write it down.

On the vast majority of computers, `float` is 32 bits, and it follows
the IEEE 754 standard (also known historically as IEC 559).
If you look at `reinterpret_cast<int&>(myfloat)` — or, in C++20,
`std::bit_cast<int>(myfloat)` — you'll find that the bits go in this order:

| Sign | Biased exponent | Mantissa |
|:----:|:---------------:|:--------:|
| 1    | 8               | 23       |

For example, `314.0f` reinterprets into `int(0x439d0000)`:

<table class="floating-point-format"><tr><td bgcolor="#cefcff">0</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td></tr></table>

The biased exponent bits are `10000111`, or 127+8. The mantissa bits
are `00111010000000000000000`. Stick the implicit leading `1` on the front
and we get $$1.0011101_2\times 2^8$$, or $$100111010_2$$, which is
binary for 314.

> Notice that this is independent of byte endianness, as long as your
> computer uses the same endianness for both ints and floats. If you
> treat `314.0f` as an array of bytes, you might find that it's
> `43 9d 00 00` on a big-endian machine and `00 00 9d 43` on a little-endian
> machine; but the `float`'s sign bit will always correspond to the
> `int`'s sign bit, and so on.

Now for the parts I always have trouble finding on the Web.

Zero is all-bits-zero.
Its biased exponent is `00000000`, or 127−127. So, if it were interpreted as
an ordinary number, its value would be $$1.0_2\times 2^{-127}$$; but it's
not. It's just "zero."

<table class="floating-point-format"><tr><td bgcolor="#cefcff">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td></tr></table>

Any non-zero number with an all-bits-zero biased exponent is "denormal"
or "subnormal"; its mantissa does _not_ have an implicit leading 1 bit,
and its effective exponent "sticks" at $$2^{-126}$$.
Here are three denormals. The first one's value is approximately `1.40e-45`.
The middle one's value is $$0.1_2\times 2^{-126}$$, or
approximately `5.88e-39`. The third's value is just a hair less than `FLT_MIN`.

<table class="floating-point-format"><tr><td bgcolor="#cefcff">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">1</td></tr></table>
<p></p>
<table class="floating-point-format"><tr><td bgcolor="#cefcff">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td></tr></table>
<p></p>
<table class="floating-point-format"><tr><td bgcolor="#cefcff">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td></tr></table>

`FLT_MIN`, a.k.a. `std::numeric_limits<float>::min()`, is approximately `1.18e-38`.
Its biased exponent is `00000001`, or 127−126, and its mantissa is all-bits-zero,
so its value is $$1.0_2\times 2^{-126}$$.

<table class="floating-point-format"><tr><td bgcolor="#cefcff">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">1</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td></tr></table>

Next come all the "normal" numbers. For example, the value `1.0` is
represented as $$1.0_2\times 2^{0}$$, for a biased exponent of 127+0
and a bit pattern of `3f800000`:

<table class="floating-point-format"><tr><td bgcolor="#cefcff">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td></tr></table>

and `2.0` is represented as $$1.0_2\times 2^{1}$$, for a biased exponent of 127+1
and a bit pattern of `40000000`:

<table class="floating-point-format"><tr><td bgcolor="#cefcff">0</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td></tr></table>

`FLT_MAX`, a.k.a. `std::numeric_limits<float>::max()`, is approximately `3.4e+38`.
Its biased exponent is `11111110`, or 127+127.

<table class="floating-point-format"><tr><td bgcolor="#cefcff">0</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">0</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td></tr></table>

`HUGE_VALF`, a.k.a. `std::numeric_limits<float>::infinity()`, looks like this.
Its biased exponent is all-bits-one, and its mantissa is all-bits-zero.

<table class="floating-point-format"><tr><td bgcolor="#cefcff">0</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td></tr></table>

The following three bit-patterns are all signaling NaNs.
`std::numeric_limits<float>::signaling_NaN()` is the middle one.
A signaling NaN's biased exponent is all-bits-one and its mantissa's top bit is `0`.
The remaining 22 mantissa bits are "payload." They can be anything except
all-bits-zero (because if the mantissa were all-bits-zero, it'd be
`HUGE_VALF` instead).

<table class="floating-point-format"><tr><td bgcolor="#cefcff">0</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">1</td></tr></table>
<p></p>
<table class="floating-point-format"><tr><td bgcolor="#cefcff">0</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td></tr></table>
<p></p>
<table class="floating-point-format"><tr><td bgcolor="#cefcff">0</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td></tr></table>

The following two bit-patterns are both quiet NaNs.
`NAN`, a.k.a. `std::numeric_limits<float>::quiet_NaN()`, is the first one.
A quiet NaN's biased exponent is all-bits-one and its mantissa's top bit is `1`.
The remaining 22 mantissa bits are "payload." They can be anything.

Observe that every signaling NaN has a "corresponding" quiet NaN: just flip the top bit
of its mantissa from `0` to `1`. However, there is exactly one quiet NaN
which does not correspond to any signaling NaN: flipping the top mantissa bit of
`NAN` gives you `HUGE_VAL`.

<table class="floating-point-format"><tr><td bgcolor="#cefcff">0</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td></tr></table>
<p></p>
<table class="floating-point-format"><tr><td bgcolor="#cefcff">0</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td></tr></table>

Flip the sign bit on any of these bit-patterns and you get negative versions
of all the preceding floats.

Negative zero:

<table class="floating-point-format"><tr><td bgcolor="#cefcff">1</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td></tr></table>

Negative denormals:

<table class="floating-point-format"><tr><td bgcolor="#cefcff">1</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">1</td></tr></table>
<p></p>
<table class="floating-point-format"><tr><td bgcolor="#cefcff">1</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td></tr></table>

`-FLT_MIN`, a.k.a. `-std::numeric_limits<float>::min()`, approximately `-1.18e-38`:

<table class="floating-point-format"><tr><td bgcolor="#cefcff">1</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">0</td><td bgcolor="#acfeb4">1</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td></tr></table>

`-FLT_MAX`, a.k.a. `std::numeric_limits<float>::lowest()`, approximately `-3.4e+38`.

<table class="floating-point-format"><tr><td bgcolor="#cefcff">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">0</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td></tr></table>

`-HUGE_VALF`, a.k.a. `-std::numeric_limits<float>::infinity()`:

<table class="floating-point-format"><tr><td bgcolor="#cefcff">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td></tr></table>

Signaling NaNs with negative sign bits:

<table class="floating-point-format"><tr><td bgcolor="#cefcff">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">1</td></tr></table>
<p></p>
<table class="floating-point-format"><tr><td bgcolor="#cefcff">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td></tr></table>

Quiet NaNs with negative sign bits:

<table class="floating-point-format"><tr><td bgcolor="#cefcff">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td><td bgcolor="#feafb2">0</td></tr></table>
<p></p>
<table class="floating-point-format"><tr><td bgcolor="#cefcff">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#acfeb4">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td><td bgcolor="#feafb2">1</td></tr></table>


## Implementing IEEE 754's `totalOrder`

IEEE 754 specifies a `totalOrder` predicate on floats
(standardized as `std::strong_order` in C++20) which orders
the floats like this:

* Negative quiet NaNs, ordered by payload bits.
* Negative signaling NaNs, ordered by payload bits.
* Negative infinity.
* Negative normal and denormal numbers.
* Negative zero.
* Positive zero.
* Positive normal and denormal numbers.
* Positive infinity.
* Positive signaling NaNs, ordered by payload bits.
* Positive quiet NaNs, ordered by payload bits.

[According to Stack Overflow](https://stackoverflow.com/questions/59348310/how-to-implement-the-totalorder-predicate-for-floating-point-values/)
this is equivalent to comparing the bit patterns as if they were
sign-magnitude integers (note: _not_ ordinary two's-complement integers)...
with the caveat that negative zero should be ordered less-than positive zero,
so if the sign bit was set, you should subtract 1 from the two's-complement
representation before comparing.
I believe this can be implemented by the following C++20 algorithm:

    constexpr std::strong_ordering totalOrder(float x, float y)
    {
        int rx = std::bit_cast<int>(x);
        int ry = std::bit_cast<int>(y);
        rx = (rx < 0) ? (INT_MIN - rx - 1) : rx;
        ry = (ry < 0) ? (INT_MIN - ry - 1) : ry;
        return rx <=> ry;
    }
