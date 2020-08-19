---
layout: post
title: 'MathJax in Jekyll'
date: 2018-08-05 00:02:00 +0000
tags:
  how-to
  knuth
  web
---

I spent most of today figuring out how to write the mathematics in
[my previous post on quantum circuits](/blog/2018/08/05/quantum-circuits).
It turned out to be way easier than I was making it.

(This post assumes that you are already familiar enough with [TeX](https://www.tug.org/whatis.html),
or should I say, $$\rm\TeX$$.
I have found the [TeX StackExchange](https://tex.stackexchange.com) to be super useful.)

[Dason Kurkiewicz's blog post from October 2012](http://dasonk.com/blog/2012/10/09/Using-Jekyll-and-Mathjax)
is still surprisingly accurate. Step one is to follow the most basic
possible instructions from [MathJax's own Getting Started
guide](http://docs.mathjax.org/en/latest/start.html#using-a-content-delivery-network-cdn):
place the snippet

    <script type="text/javascript" async
      src="https://cdnjs.cloudflare.com/ajax/libs/mathjax/2.7.5/MathJax.js?config=TeX-MML-AM_CHTML">
    </script>

somewhere that Jekyll will pick it up. (You don't even need the suggested `config=TeX-MML-AM_CHTML`
parameter; we're going to specify our own config.)

Step two is to specify the config, right above (not below!) the script tag that fetches
`MathJax.js`.

    <script type="text/x-mathjax-config">
      MathJax.Hub.Config({
        extensions: [
          "MathMenu.js",
          "MathZoom.js",
          "AssistiveMML.js",
          "a11y/accessibility-menu.js"
        ],
        jax: ["input/TeX", "output/CommonHTML"],
        TeX: {
          extensions: [
            "AMSmath.js",
            "AMSsymbols.js",
            "noErrors.js",
            "noUndefined.js",
          ]
        }
      });
    </script>

Step three is to realize that the Markdown processor used by Jekyll — its name is "Kramdown" —
has a built-in feature called ["math blocks"](https://kramdown.gettalong.org/syntax.html#math-blocks)
which can be hooked up to MathJax! This means that the rest of the integration has already been done for you.
Having followed steps 1 and 2 above, you can now just write TeX code with double-dollar-sign escapes:

    ... a given wire happens to be carrying "$$\lvert 0\rangle$$."
    By that we mean that it's carrying the linear combination
    $$\begin{psmallmatrix} 1 \\ 0 \end{psmallmatrix}$$ ...

This renders as:

... a given wire happens to be carrying "$$\lvert 0\rangle$$."
By that we mean that it's carrying the linear combination
$$\begin{psmallmatrix} 1 \\ 0 \end{psmallmatrix}$$ ...

And if you make a paragraph that contains nothing but a `$$`-escaped chunk of math,
it will be rendered using MathJax's `mode=display`, i.e., TeX display mode.

To see how to enable MathJax for your own Jekyll blog, [click through to the relevant commit
in this blog's GitHub repository](https://github.com/Quuxplusone/blog/commit/a829f7fae66a51771c30ed259739650524c60e66).

----

The rabbit-hole that I went down by accident was that I didn't realize until very late
that Kramdown already supported "math blocks." So I spent some time trying to use MathJax's
"tex2jax.js" preprocessor — which is very easy to add to the config block above, by the way.

    //...
      extensions: [
        "tex2jax.js",  // HERE
        "MathMenu.js",
        "MathZoom.js",
        "AssistiveMML.js",
        "a11y/accessibility-menu.js"
      ],
      tex2jax: {      // AND HERE
        inlineMath: [['$', '$']],
        displayMath: [['$$', '$$']]
      },
      jax: ["input/TeX", "output/CommonHTML"],
    //...

But "tex2jax.js" runs on the text of the page *after* Kramdown gets done with it; which is to say,
after Kramdown has already processed out all of the `$$`s and replaced them with `<script type="math/tex">`
tags. What's more, Kramdown overloads `$$` to mean *both* "inline" and "display," depending on the
surrounding linebreaks. So the upshot of that interaction was that I kept writing `$$` (with no
surrounding linebreaks) expecting display math, and what I got on the rendered page was inline math.
It took me forever to figure out that this was due to Kramdown, and not a bug either in my config or
in "tex2jax.js"!

The fix for this issue, of course, was to find out that Kramdown math blocks were a better solution
than "tex2jax.js".

----

UPDATE, 2020-08-19: Since this post was written, I've upgraded to MathJax v3.0.5.
See ["MathJax v3 in Jekyll"](/blog/2020/08/19/mathjax-v3-in-jekyll) for an update.
