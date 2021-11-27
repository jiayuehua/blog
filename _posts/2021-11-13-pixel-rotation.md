---
layout: post
title: "Image rotation by shifting pixels"
date: 2021-11-13 00:01:00 +0000
tags:
  image-processing
  pretty-pictures
  web
---

Via [Hacker News](https://news.ycombinator.com/item?id=29185167): You may
know that you can reverse a string by first reversing the order of the words
in the string, then reversing the characters in each word. (So `hello world`
becomes `world hello`, and then we recursively reverse each word to get `dlrow olleh`.)
Well, it turns out you can do the same kind of thing in two dimensions:
to rotate an image by 90 degrees, you can just rotate the order of the
quadrants, and then recursively rotate each quadrant.

This is neat because it provides a way to losslessly rotate a non-square
image by 90 degrees without changing its aspect ratio: a portrait
remains a portrait, and a landscape remains a landscape, even after
rotation. I say "losslessly" because all the pixels are still there:
no resampling occurs. The rotation algorithm simply shuffles the pixels
around into its best approximation of a rotated image.

Check it out on your own sample image below:

<iframe src="/blog/code/2021-11-13-pixel-rotation.html" width="512px" height="auto" onload="this.height = this.contentWindow.document.body.scrollHeight + 'px';">
[Click here to play with pixel-conserving rotations!](/blog/code/2021-11-13-pixel-rotation.html)
</iframe>

The Javascript/Canvas code embedded in the iframe above is also downloadable
[here](/blog/code/2021-11-13-pixel-rotation.html).

See also:

* ["Image rotation by three affine transformations"](/blog/2021/11/26/shear-rotation/) (2021-11-26)
