---
layout: post
title: 'Assume a spherical politician'
date: 2018-07-04 00:01:00 +0000
tags:
  morality
  science
  us-politics
---

Just in time for Independence Day comes
[this news item](https://www.cmu.edu/news/stories/archives/2017/november/i-cut-you-choose-cake-cutting-protocol-inspires-solution-to-gerrymandering.html)
from my alma mater:

> "I-Cut-You-Choose" Cake-Cutting Protocol Inspires Solution to Gerrymandering
>
> Just as having one child cut the cake and giving the second child first choice
> of the pieces avoids either feeling envious, having two political parties sequentially
> divide up a state in an "I-Cut-You-Freeze" protocol would minimize the practice of
> gerrymandering, where a dominant political party draws districts to maximize its
> electoral advantage.
>
> [The CMU protocol](http://procaccia.info/papers/gerrymandering.pdf),
> developed by Ariel Procaccia, associate professor of computer
> science, and [Wesley Pegden](http://www.math.cmu.edu/~wes/gerrymandering.html),
> associate professor of mathematical sciences, is the
> first to allow a fair division of a state into political districts without
> independent agents. It calls for one political party to divide a map of a state
> into the allotted number of districts, each with equal numbers of voters. Then
> the second party would choose one district to "freeze," so no further changes
> could be made to it, and re-map the remaining districts as it likes.
> The first party then would choose a second district to "freeze" from this map
> and redraw the remaining districts ... until all of the districts are frozen.
>
> Pegden and Procaccia acknowledge that their analysis assumes an idealized setting,
> but believe their protocol would have similar properties in the real world.

![This photo accompanies the news item above.](/blog/images/2018-07-04-gerrymandering.jpg)

Study question 1: In the above diagram, which hand represents the Democratic Party
and which represents the Republican Party?

Study question 2: What is represented by the cake in this metaphor?

Study question 3: In the above diagram, which direction is the slice of cake moving?

----

Cake-cutting protocols are [a well-known and long-studied topic in
mathematics](https://en.wikipedia.org/wiki/Fair_cake-cutting); Pegden and Procaccia's
approach is an iterated version of the "I-cut-you-choose" protocol.
"I-cut-you-choose" is designed to produce *envy-free* divisions, but not
necessarily "best" divisions by other metrics.
[Wikipedia:](https://en.wikipedia.org/w/index.php?title=Divide_and_choose&oldid=848151876#Efficiency_issues)

> Divide-and-choose might produce inefficient allocations.
>
> One commonly used example is a cake that is half vanilla and half chocolate.
> Suppose Bob likes only chocolate, and Carol only vanilla. If Bob is the cutter
> and he is unaware of Carol's preference, his safe strategy is to divide the
> cake so that each half contains an equal amount of chocolate. But then,
> regardless of Carol's choice, Bob gets only half the chocolate and the
> allocation is clearly not Pareto efficient.
>
> It is entirely possible that Bob,
> in his ignorance, would put all the vanilla (and some amount of chocolate)
> in one larger portion, so Carol gets everything she wants while he would
> receive less than what he could have gotten by negotiating.

Study question 4: In the scenario above, which U.S. political party is represented
by Bob? Which party is represented by Carol?

[Wikipedia again:](https://en.wikipedia.org/w/index.php?title=Fair_cake-cutting&oldid=841543600#Additional_requirements)

> In addition to the desired properties of the final partitions, there are also
> desired properties of the division process. One of these properties is
> <b>truthfulness</b> (a.k.a. <b>incentive compatibility</b>), which comes in two levels.
>
> <b>Weak truthfulness</b> means that if the partner reveals his true value measure to
> the algorithm, he is guaranteed to receive his fair share (e.g. 1/n of the
> value of the entire cake, in case of proportional division), regardless of
> what other partners do. Even if all other partners make a coalition with
> the only intent to harm him, he will still receive his guaranteed proportion.
> Most cake-cutting algorithms are truthful in this sense.
>
> <b>Strong truthfulness</b> means that no partner can gain from lying; i.e., telling
> the truth is a [dominant strategy](https://en.wikipedia.org/wiki/Strategic_dominance).
> Most cake-cutting protocols are not strongly truthful.

(Here Wikipedia's footnote points to the paper
["Truth, Justice, and Cake Cutting"](https://dash.harvard.edu/bitstream/handle/1/8896229/truth_justice_and_cake.pdf)
(2010), which shares two authors with [the paper](http://procaccia.info/papers/gerrymandering.pdf)
that produced our news item.)

Study question 5: Suppose now that you are the Republican Party, during a redistricting
of a certain state into 10 districts. The population of the state is about 45% Democratic,
45% Republican, and 10% Green Party. The Democratic Party cuts first, and offers a set of
ten districts — four solidly Republican, four solidly Democratic, one split 50/50, and one
solidly Green. Which district do you choose to freeze, and why?

Study question 6: What *should* the Democratic Party have done?
