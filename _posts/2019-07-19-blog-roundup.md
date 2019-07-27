---
layout: post
title: "What I'm reading lately"
date: 2019-07-19 00:01:00 +0000
tags:
  blog-roundup
  litclub
  morality
  old-shit
  sre
---

This past week has been beach week, and thus beach-reading week, for me.
What's in my outbox?

----

["C Is Not a Low-level Language"](https://queue.acm.org/detail.cfm?id=3212479)
(David Chisnall, April 2018). The author argues that "for a language to be
_close to the metal_, it must provide an abstract machine that maps easily
to [...] the target platform." The interesting thing about today's popular platforms
such as x86-64 is that the actual hardware is doing a bunch of stuff down _below_
the level of abstraction that is exposed to compiled code — it's doing register renaming,
memory caching, speculation, and so on. Compiled code never sees these things at all.
In essence, what we've got today are "fast PDP-11 emulators."

I'm not sure I see how "C" in particular is relevant here; the argument seems to be
that even assembly language — even x86-64 machine code — are no longer low-level languages,
because the abstract platform that they target ("the x86-64 ISA") is so far above the
level of the raw machine to begin with. But then, I'm an x86 chauvinist, and Chisnall's
argument isn't just about x86-64; it's also about C when compiled for PowerPC or for RISC-V
or for anything.

----

["Choose Boring Technology"](http://boringtechnology.club/) (Dan McKinley, July 2015 or
January 2018 depending on which dateline you believe). The author argues quite persuasively
that you should "prefer technology that’s well understood, with failure modes that are known."
My SRE side loves this essay.

As usual, I see plenty of aphorisms that remind me of WG21 (the C++ standardization committee),
including:

- "If you're adding a redundant piece of technology, your goal is to replace something
    with it. Your goal shouldn’t be to operate two pieces of technology that are redundant with one
    another forever."

- "The new thing won’t be better; you just aren’t aware of all of the ways it will be terrible yet."

- "The grim paradox of this law of software is that you should probably be using the tool that
    you hate the most. You hate it because you know the most about it."

- "Happiness comes from shipping stuff." (This is the ironic one for WG21, because for WG21,
    the "product" and the "[new] technology stack" are the same thing. I've observed firsthand
    how happy people become at the prospect of shipping stuff.)

----

[_War with the Newts_](http://gutenberg.net.au/ebooks06/0601981h.html) (Karel Čapek, 1936).
Most people who've heard of Czech author [Karel Čapek](https://en.wikipedia.org/wiki/Karel_%C4%8Capek) have heard
of him as the author of _Rossum's Universal Robots_, the play that introduced the word _robot_
to English-speakers. (Some readers might even know him as the namesake of Richard Pattis's
[Karel the Robot](https://amzn.to/2SryxIq),
to which I was exposed in my first high-school computer science course.)

Anyway, _War with the Newts_ is fantastically good. It's an (at times _vicious_) satire on
colonialism, race relations, gender relations, capitalism, militarism, animal experimentation,
diplomacy — even foreshadowing nuclear proliferation,
I thought, despite the fact that Čapek died in 1938.

It starts with the discovery that humans
share the planet with another intelligent race — a race of humanoid salamanders living in one
particular bay in the East Indies.

> "So what are we going to call the film?" Fred insisted. "_Milwaukee industrialist's daughter films prehistoric reptiles_?"
>
> "_Primordial lizards praise youth and beauty_," suggested Abe poetically.
>
> The captain cleared his throat. "I think the title ought to have a scientific sound to it, sir. Something formal and... well... scientific. _Anteliduvian fauna on Pacific island._"
>
> "Anteviludian," Fred corrected him. "No, wait, anteduvidian. Hell, how's it supposed to go? Anteduvidual... Antedinivian... No, that's not it. We're going to have to think up some simpler title, something that anyone can say. Judy's good at that sort of thing."
>
> "_Antediluvian_," said Judy.
>
> Fred twisted round to look at her. "That's too long, Judy. It's longer than those monsters with the tails. A title needs to be shorter. But isn't Judy great? Captain, don't you think she's great?"

The "newts" — being uniquely suited to underwater construction works — are quickly uplifted, civilized,
studied, employed, transported all around the globe... in a word, enslaved. Eventually the liberal
intellectual sphere becomes interested in the newts. Letters to the editor weigh in on
whether a newt has a soul:

> A friend of mine, the Reverend H.B. Bertram, and I observed some newts over a long period while they were building
> a dam in Aden. We also spoke with them on two or three occasions, but we found no indications of any higher feelings
> such as Honour, Faith, Patriotism or interest in Sport. And what else, may I ask, is there that could be seen as an
> indication of a soul? —Col. John W. Britton
>
> Your question left me feeling somewhat perplexed. I know, for example, that my little Chinese dog, Bibi, has a little
> and a charming soul; and I know that my Persian cat, Sidi Hanum, has a soul, so wonderful and so cruel! But newts?
> Yes, they are very talented and intelligent; the poor things are able to speak, calculate and make themselves very
> useful; but they are so _ugly!_ —Yours, Madeleine Roche
>
> They do have a soul, just as every other animal and every plant and every living thing has a soul.
> Great indeed is the secret of any life. —Sandrabhārata Nath

The civilized nations abolish the employment of newts (either for moral reasons, or simply
because they're taking jobs from Englishmen). But the newts are by this time indispensable in naval defense.
Nations continue arming their newts in secret. And then one day... well, you probably see where this is going.
It's in the title of the story, after all!

> The author frowned. Don't ask me what _I_ want. Do you think I wanted to see the continents where people live
> reduced to rubble; do you think I wanted it to end like this? That was just the logical course of events;
> what could I have done to stop that? I did everything I could; I gave people enough warning... I warned them,
> don't give the newts weapons and explosives, stop this vile trading in salamanders, and so on— and you saw
> how it all turned out. They all had a thousand good economic and political reasons why they couldn't stop.
> I'm not a politician or a businessman; how am I supposed to persuade them about these things. What are we
> supposed to do? Quite likely the world will collapse and disappear under water; but at least it will
> happen for political and economic reasons we can all understand...

----

[_The Consolation of Philosophy_](http://www.gutenberg.org/files/14328/14328-h/14328-h.htm) (Boethius,
circa 524). Wikipedia says: "It has been described as the single most important and influential work
in the West on Medieval and early Renaissance Christianity, as well as the last great Western work
of the Classical Period." But honestly I don't think it lives up to the hype. It's Stoicism, or maybe
just fatalism, leavened with a healthy dose of sophistry. It doesn't so much _grapple_ with the
[problem of evil](https://en.wikipedia.org/wiki/Problem_of_evil)
as tweak it on the nose and run away going "nyuk nyuk nyuk."

> But if to one wretched, one destitute of all good, some further evil be added besides
> those which make him wretched, is he not to be judged far more unhappy than he whose ill fortune
> is alleviated by some share of good?
>
> "It could scarcely be otherwise."
>
> Now, it is manifest that it is just for the wicked to be punished, and for them to escape unpunished is unjust.
> [...] This, too, no one can possibly deny — that all which is just is good, and, conversely, all which is
> unjust is bad.
>
> Surely, then, the wicked, when they are punished, have a good thing added to them — to wit,
> the punishment which by the law of justice is good; and likewise, when they escape punishment,
> a new evil attaches to them in that very freedom from punishment which thou hast rightly
> acknowledged to be an evil in the case of the unrighteous.
>
> "I cannot deny it."
>
> Then, the wicked are far more unhappy when indulged with an unjust freedom from punishment
> than when punished by a just retribution.

Nyuk nyuk nyuk.

At the same time, Boethius says, don't worry — the wicked will be punished after death. Maybe this
punishment remains unjust by virtue of its endlessness. Or maybe this punishment is just, being
as it's meted out by a just God. Anyway, the bottom line remains constant:

> So, shouldst thou see anything in this world happening differently from thy expectation,
> doubt not but that events are rightly ordered; it is in thy judgment that there is perverse confusion.

This may be consolation, but is it really [philosophy](https://www.etymonline.com/word/philosophy)?
