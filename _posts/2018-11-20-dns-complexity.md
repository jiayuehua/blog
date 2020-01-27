---
layout: post
title: 'How DNS got so complicated'
date: 2018-11-20 00:01:00 +0000
tags:
  networking
  sre
  wg21
---

From Bert Hubert's blog post
["The DNS Camel"](https://blog.powerdns.com/2018/03/22/the-dns-camel-or-the-rise-in-dns-complexit/) (2018-03-22):

> Before visiting I read up on recent DNS standardization activity,
> and I noted a ton of stuff was going on. In our development work,
> I had also been noticing that many of the new DNS features interact
> in unexpected ways. [...]
>
> My claim is that this rise [in complexity] is not innocent.
> As DNS becomes more complex, the number of people that "get it" also goes down. 

He identifies one imbalance of pressure in the DNS world:

> In the DNS world, we have the unique situation that ([resolver](https://icannwiki.org/Domain_Name_Resolvers))
> operator feedback is largely absent. Only a few operators manifest themselves in the standardization community [...]
> In reality, large scale resolver operators are exceptionally weary of new DNS features
> and turn off whatever features they can [...]
>
> On the developer front, the DNS world is truly blessed with some of the most gifted programmers in the world.
> The current crop of resolvers and authoritative servers is truly excellent. DNS may well be the best served
> protocol in existence today. This high level of skill also has a downside however. DNS developers frequently
> see immense complexity not as a problem but as a welcome challenge to be overcome. We say yes to things we
> should say no to. Less gifted developer communities would have to say no automatically since they simply
> would not be able to implement all that new stuff. We do not have this problem. We're also too proud to
> say we find something (too) hard.
>
> Finally, the standardization community has its own issues. A show of hands made it clear that
> almost no one in the WG session was actually on call for DNS issues. Standardizers enjoy complexity
> but do not personally bear the costs of that complexity.

Bert suggests that DNS's complexity problem, and the causes of that problem, are unique to DNS.
I'd say that these problems, and causes, are at least equally applicable to C++ standardization.
I recommend reading [all of Bert's essay](https://blog.powerdns.com/2018/03/22/the-dns-camel-or-the-rise-in-dns-complexit/) — it's a good one.
