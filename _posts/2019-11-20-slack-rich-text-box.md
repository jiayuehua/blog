---
layout: post
title: "Slack's new WYSIWYG input box is really terrible"
date: 2019-11-20 00:01:00 +0000
tags:
  rant
  slack
  web
---

Slack has just recently rolled out a "WYSIWYG text input" widget to its Web browser interface.
(Apparently, the phased rollout started at the beginning of November 2019, but it's just now starting to
hit the workspaces that I participate in.) The user experience of using this new input method is
really, really, really bad.

First of all, there is no way to go back to plain old Markdown input.
(See @SlackHQ's responses in [this massive Twitter thread](https://twitter.com/SlackHQ/status/1191761054252097541).)
If you prefer the old interface... well, screw you, says Slack.

It wouldn't be a problem if the WYSIWYG interface supported "editing" in the way that Slack users
are used to. But right now a whole lot of stuff is broken — not just "I typed some slightly wrong
sequence of characters and now the text looks messed up," but "I cannot figure out how to recover
the original formatting without deleting my entire message and starting over."

For example: In Markdown, if I have typed

    when you do `foo()` it foos the bar.

it will display, unsurprisingly, as "when you do `foo()` it foos the bar." However, in the new WYSIWYG editor,
it displays as

    when you do `foo() it foos the bar.`

That is, closing backticks are not respected! If you want the proper display, you must hit right-arrow
after the closing backtick (but before the space). That's quite a gymnastic for someone with decades
of muscle memory.

Now suppose you've gotten it displaying right, and now you realize (before hitting Enter) that you really
wanted it to say `bar.foo()` instead of `foo()`.
In the old Markdown interface, I can just left-arrow until the cursor is located immediately before the `f` in `foo`,
and add the new characters `bar.` In the WYSIWYG interface, if you follow that same sequence of steps,
_even though the cursor is clearly displayed inside the code span_ when it's located immediately before the `f`,
what you'll see after typing `bar.` is this:

    when you do bar.`foo()` it foos the bar

I think the only way to insert text at the beginning of a code span in the WYSIWYG editor is
to highlight the first character of the span and type over it (thus cloning all its formatting
onto the new text you're typing).

I wish Slack would provide a way to disable the WYSIWYG rich-text-input box. I don't think it's
useful, and it's extremely annoying to have to keep backspacing to fix mistakes. I'm already starting
to reduce the amount of formatting I use on Slack (e.g., typing "when you do bar.foo() it foos the bar"
without any code highlighting) just so that I can maintain typing speed. But I really don't want to have
to do that! I just want to be able to type Markdown at speed and have it render the way I've grown used to.

If you know someone who works at Slack, please feel free to send them a link to this post!
