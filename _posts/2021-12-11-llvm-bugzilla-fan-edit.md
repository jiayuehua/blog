---
layout: post
title: "I sweded the LLVM Bugzilla migration"
date: 2021-12-11 00:01:00 +0000
tags:
  llvm
  sre
  web
---

Over the past two weeks, Anton Korobeynikov migrated all of the
LLVM project's issues (both open and closed) from
[the old LLVM Bugzilla](https://bugs.llvm.org/) to the new
[`llvm/llvm-project` GitHub repo](https://github.com/llvm/llvm-project/issues/).
The migration is now [complete](https://docs.google.com/document/d/11_3rgYuv-QO0g1oO6T0MmkFhacqJg6o24eWFFVNSX_o/edit).

From the outside, the migration appeared to proceed in fits and starts —
lots of status emails to the LLVM mailing list — starting, hitting unforeseen issues,
stopping, starting again... By December 2, there were enough of these emails
in my inbox that I belatedly started paying attention.
On December 4 I (rather tactlessly) suggested my own approach:

> "plan it carefully, write down your deploy plan,
> test what can be tested ahead of time, do a practice run,
> _then_ do it live"
>
> [...]
> At this point I'm offering my own technical assistance,
> just to get the thing done and stop getting these emails every day.
> Send me your Bugzilla export script; I'll test it out this week
> on a blank repo, with the goal of mirroring a 100-bug subset
> of the LLVM Bugzilla publicly visible in
> https://github.com/Quuxplusone/LLVMBugzillaTest/ by EOW.

I never received any of Anton's scripts; he just kept trucking
with the migration and, as I said, it's all done now. (Since it was
happening on the live repo, I don't think switching horses in midstream
was even a plausible option at that point.)

But I _had_ set myself that end-of-week goal; and it turned out
that a Bugzilla-to-GitHub migration was just the
kind of small-data, REST-API, human-readability-centric
project that can easily nerdsnipe me into spending a week of
evenings on it, even when my effort serves no conceivable purpose.
Find my own Bugzilla-to-GitHub migration scripts
[on GitHub here](https://github.com/Quuxplusone/BugzillaToGithub#bugzilla-to-github).

|-----------------------------------------------|-----------|
| Number of bugs                                | 51567     |
| Exported data size                            | 2.9 GB    |
| Time to export from Bugzilla (6x in parallel) | 6 hours   |
| Time to transform from XML to JSON            | 7 minutes |
| Transformed data size (sans attachments)      | 350 MB    |
| Time to import into GitHub (*)                | 17 hours  |

This is a classic Export–Transform–Load job. Step 1 is to _export_
the bug data from LLVM's Bugzilla; there's a REST API for that;
it serves XML. Step 2 is to discard malformed bugs: it turns out
that whole swaths of the bug-number space are unoccupied, starting
with [PR16181](https://bugs.llvm.org/show_bug.cgi?id=16181), and
then the whole range 29172–30171 with the exception of
[PR29222](https://bugs.llvm.org/show_bug.cgi?id=29222)... Weird.
Anyway, we discard those non-existent bugs.

Step 3 is to _transform_ that XML (with its Bugzilla-specific
schema) into the JSON schema expected by GitHub's issue import API.
This is also where we deal with the fact that Bugzilla supports
plain raw text (appropriate for a bug tracker where most bugs involve
snippets of source code), but GitHub issue comments are expected to
be formatted in Markdown.

My numbers above are cheating a little, because the bulk of that
2.9 GB is base64-encoded file attachments — which I just discarded, since
I didn't know any public API to import them anyway. The official
migration couldn't just discard those files.

Step 4 is to _load_ that JSON into GitHub.

The official migration process had two major advantages over my week-of-evenings
noodling:

- In Step 3, it's useful to have a mapping between Bugzilla email addresses
    and GitHub usernames, so that you can mark issues as "Reported By"
    an actual GitHub user, and attribute comments to actual GitHub users.
    This way, their LLVM interactions show up on their activity feed.
    But that mapping from emails to GitHub usernames needs to be generated
    somehow. LLVM did it with a survey mass-emailed to the mailing lists
    several months ago; but the results of that survey, as far as I know,
    were never made public. So I didn't have access to that mapping.
    (Of course if I wanted that mapping _now_, I could scrape the officially
    migrated issues to reconstruct it!)

- In Step 4, you want to attribute comments to the right GitHub users...
    but obviously GitHub doesn't let you just randomly forge comments
    and interactions under someone else's username! Not even if you claim
    it's "for a migration." You need someone from GitHub Engineering to
    do the issue import, using their behind-the-scenes database magic
    not accessible to mere mortals like me.

My table above says "Time to import into GitHub: 17 hours," with an asterisk.
Seventeen hours is how long it takes to import 51567 issues, in serial, knowing
you don't want to hit GitHub's
"[secondary rate limit](https://docs.github.com/en/free-pro-team@latest/rest/overview/resources-in-the-rest-api#secondary-rate-limits)"
of 5000 API requests per hour.
But since you're getting someone from GitHub SRE to do Step 4 anyway,
they won't be using [the public REST API](https://gist.github.com/jonmagic/5282384165e0f86ef105)
that I was using, and they won't care about rate limits.
So they can go much faster. Remember, the _total_ amount of data
to be transferred into GitHub is only about 350 MB, plus file attachments.
I included [a disabled codepath](https://github.com/Quuxplusone/BugzillaToGithub/blob/main/json-to-github.py#L29-L41)
to just push everything to GitHub as fast as possible; if it didn't immediately
hit the rate limit, it could probably do the upload in about 10 minutes.

The advantage of an Export–Transform–Load deploy plan is that you can
do the Export in the background while you write the Transform code;
then you can run the Transform over and over on your local machine until
the results look just right; and then you can run the Load only when
you know it'll succeed. (You also can, and should, run the Load step
multiple times on blank repos before you try it on the production repo;
and make sure to look at the results to see if they're how you like them.)
The mantra here is
"[measure twice, cut once](https://en.wiktionary.org/wiki/measure_twice_and_cut_once)."

----

So basically I spent a week making a [sweded](https://en.wikipedia.org/wiki/Be_Kind_Rewind)
version of the LLVM Bugzilla migration — a completely unofficial imitation, made without
a studio budget. (And therefore not able to do things like forge comment authorship.)

What I could and did do, though, was spend more time on the Transform step.
I think it's pretty important for the GitHub issues to be at least as readable
as the plain-text Bugzilla issues. Bugzilla even goes out of its way to
hyperlink various things, such as references to other Bugzilla bugs; so I did
the same in my scripts. The resulting bugs look nicer, in my humble opinion,
than what LLVM officially ended up with.

* [PR1001](https://bugs.llvm.org/show_bug.cgi?id=1001): [official](https://github.com/llvm/llvm-project/issues/1373), [sweded](https://github.com/Quuxplusone/LLVMBugzillaTest/issues/1001)

* [PR18418](https://bugs.llvm.org/show_bug.cgi?id=18418): [official](https://github.com/llvm/llvm-project/issues/18792), [sweded](https://github.com/Quuxplusone/LLVMBugzillaTest/issues/18417)

* [PR29122](https://bugs.llvm.org/show_bug.cgi?id=29122): [official](https://github.com/llvm/llvm-project/issues/29492), [sweded](https://github.com/Quuxplusone/LLVMBugzillaTest/issues/29117)

Now, again, this entire blog post is _measuring after the thing's already been cut._
My talking about it now is not useful to the LLVM project in any way, as far as I know.
But since I spent probably 48 hours of my life on this, and I went and
published my entire deploy plan at [github.com/Quuxplusone/BugzillaToGitHub](https://github.com/Quuxplusone/BugzillaToGithub)
and all, I figured I might as well blog about it too.

If you, dear reader, are ever in a position to migrate a directory's worth of data
from Platform A to Platform B, please remember the Export–Transform–Load pattern,
"measure twice cut once," and any other lessons you can glean from this post.

My scripts might also be interesting if you ever need to scrape data from Bugzilla,
or from GitHub, or use the GitHub API; or if you ever need to translate plain-text
comments into Markdown for some reason.
