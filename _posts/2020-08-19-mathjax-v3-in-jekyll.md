---
layout: post
title: 'MathJax v3 in Jekyll'
date: 2020-08-19 00:01:00 +0000
tags:
  how-to
  knuth
  web
excerpt: |
  Last week, as I proofread my post ["Concepts can't do quantifiers"](/blog/2020/08/10/concepts-cant-do-quantifiers)
  (2020-08-10), I noticed that its inline math wasn't rendering correctly.
  I'd write `$$\neg\exists x\in X:P(x)$$` in the Markdown,
  but instead of rendering on the live page as "$$\neg\exists x\in X:P(x)$$,"
  it would render as "`\(\neg\exists x\in X:P(x)\)`" (just without the teletype font).

  I never figured out precisely what broke — it could have been some internal change to GitHub Pages'
  Kramdown implementation, or some change on the CDN from which my blog includes `mathjax.js` —
  but anyway, I solved the problem by upgrading from MathJax v2 to MathJax v3.
---

_This is an update to my two-year-old post ["MathJax in Jekyll"](/blog/2018/08/05/mathjax-in-jekyll) (2018-08-05)._

Last week, as I proofread my post ["Concepts can't do quantifiers"](/blog/2020/08/10/concepts-cant-do-quantifiers)
(2020-08-10), I noticed that its inline math wasn't rendering correctly.
I'd write `$$\neg\exists x\in X:P(x)$$` in the Markdown,
but instead of rendering on the live page as "$$\neg\exists x\in X:P(x)$$,"
it would render as "`\(\neg\exists x\in X:P(x)\)`" (just without the teletype font).

I never figured out precisely what broke — it could have been some internal change to GitHub Pages'
Kramdown implementation, or some change on the CDN from which my blog includes `mathjax.js` —
but anyway, I solved the problem by upgrading from MathJax v2 to MathJax v3.

MathJax v3 has a radically different configuration story from v2,
although it starts the same way.
You still place a snippet someplace where Jekyll will pick it up:

    <script type="text/javascript" id="MathJax-script" async
      src="https://cdn.jsdelivr.net/npm/mathjax@3/es5/tex-chtml.js">
    </script>

Step two is to specify the config, right above (not below!) that script tag.
The config object in MathJax v2 was called `MathJax.Hub.Config`; but in v3
the config goes directly into the `MathJax` object itself, and all the other stuff that
used to hang off of the `MathJax` object (e.g. `MathJax.InputJax`)
now hangs off of `MathJax._` instead (e.g. `MathJax._.input`).

My MathJax v3 config looks like this:

    <script type="text/javascript">
    window.MathJax = {
      tex: {
        packages: ['base', 'ams']
      },
      loader: {
        load: ['ui/menu', '[tex]/ams']
      }
    };
    </script>

Step three hasn't changed from my previous post ["MathJax in Jekyll"](/blog/2018/08/05/mathjax-in-jekyll/) (2018-08-05).
Having followed steps 1 and 2 above, you can now mix TeX code into your Markdown, delimiting
the TeX with double-dollar-sign escapes:

    ... a given wire happens to be carrying "$$\lvert 0\rangle$$."
    By that we mean that it's carrying the linear combination
    $$\begin{psmallmatrix} 1 \\ 0 \end{psmallmatrix}$$ ...

This renders as:

... a given wire happens to be carrying "$$\lvert 0\rangle$$."
By that we mean that it's carrying the linear combination
$$\begin{psmallmatrix} 1 \\ 0 \end{psmallmatrix}$$ ...

And if you make a paragraph that contains nothing but a `$$`-escaped chunk of math,
it will be rendered using MathJax's `mode=display`, i.e., TeX display mode.

----

There is one cheat here: MathJax doesn't come with the `\begin{psmallmatrix}` environment
set up out of the box. So I had to fake it, in both my old v2 install and my new v3 install.
You can see how that was done in the commits below. I did the v2 one myself, by cargo-culting some
JavaScript code from MathJax's implementation of the AMSmath package. For the v3 one,
I [got help](https://stackoverflow.com/questions/63428119/)
from Davide Cervone, a maintainer of the MathJax project; many thanks to him!

Considering the negligible (zero?) rendering difference between
`$$\left(\begin{smallmatrix} 1 \\ 0 \end{smallmatrix}\right)$$`
$$\left(\begin{smallmatrix} 1 \\ 0 \end{smallmatrix}\right)$$ and
`$$\begin{psmallmatrix} 1 \\ 0 \end{psmallmatrix}$$`
$$\begin{psmallmatrix} 1 \\ 0 \end{psmallmatrix}$$, if I had to do my quantum-computing
post over again, I wouldn't even try to implement `\begin{psmallmatrix}`.
But I'm extremely susceptible to [nerdsniping](https://xkcd.com/356/).

----

To see how to enable MathJax for your own Jekyll blog, click through to the relevant commits
in this blog's GitHub repository:

* [my original commit](https://github.com/Quuxplusone/blog/commit/eaafdc999a6385cba963ab471a67d6721cbd2964)
* [my v2-to-v3 upgrade commit](https://github.com/Quuxplusone/blog/commit/420dbabb235ee9ce27fadb25bacdbecf3e727a5b)

