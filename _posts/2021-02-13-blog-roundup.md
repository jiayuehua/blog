---
layout: post
title: "Blog roundup: Auftragstaktik, files, lesser-known cryptocurrencies"
date: 2021-02-13 00:01:00 +0000
tags:
  blog-roundup
  concepts
  etymology
  jokes
  templates
---

Once again I've fallen down the very deep rabbit hole of Bret Devereaux's
blog _A Collection of Unmitigated Pedantry_. (Previously on this blog:
["What I'm reading lately"](/blog/2020/07/06/blog-roundup/) (2020-07-06).)
This time it's a multi-part series on the myth of the
"universal experience of war":

* ["Part I: Soldiers, Warriors, and..."](https://acoup.blog/2021/01/29/collections-the-universal-warrior-part-i-soldiers-warriors-and/)
* ["Part IIa: The Many Faces of Battle"](https://acoup.blog/2021/02/05/collections-the-universal-warrior-part-iia-the-many-faces-of-battle/)
* ["Part IIb: A Soldier's Lot"](https://acoup.blog/2021/02/12/collections-the-universal-warrior-part-iib-a-soldiers-lot/)
* ["Part III: The Cult of the Badass"](https://acoup.blog/2021/02/19/collections-the-universal-warrior-part-iii-the-cult-of-the-badass/)

In the process, I learned that the word "shock" has many meanings — feeling shocked ([shocked!](https://www.youtube.com/watch?v=SjbPi00k_ME)),
going into shock (cardiac, septic, hemorrhagic), getting an electric shock, being hit by a shock wave —
but these are all metaphorical modernisms. The root meaning here is the one from "shock absorber"
and "shock troops": the sudden and devastating _collision_ of two bodies.
(Compare Spanish _chocar_. English "choke," on the other hand, may be derived from the same
root as "cheek.")

I also learned a bit about [_Auftragstaktik_](https://en.wikipedia.org/wiki/Mission-type_tactics),
of which Wikipedia says:

> For a mission-focused command to succeed, it is crucial that subordinate leaders:
>
> * understand the [intent](https://en.wikipedia.org/wiki/Intent_(military)) of their orders,
>
> * are given proper guidance, and
>
> * are trained to act independently.
>
> The obverse of this is the implicit requirement imposed on superior commanders:
>
> * to give their subordinates no more orders than are essential
>     (every order given is regarded as an additional constraint upon its recipient), and
>
> * to be extremely rigorous, absolutely clear, and very succinct in the expression of their commands.

Sounds kind of like the philosophy of C++ templates to me!
To write a good piece of template code, as the "superior commander," it's a good idea
not to overconstrain your function templates. Vice versa, it is sometimes crucial that
the "subordinate" (the template implementation) be able to act independently. For
example, rather than drudging forward, `++` by `++`, like some Early Modern
musketeer, `std::advance` has the freedom to exploit the local terrain and use `+=`
when possible, even though the "orders" given by its "superior commander" (its template-head)
don't explicitly mention `+=` as part of the mission parameters.

Naturally, this works only when the subordinate template is able to understand the intent
of their orders. The superior officer is expected to be "extremely rigorous" in expressing
their (minimal) syntactic constraints; but it's equally important that the subordinate
understand and respect the more fluid _semantic requirements_ of the overall strategy.

Don't look at this allegory too closely. :)

----

Quite a long while ago, I read and enjoyed Dan Luu's
["Files are fraught with peril"](https://danluu.com/deconstruct-files/) (July 2019),
and I've kept it open in a tab until I've utterly forgotten what I was going to
say about it.

I think my main takeaway was that you can start with a nice clean API, but then
as users push for more control (over performance, or over error-tolerance and correctness
for that matter), you need to expose ickier and ickier parts of the implementation
into the public API. `fprintf` works great until someone needs `fflush`... which works
great until someone needs `fsync`... which works great until...

----

Finally, via Hacker News, a
["List Of Fictional Cryptocurrencies Banned By The SEC"](https://astralcodexten.substack.com/p/list-of-fictional-cryptocurrencies)
(February 2021). This was my first awareness that Scott Alexander's _Slate Star Codex_
seems to have been resurrected as _Astral Codex Ten_?

> <b>RedCoin:</b> Karl Marx always said that communism would be a non-hierarchical economic system that
> prospered after the state withered away. A group of Marxist intellectuals took the obvious next step
> and made it an altcoin. RedCoin is notable for its reverse-proof-of-stake; you get more RedCoin in
> proportion to how <b>little</b> RedCoin you have right now, ensuring that all wallets naturally
> tend toward an equal amount.

If you liked that, be sure to check out [_UNSONG_](http://unsongbook.com/).
Or ease into _UNSONG_ with some of Scott's
["Little-Known Types Of Eclipse"](https://slatestarcodex.com/2019/05/02/little-known-types-of-eclipse/) (May 2019).
