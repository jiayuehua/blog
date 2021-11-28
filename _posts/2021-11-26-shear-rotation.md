---
layout: post
title: "Image rotation by three affine transformations"
date: 2021-11-26 00:01:00 +0000
tags:
  image-processing
  math
  pretty-pictures
  web
---

Via that same [Hacker News](https://news.ycombinator.com/item?id=29185167) piece
that generated ["Image rotation by shifting pixels"](/blog/2021/11/13/pixel-rotation/) (2021-11-13):
Any angular rotation can be expressed as the sum of three shear transformations.
Shear right, then down, then right again; the result is an angular rotation!

In writing this JavaScript code I learned about the
[`context.setTransform`](https://developer.mozilla.org/en-US/docs/Web/API/CanvasRenderingContext2D/setTransform)
function, which makes the code for shearing an image almost disappointingly easy,
and this "rotation by composing affine transformations" business almost obvious.

Unlike the previous method for rotating by 90 degrees, this sum-of-shears method
requires some pixels to move by non-integer amounts. You can deal with this either
by interpolating their color values (which introduces blurring, even when rotating
by 90 degrees) or by snapping to the pixel grid (which makes this approach
"pixel-conserving" in the same way as the previous recursive-rotation approach).

Check it out on your own sample image below:

<iframe src="/blog/code/2021-11-26-shear-rotation.html" width="512px" height="auto" onload="this.height = this.contentWindow.document.body.scrollHeight + 'px';">
[Click here to play with rotation by shearing!](/blog/code/2021-11-26-shear-rotation.html)
</iframe>

The Javascript/Canvas code embedded in the iframe above is also downloadable
[here](/blog/code/2021-11-26-shear-rotation.html).
