---
layout: post
title: "Discrete Peaceful Encampments, Bernie Sanders version"
date: 2020-03-21 00:01:00 +0000
tags:
  litclub
  math
  morality
  puzzles
  us-politics
---

Previously on this blog:

* ["Discrete Peaceful Encampments"](/blog/2019/01/24/discrete-peaceful-encampments/) (2019-01-24)

* ["Discrete Peaceful Encampments, with tables"](/blog/2019/10/18/discrete-peaceful-encampments-with-tables/) (2019-10-18)

I've been continuing to run Dmitry Kamenetsky's random-search solution-finding program in my laptop's
copious free time; you may have noticed that I've made many updates to my 2019-10-18 blog post.
That post's URL remains the "source of truth" as re my best known solutions to the peaceable coexisting
queens problem; which, again, is:

> On a board of $$n\times n$$ squares, encamp $$c$$ armies of chess queens "peaceably,"
> so that no pair of queens from different armies attacks each other. Maximize the size of
> the smallest army.

The $$c=2$$ version of this problem gives rise to [OEIS sequence A250000](https://oeis.org/A250000)
(the smaller army's size) and [OEIS A308632](https://oeis.org/A308632) (the bigger army's size).

Back in October, when I wrote the first version of that blog post, I considered the obvious generalization
of [OEIS A308632](https://oeis.org/A308632) to larger $$c$$ to be, "Given that we have maximized the
small<i>est</i> army's size, let us now maximize the size of the bigg<i>est</i> army." For example, after
encamping 4+4+4 queens on an 8x8 board, we can add 4 more queens to the biggest army for a total of 4+4+8.

However, it occurred to me the other day that it might be more productive
to try to maximize the _equitability_ of the arrangement. When our metric is to maximize the biggest army,
we end up with extremely lopsided arrangements that don't play well with Dmitry's "random walk" algorithm.
The random-walk algorithm is pretty good at incrementally improving a 4+4+4+4+4+4+4+8 solution into
a 4+4+4+4+4+4+4+9 solution; but it's highly unlikely to improve 4+4+4+4+4+4+4+9 into 5+5+5+5+5+5+5+5.

So I wrote an alternative scoring metric that tried to maximize the "equitability" of a solution. For
example, the most equitable solution for three armies on the 8x8 board has army sizes 4+5+6. I've added
a table of my best known "most equitable" solutions to
["Discrete Peaceful Encampments, with tables"](/blog/2019/10/18/discrete-peaceful-encampments-with-tables/) (2019-10-18);
go there for more information.

At the start of March 2020, my previous best solution for $$f(21,8)$$ had been 4 queens per army,
with one "rich" army of size $$g(21,8)=9$$. As soon as I started trying to maximize equitability, the computer
quickly found a better solution where all eight armies had size $$f(21,8)=h(21,8)=5$$. That's because jumping
from $$(4,4,4,4,4,4,4,9)$$ to $$(5,5,5,5,5,5,5,5)$$ in one random step is quite improbable; but if
our metric is equitability, then we can step from $$(4,4,4,4,4,4,4,9)$$ to $$(4,4,4,4,4,4,5,8)$$,
and then to $$(4,4,4,4,4,5,5,6)$$, and so on, because each step now counts as a measurable improvement
over the previous one.

----

Even before last week's dramatic stock-market crash, I had started reading
[_Once in Golconda_](https://amzn.to/2UaBgZn), John Brooks' 1969
tour de force subtitled "A True Drama of Wall Street 1920–1938" and blurbed accurately
by John Kenneth Galbraith as "civilized and superior history, superbly written."
It's got rollicking stories, economic history, casual allusions to Voltaire and Ibsen,
and even some new-to-me vocabulary words (as in the "magnificently
[demulcent](https://en.wiktionary.org/wiki/demulcent) sentence"
"There has been a little distress selling on the Stock Exchange...").

Also in the news is the Trump administration's [flirtation with something](https://www.bloomberg.com/news/articles/2020-03-18/mnuchin-proposes-500b-in-checks-based-on-income-family-size)
that sounds an awful lot like Universal Basic Income for wishy-washers.
I've been a lowkey fan of UBI ever since I read about
[the $6000-a-year experiment in Stockton, California](https://nymag.com/intelligencer/2019/10/universal-basic-income-stockton-california.html);
and then of course there was 2020 Presidential candidate Andrew Yang's $12000-a-year
["Freedom Dividend."](http://web.archive.org/web/20191230065530/https://www.yang2020.com/what-is-freedom-dividend-faq/)

As far as I know, I don't personally know anyone who'd view a one-time $1000 payment as a life-changing
amount of money... or even as adequate compensation for a month of unemployment.
(But if I'm wrong about that, tell me!)  I _definitely_ know people who'd see UBI of $12000 a year
as life-changing. For me, I'd characterize $12000 a year as "a noticeable and welcome increase
in one's tax refund check."
Above my economic class, there's the multimillionaires, for whom $12000 would be merely
a rounding error on their
[$1,528,000](https://web.archive.org/web/20190709194318/https://go.joebiden.com/page/-/vpdocs/Biden%202018%20Amended%20Federal.pdf) (Biden)
or [$3,624,000](http://web.archive.org/web/20160812185435/https://www.npr.org/2016/08/12/489776309/in-an-effort-to-pressure-trump-clinton-releases-tax-rate) (Clinton)
or [$4,619,000](http://web.archive.org/web/20150211063836/https://www.efile.com/tax-form/tax-history/Mitt-Romney-2010-Tax-Return.pdf) (Romney)
tax bill.
And then above _their_ class, there's the billionaires — what Warren Buffett
[calls](http://web.archive.org/web/20111013171249/http://online.wsj.com/public/resources/documents/buffetletter01.pdf)
the "ultra-rich" — for whom the comparison to income tax goes completely off the rails.
Warren Buffett [claims](http://web.archive.org/web/20161011073515/https://www.businesswire.com/news/home/20161010005859/en/Tax-Facts-Donald-Trump)
that his 2015 tax bill was $1,845,557; but that he donated $2,858,057,970 — roughly 1500 times that amount —
to charity in the same year. (By utter coincidence, that $2.8 billion is about what it would cost to
implement Andrew Yang's UBI plan for the adult population of Stockton, California, for a year.
To do it for the adult population of the whole United States would take 1000 times that amount.)

Anyway, I guess what I'm getting at here is that there's an awful lot of potential energy tied
up in those biggest armies; and if we can rejigger our metrics to value more equitable redistribution,
then we might find it easier to raise the size of the smallest armies even without changing anything else
about the algorithm.

Food for thought.
