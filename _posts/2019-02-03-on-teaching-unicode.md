---
layout: post
title: "On teaching Unicode in C++"
date: 2019-02-03 00:02:00 +0000
tags:
  rant
  typography
---

[I made the following three posts](https://groups.google.com/a/isocpp.org/forum/#!topic/sg20/-SZWOS1Odjc)
to the SG20 ("Teaching C++") mailing list in January 2019. I'm copying them here
so I can link people to them more easily.

My rant started in response to a comment by Chris Di Bella:

> My understanding of UTF-8 is still quite basic; however, if I understand correctly,
> there are code points, code units, abstract characters, coded characters, user-perceived
> characters, grapheme clusters, and glyphs. I consider this to be an almost overwhelming
> amount of information for myself, and do not want to burden beginners with this knowledge.

Chris, I think your understanding of UTF-8 should be improved! ;)
Almost all of the wacky terminology you listed (and more: collation, NFC and NFD,...)
is relevant to _Unicode_, but not to UTF-8 encoding proper. UTF-8 is just a way of mapping
_codepoints_ to sequences of _code units_, where "code unit" in this specific context is
a synonym for "8-bit byte."  That is, UTF-8 is a way of encoding strings of small integers
into strings of bytes.  And the beauty of UTF-8 (as opposed to some more baroque ways
humanity has tried to encode integers into bytes) is that UTF-8 works essentially
invisibly to the modern programmer!

Here's how you create an ASCII string, and do ASCII string searching and
ASCII string comparison in C++:

    std::string haystack = "hello world";
    std::string needle = "ell";
    assert(strstr(haystack.data(), needle.data()) != nullptr);
    assert(haystack > needle);  // (because ASCII 0x65 comes before ASCII 0x68)

And here's how you create a UTF-8 string, and do UTF-8 string searching and
UTF-8 string comparison in C++:

    std::string haystack = "héllö wôrld";
    std::string needle = "éll";
    assert(strstr(haystack.data(), needle.data()) != nullptr);
    assert(haystack < needle);  // (because U+00E9 LATIN SMALL LETTER E WITH ACUTE comes after U+0068 SMALL LETTER H)

UTF-8 is designed specifically to have all the "self-synchronizing" properties
that you need in order to do string searching and comparison.  UTF-8 strings
sort-with-`strcmp` in the same order that they would sort if they were tediously
converted to UTF-32 first. UTF-8 strings never contain `'\0'`
(or indeed any ASCII character) unless it represents a real `'\0'`
(or indeed the corresponding real ASCII character).

If you want to get into "user interface"-type questions, like whether maybe `"héllö wôrld"`
should be considered in some sense "equal to" `"hello world"`,
well, that's a whole kettle of Unicode fish, and I don't think students should
get into that at all — except in a course dedicated to Unicode algorithms.

However, it is a 100% intentional and lovely thing that students can come in on the first day
[and write](https://wandbox.org/permlink/zD0v1SaQKz6lLecC)

    #include <cstdio>
    int main() {
        puts("hello world! 😊");
    }

and it will compile and Just Work.  (Well, modulo font support in their terminal app.
And assuming that your classroom computers use UTF-8 throughout; but this is more
likely to be true than untrue these days.)

I don't think it makes any sense to tell students about `u8"foo"` and `u16"foo"` and so on,
since even the Committee hasn't figured out what we want the story to look like in that area yet.
Certainly students should not ever be told about `L"foo"`; we _know_ that was a mistake.

----

Yehezkel Bernat wrote:

> I believe you have missed my point of "get me the first char of the string". Any suggestion?

I suggest that "get me the first char of the string" has a really obvious answer (`s[0]`)
that works for all ASCII strings; and for general Unicode, the question doesn't make enough
sense to bother with. What's the first char of "ﷻ"? What's the first char of "ﬃ"? What's
the first char of "😊"? What's the first char of "🇺🇿"?  What's the first char of "👨🏿‍🌾" (which
displays as two glyphs in my Safari, by the way)?

> Or maybe you want to suggest how to reverse a (UTF-8) string?

Same deal. Even for plain old ASCII, I don't understand the motivation for reversing the
bytes of a string. `"Hello world"` makes sense; `"dlrow olleH"` looks like Klingon or something.
If you want to reverse the elements of an array, then use an array of ints or something;
don't try to motivate reversal using text.

> This is before we go into things like  
> U+0065 LATIN SMALL LETTER E + U+0301 COMBINING ACUTE ACCENT  
> which looks the same as  
> U+00E9 LATIN SMALL LETTER E WITH ACUTE  
> But how your sort works now?

`strcmp`-on-UTF-8-text still sorts by codepoint; so a string with two codepoints "65, 301"
will sort lexicographically before a string with one codepoint "E9". Students won't be
surprised by this. If anything, they'll be surprised at how those two different strings
actually render almost identically on their screens!

C++ students in particular may be amused by U+037E GREEK QUESTION MARK, which looks like
this: ";" and renders almost identically to U+003B SEMICOLON ";". Looks the same, behaves
differently at the bits-and-bytes level. This general phenomenon gives rise to
[IDN homograph attacks](https://en.wikipedia.org/wiki/IDN_homograph_attack) on the Web.
In a course specifically on Unicode and text processing, you could then go off into the weeds
on whether "looks the same" should be the basis for "acts the same," and draw parallels
to what happened with "Han unification," and just generally spend a week on the intersection
of politics with Unicode.

But in a C++ programming course you don't have to do any of that!  If a student asks you to
"get the first char of a string" you explain that indexing works on char arrays (and `std::string`s)
just as it does on int arrays; and if they seriously challenge you to make your solution work
for "Unicode," you throw [https://apps.timwhitlock.info/unicode/inspect/hex/1F468/1F3FF/200D/1F33E](https://apps.timwhitlock.info/unicode/inspect/hex/1F468/1F3FF/200D/1F33E)
in their lap and say "Unicode is big and complicated and there's a different class on that
if you really want to go there."

> Or even the fact that I believe U+00E9 must come before U+0068 and I don't think your
> students will be happy if it isn't.

You want a _bigger_ number to sort in between two _smaller_ numbers, based on the way the glyph _looks?_
Again, Unicode is big and complicated and there's a different class on that if you
really want to go there. ;)

----

Bjarne Stroustrup wrote:

> What kind of environment setup do I need to get such characters into my code, to read them
> from files, and to write them out again?
>
> Traditionally, I have allowed my students a free choice of systems, so this is not a rhetorical question.

I think computer science/engineering students (especially students who take a course offering
a "free choice of systems" and deliberately pick Windows) should be familiar with the idea
that when a computer system doesn't work, the problem might be at any one of a number of levels.
Maybe your C++ code is fine, but you're seeing weird output because the terminal app has the
wrong encoding settings for display; or because it has the right encoding but the glyph
you're trying to print is unsupported by your selected font; or (on a non-POSIX system)
because the shell is interfering somehow; or because the text is going over a network
and some box in the middle is misconfigured.

My point is, you have phrased the student's question correctly. "What kind of environment
setup do I need...?"  A reasonable answer would be, "Sorry, I personally don't know much
about Windows environment setup. If your neighbor can't help you, and StackOverflow can't
help you, then maybe look over your neighbor's shoulder for this one. Don't worry, we won't
be printing a lot of smiley faces in this course, anyway."

(I admit this answer would be far less reasonable in a class being taught to non-English-speakers.
However, those students would _likely already know_ how to configure their systems for correct
display of non-English text.)

My brief excursion to StackOverflow shows that yeah, Windows is kind of a mess in the Unicode
department, but you can make it kinda work sometimes:  
[https://superuser.com/questions/239810/setting-utf8-as-default-character-encoding-in-windows-7](https://superuser.com/questions/239810/setting-utf8-as-default-character-encoding-in-windows-7)  
[https://www.curlybrace.com/words/2014/10/03/windows-console-and-doublemulti-byte-character-set/](https://www.curlybrace.com/words/2014/10/03/windows-console-and-doublemulti-byte-character-set/)

This has been a bit of a digression. I think my original point was supposed to be,
students in a C++ course don't need to be taught anything about "multi-byte character sets"
or "encodings" or anything, because UTF-8 Just Works. (Except if you use a misconfigured system.
But this is a course on C++, not system configuration.)  And anyway, we're not going to be
printing a lot of non-ASCII text in this course, so if your system configuration
can't handle "é" or "😊", hey, don't worry about it.
