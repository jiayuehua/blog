---
layout: post
title: "Notes on Prince Rupert's Problem"
date: 2019-07-22 00:01:00 +0000
tags:
  math
  science
  today-i-learned
---

The other day I learned that [Prince Rupert's drops](https://en.wikipedia.org/wiki/Prince_Rupert%27s_drop)
are named for [Prince Rupert of the Rhine](https://en.wikipedia.org/wiki/Prince_Rupert_of_the_Rhine)
(1619–1682). This guy was one of the founding members of the Royal Society, as well as a cavalry officer
and (in his capacity as the first governor of the Hudson's Bay Company) the namesake of
[Rupert's Land](https://en.wikipedia.org/wiki/Rupert%27s_Land).

He was also the namesake of a mathematical oddity known as [Prince Rupert's cube](https://en.wikipedia.org/wiki/Prince_Rupert%27s_cube).
This is the observation that if you look at a unit cube along its space diagonal, it looks like a hexagon
with side length $$\sqrt{2}/\sqrt{3}$$; and within that hexagon, you can
[inscribe a square](http://www.drking.org.uk/hexagons/misc/deriv3.html)
with side length $$(3 - \sqrt{3})\cdot\sqrt{2}/\sqrt{3} = \sqrt{6} - \sqrt{2}$$,
or roughly 1.0353. So, you can take a unit cube and drill through it a square hole of
side length _more_ than unit — through which you can pass a cube even larger than the original!

What's more, by tipping the cube a little bit off of its space diagonal,
you can increase the side length of the inscribed square hole up to $$\frac{3}{4}\sqrt{2} \approx 1.0607$$.
Or, to put it another way, you can pass one unit cube through a close-fitting hole in another, and still
have 0.0607 units of "border" left for holding the outer cube's pieces together. (This construction was
discovered in the 18th century by [Pieter Nieuwland](https://en.wikipedia.org/wiki/Pieter_Nieuwland).)

[This 2005 paper](https://web.archive.org/web/20100705014816/http://www.math.usma.edu/people/Rickey/papers/ShortCourseAlbuquerque.pdf)
by logician and historian [V. Frederick Rickey](https://en.wikipedia.org/wiki/V._Frederick_Rickey) gives a
very entertaining account of this and some other historical math problems.

----

There exist [patterns](https://geekhaus.com/math103_fall2017/2017/10/05/open-project-prince-ruperts-cube/)
for 3D-printing your own Prince Rupert's Cube.
([There's a very impressive one on YouTube.](https://www.youtube.com/watch?v=e3-Ta9DF4Cg))
Notice that after you cut a unit hole through a unit cube, there's not much left.
If you didn't know it started out as a cube, you might describe it as a "crumpled ring."

Wikipedia tells me that analogous problems give rise to "Prince Rupert's tetrahedron" and
"Prince Rupert's octahedron." There doesn't seem to be nearly as much literature on these shapes, though.

Prince Rupert's tetrahedron is sad-looking — the hole demolishes one of the four
vertices, leaving only an awkward triangular ring.
(See ["A regular tetrahedron passes through a hole smaller than its face"](https://pdfs.semanticscholar.org/f673/e85125df6ebe3becb3c573019f9ac29dc833.pdf)
(Hiroshi Maehara and Norihide Tokushige, 2008). Sadly, no pictures.)

I have not yet been able to wrap my head around "Prince Rupert's octahedron." If you know of a picture or
animation of such a beast, please tell me about it!

(I also have no intuitive grasp on why there should be no "Prince Rupert's dodecahedron" or "Prince Rupert's icosahedron."
Or maybe there are such things but Wikipedia doesn't mention them.)

----

Wikipedia tells me that after we use Pieter Nieuwland's construction to cut a 1.0607-unit square hole through a unit cube,
the remaining "crumpled ring" (consisting of two triangular prisms and two irregular tetrahedra) has
volume $$\frac{25}{256} \approx 0.0976$$. My brute-force computer calculations ([code here](/blog/code/2019-07-22-prince-ruperts-cube.cpp))
tell me that if we cut a 1.0-unit square hole along the space diagonal of the cube, the remainder
(consisting of a single 3D-printable shape) has volume $$\sim 0.163$$.
And if we cut a 1.0-unit square hole along Nieuwland's axis, the remainder (consisting of a single 3D-printable shape)
has volume $$\sim 0.142$$. (Don't quote me authoritatively on these numbers! I may have made a programming error.)

Interestingly, the volume left over from Nieuwland's construction is _less_ than the volume left over from the
naïve space-diagonal construction.
However, I would expect that Nieuwland's version would probably hold together more solidly than
the naïve version — that is, its "wasp-waisted" portions would be wider.
I'd be interested to know whether this is true.
